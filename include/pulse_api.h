#pragma once

#include <pulse/pulseaudio.h>

enum class pulse_api_status {
    not_ready,
    ready,
    failed,
};

struct pulse_api {
    pa_mainloop *mainloop;
    pa_mainloop_api *mainloop_api;
    pa_context *ctx;
    pulse_api_status status;

    pulse_api();
    ~pulse_api();
    void default_iterate();
    void nonblocking_iterate();
};

inline bool is_event_source(pa_subscription_event_type_t type,
                            int source_mask) {
    return (type & PA_SUBSCRIPTION_EVENT_FACILITY_MASK) == source_mask;
}

inline bool is_event_new(pa_subscription_event_type_t type) {
    return (type & PA_SUBSCRIPTION_EVENT_TYPE_MASK) ==
           PA_SUBSCRIPTION_EVENT_NEW;
}

inline bool is_event_remove(pa_subscription_event_type_t type) {
    return (type & PA_SUBSCRIPTION_EVENT_TYPE_MASK) ==
           PA_SUBSCRIPTION_EVENT_REMOVE;
}

inline bool is_event_change(pa_subscription_event_type_t type) {
    return (type & PA_SUBSCRIPTION_EVENT_TYPE_MASK) ==
           PA_SUBSCRIPTION_EVENT_CHANGE;
}
