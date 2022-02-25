#include "MessageHandler.h"

#include <ArduinoJson.h>

#include "GpioHandler.h"
#include "Logger.h"
#include "ProtocolHandler.h"
#include "SerialHandler.h"

namespace controllino {

static message_struct_t message_struct;

void receive_message(void* data);
void do_command_action(message_struct_t* message, const String command_string);

void command_get_input(unsigned int job, const String pin);
void command_set_output(unsigned int job, const String pin, const String level);
void command_log_signal(unsigned int job, const String& pin, int period);
void command_end_log_signal(unsigned int job, const String& pin);
void command_get_pin_mode(unsigned int job, const String pin_string);
void command_set_pin_mode(unsigned int job, const String pin, const String mode);
void command_trigger_pulse(unsigned int job, const String pin_string);

void init_message_handler(void) {
    serial_set_callback(receive_message);
}

void receive_message(void* data) {
    String process_string = *(String*) data;

    if (receive_message_handler(process_string, &message_struct)) {
        String command_string = "";
        command_string.reserve(15);
        if (has_object_given_key(&message_struct, command_string, "command")) {
            do_command_action(&message_struct, command_string);
        }
    }
}

void do_command_action(message_struct_t* message, const String command_string) {
    message_struct.command = get_command(command_string);
    String tmp;
    if (not has_object_given_key(message, tmp, "job")) {
        String error_message = "received command without job id";
        build_error(COMMAND_ERROR, "NO_JOB_ID", error_message);
        return;
    }
    int job = tmp.toInt();

    switch (message_struct.command) {
        case COMMAND_GET_INPUT: {
            String pin = "";
            pin.reserve(5);
            if (has_object_given_key(message, pin, "pin")) {
                command_get_input(job, pin);
            }
            break;
        }

        case COMMAND_SET_OUTPUT: {
            String pin = "";
            pin.reserve(5);
            String level = "";
            level.reserve(5);

            if (has_object_given_key(message, pin, "pin") &&
                has_object_given_key(message, level, "level")) {
                command_set_output(job, pin, level);
            }
            break;
        }

        case COMMAND_LOG_SIGNAL: {
            String pin = "";
            pin.reserve(5);
            String period = "";
            pin.reserve(10);

            if (has_object_given_key(message, pin, "pin") &&
                has_object_given_key(message, period, "period")) {
                command_log_signal(job, pin, period.toInt());
            }
            break;
        }

        case COMMAND_END_LOG_SIGNAL: {
            String pin = "";
            pin.reserve(5);

            if (has_object_given_key(message, pin, "pin")) {
                command_end_log_signal(job, pin);
            }
            break;
        }

        case COMMAND_GET_PIN_MODE: {
            String pin = "";
            pin.reserve(5);

            if (has_object_given_key(message, pin, "pin") == true) {
                command_get_pin_mode(job, pin);
            }
            break;
        }

        case COMMAND_SET_PIN_MODE: {
            String pin = "";
            pin.reserve(5);
            String mode = "";
            mode.reserve(10);

            // FIXME Raise an error here if a specific field is missing.
            if (has_object_given_key(message, pin, "pin") &&
                has_object_given_key(message, mode, "mode")) {
                command_set_pin_mode(job, pin, mode);
            }
            break;
        }

        case COMMAND_LOAD_PIN_MODES: {
            load_pin_modes();
            build_command(COMMAND_LOAD_PIN_MODES, MSG_OUTPUT, job);
            break;
        }

        case COMMAND_SAVE_PIN_MODES: {
            save_pin_modes();
            build_command(COMMAND_SAVE_PIN_MODES, MSG_OUTPUT, job);
            break;
        }

        case COMMAND_RESET_PIN_MODES: {
            reset_pin_modes();
            build_command(COMMAND_RESET_PIN_MODES, MSG_OUTPUT, job);
            break;
        }

        case COMMAND_TRIGGER_PULSE: {
            String pin = "";
            pin.reserve(5);

            if (has_object_given_key(message, pin, "pin")) {
                command_trigger_pulse(job, pin);
            }
            break;
        }

        case COMMAND_INVALID:
        default: {
            String error_message = "Command '" + command_string + "' is not valid";
            // TODO This isn't flexible enough. Allow any string so that
            // incorrectly spelled commands can go back properly.
            build_error(COMMAND_INVALID, "INVALID_COMMAND", error_message, job);
            break;
        }
    }
}

// ====================================================================
//                  COMMAND INTERPRETER
// ====================================================================

void command_ready() {
    build_command(COMMAND_READY, MSG_OUTPUT, 0); // READY is always job 0.
}

void command_log_signal(unsigned int job, const String& pin, int period) {
    auto pin_object = get_valid_pin_type(pin);
    if (pin_object == PIN_INVALID_PIN) {
        build_error(COMMAND_LOG_SIGNAL, "INVALID_PIN", "", job);
    }

    auto pin_mode = get_pin_mode(pin_object);
    if (pin_mode != PIN_MODE_INPUT) {
        build_error(COMMAND_LOG_SIGNAL, "INVALID_INPUT_PIN", "", job);
    }

    auto error = log_signal(job, pin_object, period);
    if (error) {
        String err;
        String msg = "";
        if (error == 1) {
            err = "TOO_MANY_LOGGING_JOBS";
        } else if (error == 2) {
            err = "DUPLICATE_LOGGING_JOB";
        }
        build_error(COMMAND_LOG_SIGNAL, err, msg, job);
        return;
    }
}

void command_end_log_signal(unsigned int job, const String& pin) {
    auto pin_object = get_valid_pin_type(pin);
    auto error = end_log_signal(pin_object);
    if (error) {
        String err = "LOGGING_REQUEST_NOT_FOUND";
        String msg = "";
        build_error(COMMAND_END_LOG_SIGNAL, err, msg, job);
        return;
    }

    build_command(COMMAND_END_LOG_SIGNAL, MSG_OUTPUT, job);
}

void command_get_input(unsigned int job, const String pin_string) {
    pin_t pin = get_valid_pin_type(pin_string);
    if (pin != PIN_INVALID_PIN) {
        pin_type_t pin_type = get_pin_type(pin);
        if (pin_type == PIN_DIGITAL) {
            String pin_value = "";
            pin_value.reserve(5);
            pin_value = (read_digital_from_pin(pin) == HIGH) ? "HIGH" : "LOW";
            build_command(
                COMMAND_GET_INPUT,
                MSG_OUTPUT,
                job,
                "pin",
                pin_string,
                "level",
                pin_value);
        } else {
            auto pin_value = read_analog_from_pin(pin);
            build_command(
                COMMAND_GET_INPUT,
                MSG_OUTPUT,
                job,
                "pin",
                pin_string,
                "level",
                pin_value);
        }
    } else {
        String error_message = "Pin '" + pin_string + "' is not valid";
        build_error(COMMAND_GET_INPUT, "INVALID_PIN", error_message, job);
    }
}

void command_set_output(
    unsigned int job, const String pin_string, const String level_string) {
    pin_t pin = get_valid_pin_type(pin_string);
    if (pin != PIN_INVALID_PIN) {
        pin_mode_t pin_mode = get_pin_mode(pin);
        if (pin_mode == PIN_MODE_OUTPUT) {
            bool couldSetOutput = true;

            pin_type_t pin_type = get_pin_type(pin);
            if (pin_type == PIN_DIGITAL) {
                if (level_string.equals("HIGH")) {
                    write_digital_to_pin(pin, HIGH);
                } else if (level_string.equals("LOW")) {
                    write_digital_to_pin(pin, LOW);
                } else {
                    couldSetOutput = false;
                }

                if (couldSetOutput == true) {
                    build_command(
                        COMMAND_SET_OUTPUT,
                        MSG_OUTPUT,
                        job,
                        "pin",
                        pin_string,
                        "level",
                        level_string);
                } else {
                    String error_message = "Level '" + level_string + "' is not valid";
                    build_error(
                        COMMAND_SET_OUTPUT, "INVALID_OUTPUT_LEVEL", error_message, job);
                }
            } else // (pin_type == PIN_ANALOG)
            {
                int analog_value = level_string.toInt();
                if (analog_value <= 255) {
                    write_analog_to_pin(pin, analog_value);
                } else {
                    couldSetOutput = false;
                }

                if (couldSetOutput == true) {
                    build_command(
                        COMMAND_SET_OUTPUT,
                        MSG_OUTPUT,
                        job,
                        "pin",
                        pin_string,
                        "level",
                        analog_value);
                } else {
                    String error_message = "Level '" + level_string + "' is not valid";
                    build_error(
                        COMMAND_SET_OUTPUT, "INVALID_OUTPUT_LEVEL", error_message, job);
                }
            }

        } else {
            String error_message = "Pin '" + pin_string + "' is not an output";
            build_error(COMMAND_SET_OUTPUT, "INVALID_OUTPUT_PIN", error_message, job);
        }
    } else {
        String error_message = "Pin '" + pin_string + "' is not valid";
        build_error(COMMAND_SET_OUTPUT, "INVALID_PIN", error_message, job);
    }
}

void command_get_pin_mode(unsigned int job, const String pin_string) {
    pin_t pin = get_valid_pin_type(pin_string);
    if (pin != PIN_INVALID_PIN) {
        pin_mode_t pin_mode = get_pin_mode(pin);

        build_command(
            COMMAND_GET_PIN_MODE,
            MSG_OUTPUT,
            job,
            "pin",
            pin_string,
            "mode",
            get_pin_mode_string(pin_mode));
    } else {
        String error_message = "Pin '" + pin_string + "' is not valid";
        build_error(COMMAND_GET_PIN_MODE, "INVALID_PIN", error_message, job);
    }
}

void command_set_pin_mode(
    unsigned int job, const String pin_string, const String mode_string) {
    pin_t pin = get_valid_pin_type(pin_string);

    if (pin != PIN_INVALID_PIN) {
        pin_mode_type_t pin_mode_type = get_pin_mode_type(pin);
        pin_mode_t pin_mode_target = get_valid_pin_mode(mode_string);
        switch (pin_mode_target) {
            case PIN_MODE_INPUT:
            case PIN_MODE_INPUT_PULLUP:
                if ((pin_mode_type == PIN_MODE_TYPE_INPUT_ONLY) ||
                    (pin_mode_type == PIN_MODE_TYPE_INPUT_AND_OUTPUT)) {
                    set_pin_mode(pin, pin_mode_target);
                    build_command(
                        COMMAND_SET_PIN_MODE,
                        MSG_OUTPUT,
                        job,
                        "pin",
                        pin_string,
                        "mode",
                        mode_string);
                } else {
                    String error_message = "Pin '" + pin_string + "' is a output only";
                    build_error(
                        COMMAND_SET_PIN_MODE, "INVALID_PIN_MODE", error_message, job);
                }
                break;

            case PIN_MODE_OUTPUT:
                if ((pin_mode_type == PIN_MODE_TYPE_OUTPUT_ONLY) ||
                    (pin_mode_type == PIN_MODE_TYPE_INPUT_AND_OUTPUT)) {
                    set_pin_mode(pin, pin_mode_target);

                    build_command(
                        COMMAND_SET_PIN_MODE,
                        MSG_OUTPUT,
                        job,
                        "pin",
                        pin_string,
                        "mode",
                        mode_string);
                } else {
                    String error_message = "Pin '" + pin_string + "' is a input only";
                    build_error(
                        COMMAND_SET_PIN_MODE, "INVALID_PIN_MODE", error_message, job);
                }
                break;

            default:
                break;
        }
    } else {
        String error_message = "Pin '" + pin_string + "' is not valid";
        build_error(COMMAND_SET_PIN_MODE, "INVALID_PIN", error_message, job);
    }
}

void command_trigger_pulse(unsigned int job, const String pin_string) {
    pin_t pin = get_valid_pin_type(pin_string);

    if (pin != PIN_INVALID_PIN) {
        pin_mode_t pin_mode = get_pin_mode(pin);
        if (pin_mode == PIN_MODE_OUTPUT) {
            trigger_pulse(pin);
            build_command(COMMAND_TRIGGER_PULSE, MSG_OUTPUT, job, "pin", pin_string);
        } else {
            String error_message = "Pin '" + pin_string + "' is not an output";
            build_error(COMMAND_TRIGGER_PULSE, "INVALID_OUTPUT_PIN", error_message, job);
        }
    } else {
        String error_message = "Pin '" + pin_string + "' is not valid";
        build_error(COMMAND_TRIGGER_PULSE, "INVALID_PIN", error_message, job);
    }
}

} // namespace controllino
