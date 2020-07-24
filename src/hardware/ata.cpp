#include <kernel.h>
#include <ata.h>
#include <stdio.h>
#include <memory.h>
#include <ext2.h>
#include <ints/ints.h>

#define ATA_PRIMARY_IO 0x1F0
#define ATA_SECONDARY_IO 0x170

#define ATA_PRIMARY_DCR_AS 0x3F6
#define ATA_SECONDARY_DCR_AS 0x376

#define ATA_PRIMARY_IRQ 14
#define ATA_SECONDARY_IRQ 15

uint8_t ata_pm = 0; /* Primary master exists? */
uint8_t ata_ps = 0; /* Primary Slave exists? */
uint8_t ata_sm = 0; /* Secondary master exists? */
uint8_t ata_ss = 0; /* Secondary slave exists? */

uint8_t *ide_buf = 0;
ide_device_t* dev = 0;

uint8_t ide_get_error(uint16_t io) {
	return inportb(io + ATA_REG_ERROR);
}

void ide_select_drive(uint8_t bus, uint8_t i)
{
	if(bus == ATA_PRIMARY)
		if(i == ATA_MASTER)
			outportb(ATA_PRIMARY_IO + ATA_REG_HDDEVSEL, 0xA0);
		else outportb(ATA_PRIMARY_IO + ATA_REG_HDDEVSEL, 0xB0);
	else
		if(i == ATA_MASTER)
			outportb(ATA_SECONDARY_IO + ATA_REG_HDDEVSEL, 0xA0);
		else outportb(ATA_SECONDARY_IO + ATA_REG_HDDEVSEL, 0xB0);
}

const char* ide_drive_name(ide_device_t* dev) {
	switch(dev->drive)
	{
		case (ATA_PRIMARY << 1 | ATA_MASTER):
			return "HDA";
			break;
		case (ATA_PRIMARY << 1 | ATA_SLAVE):
			return "HDB";
			break;
		case (ATA_SECONDARY << 1 | ATA_MASTER):
			return "HDC";
			break;
		case (ATA_SECONDARY << 1 | ATA_SLAVE):
			return "HDD";
			break;
		default:
			printf("[ATA] (ide_drive_name) FATAL: unknown drive!\n");
			return "";
	}
}

extern "C" void cpp_ide_primary()
{
	_DISABLE_INTS;
	{
		//printf("IDE PRIM IRQ\n");
	}
	_END_OF_IRQ(14);
	_ENABLE_INTS;
}

extern "C" void cpp_ide_secondary()
{
	_DISABLE_INTS;
	{
		//printf("IDE SEC IRQ\n");
	}
	_END_OF_IRQ(15);
	_ENABLE_INTS;
}

uint8_t ide_identify(uint8_t bus, uint8_t drive)
{
	uint16_t io = 0;
	ide_select_drive(bus, drive);
	if(bus == ATA_PRIMARY) io = ATA_PRIMARY_IO;
	else io = ATA_SECONDARY_IO;
	/* ATA specs say these values must be zero before sending IDENTIFY */
	outportb(io + ATA_REG_SECCOUNT0, 0);
	outportb(io + ATA_REG_LBA0, 0);
	outportb(io + ATA_REG_LBA1, 0);
	outportb(io + ATA_REG_LBA2, 0);
	/* Now, send IDENTIFY */
	outportb(io + ATA_REG_COMMAND, ATA_CMD_IDENTIFY);
	//printf("[ATA] Sent IDENTIFY to (%s:%s)\n", bus==ATA_PRIMARY?"PRIMARY":"SECONDARY", drive==ATA_PRIMARY?"MASTER":"SLAVE");
	/* Now, read status port */
	uint8_t status = inportb(io + ATA_REG_STATUS);
	if(status)
	{
		/* Now, poll untill BSY is clear. */
		while(inportb(io + ATA_REG_STATUS) & ATA_SR_BSY != 0) ;
		pm_stat_read:
		status = inportb(io + ATA_REG_STATUS);

		if(status & ATA_SR_ERR)
		{
			printf("[ATA] %s:%s has ERR bit set.\n", bus==ATA_PRIMARY?"PRIMARY":"SECONDARY", drive==ATA_PRIMARY?"MASTER":"SLAVE");
			return 0;
		}

		while(!(status & ATA_SR_DRQ)) goto pm_stat_read;

		printf("[ATA] %s:%s is available.\n", bus==ATA_PRIMARY?"PRIMARY":"SECONDARY", drive==ATA_PRIMARY?"MASTER":"SLAVE");

		for(int i = 0; i<256; i++)
		{
			*(uint16_t *)(ide_buf + i*2) = inportw(io + ATA_REG_DATA);
		}

	}

    return true;
}

void ide_400ns_delay(uint16_t io)
{
	for(int i = 0;i < 4; i++)
		inportb(io + ATA_REG_ALTSTATUS);
}

void ide_poll(uint16_t io)
{

	ide_400ns_delay(io);

    uint8_t drq = 0, err = 0;
    // If either drq or err is set, stop the while loop
    do {
        drq = inportb(io + ATA_REG_STATUS) & ATA_SR_DRQ;
        err = inportb(io + ATA_REG_STATUS) & ATA_SR_ERR;

		auto erid = ide_get_error(io);

		if (erid & ATA_ER_AMNF) {
			printf("[ATA Err(%d)]: ATA_ER_AMNF (Address mark not found)\n", erid);
		}
		if (erid & ATA_ER_TK0NF) {
			printf("[ATA Err(%d)]: ATA_ER_TK0NF (Track 0 not found)\n", erid);
		}
		if (erid & ATA_ER_ABRT) {
			printf("[ATA Err(%d)]: ATA_ER_ABRT (Abort)\n", erid);
		}
		if (erid & ATA_ER_MCR) {
			printf("[ATA Err(%d)]: ATA_ER_MCR (Media change request)\n", erid);
		}
		if (erid & ATA_ER_IDNF) {
			printf("[ATA Err(%d)]: ATA_ER_IDNF (ID Not Found)\n", erid);
		}
		if (erid & ATA_ER_MC) {
			printf("[ATA Err(%d)]: ATA_ER_MC (Media changed)\n", erid);
		}
		if (erid & ATA_ER_UNC) {
			printf("[ATA Err(%d)]: ATA_ER_UNC (Uncorrectable data error)\n", erid);
		}
		if (erid & ATA_ER_BBK) {
			printf("[ATA Err(%d)]: ATA_ER_BBK (Bad block request)\n", erid);
		}
    } while(!drq && !err);

    return;
}

void outportsm(unsigned short port, unsigned char * data, unsigned long size) {
	asm volatile ("rep outsw" : "+S" (data), "+c" (size) : "d" (port));
}
void inportsm(unsigned short port, unsigned char * data, unsigned long size) {
	asm volatile ("rep insw" : "+D" (data), "+c" (size) : "d" (port) : "memory");
}

void ata_write_one(uint32_t lba, uint8_t * buf, ide_device_t* priv) {
    uint8_t drive = priv->drive;
	uint16_t io = 0;

	switch(drive)
	{
		case (ATA_PRIMARY << 1 | ATA_MASTER):
			io = ATA_PRIMARY_IO;
			drive = ATA_MASTER;
			break;
		case (ATA_PRIMARY << 1 | ATA_SLAVE):
			io = ATA_PRIMARY_IO;
			drive = ATA_SLAVE;
			break;
		case (ATA_SECONDARY << 1 | ATA_MASTER):
			io = ATA_SECONDARY_IO;
			drive = ATA_MASTER;
			break;
		case (ATA_SECONDARY << 1 | ATA_SLAVE):
			io = ATA_SECONDARY_IO;
			drive = ATA_SLAVE;
			break;
		default:
			printf("[ATA] FATAL: unknown drive!\n");
			return 0;
	}

    uint8_t cmd = (drive==ATA_MASTER?0xE0:0xF0);
	uint8_t slavebit = (drive == ATA_MASTER?0x00:0x01);
	printf("Setup write.  ");

	outportb(io + ATA_REG_HDDEVSEL, (cmd | (uint8_t)((lba >> 24 & 0x0F))));

	outportb(io + ATA_REG_CONTROL, 0x00);
	outportb(io + ATA_REG_SECCOUNT0, (uint8_t)0x1);
	outportb(io + ATA_REG_LBA0, (uint8_t)((lba)));
	outportb(io + ATA_REG_LBA1, (uint8_t)((lba) >> 8));
	outportb(io + ATA_REG_LBA2, (uint8_t)((lba) >> 16));
    outportb(io + ATA_REG_COMMAND, ATA_CMD_WRITE_PIO);

    ide_poll(io); // this will report any errors during setup

	printf("Start write.  ");
    // Start sending the data.
	for(int i=0; i < 512; i += 2) {
		uint16_t data = (uint16_t)buf[i];
		data |= ((uint16_t)buf[i+1]) << 8;
		outportw(io + ATA_REG_DATA, data);
        outportb(io + 0x07, ATA_CMD_CACHE_FLUSH);
	}

    ide_poll(io); // spew out errors during writing.
	printf(" End write.");

    ide_400ns_delay(io);
}


uint8_t ata_read_one(uint8_t *buf, uint32_t lba, ide_device_t *priv)
{
	//lba &= 0x00FFFFFF; // ignore topmost byte
	/* We only support 28bit LBA so far */
	uint8_t drive = priv->drive;
	uint16_t io = 0;
	switch(drive)
	{
		case (ATA_PRIMARY << 1 | ATA_MASTER):
			io = ATA_PRIMARY_IO;
			drive = ATA_MASTER;
			break;
		case (ATA_PRIMARY << 1 | ATA_SLAVE):
			io = ATA_PRIMARY_IO;
			drive = ATA_SLAVE;
			break;
		case (ATA_SECONDARY << 1 | ATA_MASTER):
			io = ATA_SECONDARY_IO;
			drive = ATA_MASTER;
			break;
		case (ATA_SECONDARY << 1 | ATA_SLAVE):
			io = ATA_SECONDARY_IO;
			drive = ATA_SLAVE;
			break;
		default:
			printf("[ATA] FATAL: unknown drive!\n");
			return 0;
	}
	//printf("io=0x%x %s\n", io, drive==ATA_MASTER?"Master":"Slave");
	uint8_t cmd = (drive==ATA_MASTER?0xE0:0xF0);
	uint8_t slavebit = (drive == ATA_MASTER?0x00:0x01);
	//printf("LBA = 0x%x\n", lba);
	//printf("LBA>>24 & 0x0f = %d\n", (lba >> 24)&0x0f);
	//printf("(uint8_t)lba = %d\n", (uint8_t)lba);
	//printf("(uint8_t)(lba >> 8) = %d\n", (uint8_t)(lba >> 8));
	//printf("(uint8_t)(lba >> 16) = %d\n", (uint8_t)(lba >> 16));
	//outportb(io + ATA_REG_HDDEVSEL, cmd | ((lba >> 24)&0x0f));
	outportb(io + ATA_REG_HDDEVSEL, (cmd | (uint8_t)((lba >> 24 & 0x0F))));
	//printf("issued 0x%x to 0x%x\n", (cmd | (lba >> 24)&0x0f), io + ATA_REG_HDDEVSEL);
	//for(int k = 0; k < 10000; k++) ;
	outportb(io + ATA_REG_ERROR, 0x00);
	//printf("issued 0x%x to 0x%x\n", 0x00, io + 1);
	//for(int k = 0; k < 10000; k++) ;
	outportb(io + ATA_REG_SECCOUNT0, 1);
	//printf("issued 0x%x to 0x%x\n", (uint8_t) numsects, io + ATA_REG_SECCOUNT0);
	//for(int k = 0; k < 10000; k++) ;
	outportb(io + ATA_REG_LBA0, (uint8_t)((lba)));
	//printf("issued 0x%x to 0x%x\n", (uint8_t)((lba)), io + ATA_REG_LBA0);
	//for(int k = 0; k < 10000; k++) ;
	outportb(io + ATA_REG_LBA1, (uint8_t)((lba) >> 8));
	//printf("issued 0x%x to 0x%x\n", (uint8_t)((lba) >> 8), io + ATA_REG_LBA1);
	//for(int k = 0; k < 10000; k++) ;
	outportb(io + ATA_REG_LBA2, (uint8_t)((lba) >> 16));
	//printf("issued 0x%x to 0x%x\n", (uint8_t)((lba) >> 16), io + ATA_REG_LBA2);
	//for(int k = 0; k < 10000; k++) ;
	outportb(io + ATA_REG_COMMAND, ATA_CMD_READ_PIO);
	//printf("issued 0x%x to 0x%x\n", ATA_CMD_READ_PIO, io + ATA_REG_COMMAND);

	ide_poll(io);

	//set_task(0);
	for(int i = 0; i < 256; i++)
	{
		uint16_t data = inportw(io + ATA_REG_DATA);
		*(uint16_t *)(buf + i * 2) = data;
	}
	ide_400ns_delay(io);
	//set_task(1);
	return 1;
}

void ata_read(uint8_t *buf, uint32_t lba, uint32_t numsects, ide_device_t *dev)
{
	for(int i = 0; i < numsects; i++)
	{
		ata_read_one(buf, lba + i, dev);
		buf += 512;
	}
}

//https://github.com/szhou42/osdev/blob/f942ece934237ccefe8cc3e72bc07f8d83b47ae5/src/kernel/drivers/ata.c#L165
void ata_write(uint8_t *buf, uint32_t lba, uint32_t numsects, ide_device_t *dev)
{
	for(int i = 0; i < numsects; i++)
	{
		ata_write_one(buf, lba + i, dev);
		buf += 512;
	}
}

void ata_name_print() {
    char *str = (char *)kmalloc(40);
    for(int i = 0; i < 40; i += 2) {
        str[i] = ide_buf[ATA_IDENT_MODEL + i + 1];
        str[i + 1] = ide_buf[ATA_IDENT_MODEL + i];
    }
    printf("[ATA] Using device: \"%s\"\n", str);
}

// set the device used for file loading etc.
ide_device_t* ata_set_main_dev(uint32_t driveid) {
    dev = (ide_device_t*)kmalloc(sizeof(ide_device_t));
	dev->drive = driveid;
	ata_online = true;

    return dev;
}

bool ata_online = false;

void ata_probe()
{
	if (ide_identify(ATA_PRIMARY, ATA_MASTER)) {
        ata_set_main_dev((ATA_PRIMARY << 1) | ATA_MASTER);
		printf("[ATA] Setup primary/master\n");
	}

	if (ide_identify(ATA_PRIMARY, ATA_SLAVE)) {
        ata_set_main_dev((ATA_PRIMARY << 1) | ATA_SLAVE);
		printf("[ATA] Setup primary/slave\n");
	}

	if (ide_identify(ATA_SECONDARY, ATA_MASTER)) {
        ata_set_main_dev((ATA_SECONDARY << 1) | ATA_MASTER);
		printf("[ATA] Setup secondary/master\n");
	}

	if (ide_identify(ATA_SECONDARY, ATA_SLAVE)) {
        ata_set_main_dev((ATA_SECONDARY << 1) | ATA_SLAVE);
		printf("[ATA] Setup secondary/slave\n");
	}

    if (!ata_online) {
        printf("[ATA] No ATA/IDE drive detected, file storage will not be available.\n");
    }
}

ide_device_t* ata_get_main_dev() {
	return dev;
}

bool ata_init()
{
	printf("[ATA] Checking for ATA/IDE drives\n");
	ide_buf = (uint8_t *)kmalloc(512);
	ata_probe();

	return !(dev == 0);
}

bool CMD_atainit(int argc, char** argv) {

	return true;
}

bool CMD_ata(int argc, char** argv) {
    ata_init();

    return true;
}
