/*
 * (C) Copyright 2018 Rockchip Electronics Co., Ltd
 *
 * SPDX-License-Identifier:     GPL-2.0+
 */

#include <common.h>
#include <adc.h>

DECLARE_GLOBAL_DATA_PTR;

#define KEY_DOWN_MIN_VAL        0
#define KEY_DOWN_MAX_VAL        30

int rockchip_dnl_key_pressed(void)
{
	unsigned int key_val;

	if (adc_channel_single_shot("saradc", 1, &key_val)) {
		printf("%s read adc key val failed\n", __func__);
		return false;
	}
	if (adc_channel_single_shot("saradc", 1, &key_val)) {
		printf("%s read adc key val failed\n", __func__);
		return false;
	}

	if ((key_val >= KEY_DOWN_MIN_VAL) && (key_val <= KEY_DOWN_MAX_VAL))
		return true;
	else
		return false;
}
