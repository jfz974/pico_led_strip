#ifndef DEBOUNCED_BUTTON_H
#define DEBOUNCED_BUTTON_H

#include "hardware/gpio.h"
#include "pico/stdlib.h"
#include "pico/time.h"

// A push button wired between `pin` and GND, using the pin's internal
// pull-up (so idle = high, pressed = low). Debounces in software: a level
// change only becomes "stable" once it has held for kDebounceUs without
// flickering back.
class DebouncedButton {
public:
	explicit DebouncedButton(uint pin) : pin_(pin) {
		gpio_init(pin_);
		gpio_set_dir(pin_, GPIO_IN);
		gpio_pull_up(pin_);

		last_reading_low_ = read_raw();
		stable_low_ = last_reading_low_;
		last_change_us_ = time_us_64();
	}

	// Call once per frame. Returns true exactly once per physical press
	// (a debounced released -> pressed transition).
	bool consume_press() {
		bool reading_low = read_raw();
		uint64_t now_us = time_us_64();

		if (reading_low != last_reading_low_) {
			last_reading_low_ = reading_low;
			last_change_us_ = now_us;
		}

		bool pressed_edge = false;
		if (stable_low_ != last_reading_low_ && now_us - last_change_us_ >= kDebounceUs) {
			bool was_low = stable_low_;
			stable_low_ = last_reading_low_;
			if (!was_low && stable_low_) {
				pressed_edge = true;
			}
		}

		return pressed_edge;
	}

private:
	bool read_raw() const { return !gpio_get(pin_); }

	static constexpr uint64_t kDebounceUs = 30000; // 30 ms

	uint pin_;
	bool last_reading_low_;
	bool stable_low_;
	uint64_t last_change_us_;
};

#endif
