# Co-Processor

An OpenThread Co-Processor based on
`zephyr/samples/net/openthread/coprocessor`. This is used by my homeserver
running a [Border Router](https://openthread.io/guides/border-router).

## Build

```bash
west build -b nrf52840dongle/nrf52840 smarthome-openthread-mqttsn/apps/coprocessor
```

## Flash

```bash
./smarthome-openthread-mqttsn/scripts/nrfutil \
    pkg generate --hw-version 52 --sd-req=0x00 \
    --application build/zephyr/zephyr.hex \
    --application-version 1 blinky.zip

./smarthome-openthread-mqttsn/scripts/nrfutil \
    dfu usb-serial -pkg blinky.zip -p /dev/ttyACM0
```

## Configure

- [Add device to network](../../doc/add-device-to-network.md).
- [Configure a new device](../../modules/mqttsndev/README.md#configure-a-new-device).
