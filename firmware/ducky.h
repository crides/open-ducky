#ifndef  _DUCKY_H_
#define  _DUCKY_H_

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

//-----------------------------------------------------------------------------
// 16-bit SFR Definitions for 'F32x
//-----------------------------------------------------------------------------

sfr16 TMR2RL   = 0xCA;                 // Timer2 reload value
sfr16 TMR2     = 0xCC;                 // Timer2 counter

unsigned char IN_PACKET[];

sbit LED = P1^7;

void System_Init();
void USB0_Init();

#endif  /* _DUCKY_H_ */
