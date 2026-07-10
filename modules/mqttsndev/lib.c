#include <smarthome/mqttsndev.h>

#include <errno.h>
#include <zephyr/kernel.h>
#include <zephyr/net/conn_mgr_monitor.h>
#include <zephyr/net/mqtt_sn.h>
#include <zephyr/net/net_mgmt.h>
#include <zephyr/net/socket.h>
#include <zephyr/zvfs/eventfd.h>

#include <zephyr/logging/log.h>
LOG_MODULE_REGISTER(mqttsndev, CONFIG_SMARTHOME_MQTTSN_DEVICE_LOG_LEVEL);

#include "private.h"

#define EVENT_MASK (NET_EVENT_L4_CONNECTED | NET_EVENT_L4_DISCONNECTED)
static struct net_mgmt_event_callback mgmt_cb;
static bool connected;

static bool started;

static struct mqtt_sn_client mqtt_client;
static struct mqtt_sn_transport_udp tp;
static uint8_t tx_buf[CONFIG_SMARTHOME_MQTTSN_DEVICE_BUFFER_SIZE];
static uint8_t rx_buf[CONFIG_SMARTHOME_MQTTSN_DEVICE_BUFFER_SIZE];
static bool mqtt_sn_initialized;
static bool mqtt_sn_connected;
static int64_t mqtt_sn_last_connect;

static mqttsn_publish_callback_t publish_callback;
static bool mqtt_sn_publish_requested;

static struct k_work_poll work_poll;
static bool work_poll_submitted;
static struct k_poll_event poll_events;

static struct k_work_delayable work;

static void submit_socket_poll(void)
{
	int ret;

	if (work_poll_submitted) {
		LOG_DBG("poll is submitted already");
		return;
	}
	if (tp.sock < 0) {
		LOG_DBG("no socket to poll");
		return;
	}

	struct zsock_pollfd pfd = {
		.fd = tp.sock,
		.events = ZSOCK_POLLIN,
	};
	poll_events = (struct k_poll_event){
		.type = K_POLL_TYPE_IGNORE,
	};
	struct k_poll_event *pev = &poll_events;

	ret = zsock_ioctl(tp.sock, ZFD_IOCTL_POLL_PREPARE, &pfd, &pev, pev + 1);
	if (ret < 0) {
		LOG_ERR("failed to prepare poll event: %d", ret);
		return;
	}
	if (poll_events.type == K_POLL_TYPE_IGNORE) {
		LOG_WRN("nothing to poll");
		return;
	}

	ret = k_work_poll_submit(&work_poll, &poll_events, 1, K_FOREVER);
	if (ret < 0) {
		LOG_ERR("Failed to submit poll work: %d", ret);
		return;
	}

	LOG_DBG("submitted fd %d", tp.sock);
	work_poll_submitted = true;
}

static void evt_cb(struct mqtt_sn_client *client, const struct mqtt_sn_evt *evt)
{
	switch (evt->type) {
	case MQTT_SN_EVT_CONNECTED: /* Connected to a gateway */
		LOG_INF("MQTT-SN event EVT_CONNECTED");
		mqtt_sn_connected = true;
		break;
	case MQTT_SN_EVT_DISCONNECTED: /* Disconnected */
		LOG_INF("MQTT-SN event EVT_DISCONNECTED");
		mqtt_sn_connected = false;
		break;
	case MQTT_SN_EVT_ASLEEP: /* Entered ASLEEP state */
		LOG_INF("MQTT-SN event EVT_ASLEEP");
		break;
	case MQTT_SN_EVT_AWAKE: /* Entered AWAKE state */
		LOG_INF("MQTT-SN event EVT_AWAKE");
		break;
	case MQTT_SN_EVT_PUBLISH: /* Received a PUBLISH message */
		LOG_INF("MQTT-SN event EVT_PUBLISH");
		LOG_HEXDUMP_INF(evt->param.publish.data.data, evt->param.publish.data.size,
				"Published data");
		break;
	case MQTT_SN_EVT_PINGRESP: /* Received a PINGRESP */
		LOG_INF("MQTT-SN event EVT_PINGRESP");
		break;
	case MQTT_SN_EVT_ADVERTISE: /* Received a ADVERTISE */
		LOG_INF("MQTT-SN event EVT_ADVERTISE");
		break;
	case MQTT_SN_EVT_GWINFO: /* Received a GWINFO */
		LOG_INF("MQTT-SN event EVT_GWINFO");
		break;
	case MQTT_SN_EVT_SEARCHGW: /* Received a SEARCHGW */
		LOG_INF("MQTT-SN event EVT_SEARCHGW");
		break;
	}

	k_work_reschedule(&work, K_NO_WAIT);
}

static void work_handler(struct k_work *work_)
{
	const int64_t now = k_uptime_get();
	int ret;
	k_timeout_t delay = K_SECONDS(CONFIG_SMARTHOME_MQTTSN_DEVICE_RETRY_WAIT_DURATION);

	ARG_UNUSED(work);

	if (!mqtt_sn_initialized) {
		LOG_INF("Initializing client");

		const struct mqtt_sn_data client_id = {
			.data = mqttsndev_client_id,
			.size = mqttsndev_client_id_length,
		};
		const struct sockaddr_in6 gateway = {
			.sin6_family = AF_INET6,
			.sin6_port = htons(mqttsndev_gateway_port),
			.sin6_addr = mqttsndev_gateway_ip,
		};
		const struct mqtt_sn_data gwaddr_data = {
			.data = (uint8_t *)&gateway,
			.size = sizeof(gateway),
		};

		ret = mqtt_sn_transport_udp_init(&tp, NULL, 0);
		if (ret) {
			LOG_ERR("mqtt_sn_transport_udp_init() failed %d", ret);
			goto out_reschedule;
		}

		ret = mqtt_sn_client_init(&mqtt_client, &client_id, &tp.tp, evt_cb, tx_buf,
					  sizeof(tx_buf), rx_buf, sizeof(rx_buf));
		if (ret) {
			LOG_ERR("mqtt_sn_client_init() failed %d", ret);

			if (tp.tp.deinit) {
				tp.tp.deinit(&tp.tp);
			}

			goto out_reschedule;
		}

		ret = mqtt_sn_add_gw(&mqtt_client, 0, gwaddr_data);
		if (ret) {
			LOG_ERR("mqtt_sn_add_gw() failed %d", ret);
			goto out_deinit;
		}

		mqtt_sn_initialized = true;
	}

	ret = mqtt_sn_input(&mqtt_client);
	if (ret < 0) {
		LOG_ERR("failed: input: %d", ret);
		goto out_deinit;
	}

	if (!mqtt_sn_connected) {
		LOG_INF("Connecting client");

		const int64_t reconnect_wait_s =
			CONFIG_SMARTHOME_MQTTSN_DEVICE_RECONNECT_WAIT_DURATION;
		const int64_t reconnect_wait_ms = reconnect_wait_s * MSEC_PER_SEC;
		if (mqtt_sn_last_connect != 0 && mqtt_sn_last_connect + reconnect_wait_ms > now) {
			delay = K_MSEC(mqtt_sn_last_connect + reconnect_wait_ms - now);
			goto out_reschedule;
		}

		ret = mqtt_sn_connect(&mqtt_client, false, true);
		if (ret != 0 && ret != -EAGAIN && ret != -EWOULDBLOCK && ret != -EINTR) {
			LOG_ERR("mqtt_sn_connect() failed %d", ret);
			goto out_deinit;
		}

		mqtt_sn_last_connect = now;
		delay = K_SECONDS(reconnect_wait_s);
		goto out_reschedule;
	}

	if (publish_callback && mqtt_sn_publish_requested) {
		ret = publish_callback(&mqtt_client);
		if (ret == -EAGAIN || ret == -EWOULDBLOCK || ret == -EINTR) {
			goto out_reschedule;
		}
		if (ret < 0) {
			LOG_ERR("failed: publish_callback: %d", ret);
			goto out_deinit;
		}

		mqtt_sn_publish_requested = false;
	}

	goto out_submit_poll;

out_deinit:
	mqtt_sn_client_deinit(&mqtt_client);
	mqtt_sn_initialized = false;
	mqtt_sn_connected = false;
out_reschedule:
	k_work_reschedule(&work, delay);
out_submit_poll:
	submit_socket_poll();
}
static K_WORK_DELAYABLE_DEFINE(work, work_handler);

static void work_poll_handler(struct k_work *work_)
{
	LOG_INF("work poll");
	work_poll_submitted = false;
	work_handler(&work.work);
}

static void net_event_handler(struct net_mgmt_event_callback *cb, uint64_t mgmt_event,
			      struct net_if *iface)
{
	if ((mgmt_event & EVENT_MASK) != mgmt_event) {
		return;
	}

	if (mgmt_event == NET_EVENT_L4_CONNECTED) {
		LOG_INF("Network connected");

		connected = true;
		if (!started) {
			started = true;
			k_work_reschedule(&work, K_NO_WAIT);
		}

		return;
	}

	if (mgmt_event == NET_EVENT_L4_DISCONNECTED) {
		LOG_INF("Network disconnected");
		connected = false;
		return;
	}
}

int mqttsndev_init(void)
{
	k_work_poll_init(&work_poll, work_poll_handler);

	if (IS_ENABLED(CONFIG_NET_CONNECTION_MANAGER)) {
		net_mgmt_init_event_callback(&mgmt_cb, net_event_handler, EVENT_MASK);
		net_mgmt_add_event_callback(&mgmt_cb);

		conn_mgr_mon_resend_status();
	} else {
		/* If the config library has not been configured to start the
		 * app only after we have a connection, then we can start
		 * it right away.
		 */
		k_work_reschedule(&work, K_NO_WAIT);
	}

	return 0;
}

void mqttsndev_register_publish_callback(mqttsn_publish_callback_t callback)
{
	publish_callback = callback;
}

void mqttsndev_schedule_publish_callback(void)
{
	mqtt_sn_publish_requested = true;
	k_work_reschedule(&work, K_NO_WAIT);
}

__printf_like(5, 6) int mqtt_sn_publish_fmt(struct mqtt_sn_client *client, enum mqtt_sn_qos qos,
					    struct mqtt_sn_data *topic_name, bool retain,
					    const char *fmt, ...)
{
	static char buffer[128];
	int ret;

	va_list ap;
	va_start(ap, fmt);
	ret = vsnprintf(buffer, sizeof(buffer), fmt, ap);
	va_end(ap);

	struct mqtt_sn_data pubdata = {
		.data = buffer,
		.size = MIN(sizeof(buffer), ret),
	};

	ret = mqtt_sn_publish(client, qos, topic_name, retain, &pubdata);
	if (ret) {
		LOG_ERR("failed to publish: %d", ret);
		return ret;
	}

	return 0;
}
