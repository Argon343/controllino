#ifndef CONTROLLINO_GPIO_HANDLER_H
#define CONTROLLINO_GPIO_HANDLER_H

#include <Arduino.h>

#include "ProtocolHandler.h"

namespace controllino {

pin_type_t get_pin_type(pin_t pin);
pin_mode_type_t get_pin_mode_type(pin_t pin);

void write_digital_to_pin(pin_t pin, uint8_t level);
void write_analog_to_pin(pin_t pin, int level);

int read_digital_from_pin(pin_t pin);
int read_analog_from_pin(pin_t pin);

void set_pin_mode(pin_t pin, pin_mode_t pin_mode);
pin_mode_t get_pin_mode(pin_t pin);

void load_pin_modes(void);
void save_pin_modes(void);
void reset_pin_modes(void);

void trigger_pulse(pin_t pin);

} // namespace controllino

#endif /* CONTROLLINO_GPIO_HANDLER_H */
