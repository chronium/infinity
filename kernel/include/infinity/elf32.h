/* Copyright (C) 2014 - GruntTheDivine (Sloan Crandell)
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
 
#ifndef INFINITY_ELF32_H
#define INFINITY_ELF32_H

#include <stddef.h>
#include <stdint.h>
#include <infinity/paging.h>

#define ELF_NIDENT                      16
#define ELFMAG0                         0x7F
#define ELFMAG1                         'E'
#define ELFMAG2                         'L'
#define ELFMAG3                         'F'
#define ELFDATA2LSB                     (1)
#define ELFCLASS32                      (1)
#define EM_386                          (3)
#define EV_CURRENT                      (1)
#define ELF32_ST_BIND(INFO)     ((INFO) >> 4)
#define ELF32_ST_TYPE(INFO)     ((INFO) & 0x0F)
#define ELF32_R_SYM(INFO)       ((INFO) >> 8)
#define ELF32_R_TYPE(INFO)      ((uint8_t)(INFO))
#define SHN_UNDEF                       (0x00)
#define DO_386_32(S, A) ((S) + (A))
#define DO_386_PC32(S, A, P)    ((S) + (A)-(P))
#define SHN_ABS                 0xfff1

typedef uint16_t elf32_half_t;
typedef uint32_t elf32_off_t;
typedef uint32_t elf32_addr_t;
typedef uint32_t elf32_word_t;
typedef int32_t elf32_sword_t;

struct elf32_ehdr {
    uint8_t         e_ident[ELF_NIDENT];
    elf32_half_t    e_type;
    elf32_half_t    e_machine;
    elf32_word_t    e_version;
    elf32_addr_t    e_entry;
    elf32_off_t     e_phoff;
    elf32_off_t     e_shoff;
    elf32_word_t    e_flags;
    elf32_half_t    e_ehsize;
    elf32_half_t    e_phentsize;
    elf32_half_t    e_phnum;
    elf32_half_t    e_shentsize;
    elf32_half_t    e_shnum;
    elf32_half_t    e_shstrndx;
};

struct elf32_sym {
    elf32_word_t    st_name;
    elf32_addr_t    st_value;
    elf32_word_t    st_size;
    uint8_t         st_info;
    uint8_t         st_other;
    elf32_half_t    st_shndx;
};

struct elf32_shdr {
    elf32_word_t    sh_name;
    elf32_word_t    sh_type;
    elf32_word_t    sh_flags;
    elf32_addr_t    sh_addr;
    elf32_off_t     sh_offset;
    elf32_word_t    sh_size;
    elf32_word_t    sh_link;
    elf32_word_t    sh_info;
    elf32_word_t    sh_addralign;
    elf32_word_t    sh_entsize;
};

struct elf32_phdr {
	elf32_word_t	p_type;
	elf32_off_t 	p_offset;
	elf32_addr_t	p_vaddr;
	elf32_addr_t	p_paddr;
	elf32_word_t	p_filesz;
	elf32_word_t	p_memsz;
	elf32_word_t	p_flags;
	elf32_word_t	p_align;
};

struct elf32_rel {
    elf32_addr_t    r_offset;
    elf32_word_t    r_info;
};

struct elf32_rela {
    elf32_addr_t    r_offset;
    elf32_word_t    r_info;
    elf32_sword_t   r_addend;
};

enum stt_bindings {
    STB_LOCAL               = 0,    // Local scope
    STB_GLOBAL              = 1,    // Global scope
    STB_WEAK                = 2     // Weak, (ie. __attribute__((weak)))
};

enum pt_types {
    PT_NULL         = 0,
    PT_LOAD         = 1,
    PT_DYNAMIC      = 2,
    PT_INTERP       = 3,
    PT_NOTE         = 4,
    PT_SHLIB        = 5,
};

enum sht_types {
    SHT_NULL        = 0,    // Null section
    SHT_PROGBITS    = 1,    // Program information
    SHT_SYMTAB      = 2,    // Symbol table
    SHT_STRTAB      = 3,    // String table
    SHT_RELA        = 4,    // Relocation (w/ addend)
    SHT_NOBITS      = 8,    // Not present in file
    SHT_REL         = 9,    // Relocation (no addend)
};

enum stt_types {
    STT_NOTYPE              = 0,    // No type
    STT_OBJECT              = 1,    // Variables, arrays, etc.
    STT_FUNC                = 2     // Methods or functions
};

enum ShT_Attributes {
    SHF_WRITE       = 0x01, // Writable section
    SHF_ALLOC       = 0x02  // Exists in memory
};

enum elf_ident {
    EI_MAG0         = 0,    // 0x7F
    EI_MAG1         = 1,    // 'E'
    EI_MAG2         = 2,    // 'L'
    EI_MAG3         = 3,    // 'F'
    EI_CLASS        = 4,    // Architecture (32/64)
    EI_DATA         = 5,    // Byte Order
    EI_VERSION      = 6,    // ELF Version
    EI_OSABI        = 7,    // OS Specific
    EI_ABIVERSION   = 8,    // OS Specific
    EI_PAD          = 9     // Padding
};

enum elf_type {
    ET_NONE         = 0,    // Unkown Type
    ET_REL          = 1,    // Relocatable File
    ET_EXEC         = 2     // Executable File
};

enum RtT_Types {
    R_386_NONE              = 0,
    R_386_32                = 1,
    R_386_PC32              = 2
};

static inline struct elf32_shdr *elf_sheader(struct elf32_ehdr *hdr)
{
    return (struct elf32_shdr *)((int)hdr + hdr->e_shoff);
}

static inline struct elf32_phdr *elf_pheader(struct elf32_ehdr *hdr)
{
    return (struct elf32_phdr *)((int)hdr + hdr->e_phoff);
}

static inline struct elf32_shdr *elf_section(struct elf32_ehdr *hdr, int idx)
{
    return &elf_sheader(hdr)[idx];
}

static inline char *elf_str_table(struct elf32_ehdr *hdr)
{
    if (hdr->e_shstrndx == SHN_UNDEF)
        return NULL;
    return (char *)hdr + elf_section(hdr, hdr->e_shstrndx)->sh_offset;
}

static inline char *elf_lookup_string(struct elf32_ehdr *hdr, int offset)
{
    char *strtab = elf_str_table(hdr);

    if (!strtab)
        return NULL;
    return strtab + offset;
}

static inline struct elf32_shdr *elf_section_s(struct elf32_ehdr *hdr, const char *str)
{
    for (int i = 0; i < hdr->e_shnum; i++) {
        struct elf32_shdr *sec = elf_section(hdr, i);
        if (!strcmp(elf_lookup_string(hdr, sec->sh_name), ".strtab"))
            return sec;

    }
    return NULL;
}

static inline char *elf_lookup_string_strtab(struct elf32_ehdr *hdr, int offset)
{
    char *strtab = (char *)hdr + elf_section_s(hdr, ".strtab")->sh_offset;

    if (!strtab)
        return NULL;
    return strtab + offset;
}
void *elf_open(const char *path);
void *elf_open_v(struct process *proc, const char *path);
void *elf_sym(const void *elf, const char *sym);
int elf_close(void *elf);

#endif
