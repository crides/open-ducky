#ifndef  _DUCKY_H_
#define  _DUCKY_H_

#include <c8051f320.h>

#define SYSCLK             12000000    // SYSCLK frequency in Hz

// USB clock selections (SFR CLKSEL)
#define USB_4X_CLOCK       0x00        // Select 4x clock multiplier, for USB
#define USB_INT_OSC_DIV_2  0x10        // Full Speed
#define USB_EXT_OSC        0x20
#define USB_EXT_OSC_DIV_2  0x30
#define USB_EXT_OSC_DIV_3  0x40
#define USB_EXT_OSC_DIV_4  0x50

// System clock selections (SFR CLKSEL)
#define SYS_INT_OSC        0x00        // Select to use internal oscillator
#define SYS_EXT_OSC        0x01        // Select to use an external oscillator
#define SYS_4X_DIV_2       0x02

sfr16 TMR2RL   = 0xCA;                 // Timer2 reload value
sfr16 TMR2     = 0xCC;                 // Timer2 counter

xdata unsigned char IN_PACKET[];
xdata unsigned char OUT_PACKET[];

sbit SW_1 = P0^2;
sbit SW_2 = P0^3;
sbit SW_3 = P2^7;
sbit SW_4 = P2^1;

void sys_init();
void USB0_init();

#define BASE 0x1800
#define _PTRS (BASE)
#define PTR_LOC(n) (_PTRS + 2 * (n))
//#define _COLORS (BASE + 8)
//#define COLOR_LOC(n) (_COLORS + 3 * (n))
#define DATA_LOC (BASE + 8)
#define PAGE_SIZE 0x200				// 512 byte pages

code unsigned int PTRS[];
code unsigned char COLORS[];

enum {
    STR = 0x00,
    RELEASE = 0x10,

    DELAY = 0x80,
    MOUSE = 0x90,
    TYPE = 0xA0,
    PRESS = 0xB0,
    FREQ = 0xC0,
	LED = 0xD0,
	CLICK = 0xE0,
	STOP = 0xFF,						// Also erased spaces
};

#endif  /* _DUCKY_H_ */
