#ifndef CONTROLLINO_PROTOCOL_HANDLER_H
#define CONTROLLINO_PROTOCOL_HANDLER_H

#include <Arduino.h>
#include <ArduinoJson.h>

#include "SerialHandler.h"

namespace controllino {

typedef enum
{
    COMMAND_GET_INPUT = 0,
    COMMAND_SET_OUTPUT,
    COMMAND_LOG_SIGNAL,
    COMMAND_END_LOG_SIGNAL,
    COMMAND_GET_PIN_MODE,
    COMMAND_SET_PIN_MODE,
    COMMAND_LOAD_PIN_MODES,
    COMMAND_SAVE_PIN_MODES,
    COMMAND_RESET_PIN_MODES,
    COMMAND_TRIGGER_PULSE,
    COMMAND_READY,
    COMMAND_ERROR,
    COMMAND_INVALID,
} command_type_t;

typedef enum
{
    MSG_INPUT = 0,
    MSG_OUTPUT,
    MSG_ERROR,
} msg_type_t;

typedef enum
{
    PIN_D30 = 0,
    PIN_D31,
    PIN_D32,
    PIN_D33,
    PIN_D34,
    PIN_D35,
    PIN_D36,
    PIN_D37,
    PIN_D38,
    PIN_D39,
    PIN_D40,
    PIN_D41,
    PIN_D42,
    PIN_D43,
    PIN_D44,
    PIN_D45,
    PIN_D46,
    PIN_D47,
    PIN_D48,
    PIN_D49,
    PIN_A0,
    PIN_A1,
    PIN_A2,
    PIN_A3,
    PIN_DAC0,
    PIN_DAC1,
    PIN_INVALID_PIN,
} pin_t;

typedef enum
{
    PIN_DIGITAL = 0,
    PIN_ANALOG,
} pin_type_t;

typedef enum
{
    PIN_MODE_TYPE_INPUT_ONLY = 0,
    PIN_MODE_TYPE_OUTPUT_ONLY,
    PIN_MODE_TYPE_INPUT_AND_OUTPUT,
} pin_mode_type_t;

typedef enum
{
    PIN_MODE_INPUT = 0,
    PIN_MODE_OUTPUT,
    PIN_MODE_INPUT_PULLUP,
    PIN_MODE_NOT_VALID,
} pin_mode_t;

const int capacity = JSON_OBJECT_SIZE(32);

typedef struct {
    command_type_t command;
    StaticJsonDocument<capacity> doc;
} message_struct_t;

// ====================================================================
//                  PARSER PROTOCOL JSON
// ====================================================================
bool receive_message_handler(String process_string, message_struct_t* message);

// ====================================================================
//                  INTERPRETER PROTOCOL JSON
// ====================================================================
bool has_object_given_key(message_struct_t* message, String& data, String key);
command_type_t get_command(String command_string);
pin_t get_valid_pin_type(const String& pin_string);
pin_mode_t get_valid_pin_mode(const String& pin_mode_string);

// ====================================================================
//                  BUILDER PROTOCOL JSON
// ====================================================================
String get_command_string(command_type_t command, msg_type_t type);
String get_pin_mode_string(pin_mode_t pin_mode);

// ====================================================================
//                  COMPASER PROTOCOL JSON
// ====================================================================

void build_error(const command_type_t& command, const String& type, const String& msg);

void build_error(
    const command_type_t& command,
    const String& type,
    const String& msg,
    unsigned int job);

namespace details {

template<typename Document>
void write_to_json_doc(Document& doc) {
    // noop
}

template<typename Document, typename T1, typename T2, typename... Ts>
void write_to_json_doc(Document& doc, const T1& key, const T2& value, const Ts&... ts) {
    doc[key] = value;
    write_to_json_doc(doc, ts...);
}

template<typename... Ts>
void make_command_imp(
    const command_type_t& command, const msg_type_t& type, const Ts&... data) {
    String command_string = get_command_string(command, type);

    const int capacity = JSON_OBJECT_SIZE(32); // FIXME Always sufficient?
    StaticJsonDocument<capacity> doc;

    doc["command"] = command_string;
    details::write_to_json_doc(doc, data...);

    String output;
    serializeJson(doc, output);
    serial_print_message(output);
}

} // namespace details

template<typename... Ts>
void build_command(
    const command_type_t& command,
    const msg_type_t& type,
    unsigned int job,
    const Ts&... data) {
    details::make_command_imp(command, type, "job", job, data...);
}

} // namespace controllino

#endif /* CONTROLLINO_PROTOCOL_HANDLER_H */
