#include "Logger.h"

#include <Arduino.h>

#include "GpioHandler.h"

namespace controllino {

struct Data {
    unsigned int time;
    int value;
};

class LoggingRequest {
public:
    LoggingRequest() = default;
    LoggingRequest(unsigned int job, pin_t pin, unsigned int period) :
        job_{job}, pin_{pin}, pin_type_{get_pin_type(pin)}, period_{period} {
    }

    unsigned int job() const {
        return job_;
    }

    pin_t pin() const {
        return pin_;
    }

    bool done() const {
        return done_;
    }

    void close() {
        close_ = true;
    }

    Data read() {
        last_read_ = millis();
        first_pass_ = false;
        int value;
        if (pin_type_ == PIN_DIGITAL) {
            value = read_digital_from_pin(pin_);
        } else {
            value = read_analog_from_pin(pin_);
        }

        if (close_) {
            done_ = true;
        }

        return Data{last_read_, value};
    }

    bool ready() const {
        if (first_pass_ or last_read_ + period_ < millis()) {
            return true;
        }
        return false;
    }

private:
    unsigned int job_{};
    pin_t pin_{};
    pin_type_t pin_type_{};
    unsigned int period_{}; // in number of interrupts
    unsigned int last_read_ = 0;
    bool first_pass_ = true; // FIXME Slow?
    bool done_ = false;
    bool close_ = false;
};

static LoggingRequest requests_[MAX_REQUESTS];
static unsigned int request_count_ = 0;

namespace details {

void del_request(unsigned int position) {
    for (unsigned int i = position; i < request_count_ - 1; ++i) {
        requests_[i] = requests_[i + 1];
    }
    request_count_--;
}

} // namespace details

void handle_logging_requests() {
    for (unsigned int i = 0; i < request_count_; ++i) {
        if (requests_[i].ready()) {
            auto p = requests_[i].read();
            build_command(
                COMMAND_LOG_SIGNAL,
                MSG_OUTPUT,
                requests_[i].job(),
                "time",
                p.time,
                "value",
                p.value,
                "done",
                requests_[i].done());
        }
    }

    // Remove marked requests. We're using a while loop because we remove
    // entries from the array as we iterate thru it.
    unsigned int index = 0;
    while (index < request_count_) {
        if (requests_[index].done()) {
            details::del_request(index);
        } else {
            index++;
        }
    }
}

// period in ms.
int log_signal(unsigned int job, pin_t pin, unsigned int period) {
    if (request_count_ == MAX_REQUESTS) {
        return 1; // Error - too many requests.
    }

    for (unsigned int i = 0; i < request_count_; ++i) {
        // Already have a logging job for this pin.
        if (requests_[i].pin() == pin) {
            return 2;
        }
    }

    period = period; // period in ms
    requests_[request_count_] = LoggingRequest{job, pin, period};
    request_count_++;
    return 0;
}

int end_log_signal(pin_t pin) {
    for (unsigned int i = 0; i < request_count_; ++i) {
        if (requests_[i].pin() == pin) {
            requests_[i].close();
            return 0; // Only one logging request per pin allowed!
        }
    }
    return 1; // Found no match!
}

} // namespace controllino
