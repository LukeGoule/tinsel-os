#include <input.h>
#include <stdio.h>
#include <memory.h>

/*
TODO: Implement some way of processing capslock input.
*/

#define SCAN_LEFT_ARR 	0x4B
#define SCAN_RIGHT_ARR 	0x4D
#define SCAN_UP_ARR 	0x48
#define SCAN_DOWN_ARR 	0x50

#define SHIFT_DOWN 		0x2A
#define SHIFT_UP   		0xAA

// English-UK Keyboard Layout
unsigned char* Scancodes_EN_UK[0xFF] = {
	"<ESC>",
	"`","1","2","3","4","5","6","7","8","9","0","-","=","\b",
	"\t","q","w","e","r","t","y","u","i","o","p","[","]","\n","<LCTRL>",
	"a","s","d","f","g","h","j","k","l",";","\'","`","<LSHIFT>",
	"\\","z","x","c","v","b","n","m",",",".","/","<RSHIFT>",
	"*","<LALT>"," ","<CAPSL>","","","","","","","","","","","<NUML>"
};

// English-UK Keyboard Layout
unsigned char* Scancodes_EN_UK_SHIFT[0xFF] = {
	"<ESC>",
	"¬","!","\"","£","$","%","^","&","*","(",")","_","+","\b",
	"\t","Q","W","E","R","T","Y","U","I","O","P","{","}","\n","<LCTRL>",
	"A","S","D","F","G","H","J","K","L",":","@","~","<LSHIFT>",
	"|","Z","X","C","V","B","N","M","<",">","?","<RSHIFT>",
	"*","<LALT>"," ","<CAPSL>","","","","","","","","","","","<NUML>"
};

bool kbShiftDown = false;

char* scan_2_char(uint8_t c) {
    switch (c){
	case SHIFT_DOWN:
		kbShiftDown = true;
		return "";
	case SHIFT_UP:
		kbShiftDown = false;
		return "";
	default:
		if (!(c & (1 << 7))) {
			char* scan = Scancodes_EN_UK[c];

			if (kbShiftDown)
				scan = Scancodes_EN_UK_SHIFT[c];

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
				return scan;
			}
		} else {
			return "";
		}
	}
}
