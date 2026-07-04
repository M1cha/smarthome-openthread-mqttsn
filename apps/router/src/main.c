/*
 * SPDX-License-Identifier: Apache-2.0
 */

#include <zephyr/kernel.h>
#include <smartmeter/mqttsndev.h>

#include <zephyr/logging/log.h>
LOG_MODULE_REGISTER(router, CONFIG_APP_LOG_LEVEL);

int main(void)
{
	LOG_DBG("Init");
	mqttsndev_init();
	return 0;
}
