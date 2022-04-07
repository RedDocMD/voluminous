#include "channel.h"
#include "device.h"
#include "pulse_api.h"
#include <cassert>
#include <iostream>
#include <memory>
#include <thread>

static void subscribe_cb(pa_context *ctx, pa_subscription_event_type_t type,
                         uint32_t idx, void *data);
static void source_info_list_cb(pa_context *ctx, const pa_source_info *info,
                                int eol, void *data);
static void sink_info_list_cb(pa_context *ctx, const pa_sink_info *info,
                              int eol, void *data);

struct device_list {
    std::vector<device_info> devices{};
    bool updating{false};
};

struct devices {
    device_list sources{};
    device_list sinks{};
};

static void pulseaudio_mainloop(std::shared_ptr<devices> all_devices) {
    pulse_api api{};
    bool subscribe_set{false};
    bool devices_init{false};
    for (;;) {
        if (api.status == pulse_api_status::not_ready) {
            api.default_iterate();
            continue;
        }
        if (api.status == pulse_api_status::failed)
            break;
        if (!devices_init) {
            pa_context_get_source_info_list(api.ctx, source_info_list_cb,
                                            &all_devices->sources);
            pa_context_get_sink_info_list(api.ctx, sink_info_list_cb,
                                          &all_devices->sinks);
            devices_init = true;
        }
        if (!subscribe_set) {
            pa_context_set_subscribe_callback(api.ctx, subscribe_cb,
                                              all_devices.get());
            auto event_mask = static_cast<pa_subscription_mask>(
                PA_SUBSCRIPTION_MASK_SOURCE | PA_SUBSCRIPTION_MASK_SINK);
            pa_context_subscribe(api.ctx, event_mask, nullptr, nullptr);
            subscribe_set = true;
        }
        api.default_iterate();
    }
}

int main() {
    auto all_devices = std::make_shared<devices>();
    std::thread pa_thread(pulseaudio_mainloop, all_devices);
    pa_thread.join();
    return 0;
}

void subscribe_cb(pa_context *ctx, pa_subscription_event_type_t type,
                  uint32_t idx, void *data) {
    auto *all_devices = static_cast<devices *>(data);
    if (is_event_source(type, PA_SUBSCRIPTION_EVENT_SOURCE)) {
        // if (is_event_new(type))
        //     std::cout << "Source created\n";
        // else if (is_event_remove(type))
        //     std::cout << "Source removed\n";
        // else if (is_event_change(type))
        //     std::cout << "Source changed\n";
        pa_context_get_source_info_list(ctx, source_info_list_cb,
                                        &all_devices->sources);
    } else if (is_event_source(type, PA_SUBSCRIPTION_EVENT_SINK)) {
        // if (is_event_new(type))
        //     std::cout << "Sink created\n";
        // else if (is_event_remove(type))
        //     std::cout << "Sink removed\n";
        // else if (is_event_change(type))
        //     std::cout << "Sink changed\n";
        pa_context_get_sink_info_list(ctx, sink_info_list_cb,
                                      &all_devices->sinks);
    }
}

void source_info_list_cb(pa_context *ctx, const pa_source_info *info, int eol,
                         void *data) {
    auto *sources = static_cast<device_list *>(data);
    if (!sources->updating) {
        sources->devices.clear();
        sources->updating = true;
    }
    if (eol) {
        sources->updating = false;
        return;
    }
    sources->devices.emplace_back(info);
}

void sink_info_list_cb(pa_context *ctx, const pa_sink_info *info, int eol,
                       void *data) {
    auto *sinks = static_cast<device_list *>(data);
    if (!sinks->updating) {
        sinks->devices.clear();
        sinks->updating = true;
    }
    if (eol) {
        sinks->updating = false;
        return;
    }
    sinks->devices.emplace_back(info);
}
