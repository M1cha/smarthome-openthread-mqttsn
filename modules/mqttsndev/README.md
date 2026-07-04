# MQTT-SN device library

This library implements the common parts of all of my smart devices, to shrink
the device-specific code as much as possible.

## Features

- Log OpenThread connection events.
- Feed the hardware watchdog.
- Initialize and run the MQTT-SN client. The device-specific code only needs to
  use a simple API for publishing data.
- Provide a shell command and settings storage for the MQTT-SN address, port and
  client ID.

## Configure A New Device

Connect to the Zephyr shell and do the following:

```bash
mqttsndev gateway_ip ...
mqttsndev gateway_port ...
mqttsndev client_id ...
```
