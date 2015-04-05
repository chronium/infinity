/* Copyright (C) 2015 - GruntTheDivine (Sloan Crandell)
 * 
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License
 * as published by the Free Software Foundation; either version 2
 * of the License, or (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA  02110-1301, USA.
 */

/*
 * elf32.c
 * Provides functions for loading and relocating ELF32 executables
 */


#include <stdint.h>
#include <stddef.h>
#include <infinity/heap.h>
#include <infinity/common.h>
#include <infinity/kernel.h>
#include <infinity/fcntl.h>
#include <infinity/virtfs.h>
#include <infinity/elf32.h>
#include <infinity/paging.h>
#include <infinity/memmanager.h>

static void elf_load_phdrs(void *elf);
static int elf_get_symval(struct elf32_ehdr *hdr, int table, int idx);
static int elf_alloc_sections(struct elf32_ehdr *hdr);
static int elf_move_symbols(struct elf32_ehdr *hdr);
static int elf_relocate(struct elf32_ehdr *hdr);
static int elf_do_reloc(struct elf32_ehdr *hdr, struct elf32_rel *rel, struct elf32_shdr *reltab);
static int elf_get_strindex(struct elf32_ehdr *hdr, const char *sym);

/*
 * Opens up an ELF32 executable and relocates it,
 * returning a pointer the ELF image
 * @param path      The path containing the ELF file
 */
void *elf_open(const char *path)
{
    struct file *elf = fopen(path, O_RDWR);
    void *exe = kalloc(elf->f_len);
    
    virtfs_read(elf, exe, 0, elf->f_len);
    elf_alloc_sections(exe);
    elf_relocate(exe);

    fclose(elf);
    return exe;
}

/*
 * Opens up an ELF32 executable, loading it 
 * into virtual memory
 */
void *elf_open_v(const char *path)
{
    struct file *elf = fopen(path, O_RDWR);
    void *exe = kalloc(elf->f_len);

    virtfs_read(elf, exe, 0, elf->f_len);
    
    elf_load_phdrs(exe);

    fclose(elf);

    return exe;
}

/*
 * Load the data defined in the ELF's program headers 
 * and map to virtual memory
 */
static void elf_load_phdrs(void *elf)
{
    struct elf32_ehdr *hdr = (struct elf32_ehdr *)elf;
    struct elf32_phdr *phdr = elf_pheader(hdr);
    
    for(int i = 0; i < hdr->e_phnum; i++) {
        struct elf32_phdr *p = &phdr[i];
        
        if(p->p_type == PT_LOAD) {
            for(int i = 0; i < p->p_memsz; i += 0x1000) {
                frame_alloc(p->p_vaddr + i, PAGE_RW | PAGE_USER);
            }
            memcpy((void*)p->p_vaddr, (void*)(elf + p->p_offset), p->p_filesz);
        }
    }
}

/*
 * Resolves a symbol contained inside a loaded ELF file
 * @param elf       The executable containing the symbol
 * @param sym       The symbol to resolve
 */
void *elf_sym(const void *elf, const char *sym)
{
    struct elf32_ehdr *hdr = (struct elf32_ehdr *)elf;
    struct elf32_shdr *shdr = elf_sheader(hdr);

    for (int i = 0; i < hdr->e_shnum; i++) {
        struct elf32_shdr *section = &shdr[i];

        if (section->sh_type == SHT_SYMTAB) {
            for (int idx = 0; idx < section->sh_size / section->sh_entsize; idx++) {
                struct elf32_sym *symbol = &((struct elf32_sym *)((int)hdr + section->sh_offset))[idx];

                if (strcmp(elf_lookup_string_strtab(hdr, symbol->st_name), sym) == 0) {
                    struct elf32_shdr *target = elf_section(hdr, symbol->st_shndx);
                    return elf_get_symval(hdr, i, idx);
                }
            }
        }
    }
    printk(KERN_ERR "ERROR: Could not resolve symbol '%s'\n", sym);
    return -1;
}

/*
 * Frees any resources involved in reading an ELF file
 */
int elf_close(void *elf)
{
}


static int elf_get_symval(struct elf32_ehdr *hdr, int table, int idx)
{
    if (table == SHN_UNDEF || idx == SHN_UNDEF)
        return 0;

    struct elf32_shdr *symtab = elf_section(hdr, table);

    if (idx >= symtab->sh_size)
        return -1;

    int symaddr = (int)hdr + symtab->sh_offset;
    struct elf32_sym *symbol = &((struct elf32_sym *)symaddr)[idx];
    if (symbol->st_shndx == SHN_ABS) {
        return symbol->st_value;
    } else if (symbol->st_shndx == SHN_UNDEF) {
        struct elf32_shdr *strtab = elf_section(hdr, symtab->sh_link);
        const char *name = (const char *)hdr + strtab->sh_offset + symbol->st_name;

        void *target = resolve_ksym(name);

        if (target == NULL) {
            printk(KERN_ERR "ERROR: Could not resolve kernel symbol %s", name);
            if (ELF32_ST_BIND(symbol->st_info) & STB_WEAK)
                return 0;
            else
                return 0;
        } else {
            return (int)target;
        }
    } else {
        struct elf32_shdr *target = elf_section(hdr, symbol->st_shndx);
        return (int)hdr + target->sh_offset + symbol->st_value;
    }
}

/*
 * Kernel modules will not use PIC or the GOT so it is important
 * that we update all symbol values to their appropriate location
 */
static int elf_move_symbols(struct elf32_ehdr *hdr)
{
    struct elf32_shdr *shdr = elf_sheader(hdr);

    for (int i = 0; i < hdr->e_shnum; i++) {
        struct elf32_shdr *section = &shdr[i];

        if (section->sh_type == SHT_SYMTAB) {
            for (int idx = 0; idx < section->sh_size / section->sh_entsize; idx++) {
                struct elf32_sym *symbol = &((struct elf32_sym *)((int)hdr + section->sh_offset))[idx];
                struct elf32_shdr *target = elf_section(hdr, symbol->st_shndx);
                symbol->st_value = hdr + target->sh_offset + symbol->st_value;
            }
        }
    }
}

/*
 * Allocate sections that have not been already been allocated
 */
static int elf_alloc_sections(struct elf32_ehdr *hdr)
{
    struct elf32_shdr *shdr = elf_sheader(hdr);

    for (int i = 0; i < hdr->e_shnum; i++) {
        struct elf32_shdr *section = &shdr[i];

        if (section->sh_type == SHT_NOBITS) {
            if (!section->sh_size)
                continue;
            if (section->sh_flags & SHF_ALLOC) {
                void *mem = kalloc(section->sh_size);
                memset(mem, 0, section->sh_size);
                section->sh_offset = (int)mem - (int)hdr;
            }
        }
    }
    return 0;
}


/*
 * Begins relocating an ELF file
 */
static int elf_relocate(struct elf32_ehdr *hdr)
{
    struct elf32_shdr *shdr = elf_sheader(hdr);

    for (int i = 0; i < hdr->e_shnum; i++) {
        struct elf32_shdr *section = &shdr[i];

        if (section->sh_type == SHT_REL) {
            for (int j = 0; j < section->sh_size / section->sh_entsize; j++) {
                struct elf32_rel *reltab = &((struct elf32_rel *)((int)hdr + section->sh_offset))[j];
                int result = elf_do_reloc(hdr, reltab, section);

                if (result == -1)
                    printk(KERN_WARN "Could not relocate\n");
            }
        }
    }
    return 0;
}

/*
 * Perform a relocation
 */
static int elf_do_reloc(struct elf32_ehdr *hdr, struct elf32_rel *rel, struct elf32_shdr *reltab)
{
    struct elf32_shdr *target = elf_section(hdr, reltab->sh_info);

    int addr = (int)hdr + target->sh_offset;
    int *ref = (int *)(addr + rel->r_offset);
    int symval = 0;

    if (ELF32_R_SYM(rel->r_info) != SHN_UNDEF) {
        symval = elf_get_symval(hdr, reltab->sh_link, ELF32_R_SYM(rel->r_info));
        struct elf32_sym *s = ELF32_R_SYM(rel->r_info);
        if (symval == 0) {
            return -1;
        }
    }

    switch (ELF32_R_TYPE(rel->r_info)) {
    case R_386_NONE:
        break;
    case R_386_32:
        *ref = DO_386_32(symval, *ref);
        break;
    case R_386_PC32:
        *ref = DO_386_PC32(symval, *ref, (int)ref);
        break;
    default:
        printk(KERN_WARN "WARN: Unsupported relocation type (%d)\n", ELF32_R_TYPE(rel->r_info));
        return -1;
    }
    return symval;
}
