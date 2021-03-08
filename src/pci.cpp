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

uint32_t read_dword(uint16_t bus, uint16_t slot, uint16_t func, uint32_t offset) {
    uint64_t address;
    uint64_t lbus = (uint64_t)bus;
    uint64_t lslot = (uint64_t)slot;
    uint64_t lfunc = (uint64_t)func;
    uint16_t tmp = 0;
    address = (uint64_t)((lbus << 16) | (lslot << 11) | (lfunc << 8) | (offset & 0xfc) | ((uint32_t)0x80000000));
    outportl(0xCF8, address);
    tmp = (uint32_t)((inportl (0xCFC) >> ((offset & 2) * 8)) & 0xffff);
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


bool debug = true, bar = true;
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
                uint16_t classid = pci_getclassid(bus, device, function);

                if (debug) std::printf("[%5Info%0] FOUND(v:0x%x d:0x%x c:0x%x)\n", vendor, device, classid);
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

                if (bar) std::printf("[%5Info%0]\tBar0: 0x%x Bar1: 0x%x Bar2: 0x%x Bar3: 0x%x Bar4: 0x%x Bar5: 0x%x\n",pdev->bar0,pdev->bar1,pdev->bar2,pdev->bar3,pdev->bar4,pdev->bar5);

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
#define CMD(l, s, c) ((std::strcmp(c, s) == 0) || (std::strcmp(c, l) == 0))

//https://wiki.osdev.org/RTL8139
void pci_rtl8139() {
    if (pci_devices == NULL) scan();

    pci_device* pdev = NULL;
    pdev = pci_getdevice(RTL8139_VENDOR_ID, RTL8139_DEVICE_ID);

    if (!pdev) {
        std::printf("RTL8139 not found.\n");
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

    std::printf("[%5RTL8139%0] MAC Address: %x:%x:%x:%x:%x:%x\n", mac_addr[0], mac_addr[1], mac_addr[2], mac_addr[3], mac_addr[4], mac_addr[5]);
    std::printf("[%5RTL8139%0] %s access (%x)\n", (bar_type == 0) ? "mem based": "port based", io_base);

    if (bar_type != 0)
    {
        outportb(io_base + 0x52, 0x0);
        std::cout << "RTL8139 woken up." << std::endl;

        outportb(io_base + 0x37, 0x10);
        while((inportb(io_base + 0x37) & 0x10) != 0) {}
        std::cout << "Firmware reset complete." << std::endl;

        uint8_t* rtl8139_buffer = (uint8_t*)kmalloc(8192 + 16);

        //ioaddr is obtained from PCI configuration
        outportl(io_base + 0x30, (uint32_t)rtl8139_buffer); // send uint32_t memory location to RBSTART (0x30)
        std::cout << "8K+16 buffer initialised." << std::endl;

        outportl(io_base + 0x3C, 0x0005); // Sets the TOK and ROK bits high
        std::cout << "IMR+ISR setup." << std::endl;

        outportl(io_base + 0x44, 0xf | (1 << 7)); // (1 << 7) is the WRAP bit, 0xf is AB+AM+APM+AAP
        std::cout << "recv buffer setup" << std::endl;

        outportb(io_base + 0x37, 0x0C); // Sets the RE and TE bits high
        std::cout << "recv+trans buffer set" << std::endl;
    } 
    else 
    {
        std::cout << "not woken." << std::endl;
    }
}

uint32_t pci_size_map[100];
pci_dev_t dev_zero= {0};
/*
 * Given a pci device(32-bit vars containing info about bus, device number, and function number), a field(what u want to read from the config space)
 * Read it for me !
 * */
uint32_t pci_read(pci_dev_t dev, uint32_t field) {
	// Only most significant 6 bits of the field
	dev.field_num = (field & 0xFC) >> 2;
	dev.enable = 1;
	outportl(PCI_CONFIG_ADDRESS, dev.bits);

	// What size is this field supposed to be ?
	uint32_t size = pci_size_map[field];
	if(size == 1) {
		// Get the first byte only, since it's in little endian, it's actually the 3rd byte
		uint8_t t = inportb(PCI_CONFIG_DATA + (field & 3));
		return t;
	}
	else if(size == 2) {
		uint16_t t = inports(PCI_CONFIG_DATA + (field & 2));
		return t;
	}
	else if(size == 4){
		// Read entire 4 bytes
		uint32_t t = inportl(PCI_CONFIG_DATA);
		return t;
	}
	return 0xffff;
}

/*
 * Write pci field
 * */
void pci_write(pci_dev_t dev, uint32_t field, uint32_t value) {
	dev.field_num = (field & 0xFC) >> 2;
	dev.enable = 1;
	// Tell where we want to write
	outportl(PCI_CONFIG_ADDRESS, dev.bits);
	// Value to write
	outportl(PCI_CONFIG_DATA, value);
}

/*
 * Get device type (i.e, is it a bridge, ide controller ? mouse controller? etc)
 * */
uint32_t get_device_type(pci_dev_t dev) {
	uint32_t t = pci_read(dev, PCI_CLASS) << 8;
	return t | pci_read(dev, PCI_SUBCLASS);
}

/*
 * Get secondary bus from a PCI bridge device
 * */
uint32_t get_secondary_bus(pci_dev_t dev) {
	return pci_read(dev, PCI_SECONDARY_BUS);
}

/*
 * Is current device an end point ? PCI_HEADER_TYPE 0 is end point
 * */
uint32_t pci_reach_end(pci_dev_t dev) {
	uint32_t t = pci_read(dev, PCI_HEADER_TYPE);
	return !t;
}

/*
 * The following three functions are basically doing recursion, enumerating each and every device connected to pci
 * We start with the primary bus 0, which has 8 function, each of the function is actually a bus
 * Then, each bus can have 8 devices connected to it, each device can have 8 functions
 * When we gets to enumerate the function, check if the vendor id and device id match, if it does, we've found our device !
 **/

/*
 * Scan function
 * */
pci_dev_t pci_scan_function(uint16_t vendor_id, uint16_t device_id, uint32_t bus, uint32_t device, uint32_t function, int device_type) {
	pci_dev_t dev = {0};
	dev.bus_num = bus;
	dev.device_num = device;
	dev.function_num = function;
	// If it's a PCI Bridge device, get the bus it's connected to and keep searching
	if(get_device_type(dev) == PCI_TYPE_BRIDGE) {
		pci_scan_bus(vendor_id, device_id, get_secondary_bus(dev), device_type);
	}
    uint32_t devid  = pci_read(dev, PCI_DEVICE_ID);
    uint32_t vendid = pci_read(dev, PCI_VENDOR_ID);
    std::printf("[bus:0x%x] found: v:0x%x d:0x%x\n", bus, vendid, devid);
	// If type matches, we've found the device, just return it
	if(device_type == -1 || device_type == get_device_type(dev)) {
		if(devid == device_id && vendor_id == vendid)
			return dev;
	}
	return dev_zero;
}

/*
 * Scan device
 * */
pci_dev_t pci_scan_device(uint16_t vendor_id, uint16_t device_id, uint32_t bus, uint32_t device, int device_type) {
	pci_dev_t dev = {0};
	dev.bus_num = bus;
	dev.device_num = device;

	if(pci_read(dev,PCI_VENDOR_ID) == PCI_NONE)
		return dev_zero;

	pci_dev_t t = pci_scan_function(vendor_id, device_id, bus, device, 0, device_type);
	if(t.bits) {
		return t;
    }

	if(pci_reach_end(dev))
		return dev_zero;

	for(int function = 1; function < FUNCTION_PER_DEVICE; function++) {
		if(pci_read(dev,PCI_VENDOR_ID) != PCI_NONE) {
			t = pci_scan_function(vendor_id, device_id, bus, device, function, device_type);
			if(t.bits) {
				return t;
            }
		}
	}
	return dev_zero;
}
/*
 * Scan bus
 * */
pci_dev_t pci_scan_bus(uint16_t vendor_id, uint16_t device_id, uint32_t bus, int device_type) {
	for(int device = 0; device < DEVICE_PER_BUS; device++) {
		pci_dev_t t = pci_scan_device(vendor_id, device_id, bus, device, device_type);
		if(t.bits) {
			return t;
        }
	}
	return dev_zero;
}

/*
 * Device driver use this function to get its device object(given unique vendor id and device id)
 * */
pci_dev_t pci_get_device(uint16_t vendor_id, uint16_t device_id, int device_type) {

	pci_dev_t t = pci_scan_bus(vendor_id, device_id, 0, device_type);
	if(t.bits) {
        //printf("found.\n");
		return t;
    }

	// Handle multiple pci host controllers

	if(pci_reach_end(dev_zero)) {
		//printf("PCI Get device failed...\n");
	}
	for(int function = 1; function < FUNCTION_PER_DEVICE; function++) {
		pci_dev_t dev = {0};
		dev.function_num = function;

		if(pci_read(dev, PCI_VENDOR_ID) == PCI_NONE) {
			break;
        }

        t = pci_scan_bus(vendor_id, device_id, function, device_type);

        if(t.bits) {
            //printf("found.\n");
			return t;
        }
	}
    //printf("not found.\n");
	return dev_zero;
}

/*
 * PCI Init, filling size for each field in config space
 * */
void pci_init() {
	// Init size map
	pci_size_map[PCI_VENDOR_ID] =	2;
	pci_size_map[PCI_DEVICE_ID] =	2;
	pci_size_map[PCI_COMMAND]	=	2;
	pci_size_map[PCI_STATUS]	=	2;
	pci_size_map[PCI_SUBCLASS]	=	1;
	pci_size_map[PCI_CLASS]		=	1;
	pci_size_map[PCI_CACHE_LINE_SIZE]	= 1;
	pci_size_map[PCI_LATENCY_TIMER]		= 1;
	pci_size_map[PCI_HEADER_TYPE] = 1;
	pci_size_map[PCI_BIST] = 1;
	pci_size_map[PCI_BAR0] = 4;
	pci_size_map[PCI_BAR1] = 4;
	pci_size_map[PCI_BAR2] = 4;
	pci_size_map[PCI_BAR3] = 4;
	pci_size_map[PCI_BAR4] = 4;
	pci_size_map[PCI_BAR5] = 4;
	pci_size_map[PCI_INTERRUPT_LINE]	= 1;
	pci_size_map[PCI_SECONDARY_BUS]		= 1;
}

#define RTL8139_VENDOR_ID 0x10EC
#define RTL8139_DEVICE_ID 0x8139

bool CMD_pci(int argc, char** argv) {
    pci_init();

    debug   = false;
    bar     = false;

    if (argc < 2) {
        std::printf("No args passed.\n");
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
            std::printf("Arg not recognised: '%s'\n", argv[i]);
        }
    }

    return true;
}
