#include "SerialHandler.h"

namespace controllino {

struct Callback {
    Callback(void (*f)(void*) = 0, void* d = 0) : function(f), data(d) {
    }
    void (*function)(void*);
    void* data;
};

String string_input = "";
String string_buffer = "";
bool string_complete = false;
Callback message_callback;

void serial_init(void) {
    string_input.reserve(200);
    string_input = "";
    string_buffer.reserve(200);
    string_buffer = "";

    Serial.begin(19200);
}

void serial_set_callback(void (*function)(void*)) {
    message_callback = Callback(function, &string_input);
}

void serial_process(void) {
    if (string_complete) {
        if (message_callback.function != NULL) {
            message_callback.function(message_callback.data);
        }
        string_complete = false;
    }
}

void serial_print_message(const String& string_to_print) {
    Serial.println(string_to_print);
}

} // namespace controllino

// Documentation incorrectly states that `serialEvent` doesn't work on
// Arduino Due. See https://github.com/arduino/reference-en/issues/819
// for details.
void serialEvent() {
    while (Serial.available()) {
        char inChar = (char) Serial.read();
        if (inChar == '\n') {
            controllino::string_complete = true;
            controllino::string_input = controllino::string_buffer;
            controllino::string_buffer = "";
        } else {
            controllino::string_buffer += inChar;
        }
    }
}
