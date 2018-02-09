#ifndef  _UUSB_REPORTHANDLER_H_
#define  _USB_REPORTHANDLER_H_

typedef struct{
   unsigned char Length;
   unsigned char* Ptr;
} BufferStructure;

bit send_inst();
void press_key(unsigned char modifiers, unsigned char keycode);
void mouse_move_to(unsigned char x, unsigned char y);
void click(unsigned char btn);
void process_IN(unsigned char d);
void process_OUT(unsigned char *d);
void Setup_OUT_BUFFER();

extern BufferStructure IN_BUFFER, OUT_BUFFER;

#endif
