/*
 * hdmi_demo_rx_2.h
 *
 *  Created on: Oct 17, 2012
 *      Author: klabadi
 */

#ifndef HDMI_DEMO_RX_2_H_
#define HDMI_DEMO_RX_2_H_

#define HDMI_INPUT_PORT_A 0
#define HDMI_INPUT_PORT_B 1
#define HDMI_INPUT_PORT_C 2
#define HDMI_INPUT_PORT_D 3
#define TRUE 1
#define FALSE 0

#include "platform.h"

static volatile unsigned int newValidEdid = 0;
static unsigned char rx_edidNrBlocks_top = 0;
static unsigned char rx_rawEdid_top [] =
{

	// Rx Edid memory limited to 2 blocks
		0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
		0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
		0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
		0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
		0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
		0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
		0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
		0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
		0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
		0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
		0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
		0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
		0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
		0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
		0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
		0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,

		0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
		0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
		0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
		0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
		0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
		0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
		0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
		0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
		0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
		0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
		0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
		0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
		0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
		0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
		0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
		0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00

};



int hdmi_demo_rx_register_callback (int((*top_app_callback)(int((*handler)(void *, void *, void *)), void * param1, void * param2, void * param3)));

int hdmi_demo_rx_unregister_callback();

int hdmi_demo_rx_menu_main(char buf[][TOKEN_SIZE], int , void * );

void* hdmi_demo_rx_entry_point(int *phy);

int hdmi_demo_rx_ctrl_edid_update();

void hdmi_demo_rx_new_edid_fill(unsigned char, unsigned char *);

void hdmi_demo_rx_new_edid_update();

void hdmi_demo_repeater();

void hdmi_rx_change_dcm(unsigned);

void hdmi_demo_rx_print_edid();

int hdmi_demo_rx_check_phy_status();

int hdmi_demo_rx_check_video_mode(int, unsigned *, unsigned *);

void hdmi_demo_repeater_init();

void hdmi_rx_ctrl_fast_switching(unsigned );

void hdmi_rx_bridge_cmd();

void hdmi_rx_demo_fifores();

int hdmi_rx_demo_check_audio_video_change();

void hdmi_demo_repeater_cmd();

void hdmi_rx_demo();

void hdmi_demo_repeater_manual_update();

void hdmi_demo_repeater2();

int hdmi_demo_rx_check_audio_video_mode();

int hdmi_demo_rx_only_check_audio_video_mode();

void hdmi_rx_buttons();

void hdmi_rx_buttons_ro();

void irq_handler(void *params);

//int hdmi_rx_demo_pll(unsigned long tmds);

void hdmi_rx_phy_equalizerManualSet(uint8_t value_ch0, uint8_t value_ch1, uint8_t value_ch2, uint8_t auto_equalize, uint32_t phy_version);

uint8_t hdmi_rx_phy_equalizerSwAutoset(uint32_t setTime, uint32_t phy_version);

uint8_t hdmi_rx_phy_equalizerSw_Stage1(uint32_t setTime, uint32_t errorLimit, uint32_t errorCntEnabled, uint32_t phy_version);

uint8_t hdmi_rx_phy_equalizerSw_Stage2(uint32_t setTime, uint32_t errorLimit);

uint8_t hdmi_rx_phy_equalizerSw_Stage2v2(uint32_t setTime, uint16_t useEqSettings, uint32_t errorCntEnabled, uint32_t phy_version);

uint8_t hdmi_rx_phy_equalizerSw_Stage3(uint32_t setTime, uint32_t errorLimit, uint32_t errorCntEnabled);

uint8_t hdmi_rx_phy_equalizerSw_Stage3v2(uint32_t setTime, uint32_t errorLimit, uint32_t phy_version);

uint8_t hdmi_rx_phy_equalizer_SwWA(uint32_t setTime, uint32_t useEqSettings, uint32_t errorCntEnabled, uint32_t phy_version);

int hdmi_rx_demo_pll(unsigned long tmds);

//void hdmi_demo_rx_check_5v_elliptic();

#endif /* HDMI_DEMO_RX_2_H_ */