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

const PE_REG_PARAM_T tnr_l_lut_hd_default_h15a0[] =
{
	/* nd tnr lut */
	{0x41D4, 0xFF20C020},	//TNR_MAIN_LUT_01, 00. x7y7 x6y6 reg_p2_blend_ctrl_pt
	{0x41D4, 0xA0208020},	//TNR_MAIN_LUT_01, 01. x5y5 x4y4 reg_p2_blend_ctrl_pt
	{0x41D4, 0x60204020},	//TNR_MAIN_LUT_01, 02. x3y3 x2y2 reg_p2_blend_ctrl_pt
	{0x41D4, 0x1E2000C0},	//TNR_MAIN_LUT_01, 03. x1y1 x0y0 reg_p2_blend_ctrl_pt
	{0x41D4, 0xFF006430},	//TNR_MAIN_LUT_01, 04. x5y5 x4y4 reg_p2_dist_max_pt
	{0x41D4, 0x50303C1D},	//TNR_MAIN_LUT_01, 05. x3y3 x2y2 reg_p2_dist_max_pt
	{0x41D4, 0x281D0000},	//TNR_MAIN_LUT_01, 06. x1y1 x0y0 reg_p2_dist_max_pt
	{0x41D4, 0x60004B10},	//TNR_MAIN_LUT_01, 07. x7y7 x6y6 reg_detail_iir_pt
	{0x41D4, 0x3C502C60},	//TNR_MAIN_LUT_01, 08. x5y5 x4y4 reg_detail_iir_pt
	{0x41D4, 0x14900AB0},	//TNR_MAIN_LUT_01, 09. x3y3 x2y2 reg_detail_iir_pt
	{0x41D4, 0x04B803B8},	//TNR_MAIN_LUT_01, 10. x1y1 x0y0 reg_detail_iir_pt
	{0x41D4, 0x80006004},	//TNR_MAIN_LUT_01, 11. x7y7 x6y6 reg_detail_cir_pt
	{0x41D4, 0x48083818},	//TNR_MAIN_LUT_01, 12. x5y5 x4y4 reg_detail_cir_pt
	{0x41D4, 0x203A1070},	//TNR_MAIN_LUT_01, 13. x3y3 x2y2 reg_detail_cir_pt
	{0x41D4, 0x089004B0},	//TNR_MAIN_LUT_01, 14. x1y1 x0y0 reg_detail_cir_pt
	{0x41D4, 0x60004B10},	//TNR_MAIN_LUT_01, 15. x7y7 x6y6 reg_flat_iir_pt
	{0x41D4, 0x3C502C60},	//TNR_MAIN_LUT_01, 16. x5y5 x4y4 reg_flat_iir_pt
	{0x41D4, 0x14900AB0},	//TNR_MAIN_LUT_01, 17. x3y3 x2y2 reg_flat_iir_pt
	{0x41D4, 0x04B803B8},	//TNR_MAIN_LUT_01, 18. x1y1 x0y0 reg_flat_iir_pt
	{0x41D4, 0x80006004},	//TNR_MAIN_LUT_01, 19. x7y7 x6y6 reg_flat_cir_pt
	{0x41D4, 0x48083818},	//TNR_MAIN_LUT_01, 20. x5y5 x4y4 reg_flat_cir_pt
	{0x41D4, 0x203A1060},	//TNR_MAIN_LUT_01, 21. x3y3 x2y2 reg_flat_cir_pt
	{0x41D4, 0x089004B0},	//TNR_MAIN_LUT_01, 22. x1y1 x0y0 reg_flat_cir_pt
	{0x41D4, 0xFF802098},	//TNR_MAIN_LUT_01, 23. x1y1 x0y0 reg_luma_gain_ctrl
	{0x41D4, 0xF0E07040},	//TNR_MAIN_LUT_01, 24. x3x2 x1x0 reg_skin_gain_ctrl_yy
	{0x41D4, 0x80786860},	//TNR_MAIN_LUT_01, 25. x3x2 x1x0 reg_skin_gain_ctrl_cb
	{0x41D4, 0x98928E88},	//TNR_MAIN_LUT_01, 26. x3x2 x1x0 reg_skin_gain_ctrl_cr
	{0x41D4, 0x30800810},	//TNR_MAIN_LUT_01, 27. x1y1 x0y0 reg_skin_motion_ctrl
	{0x41D4, 0x2B290049},	//TNR_MAIN_LUT_01, 28. reg_detail_luma_ctrl
	{0x41D4, 0x80003000},	//TNR_MAIN_LUT_01, 29. x3y3 x2y2 reg_NLM_Protect
	{0x41D4, 0x300016ff},	//TNR_MAIN_LUT_01, 30. x1y1 x0y0 reg_NLM_Protect
	{0x41D4, 0xFF006430},	//TNR_MAIN_LUT_01, 31. x5y5 x4y4 reg_cc_dist_max_pt
	{0x41D4, 0x50303C1D},	//TNR_MAIN_LUT_01, 32. x3y3 x2y2 reg_cc_dist_max_pt
	{0x41D4, 0x281D0000},	//TNR_MAIN_LUT_01, 33. x1y1 x0y0 reg_cc_dist_max_pt
	{0x41D4, 0xFF30FF30},	//TNR_MAIN_LUT_01, 34. x5y5 x4y4 reg_ne_dist_max_pt
	{0x41D4, 0xFF30FF30},	//TNR_MAIN_LUT_01, 35. x3y3 x2y2 reg_ne_dist_max_pt
	{0x41D4, 0xFF300030},	//TNR_MAIN_LUT_01, 36. x1y1 x0y0 reg_ne_dist_max_pt
	{0x41D4, 0x00000000},	//TNR_MAIN_LUT_01, 37. reserved
	{0x41D4, 0x00000000},	//TNR_MAIN_LUT_01, 38. reserved
	{0x41D4, 0x80006004},	//TNR_MAIN_LUT_01, 39. x7y7 x6y6 reg_simple_ma_iir_pt
	{0x41D4, 0x48083818},	//TNR_MAIN_LUT_01, 40. x5y5 x4y4 reg_simple_ma_iir_pt
	{0x41D4, 0x20301060},	//TNR_MAIN_LUT_01, 41. x3y3 x2y2 reg_simple_ma_iir_pt
	{0x41D4, 0x089004B0}	//TNR_MAIN_LUT_01, 42. x1y1 x0y0 reg_simple_ma_iir_pt
};