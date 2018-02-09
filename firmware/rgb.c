#include "rgb.h"
#include "ducky.h"

void rgb_init() {
    PCA0CN    = 0x40;
    PCA0CPM0  = 0x42;
    PCA0CPM1  = 0x42;
    PCA0CPM2  = 0x42;
}

void rgb_set(unsigned char r, unsigned char g, unsigned char b) {
	PCA0CPH2 = r;
	PCA0CPH1 = g;
	PCA0CPH0 = b;
}
