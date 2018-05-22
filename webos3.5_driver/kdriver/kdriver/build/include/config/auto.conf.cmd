deps_config := \
	../mxlib/utopia_core/Kconfig \
	../mxlib/api/xc/Kconfig \
	../mxlib/api/vdec_v3/Kconfig \
	../mxlib/api/mbx/Kconfig \
	../mxlib/api/swi2c/Kconfig \
	../mxlib/api/pnl/Kconfig \
	../mxlib/api/jpeg_ex/Kconfig \
	../mxlib/api/mhl/Kconfig \
	../mxlib/api/mfe_ex/Kconfig \
	../mxlib/api/mfe/Kconfig \
	../mxlib/api/hsl/Kconfig \
	../mxlib/api/hdmitx2/Kconfig \
	../mxlib/api/hdmitx/Kconfig \
	../mxlib/api/gpd/Kconfig \
	../mxlib/api/gopscd/Kconfig \
	../mxlib/api/gopsc/Kconfig \
	../mxlib/api/gop/Kconfig \
	../mxlib/api/gfx/Kconfig \
	../mxlib/api/dmx/Kconfig \
	../mxlib/api/dlc/Kconfig \
	../mxlib/api/dac/Kconfig \
	../mxlib/api/cec/Kconfig \
	../mxlib/api/audio/Kconfig \
	../mxlib/api/acp/Kconfig \
	../mxlib/api/ace/Kconfig \
	../mxlib/api/Kconfig \
	../mxlib/drv/seal/Kconfig \
	../mxlib/drv/xc/Kconfig \
	../mxlib/drv/wdt/Kconfig \
	../mxlib/drv/wble/Kconfig \
	../mxlib/drv/vif/Kconfig \
	../mxlib/drv/ve/Kconfig \
	../mxlib/drv/vbi/Kconfig \
	../mxlib/drv/usbhost/source4/usb_host_p4/Kconfig \
	../mxlib/drv/usbhost/source4/usb_hid_p4/Kconfig \
	../mxlib/drv/usbhost/source3/usb_host_p3/Kconfig \
	../mxlib/drv/usbhost/source3/usb_hid_p3/Kconfig \
	../mxlib/drv/usbhost/source2/usb_host_p2/Kconfig \
	../mxlib/drv/usbhost/source2/usb_hid_p2/Kconfig \
	../mxlib/drv/usbhost/source/usb_host_p1/Kconfig \
	../mxlib/drv/usbhost/source/usb_hid_p1/Kconfig \
	../mxlib/drv/usbhost/Kconfig \
	../mxlib/drv/usb_ecos/usbhost/Kconfig \
	../mxlib/drv/urdma/Kconfig \
	../mxlib/drv/uart/Kconfig \
	../mxlib/drv/mmfi2/Kconfig \
	../mxlib/drv/mmfi/Kconfig \
	../mxlib/drv/tsp3/Kconfig \
	../mxlib/drv/tsp2/Kconfig \
	../mxlib/drv/tsp/Kconfig \
	../mxlib/drv/tso/Kconfig \
	../mxlib/drv/sys/Kconfig \
	../mxlib/drv/stbdc/Kconfig \
	../mxlib/drv/smbx/Kconfig \
	../mxlib/drv/sem/Kconfig \
	../mxlib/drv/sc/Kconfig \
	../mxlib/drv/sar/Kconfig \
	../mxlib/drv/rvd/Kconfig \
	../mxlib/drv/rtc/Kconfig \
	../mxlib/drv/rasp/Kconfig \
	../mxlib/drv/pws/Kconfig \
	../mxlib/drv/pwm/Kconfig \
	../mxlib/drv/pnl/Kconfig \
	../mxlib/drv/pm/Kconfig \
	../mxlib/drv/pcmcia/Kconfig \
	../mxlib/drv/njpd_ex/Kconfig \
	../mxlib/drv/nsk2/Kconfig \
	../mxlib/drv/mspi/Kconfig \
	../mxlib/drv/mvop/Kconfig \
	../mxlib/drv/mvd_v3/Kconfig \
	../mxlib/msfs/Kconfig \
	../mxlib/msos/Kconfig \
	../mxlib/drv/mpif/Kconfig \
	../mxlib/drv/mmio/Kconfig \
	../mxlib/drv/miu/Kconfig \
	../mxlib/drv/minitsp/Kconfig \
	../mxlib/drv/mhl/Kconfig \
	../mxlib/drv/mfe_ex/Kconfig \
	../mxlib/drv/mfe/Kconfig \
	../mxlib/drv/mfc/Kconfig \
	../mxlib/drv/mbx/Kconfig \
	../mxlib/drv/irq/Kconfig \
	../mxlib/drv/ir/Kconfig \
	../mxlib/drv/ipauth/Kconfig \
	../mxlib/drv/hwi2c/Kconfig \
	../mxlib/drv/hvd_v3/Kconfig \
	../mxlib/drv/hdmitx2/Kconfig \
	../mxlib/drv/hdmitx/Kconfig \
	../mxlib/drv/gpio/Kconfig \
	../mxlib/drv/gpd/Kconfig \
	../mxlib/drv/gopsc/Kconfig \
	../mxlib/drv/gop/Kconfig \
	../mxlib/drv/ge/Kconfig \
	../mxlib/drv/flash/serial/Kconfig \
	../mxlib/drv/flash/nor/Kconfig \
	../mxlib/drv/flash/Kconfig \
	../mxlib/drv/dvb_extdemod/Kconfig \
	../mxlib/drv/dscmb2/Kconfig \
	../mxlib/drv/dscmb/Kconfig \
	../mxlib/drv/dmd/Kconfig \
	../mxlib/drv/dlc/Kconfig \
	../mxlib/drv/dip/Kconfig \
	../mxlib/drv/demod/Kconfig \
	../mxlib/drv/ddc2bi/Kconfig \
	../mxlib/drv/dac/Kconfig \
	../mxlib/drv/ch34/Kconfig \
	../mxlib/drv/cpu/Kconfig \
	../mxlib/drv/cmdq/Kconfig \
	../mxlib/drv/cipher/Kconfig \
	../mxlib/drv/cec/Kconfig \
	../mxlib/drv/ca2/Kconfig \
	../mxlib/drv/bdma/Kconfig \
	../mxlib/drv/avd/Kconfig \
	../mxlib/drv/audsp/Kconfig \
	../mxlib/drv/audio/Kconfig \
	../mxlib/drv/aesdma/Kconfig \
	../mxlib/drv/ace/Kconfig \
	../mxlib/drv/Kconfig \
	Kconfig

include/config/auto.conf: \
	$(deps_config)


$(deps_config): ;