# Power Meter

This reads power and energy consumption data from the optical interface of a
smart power meter and publishes it via MQTT-SN. The optical interface uses
[SML](https://www.bsi.bund.de/SharedDocs/Downloads/DE/BSI/Publikationen/TechnischeRichtlinien/TR03109/TR-03109-1_Anlage_Feinspezifikation_Drahtgebundene_LMN-Schnittstelle_Teilb.pdf;jsessionid=F2323041EE7292926D80680DA407BA3F.internet082?__blob=publicationFile&v=1).
I decided to implement this protocol myself instead of using libsml from the
volkszaehler project, because I didn't like two of their design decisions:
- They use the heap.
- They use a blocking API and thus require a separate thread.

Additionally, the protocol is sufficiently complex that I saw great benefit in
implementing it using async rust, to make the code easier to understand thanks
to the lack of callback-based code and manually written state machines. You can
find the implementation in
[modules/rust/](https://github.com/M1cha/smarthome-openthread-mqttsn/tree/main/modules/rust).

## Build

MCUboot:
```bash
west build -S smartmeter-nrf52dongle-nouart -b nrf52840dongle/nrf52840 -d build-mcuboot bootloader/mcuboot/boot/zephyr
```

Firmware:
```bash
west build -S smartmeter-thread-device -b nrf52840dongle/nrf52840 smarthome-openthread-mqttsn/apps/powermeter
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

- [Add device to network](../../doc/add-device-to-network.md).
- [Configure a new device](../../modules/mqttsndev/README.md#configure-a-new-device).

