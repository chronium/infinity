

global pit_irq_handler
extern perform_context_switch

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
    
    mov eax, esp 
    push eax       

    call perform_context_switch      
    mov esp, eax   
    
    mov al, 0x20    
    out 0x20, al    
    
    pop gs      
    pop fs        
    pop es       
    pop ds          
    popa           

    iret

