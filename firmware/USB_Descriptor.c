#include "USB_Registers.h"
#include "USB0_ISR.h"
#include "USB_Descriptor.h"

//-----------------------------------------------------------------------------
// Descriptor Declarations
//-----------------------------------------------------------------------------

const device_descriptor code DEVICEDESC = {
    18,                                 // bLength
    0x01,                               // bDescriptorType
    0x1001,                             // bcdUSB
    0x00,                               // bDeviceClass
    0x00,                               // bDeviceSubClass
    0x00,                               // bDeviceProtocol
    EP0_PACKET_SIZE,                    // bMaxPacketSize0
    0x5104,                             // idVendor
    0x12e0,                             // idProduct
    0x0000,                             // bcdDevice
    0x01,                               // iManufacturer
    0x02,                               // iProduct
    0x00,                               // iSerialNumber
    0x01                                // bNumConfigurations
}; //end of DEVICEDESC

// From "USB Device Class Definition for Human Interface Devices (HID)".
// Section 7.1:
// "When a Get_Descriptor(Configuration) request is issued,
// it returns the Configuration descriptor, all Interface descriptors,
// all Endpoint descriptors, and the HID descriptor for each interface."
const hid_configuration_descriptor code HIDCONFIGDESC = {

{ // configuration_descriptor hid_configuration_descriptor
    0x09,                               // Length
    0x02,                               // Type
    0x2200,                             // Totallength (= 9+9+9+7)
    0x01,                               // NumInterfaces
    0x01,                               // bConfigurationValue
    0x00,                               // iConfiguration
    0x80,                               // bmAttributes
    0x20                                // MaxPower (in 2mA units)
},

{ // interface_descriptor hid_interface_descriptor
    0x09,                               // bLength
    0x04,                               // bDescriptorType
    0x00,                               // bInterfaceNumber
    0x00,                               // bAlternateSetting
    0x01,                               // bNumEndpoints
    0x03,                               // bInterfaceClass (3 = HID)
    0x01,                               // bInterfaceSubClass
    0x00,                               // bInterfaceProcotol (Keyboard)
    0x00                                // iInterface
},

{ // class_descriptor hid_descriptor
	0x09,	                            // bLength
	0x21,	                            // bDescriptorType
	0x0101,	                            // bcdHID
	0x00,	                            // bCountryCode
	0x01,	                            // bNumDescriptors
	0x22,                               // bDescriptorType
	HID_REPORT_DESCRIPTOR_SIZE_LE       // wItemLength (tot. len. of report descriptor)
},

// IN endpoint (mandatory for HID)
{ // endpoint_descriptor hid_endpoint_in_descriptor
    0x07,                               // bLength
    0x05,                               // bDescriptorType
    0x81,                               // bEndpointAddress
    0x03,                               // bmAttributes
    EP1_PACKET_SIZE_LE,                 // MaxPacketSize (LITTLE ENDIAN)
    10                                  // bInterval
},

// OUT endpoint (optional for HID)
{ // endpoint_descriptor hid_endpoint_out_descriptor
    0x07,                               // bLength
    0x05,                               // bDescriptorType
    0x01,                               // bEndpointAddress
    0x03,                               // bmAttributes
    EP2_PACKET_SIZE_LE,                 // MaxPacketSize (LITTLE ENDIAN)
    10                                  // bInterval
}
};

const hid_report_descriptor code HIDREPORTDESC = {
    // Keyboard Section
    0x05, 0x01,           // USAGE_PAGE (Generic Desktop)
	0x09, 0x06,           // USAGE (Keyboard)
	0xA1, 0x01,           // COLLECTION (Application)
	//0x85, REPID_KB,       // REPORT_ID
	0x75, 0x01,           //   REPORT_SIZE (1)
	0x95, 0x08,           //   REPORT_COUNT (8)
	0x05, 0x07,           //   USAGE_PAGE (Keyboard)(Key Codes)
	0x19, 0xE0,           //   USAGE_MINIMUM (Keyboard LeftControl)(224)
	0x29, 0xE7,           //   USAGE_MAXIMUM (Keyboard Right GUI)(231)
	0x15, 0x00,           //   LOGICAL_MINIMUM (0)
	0x25, 0x01,           //   LOGICAL_MAXIMUM (1)
	0x81, 0x02,           //   INPUT (Data,Var,Abs) ; Modifier byte

	0x95, 0x01,           //   REPORT_COUNT (1)
	0x75, 0x08,           //   REPORT_SIZE (8)
	0x15, 0x00,           //   LOGICAL_MINIMUM (0)
	0x25, 0xE7,           //   LOGICAL_MAXIMUM (231)
	0x05, 0x07,           //   USAGE_PAGE (Keyboard)(Key Codes)
	0x19, 0x00,           //   USAGE_MINIMUM (Reserved (no event indicated))(0)
	0x29, 0xE7,           //   USAGE_MAXIMUM (Right Super)(231)
	0x81, 0x00,           //   INPUT (Data,Ary,Abs)
	0xC0,                 // END_COLLECTION
};

#define STR0LEN 4

code const unsigned char String0Desc [STR0LEN] = {
   STR0LEN, 0x03, 0x09, 0x04
}; //end of String0Desc

#define STR1LEN sizeof ("Chromium") * 2

code const unsigned char String1Desc [STR1LEN] = {
   STR1LEN, 0x03,
   'C', 0,
   'h', 0,
   'r', 0,
   'o', 0,
   'm', 0,
   'i', 0,
   'u', 0,
   'm', 0,
}; //end of String1Desc

#define STR2LEN sizeof ("Rubber Ducky") * 2

code const unsigned char String2Desc [STR2LEN] = {
   STR2LEN, 0x03,
   'R', 0,
   'u', 0,
   'b', 0,
   'b', 0,
   'e', 0,
   'r', 0,
   ' ', 0,
   'D', 0,
   'u', 0,
   'c', 0,
   'k', 0,
   'y', 0,
}; //end of String2Desc

unsigned char* const STRINGDESCTABLE [] = {
   String0Desc,
   String1Desc,
   String2Desc
};
