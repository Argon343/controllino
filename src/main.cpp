#include <Arduino.h>

#include "GpioHandler.h"
#include "Logger.h"
#include "MessageHandler.h"
#include "SerialHandler.h"

using namespace controllino;

void setup() {
    serial_init();
    load_pin_modes();
    init_message_handler();

    command_ready();
}

void loop() {
    serial_process();
    handle_logging_requests();
}
