#include <zephyr/drivers/watchdog.h>
#include <zephyr/kernel.h>

#include <zephyr/logging/log.h>
LOG_MODULE_REGISTER(smarthome_watchdog, CONFIG_SMARTHOME_WATCHDOG_LOG_LEVEL);

static const struct device *const wdt = DEVICE_DT_GET(DT_ALIAS(watchdog0));
static int wdt_channel_id = -1;

static void submit_watchdog_work(void);

static void watchdog_work_handler(struct k_work *work)
{
	ARG_UNUSED(work);

	int ret = wdt_feed(wdt, wdt_channel_id);
	if (ret) {
		LOG_ERR("Feed failed: %d", ret);
	} else {
		LOG_DBG("Watchdog fed.");
	}

	submit_watchdog_work();
}
static K_WORK_DELAYABLE_DEFINE(watchdog_work, watchdog_work_handler);

static void submit_watchdog_work(void)
{
	int ret =
		k_work_schedule(&watchdog_work, K_MSEC(CONFIG_SMARTHOME_WATCHDOG_FEED_INTERVAL_MS));
	if (ret < 0) {
		LOG_ERR("Can't schedule work: %d", ret);
	}
}

static int watchdog_init(void)
{
	const struct wdt_timeout_cfg wdt_config = {
		.flags = WDT_FLAG_RESET_SOC,
		.window =
			{
				.min = 0,
				.max = CONFIG_SMARTHOME_WATCHDOG_MAX_WINDOW_MS,
			},
	};
	int ret;

	wdt_channel_id = wdt_install_timeout(wdt, &wdt_config);
	if (wdt_channel_id < 0) {
		LOG_ERR("Watchdog install error");
		return wdt_channel_id;
	}

	ret = wdt_setup(wdt, WDT_OPT_PAUSE_HALTED_BY_DBG);
	if (ret < 0) {
		LOG_ERR("Watchdog setup error");
		return ret;
	}

	submit_watchdog_work();

	return 0;
}
SYS_INIT(watchdog_init, APPLICATION, CONFIG_KERNEL_INIT_PRIORITY_DEVICE);
