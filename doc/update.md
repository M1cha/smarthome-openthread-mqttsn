# Update

All devices have an UDP SMP server running, which you can talk to using
[smpmgr](https://github.com/intercreate/smpmgr):

```bash
smpmgr --ip IPV6_ADDRESS image state-read
```

If the device is a sleepy device, like powermeter, then you should temporarily
increase the polling rate, especially before starting an update:

```bash
smpmgr --ip IPV6_ADDRESS --timeout 61 shell 'mqttsndev thread_pollperiod 100'
```

# Do the actual update

First, upload the new image. You might have to retry this a couple of times,
because smpmgr currently does not support retries. It will continue where it
left off though:

```bash
smpmgr --ip IPV6_ADDRESS --mtu 1024 image upload --slot 1 --format mcuboot build/zephyr/zephyr.signed.bin
```

Then you have to obtain the hash of the new image:

```bash
smpmgr --ip IPV6_ADDRESS image state-read
```

Then you have to stage the update and reboot:

```bash
smpmgr --ip IPV6_ADDRESS image state-write IMAGE_HASH --confirm
smpmgr --ip IPV6_ADDRESS os reset
```

The reboot will take some time, because the bootloader has to swap the data
between slots 0 and 1. After the device is back online, confirm the new image
to make the update permanent:

```bash
smpmgr --ip IPV6_ADDRESS --timeout 61 image state-write IMAGE_HASH --confirm
```
