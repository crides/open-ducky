#include "c8051f340.h"
#include "USB0_ISR.h"
#include "ducky.h"
#include "keycodes.h"
#include "USB_Registers.h"
#include "report_handler.h"

enum {
    STR = 0x00,
    RELEASE = 0x10,

    DELAY = 0x80,
    MOUSE = 0x90,
    TYPE = 0xA0,
    PRESS = 0xB0,
    FREQ = 0xC0,
};

#define K(n) KEY_##n
#define S(k) (k | 0x80)

unsigned char IN_PACKET[2];
code unsigned char INSTS[] = {
    DELAY, 0x03,
    STR, S(K(1)), S(K(2)), K(3), K(4), 0,
    DELAY, 20,
    TYPE | MOD_CTRL, K(U),
};
unsigned int INST_LEN = sizeof(INSTS);

void Sysclk_Init ();
void Port_Init ();
void USB0_Init ();
void Timer_Init ();
void ADC0_Init ();
void Delay ();

void main() {
   System_Init ();

   while (1);
}

/*
 * The state machine for parsing the instructions
 *
 * Parsing rules:
 *
 * Instruction type 1: length of 1, M1 = 0
 *   STRING
 *     Head = 0000
 *   RELEASE
 *     Head = 0001
 *
 * Instruction type 2: length of more than 1, M1 = 1, need look forward when
 *     operating in a buffer
 *   DELAY (cycles)
 *     Head = 1000
 *   MOUSE (x, y) (or dx, dy)
 *     Head = 1001
 *   TYPE (key)
 *     Head = 1010
 *   PRESS (mod, key)
 *     Head = 1011
 *   FREQ (freq)
 *     Head = 1100
 * 
 * Note: 
 * Bit fields names:
 * M1 M2 M3 M4 L1 L2 L3 L4
 */
bit RUNNING = 1;                        // Whether the state machine is running
bit LAST_KEY = 0;                       // Last action was a key, need to reset key buffer
bit INST_STR = 0;                       // Writing a string
unsigned int INST_PTR = 0;              // Pointer for the instructions
unsigned int INST_DELAY = 0;

// Convenient definitions
#define SEND() return 1
#define IGNORE() return 0
#define NEXT(inc, ret) INST_PTR += inc; return ret

bit IN_Report() {
    char cur;
    LED = ~RUNNING;
    if (RUNNING) {
        if (LAST_KEY) {
            press_key(0, 0);
            LAST_KEY = 0;
            SEND();
        }

        if (INST_DELAY) {
            INST_DELAY --;
            IGNORE();
        }

        if (INST_PTR >= INST_LEN) {     // No more instructions
            RUNNING = 0;
            IGNORE();
        }

        cur = INSTS[INST_PTR];
        if (INST_STR) {
            if (cur) {
                // Since all printable keys on keyboard are less than 0x7F, we compress the
                // shift into it when compiling
                press_key((cur & 0x80) ? MOD_SHIFT : 0, cur & 0x7F);
                LAST_KEY = 1;
                NEXT(1, 1);
            }
            INST_STR = 0;               // STRING cmd ends
            NEXT(1, 0);
        }

        switch (cur & 0xF0) {
            case STR:                   // STRING cmd
                INST_STR = 1;
                NEXT(1, 0);
            case RELEASE:               // RELEASE cmd
                press_key(0, 0);
                NEXT(1, 1);

            case DELAY:                 // DELAY cmd
                INST_DELAY = INSTS[INST_PTR + 1] - 1;   // This already used up a cycle
                NEXT(2, 0);
            //case MOUSE:                  // MOUSE cmd
                //mouse_move();
                //break;
            case TYPE:                  // TYPE cmd (with mod)
                press_key(cur & 0x0F, INSTS[INST_PTR + 1]);
                LAST_KEY = 1;
                NEXT(2, 1);
            case PRESS:                 // PRESS cmd
                press_key(cur & 0x0F, INSTS[INST_PTR + 1]);
                NEXT(2, 1);
            case FREQ:
                TMR2RL = (INSTS[INST_PTR + 1] > 15)
                    ? -(SYSCLK/12/INSTS[INST_PTR + 1])
                    : -(SYSCLK/12/20);
                NEXT(2, 0);

            default:
                RUNNING = 0;
                IGNORE();
        }
    }
    return 0;
}

bit OUT_Report(unsigned char *d) {
	return 0;
}

void Timer2_ISR () interrupt 5 {
    SendPacket();
    TF2H = 0;                           // Clear Timer2 interrupt flag
}

void System_Init () {
   PCA0MD &= ~0x40;                    // Disable Watchdog timer
   Sysclk_Init ();                     // initialize system clock
   Port_Init ();	                     // configure cross bar
   Timer_Init ();                      // configure timer
   USB0_Init();
   EA = 1;
}

//-----------------------------------------------------------------------------
// USB0_Init
//
// - Initialize USB0
// - Enable USB0 interrupts
// - Enable USB0 transceiver
// - Enable USB0 with suspend detection
//
// ----------------------------------------------------------------------------

void USB0_Init () {

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

}

//-----------------------------------------------------------------------------
// Sysclk_Init
//
// Initialize system clock to maximum frequency.
//
// ----------------------------------------------------------------------------
void Sysclk_Init () {
   OSCICN |= 0x03;                     // Configure internal oscillator for
                                       // its maximum frequency and enable
                                       // missing clock detector, SYSCLK = 12MHz

   CLKMUL  = 0x00;                     // Select internal oscillator as
                                       // input to clock multiplier

   CLKMUL |= 0x80;                     // Enable clock multiplier
   Delay();                            // Delay for clock multiplier to begin
   CLKMUL |= 0xC0;                     // Initialize the clock multiplier
   Delay();                            // Delay for clock multiplier to begin

   while(!(CLKMUL & 0x20));            // Wait for multiplier to lock
   CLKSEL  = SYS_INT_OSC;              // Select system clock
   CLKSEL |= USB_4X_CLOCK;             // Select USB clock
}

void Port_Init() {
    P0SKIP    = 0xFF;
    P1SKIP    = 0xFF;
    P2SKIP    = 0xFF;
    P3SKIP    = 0x11;
    P3MDIN    = 0xF7;
    XBR0      = 0x04;
    XBR1      = 0x40;
}

//-----------------------------------------------------------------------------
// Timer_Init
//-----------------------------------------------------------------------------
//
// Return Value - None
// Parameters - None
//
// - Timer 2 reload, used to check if switch pressed on overflow and
// used for ADC continuous conversion
//
// ----------------------------------------------------------------------------
void Timer_Init () {
    TMR2CN  = 0x00;                     // Stop Timer2; Clear TF2; Timer2 on SYSCLK / 12

    CKCON  &= ~0xF0;                    // Timer2 clocked based on T2XCLK;
    TMR2RL = -(SYSCLK/12/20);           // Timer3 configured to overflow after
    TMR2 = TMR2RL;                      // ~25ms (for SMBus low timeout detect):
                                        // 1/.025 = 40

    ET2     = 1;                        // Enable Timer2 interrupts
    TR2     = 1;                        // Start Timer2
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
   for (x = 0; x < 500; x)
      x++;
}
