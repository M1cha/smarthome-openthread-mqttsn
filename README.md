# Smart Home: OpenThread+MQTT-SN

I've built a couple smart home devices (mostly sensors) myself and integrated
them into Home Assistant using MQTT-SN via OpenThread. The firmware for these
devices is based on the
[Zephyr RTOS](https://github.com/zephyrproject-rtos/zephyr)
and built from this repository.

## Supported Devices

Network devices:
- [coprocessor](apps/coprocessor/README.md)
- [router](apps/router/README.md)

Smart devices:
- [co2sensor](apps/co2sensor/README.md)
- [pms5003](apps/pms5003/README.md)
- [powermeter](apps/powermeter/README.md)

## FAQ

### Why MQTT-SN instead of Zigbee or Matter?

MQTT-SN can be bridged to an MQTT server using e.g.
[emqx](https://github.com/emqx/emqx), which in turn allows to integrate
[anything](https://www.home-assistant.io/integrations/?search=mqtt)
you can imagine into Home Assistant. With Zigbee or Matter, you either have to
conform to specified services, or use custom ones which don't integrate that
well.

### Why OpenThread?

OpenThread simply enables the use of MQTT-SN because it provides an IP stack for
low power devices like the nRF52840.

### Why MQTT-SN instead of MQTT?

MQTT-SN works over UDP and needs very few packets exchanged, which is great for
low-power or low-bandwidth networks like OpenThread.

### Why MQTT-SN instead of CoAP/LwM2M?

MQTT is supported by Home Assistant. While emqx also supports bridging CoAP and
LwM2M devices to MQTT, using a variant of MQTT instead of a completely
different protocol seems more reasonable to me.

Also, based on the Zephyr implementations, I wasn't happy with the size and
complexity of the code implementing the other protocols.

### How does this compare to Matter?

While Matter is also based on OpenThread, it has much more complexity. From
what I can tell, most of what it does boils down to two things: Adding
application-layer security to protect against other devices in the
network and simplifying the process of adding new devices.

Since I only have self-built open source devices in the OpenThread network, I
trust all of them and don't see the need to add additional encryption or
authentication on top of the network key.

I also don't need adding new devices to be easy for non-technical users,
because I'm a technical user and I don't intend to sell these devices.

## Further reading for developers

Inside this repository:
- [mqttsndev](modules/mqttsndev/README.md)
- [rust module](modules/rust/README.md)

Configuration files on my homeserver:
- [emqx container](https://github.com/M1cha/homeserver/blob/main/config/etc/containers/systemd/emqx.container)
- [emqx config](https://github.com/M1cha/homeserver/blob/main/config/usr/local/share/emqx/emqx.conf)
- [otbr container](https://github.com/M1cha/homeserver/blob/main/config/etc/containers/systemd/otbr.container)
- [otbt start script](https://github.com/M1cha/homeserver/blob/main/config/usr/local/share/otbr/run)

Online:
- [OpenThread](https://openthread.io/)

# Requirements

## powermeter

- `rustup`: so you can install the nightly version required by this project
- `cbindgen`: The CLI, for generating C bindings to the SML library
- rust-src for the currently used toolchain. E.g.
  `rustup component add rust-src --toolchain nightly-2023-06-01-x86_64-unknown-linux-gnu`
- `poppler`: Provides `pdftotext` which is used to convert the SML specification to code
- [Zephyr RTOS](https://docs.zephyrproject.org/3.1.0/develop/getting_started/index.html) dependencies
- Currently, the build system builds the rust part for `thumbv6m-none-eabi`
  but it should be easy to extend if more platforms are needed
