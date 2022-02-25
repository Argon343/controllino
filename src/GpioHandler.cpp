#include "GpioHandler.h"

namespace controllino {

typedef struct {
    const pin_t pin;
    const uint8_t pin_number;
    const pin_type_t pin_type;
    pin_mode_t pin_mode;
    const pin_mode_type_t pin_mode_type;
} inputs_outputs_t;

inputs_outputs_t mapping_dict[] = {
    // Digital input
    {PIN_D30, 30, PIN_DIGITAL, PIN_MODE_INPUT, PIN_MODE_TYPE_INPUT_AND_OUTPUT},
    {PIN_D31, 31, PIN_DIGITAL, PIN_MODE_INPUT, PIN_MODE_TYPE_INPUT_AND_OUTPUT},
    {PIN_D32, 32, PIN_DIGITAL, PIN_MODE_INPUT, PIN_MODE_TYPE_INPUT_AND_OUTPUT},
    {PIN_D33, 33, PIN_DIGITAL, PIN_MODE_INPUT, PIN_MODE_TYPE_INPUT_AND_OUTPUT},
    {PIN_D34, 34, PIN_DIGITAL, PIN_MODE_INPUT, PIN_MODE_TYPE_INPUT_AND_OUTPUT},
    {PIN_D35, 35, PIN_DIGITAL, PIN_MODE_INPUT, PIN_MODE_TYPE_INPUT_AND_OUTPUT},
    {PIN_D36, 36, PIN_DIGITAL, PIN_MODE_INPUT, PIN_MODE_TYPE_INPUT_AND_OUTPUT},
    {PIN_D37, 37, PIN_DIGITAL, PIN_MODE_INPUT, PIN_MODE_TYPE_INPUT_AND_OUTPUT},
    {PIN_D38, 38, PIN_DIGITAL, PIN_MODE_INPUT, PIN_MODE_TYPE_INPUT_AND_OUTPUT},
    {PIN_D39, 39, PIN_DIGITAL, PIN_MODE_INPUT, PIN_MODE_TYPE_INPUT_AND_OUTPUT},
    // Digital output
    {PIN_D40, 40, PIN_DIGITAL, PIN_MODE_OUTPUT, PIN_MODE_TYPE_INPUT_AND_OUTPUT},
    {PIN_D41, 41, PIN_DIGITAL, PIN_MODE_OUTPUT, PIN_MODE_TYPE_INPUT_AND_OUTPUT},
    {PIN_D42, 42, PIN_DIGITAL, PIN_MODE_OUTPUT, PIN_MODE_TYPE_INPUT_AND_OUTPUT},
    {PIN_D43, 43, PIN_DIGITAL, PIN_MODE_OUTPUT, PIN_MODE_TYPE_INPUT_AND_OUTPUT},
    {PIN_D44, 44, PIN_DIGITAL, PIN_MODE_OUTPUT, PIN_MODE_TYPE_INPUT_AND_OUTPUT},
    {PIN_D45, 45, PIN_DIGITAL, PIN_MODE_OUTPUT, PIN_MODE_TYPE_INPUT_AND_OUTPUT},
    {PIN_D46, 46, PIN_DIGITAL, PIN_MODE_OUTPUT, PIN_MODE_TYPE_INPUT_AND_OUTPUT},
    {PIN_D47, 47, PIN_DIGITAL, PIN_MODE_OUTPUT, PIN_MODE_TYPE_INPUT_AND_OUTPUT},
    {PIN_D48, 48, PIN_DIGITAL, PIN_MODE_OUTPUT, PIN_MODE_TYPE_INPUT_AND_OUTPUT},
    {PIN_D49, 49, PIN_DIGITAL, PIN_MODE_OUTPUT, PIN_MODE_TYPE_INPUT_AND_OUTPUT},
    // Analog input
    {PIN_A0, A0, PIN_ANALOG, PIN_MODE_INPUT, PIN_MODE_TYPE_INPUT_ONLY},
    {PIN_A1, A1, PIN_ANALOG, PIN_MODE_INPUT, PIN_MODE_TYPE_INPUT_ONLY},
    {PIN_A2, A2, PIN_ANALOG, PIN_MODE_INPUT, PIN_MODE_TYPE_INPUT_ONLY},
    {PIN_A3, A3, PIN_ANALOG, PIN_MODE_INPUT, PIN_MODE_TYPE_INPUT_ONLY},
    // Analog output
    {PIN_DAC0, DAC0, PIN_ANALOG, PIN_MODE_OUTPUT, PIN_MODE_TYPE_OUTPUT_ONLY},
    {PIN_DAC1, DAC1, PIN_ANALOG, PIN_MODE_OUTPUT, PIN_MODE_TYPE_OUTPUT_ONLY},
};

const size_t len_mapping_array = sizeof(mapping_dict) / sizeof(mapping_dict[0]);

typedef struct {
    pin_mode_t pin_mode;
    uint8_t pin_mode_define;
} pin_modes_map_t;

const pin_modes_map_t pin_modes_map[] = {
    {PIN_MODE_INPUT, INPUT},
    {PIN_MODE_OUTPUT, OUTPUT},
    {PIN_MODE_INPUT_PULLUP, INPUT_PULLUP},
};

pin_type_t get_pin_type(pin_t pin) {
    return mapping_dict[(int) pin].pin_type;
}

pin_mode_type_t get_pin_mode_type(pin_t pin) {
    return mapping_dict[(int) pin].pin_mode_type;
}

void set_pin_mode(pin_t pin, pin_mode_t pin_mode) {
    mapping_dict[(int) pin].pin_mode = pin_mode;
    uint8_t pin_number = mapping_dict[(int) pin].pin_number;
    pinMode(pin_number, pin_modes_map[(int) pin_mode].pin_mode_define);
}

pin_mode_t get_pin_mode(pin_t pin) {
    return mapping_dict[(int) pin].pin_mode;
}

void write_digital_to_pin(pin_t pin, uint8_t level) {
    uint8_t pin_number = mapping_dict[(int) pin].pin_number;
    digitalWrite(pin_number, level);
}

void write_analog_to_pin(pin_t pin, int level) {
    uint8_t pin_number = mapping_dict[(int) pin].pin_number;
    analogWrite(pin_number, level);
}

int read_digital_from_pin(pin_t pin) {
    uint8_t pin_number = mapping_dict[(int) pin].pin_number;
    return digitalRead(pin_number);
}

int read_analog_from_pin(pin_t pin) {
    uint8_t pin_number = mapping_dict[(int) pin].pin_number;
    return analogRead(pin_number);
}

void load_pin_modes(void) {
    for (uint8_t i = 0; i < (uint8_t) len_mapping_array; i++) {
        pin_mode_t eeprom_pin_mode = mapping_dict[i].pin_mode;
        // pin_mode_t eeprom_pin_mode = (pin_mode_t)EEPROM.read(pin_states[i].pin);
        if (eeprom_pin_mode > PIN_MODE_INPUT_PULLUP) {
            eeprom_pin_mode = PIN_MODE_INPUT;
        }
        set_pin_mode(mapping_dict[i].pin, eeprom_pin_mode);
    }
}

void save_pin_modes(void) {
    for (uint8_t i = 0; i < (uint8_t) len_mapping_array; i++) {
        // EEPROM.update(pin_states[i].pin, pin_states[i].pin_mode);
    }
}

void reset_pin_modes(void) {
    for (uint8_t i = 0; i < (uint8_t) len_mapping_array; i++) {
        mapping_dict[i].pin_mode = PIN_MODE_INPUT;
        // EEPROM.update(pin_states[i].pin, PIN_MODE_INPUT);
    }
}

void trigger_pulse(pin_t pin) {
    uint8_t pin_number = mapping_dict[(int) pin].pin_number;
    digitalWrite(pin_number, HIGH);
    delay(100);
    digitalWrite(pin_number, LOW);
}

} // namespace controllino
