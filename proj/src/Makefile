PROG=proj

SRCS = proj.c videoCard.c timer.c keyboard.c mouse.c sprite.c animSprite.c menu.c rtc.c game.c high_scores.c keyboard_asm.S mouse_asm.S timer_asm.S

CPPFLAGS += -pedantic #-D __LCOM_OPTIMIZED__

DPADD += ${LIBLCF}
LDADD += -llcf 

.include <minix.lcom.mk>
