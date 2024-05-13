/*
 * Copyright (c) 2024 Nordic Semiconductor ASA
 *
 * SPDX-License-Identifier: LicenseRef-Nordic-5-Clause
 */
#include <zephyr/kernel.h>
#include <stdbool.h>
#include <nfc_t4t_lib.h>
#include <zephyr/net/net_if.h>
#include <zephyr/net/wifi.h>
#include <zephyr/net/wifi_mgmt.h>
#include <zephyr/logging/log.h>
#include <net/wifi_credentials.h>
#include <net/wifi_mgmt_ext.h>
LOG_MODULE_REGISTER(nfc_prov, LOG_LEVEL_INF);

static struct wifi_credentials_personal creds;
static uint8_t ndef_msg_buf[CONFIG_NDEF_FILE_SIZE]; /**< Buffer for NDEF file. */
static struct net_if *iface;
#define WIFI_PROV_MGMT_EVENTS (NET_EVENT_WIFI_DISCONNECT_RESULT | NET_EVENT_WIFI_CONNECT_RESULT)

static struct net_mgmt_event_callback wifi_prov_mgmt_cb;

/* An experimental function to parse NFC NDEF Wi-Fi record
Assumptions:
Authentication: WPA-2 Personal
Encryption: AES
TODO:
1 - Support other modes
*/
int parse_ndef_wifi_record(size_t payloadLength, struct wifi_credentials_personal *wifiConfig)
{
	int i=0;
	uint8_t *ptr_data = (uint8_t *)ndef_msg_buf;

	// Getting the Authentication and encryption type
	//  TODO: Support other modes and extract from data
	wifiConfig->header.type = WIFI_SECURITY_TYPE_PSK;
	// Getting the SSID
	
	while (ndef_msg_buf[i]!='E'){
		i++;
	}
	ptr_data += i+1;
	wifiConfig->header.ssid_len = (((uint16_t) * (ptr_data)) << 8) | *(ptr_data + 1);
	if (wifiConfig->header.ssid_len > WIFI_SSID_MAX_LEN){
		LOG_ERR("SSID too long");
		return -1;
	}
	memcpy(wifiConfig->header.ssid, ptr_data + 2, wifiConfig->header.ssid_len);
	wifiConfig->header.ssid[wifiConfig->header.ssid_len] = '\0';
	LOG_DBG("SSID: %s SSID len %d\n", wifiConfig->header.ssid, wifiConfig->header.ssid_len);

	// Getting the password
	ptr_data += wifiConfig->header.ssid_len + 16;
	wifiConfig->password_len = (((uint16_t) * (ptr_data)) << 8) | *(ptr_data + 1);
	if (wifiConfig->password_len > WIFI_PSK_MAX_LEN){
		LOG_ERR("Password too long");
		return -1; 
	}
	memcpy(wifiConfig->password, ptr_data + 2, wifiConfig->password_len);
	wifiConfig->password[wifiConfig->password_len] = '\0';
	LOG_DBG("Password: %s Password len %d\n", wifiConfig->password, wifiConfig->password_len);
	return 0;
}

static void wifi_mgmt_event_handler(struct net_mgmt_event_callback *cb, uint32_t mgmt_event,
				    struct net_if *iface)
{
	switch (mgmt_event) {
	case NET_EVENT_WIFI_DISCONNECT_RESULT:
		LOG_INF("Turning on NFC");
		nfc_t4t_emulation_start();
		break;
	case NET_EVENT_WIFI_CONNECT_RESULT:
		// Delete temporary credentials from the application. Only TF-M should have access to
		// credentials
		memset(&creds.password, 0, creds.password_len);
		memset(&creds.password_len, 0, sizeof(unsigned int));
		memset(&creds.header.ssid, 0, creds.header.ssid_len);
		memset(&creds.header.ssid_len, 0, sizeof(unsigned int));
		memset(ndef_msg_buf, 0, CONFIG_NDEF_FILE_SIZE);
		// Stop NFC to prevent power waste
		LOG_INF("Turning off NFC");
		nfc_t4t_emulation_stop();
		break;
	default:
		break;
	}
}

/**
 * @brief Callback function for handling NFC events.
 */
static void nfc_callback(void *context, nfc_t4t_event_t event, const uint8_t *data,
			 size_t data_length, uint32_t flags)
{
	ARG_UNUSED(context);
	ARG_UNUSED(data);
	ARG_UNUSED(flags);
	switch (event) {
	case NFC_T4T_EVENT_FIELD_ON:
		// do nothing
		break;

	case NFC_T4T_EVENT_FIELD_OFF:
		// do nothing
		break;

	case NFC_T4T_EVENT_NDEF_READ:
		// do nothing
		break;

	case NFC_T4T_EVENT_NDEF_UPDATED:
		if (data_length > 0) {
			if (parse_ndef_wifi_record(data_length, &creds) < 0) {
				LOG_ERR("Error parsing NFC data or unsupported authentication type");
				return;
			}
			LOG_HEXDUMP_DBG(data, data_length, "NFC RECORD: ");
			if (wifi_credentials_set_personal_struct(&creds) < 0) {
				LOG_ERR("Error adding credentials");
				return;
			}
			net_mgmt(NET_REQUEST_WIFI_CONNECT_STORED, iface, NULL, 0);
		}
		break;

	default:
		break;
	}
}

/**
 * @brief   Function for provisioning Wi-Fi over NFC
 */
int nfc_provision(void)
{
	LOG_INF("Starting NFC Provisioning");
	iface = net_if_get_default();

	/* Set up NFC */
	int err = nfc_t4t_setup(nfc_callback, NULL);

	if (err < 0) {
		printk("Cannot setup t4t library!\n");
		return -1;
	}
	/* Run Read-Write mode for Type 4 Tag platform */
	if (nfc_t4t_ndef_rwpayload_set(ndef_msg_buf, sizeof(ndef_msg_buf)) < 0) {
		printk("Cannot set payload!\n");
		return -1;
	}
	/* Start sensing NFC field */
	if (nfc_t4t_emulation_start() < 0) {
		printk("Cannot start emulation!\n");
		return -1;
	}

	net_mgmt_init_event_callback(&wifi_prov_mgmt_cb, wifi_mgmt_event_handler,
				     WIFI_PROV_MGMT_EVENTS);

	net_mgmt_add_event_callback(&wifi_prov_mgmt_cb);
	return 0;
}
