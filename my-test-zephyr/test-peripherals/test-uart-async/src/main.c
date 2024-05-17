#include <zephyr/kernel.h>
#include <zephyr/device.h>
#include <zephyr/drivers/uart.h>
#include <zephyr/drivers/gpio.h>
#include <string.h>

/* change this to any other UART peripheral if desired */
// #define UART_DEVICE_NODE DT_CHOSEN(zephyr_shell_uart)

// #define UART_DEVICE_NODE DT_ALIAS(&usart2)

#define MSG_SIZE 10

#define LED0_NODE DT_ALIAS(led0)

static const struct gpio_dt_spec led0 = GPIO_DT_SPEC_GET(LED0_NODE, gpios);

// #define CONFIG_UART_ASYNC_API

/* queue to store up to 10 messages (aligned to 4-byte boundary) */
K_MSGQ_DEFINE(uart_msgq, MSG_SIZE, 10, 4);

/*-----delare variables---------------*/

// struct k_msgq uart_msgq;

/* receive buffer used in UART ISR callback */
// static char rx_buf[MSG_SIZE];
static char rx_buf[MSG_SIZE] = {0};
static int rx_buf_pos = 0;

const struct uart_config uart_cfg = {
	.baudrate = 115200,
	.parity = UART_CFG_PARITY_NONE,
	.stop_bits = UART_CFG_STOP_BITS_1,
	.data_bits = UART_CFG_DATA_BITS_8,
	.flow_ctrl = UART_CFG_FLOW_CTRL_NONE};

// static int rx_buf_pos;

/*----------delare prototypes----------*/
// static const struct device *const uart_dev = DEVICE_DT_GET(UART_DEVICE_NODE);
const struct device *uart_dev = DEVICE_DT_GET(DT_NODELABEL(usart6));
void print_uart(char *buf);

void serial_cb(const struct device *dev, struct uart_event *evt, void *user_data);

int main(void)
{

	// char tx_buf[MSG_SIZE];

	// k_msgq_init(&uart_msgq, rx_buf, 1, 10);

	// char st[] = "hello\n";
	int ret = 0;

	if (!gpio_is_ready_dt(&led0))
	{
		printf("FAILED\n");
	}

	ret = gpio_pin_configure_dt(&led0, GPIO_OUTPUT_ACTIVE);
	if (ret < 0)
	{
		printf("FAILED\n");
	}

	if (!device_is_ready(uart_dev))
	{
		printk("UART device not found!");
		return 0;
	}

	int err = uart_configure(uart_dev, &uart_cfg);

	if (err == -ENOSYS)
	{
		return -ENOSYS;
	}

	/* Set the callback */
	ret = uart_callback_set(uart_dev, serial_cb, NULL);

	if (ret < 0)
	{
		if (ret == -ENOTSUP)
		{
			printk("Async UART API support not enabled\n");
		}
		else if (ret == -ENOSYS)
		{
			printk("UART device does not support async API\n");
		}
		else
		{
			printk("Error setting UART callback: %d\n", ret);
		}
		return 0;
	}

	print_uart("Hello! This is a example code to test async uart\r\n");

	/* indefinitely wait for input from the user */

	ret = uart_rx_enable(uart_dev, rx_buf, sizeof(rx_buf), 1000);

	print_uart(rx_buf);

	while (1)
	{

		k_msleep(1);
	}
	return 0;
}

/*
 * Print a null-terminated string character by character to the UART interface
 */
void print_uart(char *buf)
{
	int msg_len = strlen(buf);

	for (int i = 0; i < msg_len; i++)
	{
		uart_poll_out(uart_dev, buf[i]);
	}
}

void serial_cb(const struct device *dev, struct uart_event *evt, void *user_data)
{
	// char st[20] = "";
	uint8_t ret = 0;
	switch (evt->type)
	{
	case UART_TX_DONE:
		// strcpy(st, "UART_TX_DONE\n");
		// uart_tx(uart_dev,
		// 		(uint8_t *)st,
		// 		sizeof(st),
		// 		1000);
		break;

	case UART_TX_ABORTED:
		// strcpy(st,"UART_TX_ABORTED\n");
		// uart_tx(uart_dev,
		// 		(uint8_t *)st,
		// 		sizeof(st),
		// 		1000);
		break;

	case UART_RX_RDY:

		print_uart("UART_RX_RDY\r\n");
		print_uart(rx_buf);

		switch (rx_buf[rx_buf_pos++] - 48)
		{
		case 1:
			ret = gpio_pin_set_dt(&led0, 1);
			if (ret < 0)
			{
				print_uart("set led 1 failed!!!\r\n");
			}
			memset(rx_buf, 'x', sizeof(rx_buf));
			break;
		case 2:
			ret = gpio_pin_set_dt(&led0, 0);
			if (ret < 0)
			{
				print_uart("set led 1 failed!!!\r\n");
			}
			memset(rx_buf, 'x', sizeof(rx_buf));

			break;
		default:
			break;
		}

		break;
	case UART_RX_STOPPED:

		break;
	case UART_RX_BUF_REQUEST:
		print_uart("\r\nUART_RX_BUF_REQUEST\r\n");
		print_uart(rx_buf);
		rx_buf_pos = 0;

		break;
	case UART_RX_BUF_RELEASED:
		print_uart("\r\nUART_RX_BUF_RELEASED\r\n");
		print_uart(rx_buf);

		break;
	case UART_RX_DISABLED:
		print_uart("\r\nUART_RX_DISABLED\r\n");
		print_uart(rx_buf);

		uart_rx_enable(uart_dev, rx_buf, sizeof(rx_buf), 100);
		break;
	}
}
