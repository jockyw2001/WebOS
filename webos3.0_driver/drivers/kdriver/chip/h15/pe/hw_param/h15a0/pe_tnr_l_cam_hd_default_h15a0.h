/*
	SIC LABORATORY, LG ELECTRONICS INC., SEOUL, KOREA
	Copyright(c) 2013 by LG Electronics Inc.

	This program is free software; you can redistribute it and/or
	modify it under the terms of the GNU General Public License
	version 2 as published by the Free Software Foundation.

	This program is distributed in the hope that it will be useful,
	but WITHOUT ANY WARRANTY; without even the implied warranty of
	MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
	GNU General Public License for more details.
*/

const PE_REG_PARAM_T tnr_l_cam_hd_default_h15a0[] =
{
	/* nd tnr */
	//{0x4110, 0x0000202F},	//PE0_OPMODE
	{0x4140, 0x8049CB91},	//TNR_CTRL_00
	{0x4144, 0x011CF410},	//TNR_CTRL_01
	{0x4148, 0x200C493F},	//TNR_CTRL_02
	{0x414C, 0x00000000},	//TNR_CTRL_03,dbg
	{0x4150, 0x00000000},	//TNR_CTRL_04,dbg
	{0x4154, 0x03FF1900},	//TNR_CTRL_05
	{0x4158, 0xF4A20200},	//TNR_CTRL_06
	{0x415C, 0x00000000},	//TNR_CTRL_07
	{0x4160, 0x39125610},	//TNR_CTRL_08
	{0x4164, 0x01CD2030},	//TNR_CTRL_09
	{0x4168, 0xFFD8D000},	//TNR_CTRL_10
	{0x416C, 0xC4B01113},	//TNR_CTRL_11
	//{0x4170, 0x00000000},	//TNR_CTRL_12,rsv
	//{0x4174, 0x00000000},	//TNR_CTRL_13,rsv
	//{0x4178, 0x00000000},	//TNR_CTRL_14,rsv
	//{0x417C, 0x00000000},	//TNR_CTRL_15,rsv
	{0x4180, 0x46687F68},	//TNR_CTRL_16
	{0x4184, 0x00F000B4},	//TNR_CTRL_17
	{0x4188, 0x9A939139},	//TNR_CTRL_18
	{0x418C, 0x91000000},	//TNR_CTRL_19,rsv(hidden)
	{0x4190, 0x20101008},	//TNR_CTRL_20
	{0x4194, 0x00000002},	//TNR_CTRL_21
	//{0x4198, 0x00000000},	//TNR_CTRL_22,rsv
	//{0x419C, 0x00000000},	//TNR_CTRL_23,rsv
	{0x41A0, 0xD07A29E3},	//TNR_CTRL_24
	{0x41A4, 0x2000D826},	//TNR_CTRL_25
	{0x41A8, 0x2000D820},	//TNR_CTRL_26,20140805,[0]c_blur_en:1->0,20141001,[0]0->1,20141021,[0]1->0
	{0x41AC, 0x10009CA2},	//TNR_CTRL_27,rsv(hidden)
	{0x41B0, 0xF0800308},	//TNR_CTRL_28
	{0x41B4, 0x1458FF03},	//TNR_CTRL_29
	{0x41B8, 0x0020FF11},	//TNR_CTRL_30
	{0x41BC, 0x18E44438},	//TNR_CTRL_31
	//{0x41C0, 0x00000000},	//TNR_STATUS_00
	//{0x41C4, 0x00000000},	//TNR_STATUS_01
	//{0x41C8, 0x00000000},	//TNR_STATUS_02
	//{0x41CC, 0x00000000},	//TNR_STATUS_03
	//{0x41D0, 0x00000000},	//TNR_MAIN_LUT_00
	//{0x41D4, 0x00000000},	//TNR_MAIN_LUT_01
	//{0x41D8, 0x00000000},	//RESERVED
	//{0x41DC, 0x00001500},	//TNR_STATUS_04
	{0x41E0, 0x00000900},	//TNR_CTRL_32
	//{0x41E4, 0x00000A23},	//TNR_CTRL_33,rsv
	//{0x41E8, 0x00000000},	//TNR_CTRL_34,rsv
	//{0x41EC, 0x00000000},	//TNR_CTRL_35,rsv
	//{0x41F0, 0x00000000},	//TNR_CTRL_36,rsv
	//{0x41F4, 0x00000000},	//TNR_CTRL_37,rsv
	//{0x41F8, 0x00000000},	//TNR_CTRL_38,rsv
	//{0x41FC, 0x00000000},	//TNR_CTRL_39,rsv
	//{0x43A0, 0x00010001},	//TNR_STATUS_05
	//{0x43A4, 0x00000000},	//TNR_STATUS_06
	//{0x43A8, 0x01010101},	//TNR_STATUS_07
	//{0x43AC, 0x00000000},	//TNR_STATUS_08
	//{0x43C0, 0x00000000},	//TNR_RESET
	{0x43D0, 0x0808FFDF},	//TNR_CTRL_40
	{0x43D4, 0x30108020},	//TNR_CTRL_41
	{0x43D8, 0x50901000},	//TNR_CTRL_42
	{0x43DC, 0x40FF1800},	//TNR_CTRL_43
	{0x43E0, 0xA0FF3C00},	//TNR_CTRL_44
	{0x43E4, 0xC8FF3000},	//TNR_CTRL_45
	{0x43E8, 0x70402000},	//TNR_CTRL_46
	{0x43EC, 0x70FF1800},	//TNR_CTRL_47
	{0x43F0, 0x00000000},	//TNR_CTRL_48
	{0x43F4, 0x04080C80},	//TNR_CTRL_49
	{0x43F8, 0x00000000},	//TNR_CTRL_50
	{0x43FC, 0x00000000},	//TNR_CTRL_51
};