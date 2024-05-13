/*
 * Copyright (c) 2024 Nordic Semiconductor ASA
 *
 * SPDX-License-Identifier: LicenseRef-Nordic-5-Clause
 */
#include <zephyr/kernel.h>
#include <dk_buttons_and_leds.h>
#include <zephyr/net/net_mgmt.h>
#include <zephyr/net/net_event.h>
#include <zephyr/net/wifi_mgmt.h>
#include <net/wifi_mgmt_ext.h>
#include "nfc_wifi_prov.h"
#include "udp_client.h"
LOG_MODULE_REGISTER(wifi_prov_over_nfc, LOG_LEVEL_INF);

#define L4_EVENT_MASK (NET_EVENT_L4_CONNECTED | NET_EVENT_L4_DISCONNECTED)

static struct net_mgmt_event_callback mgmt_cb;

static K_SEM_DEFINE(run_wifi_app, 0, 1);

static void net_mgmt_event_handler(struct net_mgmt_event_callback *cb, uint32_t mgmt_event,
				   struct net_if *iface)
{
	switch (mgmt_event) {
	case NET_EVENT_L4_CONNECTED:
		LOG_INF("Network connected");
		dk_set_led_on(DK_LED1);
		k_sem_give(&run_wifi_app);
		break;
	case NET_EVENT_L4_DISCONNECTED:
		dk_set_led_off(DK_LED1);
		LOG_INF("Network disconnected");
		break;
	default:
		LOG_DBG("Unknown event: 0x%08X", mgmt_event);
		return;
	}
}

int main(void)
{

	if (dk_leds_init() != 0) {
		LOG_ERR("Failed to initialize the LED library");
	}

	/* Sleep to allow initialization of Wi-Fi driver */
	k_sleep(K_SECONDS(1));

	net_mgmt_init_event_callback(&mgmt_cb, net_mgmt_event_handler, L4_EVENT_MASK);
	net_mgmt_add_event_callback(&mgmt_cb);

	if (nfc_provision() < 0) {
		LOG_ERR("Failed to start NFC provisioning");
	}
	// Apply already stored credentials, if any exists
	struct net_if *iface = net_if_get_first_by_type(&NET_L2_GET_NAME(ETHERNET));
	net_mgmt(NET_REQUEST_WIFI_CONNECT_STORED, iface, NULL, 0);

	k_sem_take(&run_wifi_app, K_FOREVER);
	// Add your Wi-Fi application after this point
	start_udp_client();
	return 0;
}