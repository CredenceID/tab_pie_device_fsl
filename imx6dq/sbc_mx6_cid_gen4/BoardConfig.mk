#
# Product-specific compile-time definitions.
#

IMX_DEVICE_PATH := device/fsl/imx6dq/sbc_mx6_cid_gen4

ifeq ($(PRODUCT_IMX_CAR),true)
include $(IMX_DEVICE_PATH)/build_id_car.mk
else
include $(IMX_DEVICE_PATH)/build_id.mk
endif # PRODUCT_IMX_CAR
include device/fsl/imx6dq/BoardConfigCommon.mk
ifeq ($(PREBUILT_FSL_IMX_CODEC),true)
-include $(FSL_CODEC_PATH)/fsl-codec/fsl-codec.mk
endif

TARGET_USES_64_BIT_BINDER := true

BUILD_TARGET_FS ?= ext4
TARGET_USERIMAGES_USE_EXT4 := true

ifeq ($(PRODUCT_IMX_CAR),true)
TARGET_RECOVERY_FSTAB = $(IMX_DEVICE_PATH)/fstab.freescale.car
else
TARGET_RECOVERY_FSTAB = $(IMX_DEVICE_PATH)/fstab.freescale
endif # PRODUCT_IMX_CAR

# Vendor Interface Manifest
ifeq ($(PRODUCT_IMX_CAR),true)
DEVICE_MANIFEST_FILE := $(IMX_DEVICE_PATH)/manifest_car.xml
else
# Vendor Interface manifest and compatibility
DEVICE_MANIFEST_FILE := $(IMX_DEVICE_PATH)/manifest.xml
DEVICE_MATRIX_FILE := $(IMX_DEVICE_PATH)/compatibility_matrix.xml
endif

TARGET_BOOTLOADER_BOARD_NAME := SBC_MX6_CID_GEN4
PRODUCT_MODEL := credencetab

TARGET_BOOTLOADER_POSTFIX := imx
TARGET_DTB_POSTFIX := -dtb

# UNITE is a virtual device support both atheros and realtek wifi(ar6103 and rtl8723as)
BOARD_WPA_SUPPLICANT_DRIVER := NL80211
BOARD_HOSTAPD_DRIVER 	    := NL80211
BOARD_HOSTAPD_PRIVATE_LIB               := lib_driver_cmd_bcmdhd
BOARD_WPA_SUPPLICANT_PRIVATE_LIB        := lib_driver_cmd_bcmdhd

BOARD_HOSTAPD_PRIVATE_LIB_RTL               := lib_driver_cmd_rtl
BOARD_WPA_SUPPLICANT_PRIVATE_LIB_RTL        := lib_driver_cmd_rtl
BOARD_WPA_SUPPLICANT_PRIVATE_LIB_BCM := lib_driver_cmd_bcmdhd
BOARD_HOSTAPD_PRIVATE_LIB_BCM 	     := lib_driver_cmd_bcmdhd
BOARD_WLAN_VENDOR := AP6212
BOARD_WLAN_DEVICE            := bcmdhd
WPA_SUPPLICANT_VERSION			 := VER_0_8_X
WIFI_DRIVER_FW_PATH_PARAM := "/sys/module/bcmdhd_ap6212/parameters/firmware_path"
WIFI_DRIVER_FW_PATH_STA := "/vendor/etc/firmware/bcm/fw_bcm43438a0.bin"
WIFI_DRIVER_FW_PATH_P2P := "/vendor/etc/firmware/bcm/fw_bcm43438a0_p2p.bin"
WIFI_DRIVER_FW_PATH_AP := "/vendor/etc/firmware/bcm/fw_bcm43438a0_apsta.bin"
WIFI_DRIVER_MODULE_NAME := "bcmdhd_ap6212"
WIFI_DRIVER_MODULE_PATH := "/vendor/lib/modules/bcmdhd_ap6212.ko"
WIFI_DRIVER_MODULE_ARG := "iface_name=wlan firmware_path=/vendor/etc/firmware/bcm/fw_bcm43438a0.bin nvram_path=/vendor/etc/firmware/bcm/ap6212_nvram.txt"

PRODUCT_COPY_FILES +=	\
	device/fsl/imx6dq/sbc_mx6_cid_gen4/firmware/bcm_ap6212/Wi-Fi/fw_bcm43438a0_apsta.bin:vendor/etc/firmware/bcm/fw_bcm43438a0_apsta.bin \
	device/fsl/imx6dq/sbc_mx6_cid_gen4/firmware/bcm_ap6212/Wi-Fi/fw_bcm43438a0_p2p.bin:vendor/etc/firmware/bcm/fw_bcm43438a0_p2p.bin \
	device/fsl/imx6dq/sbc_mx6_cid_gen4/firmware/bcm_ap6212/Wi-Fi/fw_bcm43438a0.bin:vendor/etc/firmware/bcm/fw_bcm43438a0.bin \
	device/fsl/imx6dq/sbc_mx6_cid_gen4/firmware/bcm_ap6212/Wi-Fi/nvram_ap6212.txt:vendor/etc/firmware/bcm/ap6212_nvram.txt \
	device/fsl/imx6dq/sbc_mx6_cid_gen4/firmware/bcm_ap6212/BT/BCM4343A0_001.001.034.0061.0245.hcd:vendor/etc/firmware/bcm/bcm4343a0.hcd

PRODUCT_COPY_FILES +=	\
	device/fsl/imx6dq/sbc_mx6_cid_gen4/wpa_supplicant.conf:vendor/etc/wifi/wpa_supplicant.conf

BOARD_VENDOR_KERNEL_MODULES += \
                            $(KERNEL_OUT)/drivers/net/wireless/bcmdhd_ap6212/bcmdhd_ap6212.ko

#for accelerator sensor, need to define sensor type here
BOARD_HAS_SENSOR := true
SENSOR_MMA8451 := true
BOARD_USE_SENSOR_PEDOMETER := false
BOARD_USE_LEGACY_SENSOR := true

# for recovery service
TARGET_SELECT_KEY := 28

# we don't support sparse image.
TARGET_USERIMAGES_SPARSE_EXT_DISABLED := false
# uncomment below lins if use NAND
#TARGET_USERIMAGES_USE_UBIFS = true


ifeq ($(TARGET_USERIMAGES_USE_UBIFS),true)
UBI_ROOT_INI := $(IMX_DEVICE_PATH)/ubi/ubinize.ini
TARGET_MKUBIFS_ARGS := -m 4096 -e 516096 -c 4096 -x none
TARGET_UBIRAW_ARGS := -m 4096 -p 512KiB $(UBI_ROOT_INI)
endif

ifeq ($(TARGET_USERIMAGES_USE_UBIFS),true)
ifeq ($(TARGET_USERIMAGES_USE_EXT4),true)
$(error "TARGET_USERIMAGES_USE_UBIFS and TARGET_USERIMAGES_USE_EXT4 config open in same time, please only choose one target file system image")
endif
endif

KERNEL_NAME := zImage
BOARD_KERNEL_CMDLINE := console=ttymxc0,115200 init=/init video=mxcfb0:dev=ldb,fbpix=RGB24,bpp=24 video=mxcfb1:off video=mxcfb2:off video=mxcfb3:off vmalloc=128M androidboot.console=ttymxc0 consoleblank=0 androidboot.hardware=freescale cma=320M galcore.contiguousSize=33554432 loop.max_part=7

ifeq ($(TARGET_USERIMAGES_USE_UBIFS),true)
#UBI boot command line.
# Note: this NAND partition table must align with MFGTool's config.
BOARD_KERNEL_CMDLINE +=  mtdparts=gpmi-nand:16m(bootloader),16m(bootimg),128m(recovery),-(root) gpmi_debug_init ubi.mtd=3
endif

BOARD_KERNEL_CMDLINE += androidboot.selinux=permissive
#BOARD_KERNEL_CMDLINE += androidboot.selinux=enforcing

# Broadcom BCM4339 BT
BOARD_HAVE_BLUETOOTH_BCM := true
BOARD_BLUETOOTH_BDROID_BUILDCFG_INCLUDE_DIR := $(IMX_DEVICE_PATH)/bluetooth

USE_ATHR_GPS_HARDWARE := true
USE_QEMU_GPS_HARDWARE := false

USE_ION_ALLOCATOR := false
USE_GPU_ALLOCATOR := true
USE_UBLOX_GPS_HARDWARE := true
USE_ZKW_GPS_HARDWARE := true
CID_GPS_COMPATIBILITY := true
# camera hal v3
IMX_CAMERA_HAL_V3 := true

# define frame buffer count
NUM_FRAMEBUFFER_SURFACE_BUFFERS := 3

# camera hal v3
IMX_CAMERA_HAL_V3 := true


#define consumer IR HAL support
IMX6_CONSUMER_IR_HAL := false

BOARD_PREBUILT_DTBOIMAGE := out/target/product/sbc_mx6_cid_gen4/dtbo-imx6q.img

# u-boot target for imx6q_sabresd 800MHz/10000MHz board
TARGET_BOOTLOADER_CONFIG := imx6q:imx6qsabresdandroid_defconfig
#TARGET_BOOTLOADER_CONFIG := imx6q:sbc_mx6_cid_gen3_defconfig
# u-boot target for imx6dl_sabresd board
TARGET_BOOTLOADER_CONFIG += imx6dl:imx6dlsabresdandroid_defconfig
# u-boot target for imx6q_sabresd 1.2GHz board
TARGET_BOOTLOADER_CONFIG += imx6q-ldo:imx6qsabresdandroid_defconfig
# u-boot target for imx6qp_sabresd 800MHz/10000MHz board
TARGET_BOOTLOADER_CONFIG += imx6qp:imx6qpsabresdandroid_defconfig
# u-boot target for imx6qp_sabresd 1.2GHz board
TARGET_BOOTLOADER_CONFIG += imx6qp-ldo:imx6qpsabresdandroid_defconfig

# imx6q default dts
#TARGET_BOARD_DTS_CONFIG := imx6q:imx6q-sabresd.dtb
TARGET_BOARD_DTS_CONFIG := imx6q:sbc-mx6-cid-gen4.dtb
# imx6dl default dts
TARGET_BOARD_DTS_CONFIG += imx6dl:imx6dl-sabresd.dtb
# imx6q 1.2GHz board
TARGET_BOARD_DTS_CONFIG += imx6q-ldo:imx6q-sabresd-ldo.dtb
# imx6qp default dts
TARGET_BOARD_DTS_CONFIG += imx6qp:imx6qp-sabresd.dtb
# imx6qp 1.2GHz board
TARGET_BOARD_DTS_CONFIG += imx6qp-ldo:imx6qp-sabresd-ldo.dtb
TARGET_KERNEL_DEFCONFIG := imx_v7_android_defconfig
#TARGET_KERNEL_DEFCONFIG := sbc_mx6_cid_gen4_android_defconfig
# TARGET_KERNEL_ADDITION_DEFCONF := imx_v7_android_addition_defconfig

# u-boot target used by uuu for imx6q_sabresd 800MHz/10000MHz board
TARGET_BOOTLOADER_CONFIG += imx6q-sabresd-uuu:mx6qsabresd_defconfig
#TARGET_BOOTLOADER_CONFIG += imx6q-sabresd-uuu:sbc_mx6_cid_gen3_defconfig
# u-boot target used by uuu for imx6dl_sabresd board
TARGET_BOOTLOADER_CONFIG += imx6dl-sabresd-uuu:mx6dlsabresd_defconfig
# u-boot target used by uuu for imx6q_sabresd 1.2GHz board
TARGET_BOOTLOADER_CONFIG += imx6q-ldo-sabresd-uuu:mx6qsabresd_defconfig
# u-boot target used by uuu for imx6qp_sabresd 800MHz/10000MHz board
TARGET_BOOTLOADER_CONFIG += imx6qp-sabresd-uuu:mx6qpsabresd_defconfig
# u-boot target used by uuu for imx6qp_sabresd 1.2GHz board
TARGET_BOOTLOADER_CONFIG += imx6qp-ldo-sabresd-uuu:mx6qpsabresd_defconfig

BOARD_SEPOLICY_DIRS := \
       device/fsl/imx6dq/sepolicy \
       $(IMX_DEVICE_PATH)/sepolicy

ifeq ($(PRODUCT_IMX_CAR),true)
BOARD_SEPOLICY_DIRS += \
     packages/services/Car/car_product/sepolicy \
     device/generic/car/common/sepolicy
endif

# Support gpt
BOARD_BPT_INPUT_FILES += device/fsl/common/partition/device-partitions-7GB.bpt
ADDITION_BPT_PARTITION = partition-table-14GB:device/fsl/common/partition/device-partitions-14GB.bpt \
                         partition-table-28GB:device/fsl/common/partition/device-partitions-28GB.bpt

TARGET_BOARD_KERNEL_HEADERS := device/fsl/common/kernel-headers

#Enable AVB
BOARD_AVB_ENABLE := true
TARGET_USES_MKE2FS := true
BOARD_INCLUDE_RECOVERY_DTBO := true
BOARD_USES_FULL_RECOVERY_IMAGE := true

# u-blox
ifeq ($(USE_UBLOX_GPS_HARDWARE),true)
PRODUCT_COPY_FILES +=	\
	device/fsl/imx6dq/sbc_mx6_cid_gen4/fsl-proprietary/u-blox-Android-GNSS-Driver-v3.30/gps/gps.conf:vendor/etc/gps.conf \
	device/fsl/imx6dq/sbc_mx6_cid_gen4/fsl-proprietary/u-blox-Android-GNSS-Driver-v3.30/gps/u-blox.conf:vendor/etc/u-blox.conf
endif

PRODUCT_COPY_FILES +=  \
    device/fsl/imx6dq/sbc_mx6_cid_gen4/modem/libhuawei-ril.so:vendor/lib/libhuawei-ril.so\
    device/fsl/imx6dq/sbc_mx6_cid_gen4/modem/chat:system/bin/chat\
    device/fsl/imx6dq/sbc_mx6_cid_gen4/modem/ip-up:system/etc/ppp/ip-up\
    device/fsl/imx6dq/sbc_mx6_cid_gen4/modem/ip-down:system/etc/ppp/ip-down\
    device/fsl/imx6dq/sbc_mx6_cid_gen4/modem/libreference-ril.so:vendor/lib/libquectel-ril.so

PRODUCT_COPY_FILES +=  \
    device/fsl/imx6dq/sbc_mx6_cid_gen4/fsl-proprietary/Camera/camera.imx6.so:vendor/lib/hw/camera.imx6.so

# zkw gps
#ifeq ($(USE_ZKW_GPS_HARDWARE),true)
#PRODUCT_COPY_FILES +=  \
#       device/fsl/imx6dq/sbc_mx6_cid_gen4/tools/gps.zkw.so:system/lib/hw/gps.default.so
#endif
