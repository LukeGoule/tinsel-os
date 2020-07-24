#ifndef CPU_H
#define CPU_H

#include <cpuid.h>
#include <cdefs.h>

bool    cpu_has_msr ();
void    cpu_get_msr (uint32_t msr, uint32_t *lo, uint32_t *hi);
void    cpu_set_msr (uint32_t msr, uint32_t lo, uint32_t hi);
void    cpuid       (int code, uint32_t *a, uint32_t *d);
void    cpuid       (int code, uint32_t* eax, uint32_t* ebx, uint32_t* ecx, uint32_t* edx);
int     cpuid_string(int code, uint32_t where[4]);
bool    CMD_cpuid   (char* inp);

#endif
