#ifndef  _UUSB_REPORTHANDLER_H_
#define  _USB_REPORTHANDLER_H_

typedef struct{
   unsigned char Length;
   unsigned char* Ptr;
} BufferStructure;

bit IN_Report();
bit OUT_Report(unsigned char *);
void press_key(unsigned char modifiers, unsigned char keycode);
extern void ReportHandler_IN(unsigned char);
extern void Setup_OUT_BUFFER(void);

extern BufferStructure IN_BUFFER, OUT_BUFFER;

#endif
