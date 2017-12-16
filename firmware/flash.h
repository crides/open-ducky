//-----------------------------------------------------------------------------
// F3xx_Flash.h
//-----------------------------------------------------------------------------
// Copyright 2012 Silicon Laboratories, Inc.
// http://www.silabs.com
//
// Program Description:
//
// Flash utilities for writing/erasing flash pages
//
//
// How To Test:    See Readme.txt
//
//
// Target:         C8051F320/1,
//                 C8051F326/7,
//                 C8051F34x,
//                 C8051F38x
//
// Tool chain:     Keil C51 7.50 / Keil EVAL C51
//                 Silicon Laboratories IDE version 2.6
// Command Line:   See Readme.txt
// Project Name:   USB0_Bulk
//
// Release 2.0
//    -Rewrite (CM)
//    -02 NOV 2012
//

#ifndef _F3XX_FLASH_H_
#define _F3XX_FLASH_H_

//-----------------------------------------------------------------------------
// Function Prototypes
//-----------------------------------------------------------------------------

void SetFlashKey(unsigned char key1, unsigned key2);
void EraseFlashPage(unsigned int pageAddress);
void WriteFlashPage(unsigned int address, unsigned char* buffer, unsigned int size);
void ReadFlashPage(unsigned int address, unsigned char* buffer, unsigned int size);

#endif // _F3XX_FLASH_H_
