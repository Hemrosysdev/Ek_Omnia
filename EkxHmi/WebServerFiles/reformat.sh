#!/bin/sh

if [ `uname -m` != "armv7l" ];
then
	echo "Reformat rejected - not on EKX target"
	exit 1
fi

#if [ -e /db_storage ];
#then
#	echo "Reformat rejected - new partitions already existing"
#	exit 1
#fi

killall EkxUI
killall EspHostConnector

DISK=/dev/mmcblk1
PARTITION_PREFIX=p
#umount $DISK${PARTITION_PREFIX}5
#umount $DISK${PARTITION_PREFIX}6
umount $DISK${PARTITION_PREFIX}7
umount $DISK${PARTITION_PREFIX}8
umount $DISK${PARTITION_PREFIX}9
umount $DISK${PARTITION_PREFIX}10

sgdisk -d 10 $DISK
sgdisk -d 9 $DISK
sgdisk -d 8 $DISK
sgdisk -d 7 $DISK
#sgdisk -d 6 $DISK
#sgdisk -d 5 $DISK

sync

#sgdisk  -n 5::+250M -t 5:8300 \
#	-n 6::+250M -t 6:8300

sgdisk \
	-n 7::+50M -t 7:8300 \
	-n 8::+20M -t 8:8300 \
	-n 9::+100M -t 9:8300 \
	-n 10::-512K -t 10:8300 \
	-p $DISK

umount ${DISK}${PARTITION_PREFIX}7
umount ${DISK}${PARTITION_PREFIX}8
umount ${DISK}${PARTITION_PREFIX}9
umount ${DISK}${PARTITION_PREFIX}10

mkfs.ext4 -O ^metadata_csum,^64bit ${DISK}${PARTITION_PREFIX}7 -F -L logdata
if [ $? -ne 0 ] ; then
        mkfs.ext4 ${DISK}${PARTITION_PREFIX}7 -F -L logdata  || exit 1
fi
mkfs.ext4 -O ^metadata_csum,^64bit ${DISK}${PARTITION_PREFIX}8 -F -L config
if [ $? -ne 0 ] ; then
        mkfs.ext4 ${DISK}${PARTITION_PREFIX}8 -F -L config  || exit 1
fi
mkfs.ext4 -O ^metadata_csum,^64bit ${DISK}${PARTITION_PREFIX}9 -F -L storage
if [ $? -ne 0 ] ; then
        mkfs.ext4 ${DISK}${PARTITION_PREFIX}9 -F -L storage  || exit 1
fi
mkfs.ext4 -O ^metadata_csum,^64bit ${DISK}${PARTITION_PREFIX}10 -F -L db
if [ $? -ne 0 ] ; then
        mkfs.ext4 ${DISK}${PARTITION_PREFIX}10 -F -L db  || exit 1
fi

/usr/bin/mountdata.sh

chmod 777 /storage
chmod 777 /log
chmod 777 /config
chmod 777 /db_storage

cp /etc/issue /storage/rootfs_version

sync
sleep 3
sync

reboot
