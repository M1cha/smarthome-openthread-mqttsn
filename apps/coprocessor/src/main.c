/* main.c - OpenThread NCP */

/*
 * Copyright (c) 2020 Tridonic GmbH & Co KG
 *
 * SPDX-License-Identifier: Apache-2.0
 */

#include <zephyr/logging/log.h>
LOG_MODULE_REGISTER(ot_br, LOG_LEVEL_DBG);

#define APP_BANNER "***** OpenThread RCP on Zephyr %s *****"

uint8_t *usb_update_sn_string_descriptor(void)
{
	static char sn[] = "3804D0A373C4BD19";
	return sn;
}

int main(void)
{
	LOG_INF(APP_BANNER, CONFIG_NET_SAMPLE_APPLICATION_VERSION);
	return 0;
}
