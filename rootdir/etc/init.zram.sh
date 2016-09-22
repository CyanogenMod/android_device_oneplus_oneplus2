#!/system/bin/sh

echo 536870912 > /sys/block/zram0/disksize
mkswap /dev/block/zram0
swapon /dev/block/zram0 -p 32758
