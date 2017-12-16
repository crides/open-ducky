#include "c8051f340.h"
#include "report_handler.h"
#include "USB0_ISR.h"
#include "ducky.h"
#include "USB_Descriptor.h"
#include "keycodes.h"

BufferStructure IN_BUFFER, OUT_BUFFER;

// ****************************************************************************
// For Input Reports:
// Point IN_BUFFER.Ptr to the buffer containing the report
// Set IN_BUFFER.Length to the number of bytes that will be
// transmitted.
//
// REMINDER:
// Systems using more than one report must define Report IDs
// for each report.  These Report IDs must be included as the first
// byte of an IN report.
// Systems with Report IDs should set the FIRST byte of their buffer
// to the value for the Report ID
// AND
// must transmit Report Size + 1 to include the full report PLUS
// the Report ID.
//
// ****************************************************************************

// ****************************************************************************
// Configure Setup_OUT_BUFFER
//
// Reminder:
// This function should set OUT_BUFFER.Ptr so that it
// points to an array in data space big enough to store
// any output report.
// It should also set OUT_BUFFER.Length to the size of
// this buffer.
//
// ****************************************************************************

void Setup_OUT_BUFFER(void)
{
}

// ----------------------------------------------------------------------------
// ReportHandler_IN...
// ----------------------------------------------------------------------------
//
// Return Value - None
// Parameters - Report ID
//
// These functions match the Report ID passed as a parameter
// to an Input Report Handler.
// the ...FG function is called in the SendPacket foreground routine,
// while the ...ISR function is called inside the USB ISR.  A lock
// is set whenever one function is called to prevent a call from the
// other from disrupting the routine.
// However, this should never occur, as interrupts are disabled by SendPacket
// before USB operation begins.
// ----------------------------------------------------------------------------
void ReportHandler_IN(unsigned char R_ID) {
    if (R_ID == 0) {
        IN_Report();
    }
}

void press_key(unsigned char modifiers, unsigned char keycode) {
    IN_PACKET[0] = modifiers;
    IN_PACKET[1] = keycode;

    IN_BUFFER.Ptr = IN_PACKET;
    IN_BUFFER.Length = 2;
}
