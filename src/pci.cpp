#include <pci.h>
#include <cpu.h>
#include <stdio.h>
#include <cpu.h>
#include <memory.h>

pci_device** pci_devices = 0;
uint32_t devs = 0;

pci_device** pci_getdevices() {
    return pci_devices;
}

size_t pci_numdevices() {
    return devs;
}

void add_pci_device(pci_device* device) {
    pci_devices[devs] = device;
    devs ++;
    return;
}

uint16_t readword(uint16_t bus, uint16_t slot, uint16_t func, uint16_t offset) {
    uint64_t address;
    uint64_t lbus = (uint64_t)bus;
    uint64_t lslot = (uint64_t)slot;
    uint64_t lfunc = (uint64_t)func;
    uint16_t tmp = 0;
    address = (uint64_t)((lbus << 16) | (lslot << 11) | (lfunc << 8) | (offset & 0xfc) | ((uint32_t)0x80000000));
    outportl(0xCF8, address);
    tmp = (uint16_t)((inportl (0xCFC) >> ((offset & 2) * 8)) & 0xffff);
    return (tmp);
}

uint16_t pci_getvendorid(uint16_t bus, uint16_t device, uint16_t function) {
        uint32_t r0 = readword(bus,device,function,0);
        return r0;
}

uint16_t pci_getdeviceid(uint16_t bus, uint16_t device, uint16_t function) {
        uint32_t r0 = readword(bus,device,function,2);
        return r0;
}

uint16_t pci_getclassid(uint16_t bus, uint16_t device, uint16_t function) {
        uint32_t r0 = readword(bus,device,function,0xA);
        return (r0 & ~0x00FF) >> 8;
}

uint16_t pci_getsubclassid(uint16_t bus, uint16_t device, uint16_t function) {
        uint32_t r0 = readword(bus,device,function,0xA);
        return (r0 & ~0xFF00);
}

uint16_t pci_readword(pci_device* dev, uint16_t offset) {
    return readword(dev->bus, dev->slot, dev->func, offset);
}

bool debug = false, bar = false;
int scan() {
    kfree(pci_devices);
    pci_devices = (pci_device **)kmalloc(32 * sizeof(pci_device));

    uint32_t bus;
    for(bus = 0; bus < 256; bus++) {

        uint32_t slot;
        for(slot = 0; slot < 32; slot++) {

            uint32_t function;
            for(function = 0; function < 8; function++) {
                uint16_t vendor = pci_getvendorid(bus, slot, function);
                if (vendor == 0xffff) {
                    continue;
                }
                uint16_t device = pci_getdeviceid(bus, slot, function);

                if (debug) printf("[%5Info%0] FOUND(v:0x%x d:0x%x)\n", vendor, device);

                pci_device *pdev = (pci_device *)kmalloc(sizeof(pci_device));
                pdev->vendor = vendor;
                pdev->device = device;
                pdev->func = function;
                pdev->driver = 0;
                pdev->bus = bus;
                pdev->slot = slot;
                pdev->bar0 = pci_readword(pdev, PCI_BAR0);
                pdev->bar1 = pci_readword(pdev, PCI_BAR1);
                pdev->bar2 = pci_readword(pdev, PCI_BAR2);
                pdev->bar3 = pci_readword(pdev, PCI_BAR3);
                pdev->bar4 = pci_readword(pdev, PCI_BAR4);
                pdev->bar5 = pci_readword(pdev, PCI_BAR5);

                if (bar) printf("[%5Info%0]\tBar0: 0x%x Bar1: 0x%x Bar2: 0x%x Bar3: 0x%x Bar4: 0x%x Bar5: 0x%x\n",pdev->bar0,pdev->bar1,pdev->bar2,pdev->bar3,pdev->bar4,pdev->bar5);

                add_pci_device(pdev);
            }
        }
    }

    return devs;
}

pci_device* pci_getdevice(uint16_t vendor, uint16_t device) {
    for (size_t i = 0; i < devs; i++) {
        if (pci_devices[i]->vendor == vendor && pci_devices[i]->device == device) {
            return pci_devices[i];
        }
    }

    return 0;
}

#define RTL8139_VENDOR_ID 0x10EC
#define RTL8139_DEVICE_ID 0x8139
#define CMD(l, s, c) ((strcmp(c, s) == 0) || (strcmp(c, l) == 0))

void pci_rtl8139() {
    if (pci_devices == NULL) scan();

    pci_device* pdev = NULL;
    pdev = pci_getdevice(RTL8139_VENDOR_ID, RTL8139_DEVICE_ID);

    if (!pdev) {
        printf("RTL8139 not found.\n");
        return true;
    }

    uint8_t* mac_addr = (uint8_t*)kmalloc(6);

    uint32_t io_base = pdev->bar0;
    uint32_t bar_type = io_base & 0x1;
    io_base = io_base & (~0x3);

    uint32_t mac_part1 = inportl(io_base + 0x00);
    uint16_t mac_part2 = inports(io_base + 0x04);

    mac_addr[0] = mac_part1 >> 0;
    mac_addr[1] = mac_part1 >> 8;
    mac_addr[2] = mac_part1 >> 16;
    mac_addr[3] = mac_part1 >> 24;
    mac_addr[4] = mac_part2 >> 0;
    mac_addr[5] = mac_part2 >> 8;

    printf("[%5RTL8139%0] MAC Address: %x:%x:%x:%x:%x:%x\n", mac_addr[0], mac_addr[1], mac_addr[2], mac_addr[3], mac_addr[4], mac_addr[5]);
    printf("[%5RTL8139%0] %s access (%x)\n", (bar_type == 0)? "mem based":"port based", io_base);
}

bool CMD_pci(int argc, char** argv) {
    debug   = false;
    bar     = false;

    if (argc < 2) {
        printf("No args passed.\n");
        return true;
    }

    for (size_t i = 1; i < argc; i++) {
        char* cmd = argv[i];

        if (CMD("-rtl8139", "-RTL", cmd)) {
            pci_rtl8139();
        } else if (CMD("-debug", "-D", cmd)) {
            debug = true;
        } else if (CMD("-bar", "-B", cmd)) {
            bar = true;
        } else if (CMD("-scan", "-S", cmd)) {
            scan();
        } else {
            printf("Arg not recognised: '%s'\n", argv[i]);
        }
    }

    return true;
}
