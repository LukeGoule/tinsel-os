#include <cpu.h>
#include <cdefs.h>
#include <stdio.h>

const uint32_t CPUID_FLAG_MSR = 1 << 5;

void cpuid(int code, uint32_t *a, uint32_t *d) {
  asm volatile("cpuid":"=a"(*a),"=d"(*d):"a"(code):"ecx","ebx");
}

void cpuid(int code, uint32_t* eax, uint32_t* ebx, uint32_t* ecx, uint32_t* edx) {
    asm volatile("cpuid" : "=a" (eax), "=b" (ebx), "=c" (ecx), "=d" (edx) : "a" (code), "c" (0));
}

int cpuid_string(int code, uint32_t where[4]) {
  asm volatile("cpuid":"=a"(*where), "=b"(*(where+1)), "=c"(*(where+2)), "=d"(*(where+3)):"a"(code));
  return (int)where[0];
}

bool cpu_has_msr() {
   uint32_t a, d; // eax, edx
   cpuid(1, &a, &d);
   return d & CPUID_FLAG_MSR;
}

void cpu_get_msr(uint32_t msr, uint32_t *lo, uint32_t *hi) {
   asm volatile("rdmsr" : "=a"(*lo), "=d"(*hi) : "c"(msr));
}

void cpu_set_msr(uint32_t msr, uint32_t lo, uint32_t hi) {
   asm volatile("wrmsr" : : "a"(lo), "d"(hi), "c"(msr));
}

bool CMD_cpuid(char* inp) {
    printf("CPU MSR: %s\n", cpu_has_msr() ? "Yes" : "No");
    return true;
}
