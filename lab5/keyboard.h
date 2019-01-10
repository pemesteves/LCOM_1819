#pragma once

#ifdef LAB3

int (sys_inb_cnt)(port_t port, uint32_t *byte);

#else

#define sys_inb_cnt(p, q) sys_inb(p, q)

#endif

int kbd_subscribe_int(uint8_t *bit_no); //Function that subscribes keyboard interrupts

int kbd_unsubscribe_int(); //Function that unsubscribes keyboard interrupts

uint8_t readScanCode(); //Function to read scancodes

void enableInterrupts(); //Function to enable interrupts

extern uint32_t counterSysIn; //Global variable to count number of sys_inb calls
extern uint8_t code; //Global variable to save scancodes
