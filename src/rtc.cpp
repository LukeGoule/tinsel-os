#include <rtc.h>
#include <stdio.h>
#include <memory.h>
#include <ints/ints.h>
#include <mouse.h>

RealTimeClock* rtc_instance = NULL;

uint8_t rtc_reg(int reg) {
	outportb(cmos_address, reg);
	return (inportb(cmos_data));
}

// previous second, if it's not changed, then don't update the screen. doesn't need to be done
// for any other registers.
uint8_t _second 	= 0x00;

extern "C" void cpp_rtc_handler() {
	_DISABLE_INTS;

	uint8_t second 	= rtc_reg(0x00);
	second 	= (second & 0x0F) 	+ ((second / 16) * 10);
	if (second == _second) {
		// Read everything away so the PIC shuts up.
		outportb(0x70, 0x0C);
		inportb(0x71);

		_END_OF_IRQ(8);
		_ENABLE_INTS;

		return;
	} else {
		_second = second;
	}

	uint8_t minute 	= rtc_reg(0x02);
	minute 	= (minute & 0x0F) + ((minute / 16) * 10);

	uint8_t hour 	= rtc_reg(0x04);
	hour 	= (hour & 0x0F) + (((hour & 0x70) / 16) * 10) | (hour & 0x80);

	uint8_t day 	= rtc_reg(0x07);
	day 	= (day & 0x0F) + ((day / 16) * 10);

	uint8_t month 	= rtc_reg(0x08);
	month 	= (month & 0x0F) + ((month / 16) * 10);

	uint8_t year 	= rtc_reg(0x09);
	year 	= (year & 0x0F) + ((year / 16) * 10);

	cursor_t* c = std::get_cursor();
	uint32_t ox = c->x, oy = c->y;

	/*
	Code used here should execute only once per second, roughly on the second (+-1/1024 seconds)
	I could time something here, which is always useful.
	*/

	c->x = 0;
	c->y = 0;

	// clear so we don't get the year showing as 199.
	std::printf("                               "); c->x = 0; //go to the line start
	std::printf("TINSEL V1 %d:%d:%d -- %d/%d/%d", hour, minute, second, day, month, year	);

	// go back to where the cursor originally was.
	c->x = ox;
	c->y = oy;

	outportb(0x70, 0x0C);	// select register C
	inportb(0x71);			// just throw away contents

	_END_OF_IRQ(8);
	_ENABLE_INTS;
}


RealTimeClock::RealTimeClock() {

}

int RealTimeClock::get_update_in_progress_flag() {
	outportb(cmos_address, 0x0A);
	return (inportb(cmos_data) & 0x80);
}

uint8_t RealTimeClock::get_RTC_register(int reg) {
	outportb(cmos_address, reg);
	return (inportb(cmos_data));
}

int century_register = 0x00;

void RealTimeClock::read_rtc() {
	unsigned char century;
	unsigned char last_second;
	unsigned char last_minute;
	unsigned char last_hour;
	unsigned char last_day;
	unsigned char last_month;
	unsigned char last_year;
	unsigned char last_century;
	unsigned char registerB;

	while (this->get_update_in_progress_flag());
	second = get_RTC_register(0x00);
	minute = get_RTC_register(0x02);
	hour = get_RTC_register(0x04);
	day = get_RTC_register(0x07);
	month = get_RTC_register(0x08);
	year = get_RTC_register(0x09);
	if (century_register != 0) {
		century = get_RTC_register(century_register);
	}

	do {
		last_second = second;
		last_minute = minute;
		last_hour = hour;
		last_day = day;
		last_month = month;
		last_year = year;
		last_century = century;

		while (get_update_in_progress_flag());
		second = get_RTC_register(0x00);
		minute = get_RTC_register(0x02);
		hour = get_RTC_register(0x04);
		day = get_RTC_register(0x07);
		month = get_RTC_register(0x08);
		year = get_RTC_register(0x09);
		if (century_register != 0) {
			century = get_RTC_register(century_register);
		}
	} while (last_second != second || last_minute != minute || last_hour != hour || last_day != day || last_month != month || last_year != year || last_century != century);

	registerB = get_RTC_register(0x0B);

	if (!(registerB & 0x04)) {
		second = (second & 0x0F) + ((second / 16) * 10);
		minute = (minute & 0x0F) + ((minute / 16) * 10);
		hour = (hour & 0x0F) + (((hour & 0x70) / 16) * 10) | (hour & 0x80);
		day = (day & 0x0F) + ((day / 16) * 10);
		month = (month & 0x0F) + ((month / 16) * 10);
		year = (year & 0x0F) + ((year / 16) * 10);
		if (century_register != 0) {
			century = (century & 0x0F) + ((century / 16) * 10);
		}
	}

	if (!(registerB & 0x02) && (hour & 0x20)) {
		hour = ((hour & 0x7F) + 12) % 24;
	}

	if (century_register != 0) {
		year += century;
	} else {
		year += (CURRENT_YEAR / 100) * 100;
		if (year < CURRENT_YEAR) year += 100;
	}
}

RealTimeClock* cmd_clock = NULL;

bool CMD_Time(int argc, char** argv) {
	if (!cmd_clock) {
		cmd_clock = new RealTimeClock();
	}

	cmd_clock->read_rtc();

	// I can't be arsed writing an sprintf function.
	std::printf("[%5Info%0] Time: ");
	if (cmd_clock->minute < 10) {
		std::printf("%d:0%d", cmd_clock->hour, cmd_clock->minute);
	} else {
		std::printf("%d:%d", cmd_clock->hour, cmd_clock->minute);
	}

	std::printf("\n");

	return true;
}

void rtc_irq() {
	// https://wiki.osdev.org/RTC
	__asm__("cli");
	outportb(0x70, 0x8B);			// select register B, and disable NMI
    char prev = inportb(0x71);		// read the current value of register B
    outportb(0x70, 0x8B);			// set the index again (a read will reset the index to register D)
    outportb(0x71, prev | 0x40);	// write the previous value ORed with 0x40. This turns on bit 6 of register B
	__asm__("sti");
}
