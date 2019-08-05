#! /bin/sh

#source /opt/fsl-imx-x11/3.14.52-1.1.0/environment-setup-cortexa9hf-vfp-neon-poky-linux-gnueabi 
export ARCH=arm
export CROSS_COMPILE=$TARGET_PREFIX
#unset LDFLAGS
#make imx_v7_defconfig
make uImage LOADADDR=0x10008000

#cp arch/arm/boot/zImage /mnt/hgfs/yocto_share/
#cp arch/arm/boot/zImage /TFTP
