#ifndef ACPI_H
#define ACPI_H

#include <cdefs.h>

#define RSDP_MEM_MIN 0x000E0000
#define RSDP_MEM_MAX 0x000FFFFF

struct RSDPtr
{
	char Signature[8];
	char CheckSum;
	char OemID[6];
	char Revision;
	uint32_t* RsdtAddress;
};

struct ACPISDTHeader {
  char Signature[4];
  uint32_t Length;
  uint8_t Revision;
  uint8_t Checksum;
  char OEMID[6];
  char OEMTableID[8];
  uint32_t OEMRevision;
  uint32_t CreatorID;
  uint32_t CreatorRevision;
};

struct FACP {
	char Signature[8];
	uint32_t Length;
	char unused1[40-8];
	uint32_t* DSDT;
	char unused2[48-44];
	uint32_t* SMI_CMD;
	char ACPI_ENABLE;
	char ACPI_DISABLE;
	char unused3[64-54];
	uint32_t* PM1a_CNT_BLK;
	uint32_t* PM1b_CNT_BLK;
	char unused4[89-72];
	char PM1_CNT_LEN;
};

typedef struct GenericAddressStructure
{
    uint8_t AddressSpace;
    uint8_t BitWidth;
    uint8_t BitOffset;
    uint8_t AccessSize;
    uint64_t Address;
};

struct FADT
{
    struct   ACPISDTHeader h;
    uint32_t FirmwareCtrl;
    uint32_t Dsdt;

    // field used in ACPI 1.0; no longer in use, for compatibility only
    uint8_t  Reserved;

    uint8_t  PreferredPowerManagementProfile;
    uint16_t SCI_Interrupt;
    uint32_t SMI_CommandPort;
    uint8_t  AcpiEnable;
    uint8_t  AcpiDisable;
    uint8_t  S4BIOS_REQ;
    uint8_t  PSTATE_Control;
    uint32_t PM1aEventBlock;
    uint32_t PM1bEventBlock;
    uint32_t PM1aControlBlock;
    uint32_t PM1bControlBlock;
    uint32_t PM2ControlBlock;
    uint32_t PMTimerBlock;
    uint32_t GPE0Block;
    uint32_t GPE1Block;
    uint8_t  PM1EventLength;
    uint8_t  PM1ControlLength;
    uint8_t  PM2ControlLength;
    uint8_t  PMTimerLength;
    uint8_t  GPE0Length;
    uint8_t  GPE1Length;
    uint8_t  GPE1Base;
    uint8_t  CStateControl;
    uint16_t WorstC2Latency;
    uint16_t WorstC3Latency;
    uint16_t FlushSize;
    uint16_t FlushStride;
    uint8_t  DutyOffset;
    uint8_t  DutyWidth;
    uint8_t  DayAlarm;
    uint8_t  MonthAlarm;
    uint8_t  Century;

    // reserved in ACPI 1.0; used since ACPI 2.0+
    uint16_t BootArchitectureFlags;

    uint8_t  Reserved2;
    uint32_t Flags;

    // 12 byte structure; see below for details
    GenericAddressStructure ResetReg;

    uint8_t  ResetValue;
    uint8_t  Reserved3[3];

    // 64bit pointers - Available on ACPI 2.0+
    uint64_t                X_FirmwareControl;
    uint64_t                X_Dsdt;

    GenericAddressStructure X_PM1aEventBlock;
    GenericAddressStructure X_PM1bEventBlock;
    GenericAddressStructure X_PM1aControlBlock;
    GenericAddressStructure X_PM1bControlBlock;
    GenericAddressStructure X_PM2ControlBlock;
    GenericAddressStructure X_PMTimerBlock;
    GenericAddressStructure X_GPE0Block;
    GenericAddressStructure X_GPE1Block;
};

typedef struct RSDPDescriptor {
	char Signature[8];
	uint8_t Checksum;
 	char OEMID[6];
 	uint8_t Revision;
 	uint32_t RsdtAddress;
} __attribute__ ((packed));

typedef struct RSDPDescriptor20 {
 	RSDPDescriptor firstPart;

 	uint32_t Length;
 	uint64_t XsdtAddress;
 	uint8_t ExtendedChecksum;
 	uint8_t reserved[3];
} __attribute__ ((packed));

struct RSDT {
  struct ACPISDTHeader h;
  uint32_t PointerToOtherSDT[256];
};

RSDPDescriptor20* rsdp_init(bool debug);

RSDT* rsdt_init(RSDPDescriptor20* dp, bool debug);
bool rsdt_check(ACPISDTHeader *tableHeader);

FADT* facp_find(RSDT *RootSDT, bool debug);

void acpi_init();
void acpi_reboot();
void acpi_poweroff();


#endif
