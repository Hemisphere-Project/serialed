#include "driver/rmt.h"

#define NUM_LEDS 3

#define RED 0xFF0000
#define GREEN 0x00FF00
#define BLUE  0x0000FF

// Configure these based on your project needs ********
#define LED_RMT_TX_CHANNEL RMT_CHANNEL_0
#define LED_RMT_TX_GPIO GPIO_NUM_18
// ****************************************************

#define BITS_PER_LED_CMD 24
#define LED_BUFFER_ITEMS ((NUM_LEDS * BITS_PER_LED_CMD))

// These values are determined by measuring pulse timing with logic analyzer and adjusting to match datasheet.
#define T0H 14  // 0 bit high time
#define T1H 52  // 1 bit high time
#define TL  52  // low time for either bit

rmt_item32_t led_data_buffer[LED_BUFFER_ITEMS];

struct led_state {
    uint32_t leds[NUM_LEDS];
};

void setup(void) {
  config_leds();

  struct led_state new_state;
  new_state.leds[0] = RED;
  new_state.leds[1] = GREEN;
  new_state.leds[2] = BLUE;

  write_leds(new_state);
}

void loop() {
  struct led_state new_state;
  new_state.leds[0] = RED;
  new_state.leds[1] = GREEN;
  new_state.leds[2] = BLUE;
  write_leds(new_state);
}

void config_leds() {
  rmt_config_t config;
  config.rmt_mode = RMT_MODE_TX;
  config.channel = LED_RMT_TX_CHANNEL;
  config.gpio_num = LED_RMT_TX_GPIO;
  config.mem_block_num = 3;
  config.tx_config.loop_en = false;
  config.tx_config.carrier_en = false;
  config.tx_config.idle_output_en = true;
  config.tx_config.idle_level = RMT_IDLE_LEVEL_LOW;
  config.clk_div = 2;

  rmt_config(&config);
  rmt_driver_install(config.channel, 0, 0);
}



void write_leds(struct led_state new_state) {
  for (uint32_t led = 0; led < NUM_LEDS; led++) {
    uint32_t bits_to_send = new_state.leds[led];
    uint32_t mask = 1 << (BITS_PER_LED_CMD - 1);
    for (uint32_t bit = 0; bit < BITS_PER_LED_CMD; bit++) {
      uint32_t bit_is_set = bits_to_send & mask;
      led_data_buffer[led * BITS_PER_LED_CMD + bit] = bit_is_set ?
                                                      (rmt_item32_t){{{T1H, 1, TL, 0}}} :
                                                      (rmt_item32_t){{{T0H, 1, TL, 0}}};
      mask >>= 1;
    }
  }
  rmt_write_items(LED_RMT_TX_CHANNEL, led_data_buffer, LED_BUFFER_ITEMS, false);
  rmt_wait_tx_done(LED_RMT_TX_CHANNEL, portMAX_DELAY);
}
