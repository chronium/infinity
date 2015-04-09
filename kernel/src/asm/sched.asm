

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

