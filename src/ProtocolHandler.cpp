#include <Arduino.h>

#include "ProtocolHandler.h"

namespace controllino {

typedef struct {
    command_type_t command;
    String command_string;
} command_struct_t;

// FIXME: Warning! These commands must be in the same order as in
// `command_type_t`!
const command_struct_t command_mapping[] = {
    {COMMAND_GET_INPUT, "GET_INPUT"},
    {COMMAND_SET_OUTPUT, "SET_OUTPUT"},
    {COMMAND_LOG_SIGNAL, "LOG_SIGNAL"},
    {COMMAND_END_LOG_SIGNAL, "END_LOG_SIGNAL"},
    {COMMAND_GET_PIN_MODE, "GET_PIN_MODE"},
    {COMMAND_SET_PIN_MODE, "SET_PIN_MODE"},
    {COMMAND_LOAD_PIN_MODES, "LOAD_PIN_MODES"},
    {COMMAND_SAVE_PIN_MODES, "SAVE_PIN_MODES"},
    {COMMAND_RESET_PIN_MODES, "RESET_PIN_MODES"},
    {COMMAND_TRIGGER_PULSE, "TRIGGER_PULSE"},
    {COMMAND_READY, "READY"},
    {COMMAND_ERROR, "ERROR"},
    {COMMAND_INVALID, "ERROR"},
};

const size_t len_command_array = sizeof(command_mapping) / sizeof(command_mapping[0]);

typedef struct {
    pin_t pin;
    char pin_name[5];
} io_mapping_t;

const io_mapping_t input_output_mapping[] = {
    {PIN_D30, "D30"},   {PIN_D31, "D31"},   {PIN_D32, "D32"}, {PIN_D33, "D33"},
    {PIN_D34, "D34"},   {PIN_D35, "D35"},   {PIN_D36, "D36"}, {PIN_D37, "D37"},
    {PIN_D38, "D38"},   {PIN_D39, "D39"},   {PIN_D40, "D40"}, {PIN_D41, "D41"},
    {PIN_D42, "D42"},   {PIN_D43, "D43"},   {PIN_D44, "D44"}, {PIN_D45, "D45"},
    {PIN_D46, "D46"},   {PIN_D47, "D47"},   {PIN_D48, "D48"}, {PIN_D49, "D49"},
    {PIN_A0, "A0"},     {PIN_A1, "A1"},     {PIN_A2, "A2"},   {PIN_A3, "A3"},
    {PIN_DAC0, "DAC0"}, {PIN_DAC1, "DAC1"},
    //{PIN_IN0, "IN0"},
    //{PIN_IN1, "IN1"},
};

const size_t len_io_array =
    sizeof(input_output_mapping) / sizeof(input_output_mapping[0]);

typedef struct {
    uint8_t pin_mode_number;
    char pin_level[5];
} pin_level_t;

const pin_level_t pin_level_mapping[] = {
    {HIGH, "HIGH"},
    {LOW, "LOW"},
};

typedef struct {
    pin_mode_t pin_mode;
    char pin_mode_string[15];
} pin_modes_mapping_t;

const pin_modes_mapping_t pin_modes_mapping[] = {
    {PIN_MODE_INPUT, "INPUT"},
    {PIN_MODE_OUTPUT, "OUTPUT"},
    {PIN_MODE_INPUT_PULLUP, "INPUT_PULLUP"},
};

const size_t len_pin_mode_array =
    sizeof(pin_modes_mapping) / sizeof(pin_modes_mapping[0]);

// ====================================================================
//                  PARSER PROTOCOL JSON
// ====================================================================

bool receive_message_handler(String process_string, message_struct_t* message) {
    bool couldDeserializeMessage = true;

    // Deserialize the JSON document
    DeserializationError error = deserializeJson(message->doc, process_string);
    // Test if parsing succeeds.
    if (error != DeserializationError::Code::Ok) {
        build_error(COMMAND_ERROR, "DESERIALIZE_JSON_FAILED", error.c_str());
        couldDeserializeMessage = false;
    }
    return couldDeserializeMessage;
}

// ====================================================================
//                  INTERPRETER PROTOCOL JSON
// ====================================================================

bool has_object_given_key(message_struct_t* message, String& data, String key_word) {
    bool foundKey = false;

    if (message->doc.containsKey(key_word)) {
        data = message->doc[key_word].as<String>();
        foundKey = true;
    } else {
        String error_message = "Key '" + key_word + "' is missing";
        build_error(message->command, "INVALID_KEY", error_message);
    }

    return foundKey;
}

command_type_t get_command(String command_string) {
    for (uint16_t i = 0; i < (uint16_t) len_command_array; i++) {
        if (command_string == command_mapping[i].command_string) {
            return command_mapping[i].command;
        }
    }
    return COMMAND_INVALID;
}

pin_mode_t get_valid_pin_mode(const String& pin_mode_string) {
    for (uint16_t i = 0; i < (uint16_t) len_pin_mode_array; i++) {
        if (pin_mode_string.equals(pin_modes_mapping[i].pin_mode_string)) {
            return pin_modes_mapping[i].pin_mode;
        }
    }

    return PIN_MODE_NOT_VALID;
}

// ====================================================================
//                  BUILDER PROTOCOL JSON
// ====================================================================

String get_command_string(command_type_t command, msg_type_t type) {
    String command_string = command_mapping[(int) command].command_string;

    switch (type) {
        case MSG_INPUT:
            return "RX_" + command_string;
            break;

        case MSG_OUTPUT:
            return "RX_" + command_string;
            break;

        case MSG_ERROR:
            return (command != COMMAND_ERROR) ? "ERR_" + command_string : command_string;
            break;

        default:
            break;
    }

    return command_mapping[(int) COMMAND_ERROR].command_string;
}

// TODO Rename this function to get_valid_pin; or rather string_to_pin?
pin_t get_valid_pin_type(const String& pin_string) {
    for (uint16_t i = 0; i < (uint16_t) len_io_array; i++) {
        if (pin_string.equals(input_output_mapping[i].pin_name)) {
            return input_output_mapping[i].pin;
        }
    }

    return PIN_INVALID_PIN;
}

String get_pin_mode_string(pin_mode_t pin_mode) {
    return pin_modes_mapping[(int) pin_mode].pin_mode_string;
}

// ====================================================================
//                  COMPASER PROTOCOL JSON
// ====================================================================

void build_error(const command_type_t& command, const String& error, const String& msg) {
    details::make_command_imp(command, MSG_ERROR, "error", error, "msg", msg);
}

void build_error(
    const command_type_t& command,
    const String& error,
    const String& msg,
    unsigned int job) {
    build_command(command, MSG_ERROR, job, "error", error, "msg", msg);
}

} // namespace controllino
