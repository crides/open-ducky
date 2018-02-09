#include "c8051f320.h"
#include "USB0_ISR.h"
#include "ducky.h"
#include "USB_Registers.h"
#include "flash.h"
#include "keycodes.h"
#include "rgb.h"

void Delay ();

void main() {
   sys_init ();

   while (1);
}

//void T2_ISR () interrupt INTERRUPT_TIMER2 {
//    SendPacket();
//    TF2H = 0;                           // Clear Timer2 interrupt flag
//}

code unsigned char INITED _at_ BASE - 1;
void sys_init () {
	unsigned char i = 0;
	unsigned char d[2];

    PCA0MD &= ~0x40;                     // Disable Watchdog timer

    // --- sysclk_init ---
    OSCICN |= 0x03;                     // Configure internal oscillator for
                                        // its maximum frequency and enable
                                        // missing clock detector, SYSCLK = 12MHz

    CLKMUL = 0x80;                      // Select internal oscillator as
                                        // input to clock multiplier
                                        // Enable clock multiplier
    Delay();                            // Delay for clock multiplier to begin
    CLKMUL |= 0xC0;                     // Initialize the clock multiplier
    Delay();                            // Delay for clock multiplier to begin

    while(!(CLKMUL & 0x20));            // Wait for multiplier to lock
    CLKSEL = SYS_INT_OSC | USB_4X_CLOCK;// Select system clock, SYSCLK = 12 MHz
                                        // Select USB clock

    // --- Port_init ---
    P0SKIP    = 0x6F;
    P1SKIP    = 0xFF;
    P2SKIP    = 0x07;
    XBR1      = 0x43;

//    SW_1 = 1;
//    SW_2 = 1;
//    SW_3 = 1;
//    SW_4 = 1;

    // --- T2_init ---
//    TMR2CN  = 0x00;                     // Stop Timer2; Clear TF2; Timer2 on SYSCLK / 12

//    CKCON  &= ~0xF0;                    // Timer2 clocked based on T2XCLK;
    TMR2RL = -(SYSCLK/12/20);//TREL_20HZ;                 // Timer2 configured to overflow after
    TMR2 = TMR2RL;                      // ~25ms (for SMBus low timeout detect):
                                        // 1/.025 = 40

    ET2 = 1;                        // Enable Timer2 interrupts
    TR2 = 1;                        // Start Timer2

    // --- USB0_init ---
    POLL_WRITE_BYTE (POWER, 0x08);      // Force Asynchronous USB Reset
    POLL_WRITE_BYTE (IN1IE, 0x07);      // Enable Endpoint 0-1 in interrupts
    POLL_WRITE_BYTE (OUT1IE,0x07);      // Enable Endpoint 0-1 out interrupts
    POLL_WRITE_BYTE (CMIE, 0x07);       // Enable Reset, Resume, and Suspend
                                        // interrupts
    USB0XCN = 0xE0;                     // Enable transceiver; select full speed
    POLL_WRITE_BYTE (CLKREC,0x80);      // Enable clock recovery, single-step
                                        // mode disabled

    EIE1 |= 0x02;                       // Enable USB0 Interrupts

                                        // Enable USB0 by clearing the USB
    POLL_WRITE_BYTE (POWER, 0x01);      // Inhibit Bit and enable suspend
                                        // detection

    rgb_init();

    SetFlashKey(0xA5, 0xF1);

    // --- ducky_init ---
	if (INITED == 0xFF) {				// Erased value is 0xFF
		rgb_set(0, 100, 100);
//        WriteFlashPage(BASE - 1, d, 9);
		WriteFlashPage(BASE - 1, &i, 1);
		d[0] = DATA_LOC >> 8;
		d[1] = DATA_LOC;

		for ( ; i < 4; i ++) {
			WriteFlashPage(PTR_LOC(i), d, 2);
		}
		rgb_set(0, 0, 0);
	}

    EA = 1;
}
//-----------------------------------------------------------------------------
// Delay
//-----------------------------------------------------------------------------
//
// Return Value - None
// Parameters - None
//
// Used for a small pause, approximately 80 us in Full Speed,
// and 1 ms when clock is configured for Low Speed
//
// ----------------------------------------------------------------------------
void Delay () {
   int x;
   for (x = 0; x < 500; x++);
}
