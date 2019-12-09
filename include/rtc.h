#ifndef RTC_H
#define RTC_H

#include <cdefs.h>

#define CURRENT_YEAR 2019
enum {
	cmos_address 	= 0x70,
	cmos_data 		= 0x71
};

class RealTimeClock {
public:
    uint8_t second;
	uint8_t minute;
	uint8_t hour;
	uint8_t day;
	uint8_t month;
	uint8_t year;

    RealTimeClock();
    int         get_update_in_progress_flag();
    uint8_t     get_RTC_register(int reg);
    void        read_rtc();
};

bool CMD_Time(int argc, char** argv);

#endif
