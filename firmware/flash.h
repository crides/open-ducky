#ifndef _F3XX_FLASH_H_
#define _F3XX_FLASH_H_

//-----------------------------------------------------------------------------
// Function Prototypes
//-----------------------------------------------------------------------------

void SetFlashKey(unsigned char k1, unsigned char k2);
void EraseFlashPage(unsigned int pageAddress);
void WriteFlashPage(unsigned int address, unsigned char* buffer, unsigned int size);
void ReadFlashPage(unsigned int address, unsigned char* buffer, unsigned int size);

#endif // _F3XX_FLASH_H_
