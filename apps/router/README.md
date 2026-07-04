# Router

This is an OpenThread device which joins the network as a router without
providing any MQTT-SN functionality itself. It is used to increase the range of
the network.

## Build

MCUboot:
```bash
west build -S smartmeter-nrf52dk-nouart -b nrf52840dk/nrf52840 -d build-mcuboot bootloader/mcuboot/boot/zephyr
```

Firmware:
```bash
west build -S smartmeter-thread-device -S smartmeter-nrf52dk-nouart -b nrf52840dk/nrf52840 smarthome-openthread-mqttsn/apps/router
```

## Flash

MCUboot:
```bash
west flash -d build-mcuboot -r openocd
```

Firmware:
```bash
west flash -r openocd
```

## Configure

See [Add device to network](../../doc/add-device-to-network.md).
