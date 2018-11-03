#pragma once

int mouse_subscribe_int(uint8_t *bit_no); //Function that subscribes mouse interrupts

int mouse_unsubscribe_int(); //Function that unsubscribes mouse interrupts

int enableDataReport(); //Function to enable stream mode

int disableDataReport(); //Function to disable mouse data reporting

int setRemoteMode();

int issueReadData();

int setStreamMode();

uint8_t readMouseByte();

extern uint8_t mouseByte;
