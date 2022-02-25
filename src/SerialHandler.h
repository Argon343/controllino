#ifndef CONTROLLINO_SERIAL_HANDLER_H
#define CONTROLLINO_SERIAL_HANDLER_H

#include <Arduino.h>
#include <ArduinoJson.h>

namespace controllino {

void serial_init(void);
void serial_set_callback(void (*function)(void*));
void serial_process(void);
void serial_print_message(const String& string_to_print);

} // namespace controllino

#endif /* CONTROLLINO_SERIAL_HANDLER_H */
