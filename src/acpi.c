#include <acpi.h>
#include <stdio.h>

uint32_t 	*SMI_CMD;
uint8_t 	ACPI_ENABLE;
uint8_t 	ACPI_DISABLE;
uint32_t 	*PM1a_CNT;
uint32_t 	*PM1b_CNT;
uint16_t 	SLP_TYPa;
uint16_t 	SLP_TYPb;
uint16_t 	SLP_EN;
uint16_t 	SCI_EN;
uint8_t 	PM1_CNT_LEN;

FADT* facp;

int32_t* rsdp_scan_mem() {
	uint32_t* ptr = (uint32_t*)RSDP_MEM_MIN;

	for (; (uint32_t)ptr < RSDP_MEM_MAX; *ptr++) {
		RSDPDescriptor20* desc = (RSDPDescriptor20*)ptr;

		if (!strcmpl(desc->firstPart.Signature, "RSD PTR ", 8)) {
			return ptr;
		}
	}

	return NULL;
}

RSDPDescriptor20* rsdp_init(bool debug) {
	uint32_t* ptr = rsdp_scan_mem();

	if (ptr != NULL)
		if (debug) printf("[RSDP] Ptr found 0x%x\n", ptr);


	return (RSDPDescriptor20*)ptr;
}

RSDT* rsdt_init(RSDPDescriptor20* dp, bool debug) {
	if (dp == NULL) {
		printf("[RSDT] RSDP NULL! ABORTING\n");
		return;
	}

	ACPISDTHeader* dt = (ACPISDTHeader*)dp->firstPart.RsdtAddress;

	if (!rsdt_check(dt)) {
		printf("[RSDT] Checksum not valid!\n");
		return;
	}

	RSDT* complete = (RSDT*)dt;

	return complete;
}

bool rsdt_check(ACPISDTHeader *tableHeader)
{
    unsigned char sum = 0;

    for (int i = 0; i < tableHeader->Length; i++)
    {
        sum += ((char *) tableHeader)[i];
    }

    return sum == 0;
}

FADT* facp_find(RSDT* RootSDT, bool debug)
{
    RSDT* rsdt = RootSDT;
    int entries = (rsdt->h.Length - sizeof(rsdt->h)) / 4;
 	FADT* out = NULL;

    for (int i = 0; i < entries; i++)
    {
        ACPISDTHeader* h = (ACPISDTHeader*) rsdt->PointerToOtherSDT[i];
        if (!strcmpl(h->Signature, "FACP", 4)) {
        	if (debug) printf("[FADT] Header found at 0x%x\n", h);
            out = (FADT*)h;
        }

    }

    return out;
}

char* acpi_find_dsdt_string(const char* string, uint32_t* dsdt) {
	char* current_address = (char*)dsdt;
	int dsdt_length = *(dsdt + 0x1);

	while (0 < dsdt_length--) {
		if (strcmpl(current_address, string, strlen(string)) == 0) {
			break;
		}

		current_address++;
	}

	if (current_address > 0) {
		return current_address;
	} else {
		printf("[acpi_find_dsdt_string] Failed to find '%s'\n", string);
		return NULL;
	}
}

void acpi_setup_shutdown() {
	auto S5Addr = acpi_find_dsdt_string("_S5_", (uint32_t*)(facp->Dsdt));

	if (S5Addr != NULL) {
		S5Addr += 5;
        S5Addr += ((*S5Addr &0xC0)>>6) +2;   // calculate PkgLength size

        if (*S5Addr == 0x0A)
        	S5Addr++;   // skip byteprefix
        SLP_TYPa = *(S5Addr)<<10;
        S5Addr++;

        if (*S5Addr == 0x0A)
            S5Addr++;   // skip byteprefix
        SLP_TYPb = *(S5Addr)<<10;

        SMI_CMD = facp->SMI_CommandPort;

        ACPI_ENABLE = facp->AcpiEnable;
        ACPI_DISABLE = facp->AcpiDisable;

        PM1a_CNT = facp->PM1aControlBlock;
        PM1b_CNT = facp->PM1bControlBlock;

        PM1_CNT_LEN = facp->PM1ControlLength;

        SLP_EN = 1<<13;
        SCI_EN = 1;
	} else {
		printf("[acpi_setup_shutdown] Failed to setup shutdown (S5 missing?)\n");
	}
}

void acpi_init() {
	RSDPDescriptor20* rsdp 	= rsdp_init(true);
	RSDT* rsdt 				= rsdt_init(rsdp, true);
	facp 					= facp_find(rsdt, true);

	if (facp->SMI_CommandPort != 0) {
		outportb(facp->SMI_CommandPort, facp->AcpiEnable);
		printf("[ACPI] Enabled ACPI system.\n");
	} else {
		printf("[ACPI] Failed to init ACPI\n - Power management will not be available.\n");
		return;
	}

	uint32_t* dsdt = (uint32_t*)(facp->Dsdt);

	if (strcmpl((uint8_t*)dsdt, "DSDT", 4) == 0) {
		acpi_setup_shutdown();
	} else {
		printf("DSDT Not supported??\n");
	}
}

void acpi_reboot() {

	printf("0x%x: 0x%x", facp->ResetReg, facp->ResetValue);
}

void acpi_poweroff() {
	// SCI_EN is set to 1 if acpi shutdown is possible
   	if (SCI_EN == 0)
    	return;

   	// send the shutdown command
   	outportw((unsigned int) PM1a_CNT, SLP_TYPa | SLP_EN );
   	if (PM1b_CNT != 0)
    	outportw((unsigned int) PM1b_CNT, SLP_TYPb | SLP_EN );

   	printf("acpi poweroff failed.\n");
}

// Command callback to shutdown the system.
bool CMD_Shutdown(char* inp) {

    acpi_poweroff();

    return false;
}
