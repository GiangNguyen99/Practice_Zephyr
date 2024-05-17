/*
 * Copyright (c) 2016 Intel Corporation
 *
 * SPDX-License-Identifier: Apache-2.0
 */

#include <stdio.h>
#include <zephyr/kernel.h>
// #include </home/giang/zephyrproject/zephyr/include/zephyr/kernel.h>

#include <zephyr/drivers/gpio.h>

#define MY_STACK_SIZE 500
#define MY_PRIORITY 5

/* 1000 msec = 1 sec */
#define SLEEP_TIME_MS 1000
#define SLEEP_TIME_MS_LED1 500

/* The devicetree node identifier for the "led0" alias. */
#define LED0_NODE DT_ALIAS(led0)
#define LED1_NODE DT_ALIAS(led1)
#define LED2_NODE DT_ALIAS(led2)

/*
 * A build error on this line means your board is unsupported.
 * See the sample documentation for information on how to fix this.
 */
static const struct gpio_dt_spec led0 = GPIO_DT_SPEC_GET(LED0_NODE, gpios);
static const struct gpio_dt_spec led1 = GPIO_DT_SPEC_GET(LED1_NODE, gpios);
static const struct gpio_dt_spec led2 = GPIO_DT_SPEC_GET(LED2_NODE, gpios);

K_THREAD_STACK_DEFINE(my_stack_area_thread1, MY_STACK_SIZE);
K_THREAD_STACK_DEFINE(my_stack_area_thread2, MY_STACK_SIZE);

static struct k_thread thread1;
static struct k_thread thread2;

void entry_func_thread1(void *, void *, void *);
void entry_func_thread2(void *, void *, void *);

struct k_timer my_timer;
extern void my_expiry_function(struct k_timer *timer_id);

int ret2;
bool led_state2 = true;

struct data_item_type
{
	uint32_t field1;
	uint32_t field2;
	uint32_t field3;
};

char my_msgq_buffer[10 * sizeof(struct data_item_type)];
struct k_msgq my_msgq;

int main(void)
{

	if (!gpio_is_ready_dt(&led2))
	{
		return;
	}

	ret2 = gpio_pin_configure_dt(&led2, GPIO_OUTPUT_ACTIVE);
	if (ret2 < 0)
	{
		return;
	}

	k_thread_create(&thread1, my_stack_area_thread1,
					K_THREAD_STACK_SIZEOF(my_stack_area_thread1),
					entry_func_thread1,
					NULL, NULL, NULL,
					MY_PRIORITY, 0, K_NO_WAIT);

	k_thread_create(&thread2, my_stack_area_thread2,
					K_THREAD_STACK_SIZEOF(my_stack_area_thread2),
					entry_func_thread2,
					NULL, NULL, NULL,
					MY_PRIORITY, 0, K_NO_WAIT);

	k_timer_init(&my_timer, my_expiry_function, NULL);
	k_timer_start(&my_timer, K_MSEC(2000), K_MSEC(100));

	k_msgq_init(&my_msgq, my_msgq_buffer, sizeof(struct data_item_type), 10);

	return 0;
}

struct data_item_t
{
	// void *fifo_reserved;   /* 1st word reserved for use by FIFO */
	int value1;
};

void entry_func_thread1(void *unused1, void *unused2, void *unused3)
{

	int ret;
	bool led_state = true;

	if (!gpio_is_ready_dt(&led0))
	{
		return;
	}

	ret = gpio_pin_configure_dt(&led0, GPIO_OUTPUT_ACTIVE);
	if (ret < 0)
	{
		return;
	}

	struct data_item_type data;

	while (1)
	{
		data.field1 = 1;
		data.field2 = 2;
		data.field3 = 3;

		/* send data to consumers */
		while (k_msgq_put(&my_msgq, &data, K_NO_WAIT) != 0)
		{
			/* message queue is full: purge old data & try again */
			k_msgq_purge(&my_msgq);
		}

		ret = gpio_pin_toggle_dt(&led0);
		if (ret < 0)
		{
			return;
		}

		led_state = !led_state;
		printf("LED0 state: %s\n", led_state ? "ON" : "OFF");
		k_msleep(SLEEP_TIME_MS);
	}

	/* thread terminates at end of entry point function */
}

void entry_func_thread2(void *unused1, void *unused2, void *unused3)
{
	int ret;
	bool led_state = true;

	struct data_item_type data;

	if (!gpio_is_ready_dt(&led1))
	{
		return;
	}

	ret = gpio_pin_configure_dt(&led1, GPIO_OUTPUT_ACTIVE);
	if (ret < 0)
	{
		return;
	}
	while (1)
	{
		k_msgq_get(&my_msgq, &data, K_NO_WAIT);
		if (data.field1 == 1 && data.field2 == 2 && data.field3 == 3)
		{

			ret = gpio_pin_toggle_dt(&led1);
			if (ret < 0)
			{
				return;
			}

			led_state = !led_state;
			printf("LED1 state: %s\n", led_state ? "ON" : "OFF");
		}
		k_msleep(SLEEP_TIME_MS_LED1);
	}

	/* thread terminates at end of entry point function */
}

void my_expiry_function(struct k_timer *timer_id)
{

	ret2 = gpio_pin_toggle_dt(&led2);
	if (ret2 < 0)
	{
		return;
	}

	led_state2 = !led_state2;
	printf("LED3 state: %s\n", led_state2 ? "ON" : "OFF");
}
