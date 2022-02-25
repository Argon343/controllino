#ifndef CONTROLLINO_MESSAGE_HANDLER_H
#define CONTROLLINO_MESSAGE_HANDLER_H

#include <Arduino.h>

namespace controllino {

void init_message_handler(void);
void command_ready();

} // namespace controllino

#endif /* CONTROLLINO_MESSAGE_HANDLER_H */
