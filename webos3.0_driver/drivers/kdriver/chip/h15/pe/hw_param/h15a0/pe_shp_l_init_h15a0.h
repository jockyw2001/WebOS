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

const PE_REG_PARAM_T shp_l_init_h15a0[] =
{
	/* pe1 shp */
	{0xD4B0, 0x07030109},	//PE1_DERH_CTRL_00
	{0xD4B4, 0x00004100},	//PE1_DERH_CTRL_01
	{0xD4B8, 0x0801F808},	//PE1_DERH_CTRL_02
	{0xD4BC, 0xC80C0000},	//PE1_DERH_CTRL_03
	{0xD4C0, 0x0000FF01},	//PE1_DERH_CTRL_04
	{0xD4C4, 0x1E100401},	//PE1_DERH_CTRL_05
	{0xD4C8, 0x850C0C00},	//PE1_DERH_CTRL_06
	{0xD4CC, 0x60000200},	//PE1_DERH_CTRL_07
	{0xD4D0, 0x30FF1010},	//PE1_DERH_CTRL_08
	{0xD4D4, 0x00004020},	//PE1_DERH_CTRL_09
	{0xD4D8, 0x07101000},	//PE1_DERH_CTRL_0A
	{0xD4DC, 0x05034020},	//PE1_DERH_CTRL_0B
	{0xD4E0, 0x00002028},	//PE1_DERH_CTRL_0C
	{0xD4E4, 0x00000000},	//PE1_CORING_CTRL_10
	{0xD4F0, 0x0006500B},	//PE1_DERV_CTRL_0
	{0xD4F4, 0x00000008},	//PE1_DERV_CTRL_1
	{0xD4F8, 0x44280000},	//PE1_DERV_CTRL_2
	{0xD4FC, 0x00003838},	//PE1_DERV_CTRL_3
	{0xD510, 0x00000000},	//PE1_DP_CTRL_00
	{0xD514, 0x28281020},	//PE1_DP_CTRL_01
	{0xD518, 0x40401020},	//PE1_DP_CTRL_02
	{0xD51C, 0x00000000},	//PE1_DP_CTRL_03
	{0xD530, 0x07000001},	//PE1_SP_CTRL_00
	{0xD534, 0x18282828},	//PE1_SP_CTRL_01
	{0xD538, 0x20015024},	//PE1_SP_CTRL_02
	{0xD53c, 0x00000000},	//PE1_SP_CTRL_03
	{0xD540, 0x01401000},	//PE1_SP_CTRL_04
	{0xD544, 0x00000000},	//PE1_SP_CTRL_05
	{0xD548, 0x00001000},	//PE1_SP_CTRL_06
	{0xD54C, 0x000C0C01},	//PE1_SP_CTRL_07
	{0xD550, 0x00C81919},	//PE1_SP_CTRL_08
	{0xD554, 0xD8D06040},	//PE1_SP_CTRL_09
	{0xD558, 0x38FFFFFF},	//PE1_SP_CTRL_0A
	{0xD55C, 0xFFD8D048},	//PE1_SP_CTRL_0B
	{0xD560, 0x0000FFFF},	//PE1_SP_CTRL_0C
	{0xD570, 0x07000001},	//PE1_MP_CTRL_00
	{0xD574, 0x18202828},	//PE1_MP_CTRL_01
	{0xD578, 0x20013824},	//PE1_MP_CTRL_02
	{0xD57c, 0x00000000},	//PE1_MP_CTRL_03
	{0xD580, 0x01401000},	//PE1_MP_CTRL_04
	{0xD584, 0x00000000},	//PE1_MP_CTRL_05
	{0xD588, 0x00001011},	//PE1_MP_CTRL_06
	{0xD58C, 0x000C0C01},	//PE1_MP_CTRL_07
	{0xD590, 0x00B01919},	//PE1_MP_CTRL_08
	{0xD594, 0xD8D06040},	//PE1_MP_CTRL_09
	{0xD598, 0x38FFFFFF},	//PE1_MP_CTRL_0A
	{0xD59C, 0xFFD8D049},	//PE1_MP_CTRL_0B
	{0xD5A0, 0x0000FFFF},	//PE1_MP_CTRL_0C
	{0xD5B0, 0x0202FF1C},	//PE1_CORING_CTRL_00
	{0xD5B4, 0x20302818},	//PE1_CORING_CTRL_01
	{0xD5B8, 0x50FF1800},	//PE1_CORING_CTRL_02
	{0xD5BC, 0x48580010},	//PE1_CORING_CTRL_03
	{0xD5C0, 0x50FF1000},	//PE1_CORING_CTRL_04
	{0xD5C4, 0x58FF0800},	//PE1_CORING_CTRL_05
	{0xD5C8, 0xA0900000},	//PE1_CORING_CTRL_06
	{0xD5CC, 0x60602000},	//PE1_CORING_CTRL_07
	{0xD5D0, 0x301001FF},	//PE1_CORING_CTRL_08
	{0xD5D4, 0x28281020},	//PE1_CORING_CTRL_09
	{0xD600, 0x20208001},	//PE1_PTI_CTRL_00
	{0xD604, 0x50208015},	//PE1_PTI_CTRL_01
	{0xD608, 0x10308015},	//PE1_STI_CTRL_00
	{0xD60C, 0x00000000},	//PE1_STI_CTRL_01
	{0xD620, 0xB0500165},	//PE1_DJ_CTRL_00
	{0xD624, 0x02804000},	//PE1_DJ_CTRL_01
	{0xD628, 0x00001408},	//PE1_DJ_CTRL_02
	{0xD62C, 0x100B1C40},	//PE1_DJ_CTRL_03
	{0xD630, 0x00004051},	//PE1_CTI_CTRL_00
	{0xD634, 0x00011810},	//PE1_CTI_CTRL_01
	{0xD638, 0x00880852},	//PE1_CTI_CTRL_02
	{0xD650, 0x00000000},   //PE1_DEBUG_CTRL_00
	{0xD654, 0x00000000},   //PE1_DEBUG_CTRL_01
	{0xD658, 0x00000000},   //PE1_DEBUG_CTRL_02
	{0xD6A0, 0x00000000},	//PE1_TGEN_CTRL_00
	{0xD6A4, 0x04400810},	//PE1_TGEN_CTRL_01
	{0xD6A8, 0x00000514},	//PE1_TGEN_CTRL_02
	{0xD6AC, 0x48AB6D1E},	//PE1_TGEN_CTRL_03
	{0xD6B0, 0x01010C02},	//PE1_TGEN_CTRL_04
	{0xD6B4, 0x0F0F0050},	//PE1_TGEN_CTRL_05
	{0xD6B8, 0x001A1005},	//PE1_TGEN_CTRL_06
	{0xD6BC, 0x001A1005},	//PE1_TGEN_CTRL_07
	{0xD6C0, 0x3CFF0E1F},	//PE1_TGEN_CTRL_08
	{0xD6C4, 0x00000000}	//PE1_TGEN_CTRL_09
};