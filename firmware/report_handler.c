#include "report_handler.h"
#include "USB0_ISR.h"
#include "ducky.h"
#include "flash.h"
#include "USB_Descriptor.h"
#include "keycodes.h"
#include "rgb.h"

BufferStructure IN_BUFFER, OUT_BUFFER;

#define K(n) KEY_##n
#define S(k) (k | 0x80)

xdata unsigned char IN_PACKET[EP1_PACKET_SIZE];
xdata unsigned char OUT_PACKET[EP1_PACKET_SIZE];

code unsigned int PTRS[4] _at_ _PTRS;
//code unsigned char COLORS[16] _at_ _COLORS;

unsigned char RUNNING = 0;              // Whether the state machine is running, if running, in which section
bit LAST_KEY = 0;                       // Last action was a key, need to reset key buffer
bit LAST_CLICK = 0;                     // Last action was a mouse click, need to reset click
bit INST_STR = 0;                       // Writing a string
unsigned char code *INST_PTR = DATA_LOC;
unsigned int INST_DELAY = 0;
bit SW_1_old = 0, SW_2_old = 0, SW_3_old = 0, SW_4_old = 0;
extern bit LOCK;						// When locked, skip state machine

// Convenient definitions
#define SEND() return 1
#define IGNORE() return 0
#define NEXT(inc, ret) INST_PTR += inc; return ret

bit send_inst() {
    char cur;
    bit ptr_changed = 1;

    if (SW_1_old && !SW_1) {			// TODO FIX PHANTOM KEY PROBLEMS
		RUNNING = 1;
	} else if (SW_2_old && !SW_2) {
		RUNNING = 2;
	} else if (SW_3_old && !SW_3) {
		RUNNING = 3;
	} else if (SW_4_old && !SW_4) {
		RUNNING = 4;
	} else {
		ptr_changed = 0;
	}

    if (ptr_changed) {
    	INST_PTR = (unsigned char code *) PTRS[RUNNING - 1];
//    	cur = 3 * (RUNNING - 1);		// Not so good, but borrow it for a sec
//    	rgb_set(COLORS[cur], COLORS[cur + 1], COLORS[cur + 2]);
    }

	SW_1_old = SW_1;
	SW_2_old = SW_2;
	SW_3_old = SW_3;
	SW_4_old = SW_4;

    if (/*!LOCK && */RUNNING) {
        if (LAST_KEY) {
            press_key(0, 0);
            LAST_KEY = 0;
            SEND();
        }
        if (LAST_CLICK) {
        	click(0);
        	LAST_CLICK = 0;
        	SEND();
        }

        if (INST_DELAY) {
            INST_DELAY --;
            IGNORE();
        }

        cur = *INST_PTR;
        if (INST_STR) {
            INST_PTR ++;
            if (cur) {
                // Since all printable keys on keyboard are less than 0x7F, we compress the
                // shift into it when compiling
                press_key((cur & 0x80) ? MOD_SHIFT : 0, cur & 0x7F);
                LAST_KEY = 1;
                SEND();
            }
            INST_STR = 0;               // STRING cmd ends
            IGNORE();
        }

        switch (cur & 0xF0) {
            case STR:                   // STRING cmd
                INST_STR = 1;
                NEXT(1, 0);
            case RELEASE:               // RELEASE cmd
//            	click(0);				// Required when releasing mouse btns
//            	LAST_KEY = 1;
                press_key(0, 0);
                NEXT(1, 1);
            case CLICK:
            	click(cur);
            	LAST_CLICK = 1;
            	NEXT(1, 1);

            case DELAY:                 // DELAY cmd
                INST_DELAY = INST_PTR[1];// - 1;   // This already used up a cycle
                NEXT(2, 0);
            case MOUSE:                  // MOUSE cmd
                mouse_move_to(INST_PTR[1], INST_PTR[2]);
                NEXT(3, 1);
            case TYPE:                  // TYPE cmd (with mod)
                press_key(cur & 0x0F, INST_PTR[1]);
                LAST_KEY = 1;
                NEXT(2, 1);
            case PRESS:                 // PRESS cmd
                press_key(cur & 0x0F, INST_PTR[1]);
                NEXT(2, 1);
            case FREQ:
                TMR2RL = (INST_PTR[1] > 15)
                    ? -(SYSCLK/12/INST_PTR[1])
                    : -(SYSCLK/12/20);
                NEXT(2, 0);

            case LED:					// Set RGB LED color; now in 4 bytes
            	if (cur & 0x08) {		// Shortened?
            		rgb_set(cur & 0x04 ? 0xFF : 0,
            				cur & 0x02 ? 0xFF : 0,
            				cur & 0x01 ? 0xFF : 0);
            		NEXT(1, 1);
            	} else {
            		rgb_set(INST_PTR[1], INST_PTR[2], INST_PTR[3]);
            		NEXT(4, 1);
            	}

            default:					// STOP is actually taken care of
                RUNNING = 0;
                rgb_set(0, 0, 0);		// Indicate that it's over
                IGNORE();
        }
    }
    return 0;
}

#define PACKET_SIZE 16

void Setup_OUT_BUFFER() {
	OUT_BUFFER.Ptr = OUT_PACKET;
	OUT_BUFFER.Length = PACKET_SIZE;
}

bit LOCK = 0;							// Flag to indicate if IN or OUT operation is going on
bit READWRITE = 0;						// Read: 0, Write: 1
unsigned int DATA_LEFT = 0;
unsigned int CUR_ADDR = DATA_LOC;

//extern unsigned char RUNNING;

// TODO [ ] Change data packet addrs to match the 20 byte package
// 		[ ] Set up flags to enable write for a single direction
//		[ ] Change flash addrs to match the scheme
//		[ ] Set up routines to analyze headers
//		[ ] Change `process_IN' like `process_OUT' when IN report is added

//void process_IN(unsigned char d) {
//	if (d)
//		return;
//    if (LOCK) {
//        if (DATA_LEFT >= PACKET_SIZE) {
//        	IN_PACKET[0] = DATA_LEFT >> 8;
//        	IN_PACKET[1] = DATA_LEFT;
//
//        	IN_PACKET[2] = CUR_ADDR >> 8;
//        	IN_PACKET[3] = CUR_ADDR;
//
//        	IN_PACKET[7] = READWRITE;
//
//            ReadFlashPage(CUR_ADDR, IN_PACKET + PACKET_SIZE, PACKET_SIZE);
//            DATA_LEFT -= PACKET_SIZE;
//            CUR_ADDR += PACKET_SIZE;
//            IN_BUFFER.Length = EP1_PACKET_SIZE;
//        } else {
//        	IN_PACKET[0] = DATA_LEFT >> 8;
//        	IN_PACKET[1] = DATA_LEFT;
//
//        	IN_PACKET[2] = CUR_ADDR >> 8;
//        	IN_PACKET[3] = CUR_ADDR;
//
//        	IN_PACKET[7] = 0xFF;
//
//            ReadFlashPage(CUR_ADDR, IN_PACKET + PACKET_SIZE, DATA_LEFT);
//            IN_BUFFER.Length = DATA_LEFT + PACKET_SIZE;
//            DATA_LEFT = 0;
//            LOCK = 0;
//            CUR_ADDR = 0x2003;
//// 		    SEND_IN_PACKET = 0;
////            TR2 = 1;
//        }
//    } else {
////    	TR2 = 0;
//		LOCK = 1;
//		CUR_ADDR = 0x2003;
//		ReadFlashPage(0x2001, IN_PACKET, 2);
//		DATA_LEFT = (IN_PACKET[0] << 8) + IN_PACKET[1];
//		IN_BUFFER.Ptr = IN_PACKET;
//		IN_BUFFER.Length = 2;
//    }
//}

/*
 * Flash storage scheme: (for C8051F340)
 *   Items to store:
 *     0. Init flag (1 byte at BASE), if not inited then assign all four ptrs to DATA_LOC
 *     1. 4 ptrs to 4 sections (8 bytes at BASE + 1)
 *     2. 4 colors for each sections (12 bytes, for alignment at BASE + 9 + 4 * n)
 *     3. Intruction data (at BASE + 24, till the end at 0xF9FF
 */

void process_OUT(unsigned char *d) {
	unsigned char i = 0;				// Reserved for possible inner loop
	if (d[1] != 0xCA) {
		rgb_set(255, 0, 0);
		return;
	}
    if (LOCK) {
		rgb_set(100, 0, 100);
    	if (!READWRITE) {					// Reading
    		return;
    	}
        if (DATA_LEFT >= PACKET_SIZE) {
            WriteFlashPage(CUR_ADDR, d + 4, PACKET_SIZE);
            DATA_LEFT -= PACKET_SIZE;
            CUR_ADDR += PACKET_SIZE;
        } else {
            WriteFlashPage(CUR_ADDR, d + 4, DATA_LEFT);
            rgb_set(0, 0, 0);
            LOCK = 0;					// Unlock
        }
    } else {
		rgb_set(255, 255, 255);
		DATA_LEFT = (d[2] << 8) + d[3];
		LOCK = 1;					// Lock on
		READWRITE = 1;				// And set flag to write operation
		i = DATA_LEFT / PAGE_SIZE + 1;  // Pages to erase
		while (i--) {
			EraseFlashPage(_PTRS + i * PAGE_SIZE);	// TODO Not erasing
		}
		CUR_ADDR = _PTRS;
    }
}

void press_key(unsigned char modifiers, unsigned char keycode) {
    IN_PACKET[0] = REPID_KB;
    IN_PACKET[1] = modifiers;
    IN_PACKET[2] = keycode;

    IN_BUFFER.Ptr = IN_PACKET;
    IN_BUFFER.Length = 3;
}

void mouse_move_to(unsigned char x, unsigned char y) {
    IN_PACKET[0] = REPID_MOUSE_ABS;
    IN_PACKET[1] = x;
    IN_PACKET[2] = y;

    IN_BUFFER.Ptr = IN_PACKET;
    IN_BUFFER.Length = 3;
}

void click(unsigned char btn) {
	IN_PACKET[0] = REPID_MOUSE_BTN;
	IN_PACKET[1] = btn;

	IN_BUFFER.Ptr = IN_PACKET;
	IN_BUFFER.Length = 2;
}
