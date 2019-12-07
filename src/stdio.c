#include <stdio.h>
#include <vga.h>

cursor_t cur;

/*
Important:
    Run this function after every change to the cursor position for safety.
*/
void safe_cursor_pos() {
    vga_info_t* vga = vga_get_info();

    if (cur.x > vga->width) {
        cur.x = 0;
        cur.y += CHAR_HEIGHT + 1;
    }

    if (cur.y > vga->height) {
        cur.x = 0;
        cur.y = 0;

        vga_prims_box(0, 0, vga->width, vga->height, cur.bg_clr);
    }
}

void cursor_backspace() {
    cur.x -= (CHAR_WIDTH + 1);
    putc(' ');
    cur.x -= (CHAR_WIDTH + 1);
}

/*
Why there's a +1 on everything you may ask?
It's because I want to have this looking as clean as possible so I'm giving every character a space of 1 pixel so they don't conjoin and become weird.
*/
void putc(char c) {
    switch (c) {
    case '\0':
        return;
    case '\n':
        cur.y += CHAR_HEIGHT + 1;
        cur.x = 0;
        safe_cursor_pos();

        break;
    case '\t':
        cur.x += TAB_WIDTH + 1;
        safe_cursor_pos();

        break;
    case '\b':
        cur.x -= (CHAR_WIDTH + 1);
        safe_cursor_pos();

        vga_printc(' ', cur.x, cur.y, cur.bg_clr, cur.bg_clr); // is this cheating ?

        break;
    default:
        vga_printc(c, cur.x, cur.y, cur.fg_clr, cur.bg_clr);

        cur.x += CHAR_WIDTH + 1;
        safe_cursor_pos();
    }
}

void puts(char* str) {
    char c;
    while ((c = *str++) != '\0') {
        putc(c);
    }
}

/*
Highly modified implementation of the multiboot printf source from GNU.
Implemented custom colour changing stuff & basic float/double print.

Implementation of %<code>:
    %d => int
    %u => uint
    %x => hex
    %f => float
    %s => string

    %0 => reset colours
    %1 => inverted default
    %2 => red
    %3 => green
    %4 => blue
*/
void printf(const char* format, ...) {
    char **arg = (char**) &format;
	int c, a;
    double f;
	char buf[20];

	arg++;

	while ((c = *format++) != '\0') {
		char* p;

		if (c != '%') {
			putc(c);
		} else {
			c = *format++;
			switch (c) {
			case 'd':
			case 'u':
			case 'x':
				itoa(buf, c, *((int*) arg++));
				puts(buf);
				break;
			case 's':
				puts(*((int*) arg++));
				break;
			case 'f':
				f = *((double*)arg++);
				a = (int)f;

				printf("%d.", a);                   // Output the int & the dot

				if(f<0)                             //  If negative, remove the sign so we don't get like 1.-1000000
				   f = -f;
				f = (f - (int)f) * 1000000;         //  Get numbers after decimal point by x(10^8) because there's 8 digits in the bit after decimal

				printf("%d", (int)f);

				break;
            case '0': // default colour settings
                cur.bg_clr = 0x000000;
                cur.fg_clr = 0xFFFFFF;
                break;
            case '1': // inverted colours
                cur.bg_clr = 0xFFFFFF;
                cur.fg_clr = 0x000000;
                break;
            case '2': // red
                cur.fg_clr = 0xFF0000;
                cur.bg_clr = 0x000000;
                break;
            case '3': // green
                cur.fg_clr = 0x00FF00;
                cur.bg_clr = 0x000000;
                break;
            case '4': // blue
                cur.fg_clr = 0x0000FF;
                cur.bg_clr = 0x000000;
                break;
            case '5': // orange
                cur.fg_clr = 0xFF7F50;
                cur.bg_clr = 0x000000;
                break;
			default:
				putc(*((int*) arg++));
				break;
			}
		}
	}
}

/* Copied from multiboot GNU source code:
Convert the integer D to a string and save the string in BUF. If
BASE is equal to ’d’, interpret that D is decimal, and if BASE is
equal to ’x’, interpret that D is hexadecimal.
*/
void itoa(char *buf, int base, int d) {
	char *p = buf;
	char *p1, *p2;
	unsigned long ud = d;
	int divisor = 10;
	/* If %d is specified and D is minus, put ‘-’ in the head. */
	if (base == 'd' && d < 0)
	{
		*p++ = '-';
		buf++;
		ud = -d;
	}
	else if (base == 'x')
		divisor = 16;
	/* Divide UD by DIVISOR until UD == 0. */
	do
	{
		int remainder = ud % divisor;
		*p++ = (remainder < 10) ? remainder + '0' : remainder + 'a' - 10;
	}
	while (ud /= divisor);
	/* Terminate BUF. */
	*p = 0;
	/* Reverse BUF. */
	p1 = buf;
	p2 = p - 1;
	while (p1 < p2)
	{
		char tmp = *p1;
		*p1 = *p2;
		*p2 = tmp;
		p1++;
		p2--;
	}
}

size_t strlen(const char* str) {
	size_t len =0;
	while (str[len])
		len++;
	return len;
}

size_t strcmp(const char* s1, const char* s2) {
	while (*s1 == *s2) {
		if (*s1 == '\0') {
			return 0x0;
		}

		s1++;
		s2++;
	}

	return *s1 - *s2;
}

size_t strcmpl(const char* s1, const char* s2, size_t d) {
	for (int i = 0; i < d; i++) {
		if (*s1 != *s2) {
			return 1;
		}

		if (*s1 == '\0' || *s2 == '\0') return 2;

		s1++;
		s2++;
	}

	return 0;
}

uint8_t inportb(uint16_t port) {
	uint8_t ret;
	__asm__ __volatile__ ("inb %1, %0" : "=a" (ret) : "dN" (port));
	return ret;
}
void outportb(uint16_t port, uint8_t data) {
	__asm__ __volatile__ ("outb %1, %0" : : "dN" (port), "a" (data));
}

uint16_t inportw(uint16_t port) {
	uint16_t ret;
	asm volatile("inw %%dx, %%ax":"=a"(ret):"d"(port));
	return ret;
}
void outportw(uint16_t port, uint16_t value) {
	asm volatile("outw %%ax, %%dx": :"d" (port), "a" (value));
}

uint32_t inportl(uint16_t port) {
	uint32_t ret;
	asm volatile("inl %%dx, %%ax":"=a"(ret):"d"(port));
	return ret;
}
void outportl(uint16_t port, uint32_t value) {
	asm volatile("outl %%ax, %%dx": :"d" (port), "a" (value));
}

void stdio_debug_tests() {
    printf("[%5Float%0]:  %f\n", 0.100); // expect 0.1000000
    printf("[%5Hex%0]:    0x%x\n", 100);   // expect 0x64
    printf("[%5Int%0]:    %d\n", 100);
    printf("[%5strlen%0]: %d\n", strlen("hello")); // expect 5
    printf("[%5VGAPtr%0]: 0x%x\n", (uint32_t)vga_get_info());
}
