/**
 * Copyright (c) 2020 Raspberry Pi (Trading) Ltd.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#include <FreeRTOS.h>
#include <task.h>
#include <semphr.h>
#include <queue.h>


#include <stdio.h>
#include <string.h> 
#include "pico/stdlib.h"
#include "hardware/gpio.h"
#include "hardware/timer.h"
#include "hardware/irq.h"

const int BTN_PIN_R = 28;
const int BTN_PIN_Y = 21;

const int LED_PIN_R = 5;
const int LED_PIN_Y = 10;

QueueHandle_t xQueueBtn;
SemaphoreHandle_t xSemaphoreLedR;
SemaphoreHandle_t xSemaphoreLedY;


void btn_callback(uint gpio, uint32_t events) {
    if (events & GPIO_IRQ_EDGE_RISE) {
        xQueueSendFromISR(xQueueBtn, &gpio, NULL);
    }
    else if (events & GPIO_IRQ_EDGE_FALL) {
        sleep_ms(100);
    }
}


void led_1_task(void* p) {

    gpio_init(LED_PIN_R);
    gpio_set_dir(LED_PIN_R, GPIO_OUT);

    while (1) {

        gpio_put(LED_PIN_R, 1);
        vTaskDelay(pdMS_TO_TICKS(100));
        gpio_put(LED_PIN_R, 0);
        vTaskDelay(pdMS_TO_TICKS(100));
    }

}


void led_r_task(void* p) {

    gpio_init(LED_PIN_R);
    gpio_set_dir(LED_PIN_R, GPIO_OUT);

    int status = 0;

    while (1) {

        if (xSemaphoreTake(xSemaphoreLedR, 0)){
            status = !status;
        }

        if (status) {
            gpio_put(LED_PIN_R, 1);
            vTaskDelay(pdMS_TO_TICKS(100));
            gpio_put(LED_PIN_R, 0);
            vTaskDelay(pdMS_TO_TICKS(100));
        } else {
            gpio_put(LED_PIN_R, 0);
        }

    }

}


void led_y_task(void* p) {

    gpio_init(LED_PIN_Y);
    gpio_set_dir(LED_PIN_Y, GPIO_OUT);

    int status = 0;

    while (1) {

        if (xSemaphoreTake(xSemaphoreLedY, 0)){
            status = !status;
        }

        if (status) {
            gpio_put(LED_PIN_Y, 1);
            vTaskDelay(pdMS_TO_TICKS(100));
            gpio_put(LED_PIN_Y, 0);
            vTaskDelay(pdMS_TO_TICKS(100));
        } else {
            gpio_put(LED_PIN_Y, 0);
        }
    }
}


void btn_task(void* p) {

    gpio_init(BTN_PIN_R);
    gpio_set_dir(BTN_PIN_R, GPIO_IN);
    gpio_pull_up(BTN_PIN_R);
    gpio_set_irq_enabled_with_callback(BTN_PIN_R,
        GPIO_IRQ_EDGE_FALL | GPIO_IRQ_EDGE_RISE,
        true,
        &btn_callback);

    gpio_init(BTN_PIN_Y);
    gpio_set_dir(BTN_PIN_Y, GPIO_IN);
    gpio_pull_up(BTN_PIN_Y);
    gpio_set_irq_enabled_with_callback(BTN_PIN_Y,
        GPIO_IRQ_EDGE_FALL | GPIO_IRQ_EDGE_RISE,
        true,
        &btn_callback);

    int gpio = 0;
    while (true) {
        if (xQueueReceive(xQueueBtn, &gpio, 100)) {
            if (gpio == BTN_PIN_R)
                xSemaphoreGive(xSemaphoreLedR);
            else
                xSemaphoreGive(xSemaphoreLedY);

        }
    }
}



int main() {
    stdio_init_all();


    xQueueBtn = xQueueCreate(32, sizeof(int));
    xSemaphoreLedR = xSemaphoreCreateBinary();
    xSemaphoreLedY = xSemaphoreCreateBinary();

    xTaskCreate(led_r_task, "LED_Task R", 256, NULL, 1, NULL);
    xTaskCreate(led_y_task, "LED_Task R", 256, NULL, 1, NULL);

    xTaskCreate(btn_task, "BTN_Task 1", 256, NULL, 1, NULL);

    vTaskStartScheduler();

    return 0;
}
