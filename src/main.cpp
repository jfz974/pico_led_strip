#include <Arduino.h>
#include <Adafruit_NeoPixel.h>

// 4x WS2815 LED strips, 400 LEDs each, driven from the Pico's PIO-based
// WS2812-compatible timing (WS2815 shares the same one-wire protocol as
// WS2812/NeoPixel; it only differs electrically by running at 12V and
// having a redundant backup data line, which isn't modeled here).

static const uint8_t NUM_STRIPS = 4;
static const uint16_t LEDS_PER_STRIP = 400;
static const uint8_t STRIP_PINS[NUM_STRIPS] = {2, 3, 4, 5};

Adafruit_NeoPixel strips[NUM_STRIPS] = {
    Adafruit_NeoPixel(LEDS_PER_STRIP, STRIP_PINS[0], NEO_GRB + NEO_KHZ800),
    Adafruit_NeoPixel(LEDS_PER_STRIP, STRIP_PINS[1], NEO_GRB + NEO_KHZ800),
    Adafruit_NeoPixel(LEDS_PER_STRIP, STRIP_PINS[2], NEO_GRB + NEO_KHZ800),
    Adafruit_NeoPixel(LEDS_PER_STRIP, STRIP_PINS[3], NEO_GRB + NEO_KHZ800),
};

static uint16_t hueOffset = 0;

void setup() {
  Serial.begin(115200);

  for (uint8_t s = 0; s < NUM_STRIPS; s++) {
    strips[s].begin();
    strips[s].setBrightness(64);
    strips[s].show(); // all pixels off
  }
}

void loop() {
  for (uint8_t s = 0; s < NUM_STRIPS; s++) {
    for (uint16_t i = 0; i < LEDS_PER_STRIP; i++) {
      uint16_t hue = (hueOffset + (uint32_t)i * 65536UL / LEDS_PER_STRIP +
                      s * 16384) &
                     0xFFFF;
      uint32_t color = strips[s].gamma32(strips[s].ColorHSV(hue));
      strips[s].setPixelColor(i, color);
    }
    strips[s].show();
  }

  hueOffset += 512;
  delay(20);
}
