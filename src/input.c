#include <input.h>
#include <stdio.h>

#define SCAN_LEFT_ARR 0x4B
#define SCAN_RIGHT_ARR 0x4D
#define SCAN_UP_ARR 0x48
#define SCAN_DOWN_ARR 0x50

// Poll the keyboard for input.
uint8_t input_getscancode() {
	uint8_t flag = inportb(0x64);
	while (!(flag & 1)) {
		flag = inportb(0x64);
	}
	return inportb(0x60);
}

// English-UK Keyboard Layout
unsigned char* Scancodes_EN_UK[0xFF] = {
	"<ESC>",
	"`","1","2","3","4","5","6","7","8","9","0","-","=","\b",
	"\t","q","w","e","r","t","y","u","i","o","p","[","]","\n","<LCTRL>",
	"a","s","d","f","g","h","j","k","l",";","\'","`","<LSHIFT>",
	"\\","z","x","c","v","b","n","m",",",".","/","<RSHIFT>",
	"*","<LALT>"," ","<CAPSL>","","","","","","","","","","","<NUML>"
};

char* input_getchar() {
	uint8_t c = input_getscancode();

	if (!(c & (1 << 7))) {
		char* scan = Scancodes_EN_UK[c];

        switch (c) {
		case SCAN_LEFT_ARR:
			return "<LARR>";
		case SCAN_RIGHT_ARR:
			return "<RARR>";
		case SCAN_UP_ARR:
			return "<UARR>";
		case SCAN_DOWN_ARR:
			return "<DARR>";
		default:
			return Scancodes_EN_UK[c];
		}
	} else {
		return "";
	}
}
