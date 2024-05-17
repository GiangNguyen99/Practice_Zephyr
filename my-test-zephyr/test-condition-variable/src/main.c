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
#define MY_PRIORITY_THREAD1 5
#define MY_PRIORITY_THREAD2 4

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

K_EVENT_DEFINE(my_event);

int main(void)
{

	if (!gpio_is_ready_dt(&led2))
	{
		printf("gpio_is_ready_dt failed!\n");
	}

	ret2 = gpio_pin_configure_dt(&led2, GPIO_OUTPUT_ACTIVE);
	if (ret2 < 0)
	{
		printf("gpio_pin_configure_dt failed!\n");
	}

	k_thread_create(&thread1, my_stack_area_thread1,
					K_THREAD_STACK_SIZEOF(my_stack_area_thread1),
					entry_func_thread1,
					NULL, NULL, NULL,
					MY_PRIORITY_THREAD1, 0, K_NO_WAIT);

	k_thread_create(&thread2, my_stack_area_thread2,
					K_THREAD_STACK_SIZEOF(my_stack_area_thread2),
					entry_func_thread2,
					NULL, NULL, NULL,
					MY_PRIORITY_THREAD2, 0, K_NO_WAIT);

	k_timer_init(&my_timer, my_expiry_function, NULL);
	k_timer_start(&my_timer, K_MSEC(2000), K_MSEC(100));

	// k_event_init(&my_event);

	return 0;
}

void entry_func_thread1(void *unused1, void *unused2, void *unused3)
{

	int ret;
	bool led_state = true;

	if (!gpio_is_ready_dt(&led0))
	{
		printf("gpio_is_ready_dt failed!\n");
	}

	ret = gpio_pin_configure_dt(&led0, GPIO_OUTPUT_ACTIVE);
	if (ret < 0)
	{
		printf("gpio_pin_configure_dt failed!\n");
	}

	while (1)
	{
		/* notify threads that data is available */

		// k_event_post(&my_event, 0x001);
		k_event_set(&my_event, 0x001);
		ret = gpio_pin_toggle_dt(&led0);
		if (ret < 0)
		{
			printf("gpio_pin_toggle_dt failed!\n");
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

	if (!gpio_is_ready_dt(&led1))
	{
		printf("gpio_is_ready_dt failed!\n");
	}

	ret = gpio_pin_configure_dt(&led1, GPIO_OUTPUT_ACTIVE);
	if (ret < 0)
	{
		printf("gpio_pin_configure_dt failed!\n");
	}
	uint32_t events = 0;

	while (1)
	{

		events = k_event_wait_all(&my_event, 0xfff, false, K_MSEC(500));
		if (events != 0)
		{
			ret = gpio_pin_toggle_dt(&led1);
			if (ret < 0)
			{
				printf("gpio_pin_toggle_dt failed!\n");
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
		printf("gpio_pin_toggle_dt failed!\n");
	}

	led_state2 = !led_state2;
	printf("LED3 state: %s\n", led_state2 ? "ON" : "OFF");
}
