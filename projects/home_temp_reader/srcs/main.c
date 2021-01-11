#include "parameters.h"
#include "platform_init.h"
#include "error.h"
#include "wifi.h"
#include "tcp_socket.h"
#include "gpio.h"
#include "delay.h"
#include "print_log.h"
#include <string.h>
#include "aducm3029_adc.h"

static struct gpio_desc *gpios[TOTAL_GPIOS];
static int32_t gpios_nb[TOTAL_GPIOS] = {
		RELAY_GPIO_NB,
		RED_GPIO_NB,
		GREEN_GPIO_NB
};


int32_t init_and_connect_wifi(struct wifi_desc **wifi)
{
	static struct irq_ctrl_desc	*irq_ctrl;
	static struct uart_desc		*udesc;

	struct irq_init_param		irq_par;
	struct wifi_init_param		wifi_param;
	struct uart_init_param		uart_param;
	struct aducm_uart_init_param	uart_platform_param;
	int32_t				ret;

	/* Initialize irq controller */
	irq_par = (struct irq_init_param) {
		.irq_ctrl_id = 0,
		.extra = 0
	};
	ret = irq_ctrl_init(&irq_ctrl, &irq_par);
	CHECK_RET(ret, "Error irq_ctrl_init");

	/* Initialize uart device */
	uart_platform_param = (struct aducm_uart_init_param) {
		.parity = UART_CONFIG_PARITY,
		.stop_bits = UART_CONFIG_STOP_BITS,
		.word_length = UART_CONFIG_WORD_LEN
	};
	uart_param = (struct uart_init_param) {
		.device_id = UART_DEVICE_ID,
		.baud_rate = UART_CONFIG_BAUDRATE,
		.extra = &uart_platform_param
	};
	ret = uart_init(&udesc, &uart_param);
	CHECK_RET(ret, "Error uart_init");

	/* Initialize wifi descriptor */
	wifi_param = (struct wifi_init_param) {
		.irq_desc = irq_ctrl,
		.uart_desc = udesc,
		.uart_irq_conf = udesc,
		.uart_irq_id = UART_CONFIG_IRQ_ID
	};
	ret = wifi_init(wifi, &wifi_param);
	CHECK_RET(ret, "Error wifi_init");

	/* Connect to wifi network */
	ret = wifi_connect(*wifi, WIFI_SSID, WIFI_PASS);
	CHECK_RET(ret, "Error wifi_connect");

	printf("Connected to: %s\n", WIFI_SSID);

	return SUCCESS;
}

int init_and_connect_to_mqtt_broker(struct mqtt_desc **mqtt,
				    struct wifi_desc *wifi)
{
	static uint8_t			send_buff[BUFF_LEN];
	static uint8_t			read_buff[BUFF_LEN];
	static struct tcp_socket_desc	*sock;

	struct mqtt_init_param		mqtt_init_param;
	struct tcp_socket_init_param	socket_init_param;
	struct socket_address		mqtt_broker_addr;
	struct mqtt_connect_config	conn_config;
	int32_t				ret;

	/* Initialize socket structure */
	socket_init_param.max_buff_size = 0; //Default buffer size
	ret = wifi_get_network_interface(wifi, &socket_init_param.net);
	CHECK_RET(ret, "Error wifi_get_network_interface");

	ret = socket_init(&sock, &socket_init_param);
	CHECK_RET(ret, "Error socket_init");

	/* Connect socket to mqtt borker server */
	mqtt_broker_addr = (struct socket_address) {
		.addr = SERVER_ADDR,
		.port = SERVER_PORT
	};
	ret = socket_connect(sock, &mqtt_broker_addr);
	CHECK_RET(ret, "Error socket_connect");

	printf("Connection with \"%s\" established\n", SERVER_ADDR);

	/* Initialize mqtt descriptor */
	mqtt_init_param = (struct mqtt_init_param) {
		.timer_id = TIMER_ID,
		.extra_timer_init_param = NULL,
		.sock = sock,
		.command_timeout_ms = MQTT_CONFIG_CMD_TIMEOUT,
		.send_buff = send_buff,
		.read_buff = read_buff,
		.send_buff_size = BUFF_LEN,
		.read_buff_size = BUFF_LEN,
		.message_handler = NULL
	};
	ret = mqtt_init(mqtt, &mqtt_init_param);
	CHECK_RET(ret, "Error mqtt_init");

	/* Connect to mqtt broker */
	conn_config = (struct mqtt_connect_config) {
		.version = MQTT_CONFIG_VERSION,
		.keep_alive_ms = MQTT_CONFIG_KEEP_ALIVE,
		.client_name = MQTT_CONFIG_CLIENT_NAME,
		.username = MQTT_CONFIG_CLI_USER,
		.password = MQTT_CONFIG_CLI_PASS
	};
	ret = mqtt_connect(*mqtt, &conn_config, NULL);
	CHECK_RET(ret, "Error mqtt_connect");

	printf("Connected to mqtt broker\n");

	return SUCCESS;
}

#include <drivers/gpio/adi_gpio.h>

/** Get GPIO pin from GPIO number */
#define PIN(nb)		(1u << ((nb) & 0x0F))
/** Get GPIO port from GPIO number */
#define PORT(nb)	(((nb) & 0xF0) >> 4)

int32_t init_gpios()
{
	struct gpio_init_param default_init;
	int32_t i;
	int32_t ret;

	for (i = 0; i < TOTAL_GPIOS; i++)
	{
		default_init.number = gpios_nb[i];
		ret = gpio_get(&gpios[i], &default_init);
		CHECK_RET(ret, "Gpio get failed");
	}

	ret = gpio_direction_output(gpios[RED], GPIO_HIGH);
	CHECK_RET(ret, "gpio_direction_output failed");
	ret = gpio_direction_output(gpios[GREEN], GPIO_HIGH);
	CHECK_RET(ret, "gpio_direction_output failed");
	ret = gpio_direction_input(gpios[RELAY]);
	CHECK_RET(ret, "gpio_direction_output failed");
	ret = adi_gpio_PullUpEnable(PORT(gpios[RELAY]->number),
			PIN(gpios[RELAY]->number), true);
	CHECK_RET(ret, "adi_gpio_PullUpEnable failed");

	return 0;
}

int32_t set_status(int success)
{
	int32_t val = GPIO_HIGH;
	int32_t ret = SUCCESS;

	if (success)
		val = !val;

	ret |= gpio_set_value(gpios[RED], val);
	ret |= gpio_set_value(gpios[GREEN], !val);

	return ret;
}

int main(void)
{
	int ret;
	struct mqtt_desc *mqtt;
	struct wifi_desc *wifi;

	ret = platform_init();
	CHECK_RET(ret, "Platform init failed\n");

	ret = init_gpios();
	CHECK_RET(ret, "init_gpios failed\n");

	ret = set_status(false);
	CHECK_RET(ret, "set_status failed\n");

	ret = init_and_connect_wifi(&wifi);
	CHECK_RET(ret, "init_and_connect_wifi Failed\n");

	ret = init_and_connect_to_mqtt_broker(&mqtt, wifi);
	CHECK_RET(ret, "init_and_connect_to_mqtt_broker failed\n");

	ret = set_status(true);
	CHECK_RET(ret, "set_status failed\n");

	while (true) {
		uint8_t val;
		char	*payload;

		ret = gpio_get_value(gpios[RELAY], &val);
		if (IS_ERR_VALUE(ret))
			break;
		if (val == GPIO_LOW)
			payload = "on";
		else
			payload = "off";

		printf("Payload: %s\n", payload);
		struct mqtt_message msg = {
				.payload = payload,
				.len = strlen(payload),
				.qos = MQTT_QOS0,
				.retained = true
		};
		ret = mqtt_publish(mqtt, MQTT_SUBSCRIBE_TOPIC, &msg);
		if (IS_ERR_VALUE(ret))
			break;
		mdelay(60000);
	}

	set_status(false);
	while (true);

	return 0;
}
