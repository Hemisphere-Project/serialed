#include <stdio.h>
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "driver/gpio.h"
#include "driver/uart.h"
#include "driver/rmt.h"
#include "sdkconfig.h"
#include "esp-hal-rmt.h"

#define LED_BUILTIN 2

#define NR_OF_PANELS 4
#define NR_OF_LEDS   256
#define NR_OF_ALL_BITS 24*NR_OF_LEDS

// void blink_task(void *pvParameter)
// {
//     /* Configure the IOMUX register for pad BLINK_GPIO (some pads are
//        muxed to GPIO on reset already, but some default to other
//        functions and need to be switched to GPIO. Consult the
//        Technical Reference for a list of pads and their default
//        functions.)
//     */
//     gpio_pad_select_gpio(LED_BUILTIN);
//     /* Set the GPIO as a push/pull output */
//     gpio_set_direction(LED_BUILTIN, GPIO_MODE_OUTPUT);
//     while(1) {
//         /* Blink off (output low) */
//         gpio_set_level(LED_BUILTIN, 0);
//         vTaskDelay(1000 / portTICK_PERIOD_MS);
//         /* Blink on (output high) */
//         gpio_set_level(LED_BUILTIN, 1);
//         vTaskDelay(1000 / portTICK_PERIOD_MS);
//     }
// }

char rmtPins[NR_OF_PANELS] = {5, 18, 19, 21};
rmt_data_t led_data[NR_OF_PANELS][NR_OF_ALL_BITS];
rmt_obj_t* rmt_send[NR_OF_PANELS];
bool panelDirty[NR_OF_PANELS];

int incomingByte = 0;
int byteCounter = 0;


char MODE = 0;
char PANEL = 0;
char LED = 0;
int COLOR[] = {0,0,0};

void loop(void *pvParameter) {

}

void app_main()
{
    const int uart_num = UART_NUM_0;
    uart_config_t uart_config = {
      .baud_rate = 115200,
      .data_bits = UART_DATA_8_BITS,
      .parity = UART_PARITY_DISABLE,
      .stop_bits = UART_STOP_BITS_1,
      .flow_ctrl = UART_HW_FLOWCTRL_CTS_RTS,
      .rx_flow_ctrl_thresh = 122,
    };
    // Configure UART parameters
    ESP_ERROR_CHECK(uart_param_config(uart_num, &uart_config));

    xTaskCreate(&loop, "loop", configMINIMAL_STACK_SIZE, NULL, 5, NULL);


}
