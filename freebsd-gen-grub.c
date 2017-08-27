/* vim: ts=2:sw=2 */
#define _GNU_SOURCE   
#include <stdio.h>
#include <string.h>
#include <unistd.h>
#include <parted/parted.h>

int		debug	= 0;
#define debugf(f,...)		({ debug && fprintf(stderr, "DEBUG: %s,%s(),%04d: " f, __FILE__,__func__,__LINE__, ##__VA_ARGS__); })

char *fbsd_dev_name[] = {
	[PED_DEVICE_UNKNOWN]	= "unknown",
	[PED_DEVICE_SCSI]			= "da",				/* SCSI */
	[PED_DEVICE_IDE]			= "ada",			/* ATA */
	[PED_DEVICE_DAC960]		= "xxxraid", 	/* SCSI RAID - depending on driver should be aacd, mlxd, myld, amrd, idad, or twed */
	[PED_DEVICE_CPQARRAY]	= "xxx",
	[PED_DEVICE_FILE]			= "xxx",
	[PED_DEVICE_ATARAID]	= "xxx",
	[PED_DEVICE_I2O]			= "xxx", 
	[PED_DEVICE_UBD]			=	"xxx",
	[PED_DEVICE_DASD]			= "xxx",
	[PED_DEVICE_VIODASD]	= "xxx",
	[PED_DEVICE_SX8]			=	"xxx" 
};

struct menuentry_callback_type {
	char *fsname;
	int (*generator)(PedDevice *, PedDisk *, PedPartition *);
};

int
freebsd_menuentry(PedDevice *dev, PedDisk *disk __attribute__ ((unused)), PedPartition *part)
{
	static int entry = 1;
	int devtype = dev->type;
	/* If the device mode has ATA or IDE in it, we assume it is really a PED_DEVICE_IDE */
	if ((strcasestr(dev->model, "ide") != NULL) || (strcasestr(dev->model, "ata") != NULL))
		devtype = PED_DEVICE_IDE;

	char *devname = fbsd_dev_name[devtype];

	printf("\
menuentry \"FreeBSD Installation #%d\" --class freebsd --class bsd --class os {\n\
	insmod ufs2\n\
	insmod bsd\n\
	set root=(hd%d,%d)\n\
	kfreebsd /boot/kernel/kernel\n\
	kfreebsd_loadenv /boot/device.hints\n\
	set kFreeBSD.vfs.root.mountfrom=ufs:/dev/%s%ds%da\n\
	set kFreeBSD.vfs.root.mountfrom.options=rw\n\
}\n",
		entry,
		dev->did, part->num,
		devname, dev->did, part->num);

	return ++entry;
}

int
main(int argc, char *argv[])
{

	int arg;
	while ((arg = getopt(argc, argv, "d")) != -1) {
		switch (arg) {
			case 'd':
				debug++;
				break;
		}
	}

	struct menuentry_callback_type menuentry_callbacks[] = {
		{ "freebsd-ufs", freebsd_menuentry },
		{ NULL, NULL }
	};

	ped_device_probe_all();

	for (PedDevice *dev = ped_device_get_next(NULL); dev != NULL; dev = ped_device_get_next(dev)) {
		PedDisk *disk = ped_disk_new(dev);
		for (PedPartition *part = ped_disk_next_partition(disk,NULL); part != NULL; part = ped_disk_next_partition(disk,part)) {

			debugf("%s (%u) %5d %-20.20s -  %s - %s - %d - %d\n",
				dev->path,
				dev->host,
				part->num > 0 ? part->num : 0,
				part->fs_type != NULL ? part->fs_type->name : "-" ,
				dev->model,
				ped_partition_is_active(part) && ped_partition_get_flag(part, PED_PARTITION_BOOT) ? "BOOTABLE" : "",
				dev->type,
				PED_DEVICE_SCSI);

			if (part->fs_type != NULL && part->fs_type->name != NULL && ped_partition_get_flag(part,PED_PARTITION_BOOT)) {
				for (struct menuentry_callback_type *mecb = menuentry_callbacks; mecb->fsname != NULL; mecb++) {
					if (strcmp(mecb->fsname, part->fs_type->name) == 0) {
						mecb->generator(dev, disk, part);
						break;
					}
				}
			}
		}
	}

	exit(0);
}
