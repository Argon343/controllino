#ifndef CONTROLLINO_LOGGER_H
#define CONTROLLINO_LOGGER_H

#include "ProtocolHandler.h"

#define MAX_REQUESTS 8

namespace controllino {

void handle_logging_requests();
int log_signal(unsigned int job, pin_t pin, unsigned int period);
int end_log_signal(pin_t);

} // namespace controllino

#endif /* CONTROLLINO_LOGGER_H */
