/*void kbd_asm_ih();*/
/*using _C_LABEL() from <machine/asm.h>*/
#include  <machine/asm.h>

#include "8042.h"

.intel_syntax

.global _C_LABEL(code)
.global _C_LABEL(kbd_asm_ih)
.data

.text
_C_LABEL(kbd_asm_ih):
          xor eax, eax //eax will contain the Status in the first part
          in al, OUT_BUF //The status only has a byte
          cmp al, (PAR_ERR | TO_ERR)) //Comparing the status with the two most significant bits that shows if it is an error
          jz noError //If there is no error, the scancode is valid
          cmp al, AUX
          jnz noError
          mov byte ptr _C_LABEL(code), 0 //If there is an error, the value in code will be 0

noError:  xor eax, eax //Variable that will contain a byte from the scancode
          in al, OUT_BUF //input: al has now a byte from the scancode
          mov byte ptr _C_LABEL(code), al //Move to the global variable code the value in al
Fim:      ret
.end
