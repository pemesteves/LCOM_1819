#ifndef _LCOM_8042_H_
#define _LCOM_8042_H_

/*
  Constants for programming the 8042 Keyboard Controller.
 */

#define BIT(n) (0x01 << (n))

#define KEYBOARD_IRQ 1 //Keyboard IRQ line
#define MOUSE_IRQ 12   //Mouse IRQ line

#define ESC 0x81 //ESC key: used to stop the cicle of pollings

#define FIRST_B_SCAN 0xE0 //Usual value of the first byte of two byte scancodes

#define KBC_CMD_REG 0x64 //Address of the status register variable and register to write kbc commands

#define OUT_BUF 0x60 //Output Buffer

#define PAR_ERR BIT(7) //Parity error -> bit 7 of the status
#define TO_ERR BIT(6)  //Timeout error -> bit 6 of the status
#define OBF BIT(0)     //Output buffer full - data available for reading -> bit 0 of the status
#define IBF BIT(1)     //Input buffer full - don’t write commands or arguments -> bit 1 of the status
#define AUX BIT(5)

#define READ_COM_B 0x20       //Read Command Byte
#define WRITE_COM_B 0x60      //Write Command Byte
#define CHECK_KBC 0xAA        //Check KBC (Self-test)
#define CHECK_KBD_INT 0xAB    //Check Keyboard Interface
#define DIS_KBD_INT 0xAD      //Disable KBD Interface
#define EN_KBD_INT 0xAE       //Enable KBD Interface
#define WRITE_BYTE_MOUSE 0xD4 //Write byte to mouse

#define DELAY_US 20000 //Argument of tickdelay function

#define LARGEST_NUMBER 0xFF //FF is the largest binary number

#define BREAK_CODE BIT(7) //If the bit 7 of the scancode is set to 1 it means that is a makecode

#define ENABLE_INT_KBD BIT(0)        //Value to use in the command byte to enable keyboard interrupts
#define ENABLE_INT_MOUSE BIT(1)      //Value to use in the command byte to enable mouse interrupts
#define DISABLE_MOUSE BIT(5)         //Value to use in the command byte to disable mouse
#define DISABLE_KBD_INTERFACE BIT(4) //Value to use in the command byte to disable keyboard interface

#define LB BIT(0)     //Left button bit
#define RB BIT(1)     //Right button bit
#define MB BIT(2)     //Middle button bit
#define X_OV BIT(6)   //x overflow bit
#define Y_OV BIT(7)   //y overflow bit
#define X_SIGN BIT(4) //x sign bit
#define Y_SIGN BIT(5) //y sign bit

//PS/2 Mouse Commands
#define READ_DATA 0xEB           //Send data packet request
#define STREAM_MODE 0xEA         //Sed data on events
#define ENABLE_DATA_REPORT 0xF4  //In stream mode only
#define DISABLE_DATA_REPORT 0xF5 //Disable (Data Reporting)
#define REMOTE_MODE 0xF0         //Set remote mode - Send data on request only

//Acknowledgement byte values
#define ACK 0xFA
#define NACK 0xFE
#define ERROR 0xFC

//Values passed to function mouseIrqSet to disable and enable irq line
#define DISABLE_IRQ_LINE 0 
#define ENABLE_IRQ_LINE 1

#endif /* _LCOM_8042_H */
