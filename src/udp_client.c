/*
 * Copyright (c) 2024 Nordic Semiconductor ASA
 *
 * SPDX-License-Identifier: LicenseRef-Nordic-5-Clause
 */
#include <errno.h>
#include <stddef.h>
#include <string.h>
#include <stdio.h>
#include <zephyr/kernel.h>
#include <zephyr/types.h>
#include <zephyr/logging/log.h>
#include <zephyr/net/wifi.h>
#include <zephyr/net/wifi_mgmt.h>
#include <zephyr/net/net_mgmt.h>
#include <dk_buttons_and_leds.h>
#include "udp_client.h"
#include <zephyr/net/socket.h>

#define SERVER_HOSTNAME "udp-echo.nordicsemi.academy"
#define SERVER_PORT	"2444"
#define MESSAGE_SIZE	256
#define MESSAGE_TO_SEND "Hello from nRF70 Series"
#define SSTRLEN(s)	(sizeof(s) - 1)
static int sock;
static struct sockaddr_storage server;

static uint8_t recv_buf[MESSAGE_SIZE];
LOG_MODULE_REGISTER(udp_connection, LOG_LEVEL_INF);

static int server_resolve(void)
{
	int err;
	struct addrinfo *result;
	struct addrinfo hints = {
		.ai_family = AF_INET,
		.ai_socktype = SOCK_DGRAM,
	};

	err = getaddrinfo(SERVER_HOSTNAME, SERVER_PORT, &hints, &result);
	if (err != 0) {
		LOG_INF("getaddrinfo() failed, err: %d", err);
		return -EIO;
	}

	if (result == NULL) {
		LOG_INF("Error, address not found");
		return -ENOENT;
	}

	struct sockaddr_in *server4 = ((struct sockaddr_in *)&server);

	server4->sin_addr.s_addr = ((struct sockaddr_in *)result->ai_addr)->sin_addr.s_addr;
	server4->sin_family = AF_INET;
	server4->sin_port = ((struct sockaddr_in *)result->ai_addr)->sin_port;

	char ipv4_addr[NET_IPV4_ADDR_LEN];
	inet_ntop(AF_INET, &server4->sin_addr.s_addr, ipv4_addr, sizeof(ipv4_addr));
	LOG_INF("IPv4 address of server found %s", ipv4_addr);

	freeaddrinfo(result);

	return 0;
}

static int server_connect(void)
{
	int err;
	sock = socket(AF_INET, SOCK_DGRAM, IPPROTO_UDP);
	if (sock < 0) {
		LOG_INF("Failed to create socket, err: %d, %s", errno, strerror(errno));
		return -errno;
	}

	err = connect(sock, (struct sockaddr *)&server, sizeof(struct sockaddr_in));
	if (err < 0) {
		LOG_INF("Connecting to server failed, err: %d, %s", errno, strerror(errno));
		return -errno;
	}
	LOG_INF("Successfully connected to server");

	return 0;
}
static void button_handler(uint32_t button_state, uint32_t has_changed)
{
	/* STEP 8 - Send a message every time button 1 is pressed */
	if (has_changed & DK_BTN1_MSK && button_state & DK_BTN1_MSK) {
		int err = send(sock, MESSAGE_TO_SEND, SSTRLEN(MESSAGE_TO_SEND), 0);
		if (err < 0) {
			LOG_INF("Failed to send message, %d", errno);
			return;
		}
		LOG_INF("Successfully sent message: %s", MESSAGE_TO_SEND);
	}
}

int start_udp_client(void)
{
	int received;

	if (dk_buttons_init(button_handler) != 0) {
		LOG_ERR("Failed to initialize the buttons library");
	}

	if (server_resolve() != 0) {
		LOG_INF("Failed to resolve server name");
		return 0;
	}

	if (server_connect() != 0) {
		LOG_INF("Failed to initialize client");
		return 0;
	}

	LOG_INF("Press button 1 on your DK to send your message");

	while (1) {
		received = recv(sock, recv_buf, sizeof(recv_buf) - 1, 0);

		if (received < 0) {
			LOG_ERR("Socket error: %d, exit", errno);
			break;
		}

		if (received == 0) {
			LOG_ERR("Empty datagram");
			break;
		}

		recv_buf[received] = 0;
		LOG_INF("Data received from the server: (%s)", recv_buf);
	}
	close(sock);
	return 0;
}