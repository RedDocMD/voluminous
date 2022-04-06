#include "pulse_api.h"

static void set_state_cb(pa_context *ctx, void *data) {
    auto *status = static_cast<pulse_api_status *>(data);
    auto pa_status = pa_context_get_state(ctx);
    if (pa_status == PA_CONTEXT_READY)
        *status = pulse_api_status::ready;
    else if (pa_status == PA_CONTEXT_FAILED ||
             pa_status == PA_CONTEXT_TERMINATED)
        *status = pulse_api_status::failed;
}

pulse_api::pulse_api() {
    mainloop = pa_mainloop_new();
    mainloop_api = pa_mainloop_get_api(mainloop);
    ctx = pa_context_new(mainloop_api, "pulsar");
    status = pulse_api_status::not_ready;

    pa_context_connect(ctx, nullptr, static_cast<pa_context_flags_t>(0),
                       nullptr);
    pa_context_set_state_callback(ctx, set_state_cb, &status);
}

pulse_api::~pulse_api() {
    pa_context_disconnect(ctx);
    pa_context_unref(ctx);
    pa_mainloop_free(mainloop);
}

void pulse_api::default_iterate() {
    pa_mainloop_iterate(mainloop, true, nullptr);
}

