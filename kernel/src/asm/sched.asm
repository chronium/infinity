; Copyright (C) 2015 - GruntTheDivine (Sloan Crandell)
;
; This program is free software; you can redistribute it and/or
; modify it under the terms of the GNU General Public License
; as published by the Free Software Foundation; either version 2
; of the License, or (at your option) any later version.
;
; This program is distributed in the hope that it will be useful,
; but WITHOUT ANY WARRANTY; without even the implied warranty of
; MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
; GNU General Public License for more details.
;
; You should have received a copy of the GNU General Public License
; along with this program; if not, write to the Free Software
; Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA  02110-1301, USA.
;


global pit_irq_handler
extern perform_context_switch
extern next_page_directory

current_pdir resw 1

pit_irq_handler:
    pusha      
    push ds        
    push es        
    push fs         
    push gs       
    mov eax, 0x10   ;
    mov ds, eax             
    mov es, eax
    mov fs, eax
    mov gs, eax             
    
    
    mov ebx, cr3
    mov eax, esp
    push eax   

    call perform_context_switch
        
    mov esp, eax   
    
    mov eax, [next_page_directory]
    cmp eax, 0
    jz .nopdir
    mov cr3, eax
    mov eax, cr3
    mov cr3, eax
.nopdir:
    mov al, 0x20    
    out 0x20, al    
    
    pop gs      
    pop fs        
    pop es       
    pop ds          
    popa           

    iret

