#ifndef __PARAMETERS_H__
#define __PARAMETERS_H__

#include "irq_extra.h"
#include "mqtt_client.h"
#include "uart_extra.h"
#include "uart.h"

/* Connect to local mqtt broker */
#define LOCAL				0
/* Connect to online broker */
#define TEST_MOSQUITTO		1
#define MQTT_ECLIPSE		2

#define SERVER_TO_USE		TEST_MOSQUITTO

#if SERVER_TO_USE==TEST_MOSQUITTO
#define SERVER_ADDR		"test.mosquitto.org"
#elif SERVER_TO_USE==MQTT_ECLIPSE
#define SERVER_ADDR		"mqtt.eclipse.org"
#else //local
#define SERVER_ADDR		"192.168.100.5"
#endif

#define SERVER_PORT		1883

/* Maximum buffer size for a message received from mqtt broker */
#define BUFF_LEN		200

/* Wifi network parameters */
#define WIFI_SSID		"guest_wifi"
#define WIFI_PASS		"guest_pass"


/* Default UART parameters for esp8266 */
#define UART_CONFIG_BAUDRATE	BD_115200
#define UART_CONFIG_PARITY		UART_NO_PARITY
#define UART_CONFIG_STOP_BITS	UART_ONE_STOPBIT
#define UART_CONFIG_WORD_LEN	UART_WORDLEN_8BITS
#define UART_CONFIG_IRQ_ID		ADUCM_UART_INT_ID

/* Aducm3029 only have one UART periferal so only 0 can be used */
#define UART_DEVICE_ID			0
/* Timer ID to be used for mqtt timeouts */
#define TIMER_ID				1

/* Mqtt configuration */
#define MQTT_SUBSCRIBE_TOPIC	"CfUBFUQvV80YNweCrQAO/tmp"
#define MQTT_CONFIG_CMD_TIMEOUT	20000
#define MQTT_CONFIG_VERSION		MQTT_VERSION_3_1
#define MQTT_CONFIG_KEEP_ALIVE	7200
#define MQTT_CONFIG_CLIENT_NAME	"aducm3029-client"
#define MQTT_CONFIG_CLI_USER	NULL
//#define MQTT_CONFIG_CLI_USER	"username"
#define MQTT_CONFIG_CLI_PASS	NULL
//#define MQTT_CONFIG_CLI_PASS	"password"

#define RELAY_GPIO_NB		8
#define RED_GPIO_NB		31
#define GREEN_GPIO_NB		32

enum {
	RELAY,
	RED,
	GREEN,
	TOTAL_GPIOS
};

#endif // __PARAMETERS_H__
