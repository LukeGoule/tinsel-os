#ifndef PCI_H
#define PCI_H

#include <cdefs.h>

// Ports
#define PCI_CONFIG_ADDRESS  0xCF8
#define PCI_CONFIG_DATA     0xCFC

// Config Address Register

// Offset
#define PCI_VENDOR_ID            0x00
#define PCI_DEVICE_ID            0x02
#define PCI_COMMAND              0x04
#define PCI_STATUS               0x06
#define PCI_REVISION_ID          0x08
#define PCI_PROG_IF              0x09
#define PCI_SUBCLASS             0x0a
#define PCI_CLASS                0x0b
#define PCI_CACHE_LINE_SIZE      0x0c
#define PCI_LATENCY_TIMER        0x0d
#define PCI_HEADER_TYPE          0x0e
#define PCI_BIST                 0x0f
#define PCI_BAR0                 0x10
#define PCI_BAR1                 0x14
#define PCI_BAR2                 0x18
#define PCI_BAR3                 0x1C
#define PCI_BAR4                 0x20
#define PCI_BAR5                 0x24
#define PCI_INTERRUPT_LINE       0x3C
#define PCI_SECONDARY_BUS        0x09

// Device type
#define PCI_HEADER_TYPE_DEVICE  0
#define PCI_HEADER_TYPE_BRIDGE  1
#define PCI_HEADER_TYPE_CARDBUS 2
#define PCI_TYPE_BRIDGE 0x0604
#define PCI_TYPE_SATA   0x0106
#define PCI_NONE 0xFFFF

#define DEVICE_PER_BUS           32
#define FUNCTION_PER_DEVICE      32

typedef struct {
	uint32_t vendor;
	uint32_t device;
	uint32_t func;
	uint32_t bus;
	uint32_t slot;
	uint16_t bar0, bar1, bar2, bar3, bar4, bar5;
	struct __pci_driver *driver;
} pci_device;

int scan();
void dump();
uint16_t pci_readword(pci_device* dev, uint16_t offset);
pci_device* pci_getdevice(uint16_t vendor, uint16_t device);
bool CMD_pci(int argc, char** argv);

#endif
