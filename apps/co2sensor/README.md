# CO2 sensor

This is an nRF52840 Dongle connected to a [Senseair S88
Residential](https://senseair.com/product/s88/) for measuring CO2 levels.

## Build

MCUboot:
```bash
west build -S smartmeter-nrf52dongle-nouart -b nrf52840dongle/nrf52840 -d build-mcuboot bootloader/mcuboot/boot/zephyr
```

Firmware:
```bash
west build -S smartmeter-thread-device -b nrf52840dongle/nrf52840 smarthome-openthread-mqttsn/apps/co2sensor
```

## Flash

MCUboot:
```bash
./smarthome-openthread-mqttsn/scripts/nrfutil \
    pkg generate --hw-version 52 --sd-req=0x00 \
    --application build-mcuboot/zephyr/zephyr.hex \
    --application-version 1 mcuboot.zip

./smarthome-openthread-mqttsn/scripts/nrfutil \
    dfu usb-serial -pkg mcuboot.zip -p /dev/ttyACM0
```

Firmware:

> [!IMPORTANT]
> I did this with [smpmgr](https://github.com/intercreate/smpmgr) and will
> document it properly the next time I use it.

## Configure

- [Add device to network](../../doc/add-device-to-network.md).
- [Configure a new device](../../modules/mqttsndev/README.md#configure-a-new-device).
- [Update](../../doc/update.md).
