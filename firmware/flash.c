#include "c8051f320.h"
#include "flash.h"

//-----------------------------------------------------------------------------
// Static Global Variables
//-----------------------------------------------------------------------------

static unsigned char FlashKey[2];

//-----------------------------------------------------------------------------
// Global Functions
//-----------------------------------------------------------------------------

//-----------------------------------------------------------------------------
// SetFlashKey
//-----------------------------------------------------------------------------
//
// Return Value : None
// Parameters   :
//                1) unsigned char key[2] : Sets the flash key code used to write/erase
//                               flash
//
// Sets the flash key code for flash writes/erases. The MCU will be more
// resistant to flash corruption if the key codes are not stored on chip.
// For example, the flash key can be set from a USB packet prior to programming
// and then cleared once finished to ensure that the flash key is not available.
//
// Enable flash writes:  key[2] = { 0xA5, 0xF1 }
// Disable flash writes: key[2] = { 0x00, 0x00 }
//
//-----------------------------------------------------------------------------
void SetFlashKey(unsigned char k1, unsigned char k2) {
   FlashKey[0] = k1;
   FlashKey[1] = k2;
}

//-----------------------------------------------------------------------------
// EraseFlashPage
//-----------------------------------------------------------------------------
//
// Return Value : None
// Parameters   :
//                1) unsigned int pageAddress : the address of the flash page to erase
//
// Erases the specified flash page
//
//-----------------------------------------------------------------------------
void EraseFlashPage(unsigned int pageAddress) {
   unsigned char xdata * pAddr = (unsigned char xdata *) pageAddress;

   // Disable interrupts
   EA = 0;

   // Write flash key codes
   FLKEY = FlashKey[0];
   FLKEY = FlashKey[1];

   // Enable program erase
   PSCTL = 0x03;

   // Erase page by writing to a byte within the page
   *pAddr = 0x00;

   // Disable program erase
   PSCTL = 0x00;

   // Restore interrupts
   EA = 1;
}

//-----------------------------------------------------------------------------
// WriteFlashPage
//-----------------------------------------------------------------------------
//
// Return Value : None
// Parameters   :
//                1) unsigned int address : the address of the flash page to write
//                2) unsigned char buffer[] : a buffer to write to the flash page starting
//                                 at the specified page address
//                3) unsigned int size : the number of bytes in the buffer to write
//
// Write the specified number of bytes in the buffer to the specified address.
//
//-----------------------------------------------------------------------------
void WriteFlashPage(unsigned int address, unsigned char* buffer, unsigned int size) {
   unsigned char xdata * pAddr = (unsigned char xdata *) address;
   unsigned int i;

   // Disable interrupts
   EA = 0;

   // Enable program writes
   PSCTL = 0x01;

   for (i = 0; i < size; i++) {
      // Write flash key codes
      FLKEY = FlashKey[0];
      FLKEY = FlashKey[1];

      // Write a single byte to the page
      pAddr[i] = buffer[i];
   }

   // Disable program writes
   PSCTL = 0x00;

   // Restore interrupts
   EA = 1;
}
