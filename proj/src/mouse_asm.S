/*void mouse_asm_ih();*/
/*using _C_LABEL() from <machine/asm.h>*/
#include  <machine/asm.h>

#include "8042.h"

.intel_syntax

.global _C_LABEL(mouseByte)
.global _C_LABEL(mouse_asm_ih)
.data

.text
_C_LABEL(mouse_asm_ih):
          xor eax, eax //eax will contain the Status in the first part
          
          in al, KBC_CMD_REG //The status only has a byte
          
          test al, OBF //stat & OBF 
          jz Error

          test al, (PAR_ERR | TO_ERR)) //Comparing the status with the two most significant bits that shows if it is an error
          jnz Error //If there is no error, the mouse byte is valid
          

          in al, OUT_BUF //input: al has now a byte from the mouse packet
          mov byte ptr _C_LABEL(mouseByte), al //Move to the global variable code the value in al
          jmp Fim

Error:          
          mov byte ptr _C_LABEL(mouseByte), 0 //If there is an error, the value in code will be 0
      
Fim:      
          ret
.end
