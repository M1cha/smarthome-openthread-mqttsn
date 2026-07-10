if (CONFIG_SMARTHOME_MQTTSN_DEVICE)
	zephyr_interface_library_named(SMARTHOME_MQTTSN_DEVICE)
	target_include_directories(SMARTHOME_MQTTSN_DEVICE INTERFACE
		include
	)

	zephyr_library()
	zephyr_library_sources(
		lib.c
		settings.c
	)
	zephyr_library_link_libraries(SMARTHOME_MQTTSN_DEVICE)
endif()
