/*void timer_asm_ih();*/
/*using _C_LABEL() from <machine/asm.h>*/
#include  <machine/asm.h>

.intel_syntax

.global _C_LABEL(counter)
.global _C_LABEL(timer_asm_ih)
.data

.text
_C_LABEL(timer_asm_ih):
          mov eax, _C_LABEL(counter) //Move to eax the counter value 
          inc eax //Increment counter value in eax
          mov _C_LABEL(counter), eax //Move value of eax to counter eax
          ret
.end
