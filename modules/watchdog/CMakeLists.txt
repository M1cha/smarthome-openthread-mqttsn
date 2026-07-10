if (CONFIG_SMARTHOME_WATCHDOG)
	zephyr_library()
	zephyr_library_sources(
		lib.c
	)
endif()
