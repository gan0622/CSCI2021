.text
.global  set_temp_from_ports
        
## ENTRY POINT FOR REQUIRED FUNCTION
set_temp_from_ports:
        ## assembly instructions here

        ## a useful technique for this problem
        movw  THERMO_SENSOR_PORT(%rip), %dx   # copy global var to reg dx  (16-bit word)
        movb  THERMO_STATUS_PORT(%rip), %cl   # copy global var to reg cl  (8-bit byte)
        

        cmpw $64000, %dx        #check corner case
        ja .Error

        movw %dx, %r8w         #tem = THERMO_SENSOR_PORT
        shrw $6, %r8w           #short tem = THERMO_SENSOR_PORT / 64
        movw %dx, %r9w         #temp 
        andw $63, %r9w          #edx % 64
        cmpw $32,%r9w           #see if greater than 32 or not
        jl   .Second_Part
        incw %r8w

.Second_Part:
        testb $1 , %cl        #check if THERMO_STATUS_PORT is 1
        jz    .Celsius
        addw   $-500, %r8w
        imulw  $9, %r8w
        movw   %r8w, %ax      #move to ax and div ax
        cwtl                  # sign extend ax to long word
        cltq                  # sign extend eax to quad word
        cqto                  # sign extend ax to dx
        movw  $5, %cx         # assign value to cx  
        idivw %cx             
        movw  %ax , %r8w
        addw  $320, %r8w       
        movw  %r8w, (%rdi)    #change tenths into degrees
        movb  $1, 2(%rdi)     # set is_fahrenheit 1
        movl  $0, %eax
        ret
.Celsius:
        addw $-500, %r8w      
        movw %r8w, (%rdi)     #move without converation
        movb $0,  2(%rdi)     #is_fahrenheit 0  
        movl $0,  %eax          
        ret
.Error:
        movl    $1, %eax
        ret

### Data area associated with the next function
// .data

my_array:
        .int 0b1111110              #num 1-9
        .int 0b0001100
        .int 0b0110111
        .int 0b0011111
        .int 0b1001101
        .int 0b1011011    
        .int 0b1111011
        .int 0b0001110
        .int 0b1111111
        .int 0b1011111
        .int 0b0000000              #blank
        .int 0b0000001              #negative
      
// ### Change back to defining functions/execurable instructions
.text
.global  set_display_from_temp

// // ## ENTRY POINT FOR REQUIRED FUNCTION
set_display_from_temp:  
//         ## assembly instructions here

// 	## two useful techniques for this problem
//         // movl    my_int(%rip),%eax    # load my_int into register eax
        leaq    my_array(%rip),%r8   # load pointer to beginning of my_array into edx
        movl    %edi, %r9d
        sarq    $16,  %r9              
        movw    %di, %ax                #move di to si
        movl    $0, %r12d               #init start value
        cmpb    $1, %r9b                # if temp.is_fahrenheit != 1 && temp.is_fahrenheit != 0 
        je      .is_Fahrenheit 
        cmpb    $0, %r9b               
        je      .is_Celsius
        jmp     .error

.is_Celsius:
        cmpw    $500, %ax               #check corner case
        jg      .error                  
        cmpw    $-500, %ax              
        jl      .error
        jmp     .Mask

.is_Fahrenheit:
        cmpw    $1220, %ax              #check corner case
        jg      .error
        cmpw    $-580, %ax
        jl      .error
        jmp     .Mask

.Mask:
        cmpw  $0,      %ax              #check tenths_degrees
        jge   .temp_hundreds            
        movl  $11,     %r10d             
        orl   (%r8, %r10, 4),  %r12d     
        sall  $7,      %r12d
        imulw $-1,     %ax

.temp_hundreds:
        movw %ax, %r11w                 #check hundred degrees
        movl $1000, %ecx                #prepare for division 
        cwtl
        cltq
        cqto                  
        idivl %ecx              
        cmpl  $0, %eax
        je    .temp_tens

        movl  %eax, %r10d               #move the value to r10
        orl   (%r8, %r10, 4),  %r12d    #get idex from array
        sall  $7,      %r12d            #shift 
        movw  %dx, %ax


.temp_tens:
        movw %dx, %ax                 #check temp_tens
        movl $100, %ecx                 #prepare for division
        cwtl
        cltq
        cqto                             
        idivl %ecx          
        cmpl  $0,   %eax
        je    .temp_ones
        movl  %eax,    %r10d            #move the value to r10
        orl   (%r8, %r10, 4),  %r12d    #get idex from array
        sall  $7,      %r12d            #shift

.temp_ones:
        movl %edx, %eax                 #remainder transfers back to eax
        movl $10, %ecx
        cwtl
        cltq
        cqto                 
        idivl %ecx
        movl  %eax,    %r10d           
        orl   (%r8, %r10, 4), %r12d      
        sall  $7,      %r12d

.temp_tenths:
        movl  %edx, %eax                #remainder transfers back to eax
        movl  %eax,    %r10d          
        orl   (%r8, %r10, 4),  %r12d

.is_fahrenheit:
        cmpb $1,  %r9b                  #check if is_fahrenheit
        jne  .is_celsius                
        movl $1,  %ecx
        sall $29, %ecx
        orl  %ecx, %r12d
        jmp  .Done
.is_celsius:
        movl $1,  %ecx
        sall $28, %ecx
        orl  %ecx, %r12d
        jmp  .Done
.Done:
        movl %r12d, (%rsi)              #update display
        movl $0, %eax
        ret
.error:
        movl $1, %eax
        ret

.text
.global thermo_update
        
// // // ## ENTRY POINT FOR REQUIRED FUNCTION
thermo_update:
// 	## assembly instructions here
        pushq $0                              # push any 64-bit register onto the stack
        movq %rsp,  %rdi 
        call set_temp_from_ports              # stack aligned, call function
        cmpl $1, %eax                         
        je  .error1
        movq (%rsp), %rdi     
        movq $0, (%rsp)                      
        movq %rsp, %rsi                      
        call  set_display_from_temp           # stack aligned, call function
        cmpl $1, %eax      
        je  .error1
        popq %rdx                              #restore the stack
        movl %edx, THERMO_DISPLAY_PORT(%rip)   
        movl $0, %eax                          #return 0 after success
        ret
.error1:
        movl $1, %eax
        popq %rdx 
        ret
