#pragma once

enum state_t { //Enumeration that represents all the states of the state machine
  INIT,        //Initial state
  LLINE,       //Left line of the inverted V
  VERTEX,      //Vertex of the inverted V
  RLINE,       //Right line of the inverted V
  COMP         //Final state
};

int mouse_subscribe_int(uint8_t *bit_no); //Function that subscribes mouse interrupts

int mouse_unsubscribe_int(); //Function that unsubscribes mouse interrupts

uint8_t readMouseByte(); //Function that reads a byte from the mouse

void parsePacket(struct packet *pp, uint8_t bytes[]); //Function that parses the mouse packet

int issueMouseCommand(uint8_t cmd); //Function that sends a command to mouse

int mouseIrqSet(bool enable); //Function to enable and disable irq line

int writeDafaultCmdByte(); //Function to write the deafult command byte

void mouseDetectEvent(struct packet *pp, enum state_t state); //Function that detects an event provide from the mouse

extern uint8_t mouseByte; //Extern variable that contains the byte from the mouse

extern struct mouse_ev m; //Struct that contains an event from the mouse and delta x and y
