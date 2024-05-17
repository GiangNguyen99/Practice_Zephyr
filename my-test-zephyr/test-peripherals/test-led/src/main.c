/*
 * Copyright (c) 2016 Intel Corporation
 *
 * SPDX-License-Identifier: Apache-2.0
 */

#include <stdio.h>
#include <zephyr/kernel.h>

#include <zephyr/drivers/gpio.h>

#define MY_STACK_SIZE 500
#define MY_PRIORITY_THREAD1 6
#define MY_PRIORITY_THREAD2 5
#define MY_PRIORITY_THREAD3 4
#define MY_PRIORITY_THREAD4 3

/* 1000 msec = 1 sec */
#define SLEEP_TIME_THREAD1 500
#define SLEEP_TIME_THREAD2 500
#define SLEEP_TIME_THREAD3 1000
#define SLEEP_TIME_THREAD4 500

/* The devicetree node identifier for the "led0" alias. */
#define LED0_NODE DT_ALIAS(led0)
#define LED1_NODE DT_ALIAS(led1)
#define LED2_NODE DT_ALIAS(led2)
#define LED3_NODE DT_ALIAS(led3)
/*
 * A build error on this line means your board is unsupported.
 * See the sample documentation for information on how to fix this.
 */
static const struct gpio_dt_spec led0 = GPIO_DT_SPEC_GET(LED0_NODE, gpios);
static const struct gpio_dt_spec led1 = GPIO_DT_SPEC_GET(LED1_NODE, gpios);
static const struct gpio_dt_spec led2 = GPIO_DT_SPEC_GET(LED2_NODE, gpios);
// static const struct gpio_dt_spec led3 = GPIO_DT_SPEC_GET(LED3_NODE, gpios);

K_THREAD_STACK_DEFINE(my_stack_area_thread1, MY_STACK_SIZE);
K_THREAD_STACK_DEFINE(my_stack_area_thread2, MY_STACK_SIZE);
K_THREAD_STACK_DEFINE(my_stack_area_thread3, MY_STACK_SIZE);
K_THREAD_STACK_DEFINE(my_stack_area_thread4, MY_STACK_SIZE);

static struct k_thread thread1;
static struct k_thread thread2;
static struct k_thread thread3;
static struct k_thread thread4;

void entry_func_thread1(void *, void *, void *);
void entry_func_thread2(void *, void *, void *);
void entry_func_thread3(void *, void *, void *);
void entry_func_thread4(void *, void *, void *);

struct k_timer my_timer1;
struct k_timer my_timer2;

extern void my_expiry_function1(struct k_timer *timer_id);
extern void my_expiry_function2(struct k_timer *timer_id);

int main(void)
{
	int ret;
	if (!gpio_is_ready_dt(&led2))
	{
		printf("FAILED\n");
	}

	ret = gpio_pin_configure_dt(&led2, GPIO_OUTPUT_ACTIVE);
	if (ret < 0)
	{
		printf("FAILED\n");
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
	k_thread_create(&thread3, my_stack_area_thread3,
					K_THREAD_STACK_SIZEOF(my_stack_area_thread3),
					entry_func_thread3,
					NULL, NULL, NULL,
					MY_PRIORITY_THREAD3, 0, K_NO_WAIT);

	k_thread_create(&thread4, my_stack_area_thread4,
					K_THREAD_STACK_SIZEOF(my_stack_area_thread4),
					entry_func_thread4,
					NULL, NULL, NULL,
					MY_PRIORITY_THREAD4, 0, K_NO_WAIT);

	// k_sem_init(&my_sem, 0, 1);

	k_timer_init(&my_timer1, my_expiry_function1, NULL);
	k_timer_init(&my_timer2, my_expiry_function2, NULL);

	k_timer_start(&my_timer1, K_MSEC(2000), K_MSEC(2000));
	k_timer_start(&my_timer2, K_MSEC(500), K_MSEC(1000));

	return 0;
}

void entry_func_thread1(void *unused1, void *unused2, void *unused3)
{
	int ret;
	bool led_state = true;

	if (!gpio_is_ready_dt(&led0))
	{
		printf("FAILED\n");
	}

	ret = gpio_pin_configure_dt(&led0, GPIO_OUTPUT_ACTIVE);
	if (ret < 0)
	{
		printf("FAILED\n");
	}
	while (1)
	{

		ret = gpio_pin_toggle_dt(&led0);
		if (ret < 0)
		{
			printf("FAILED\n");
		}

		led_state = !led_state;
		printf("LED0 state: %s\n", led_state ? "ON" : "OFF");
		k_msleep(SLEEP_TIME_THREAD1);
	}

	/* thread terminates at end of entry point function */
}

void entry_func_thread2(void *unused1, void *unused2, void *unused3)
{
	int ret;
	bool led_state = true;

	if (!gpio_is_ready_dt(&led1))
	{
		printf("FAILED\n");
	}

	ret = gpio_pin_configure_dt(&led1, GPIO_OUTPUT_ACTIVE);
	if (ret < 0)
	{
		printf("FAILED\n");
	}
	while (1)
	{

		// ret = gpio_pin_toggle_dt(&led1);
		// if (ret < 0)
		// {
		// 	printf("FAILED\n");
		// }

		led_state = !led_state;
		printf("LED1 state: %s\n", led_state ? "ON" : "OFF");
		k_msleep(SLEEP_TIME_THREAD2);
	}

	/* thread terminates at end of entry point function */
}

void entry_func_thread3(void *unused1, void *unused2, void *unused3)
{

	while (1)
	{
		k_msleep(SLEEP_TIME_THREAD3);
	}

	/* thread terminates at end of entry point function */
}

void entry_func_thread4(void *unused1, void *unused2, void *unused3)
{

	while (1)
	{

		k_msleep(SLEEP_TIME_THREAD4);
	}

	/* thread terminates at end of entry point function */
}

void my_expiry_function1(struct k_timer *timer_id)
{
	int ret;

	ret = gpio_pin_set_dt(&led2, 0);
	if (ret < 0)
	{

	}
	// ret = gpio_pin_toggle_dt(&led2);
	// if (ret < 0)
	// {

	// }
}

void my_expiry_function2(struct k_timer *timer_id)
{
	int ret;

	ret = gpio_pin_set_dt(&led2, 1);
	if (ret < 0)
	{
	}

}