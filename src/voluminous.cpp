#include "channel.h"
#include "command.h"
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

    using iterator = std::vector<device_info>::iterator;
    using const_iterator = std::vector<device_info>::const_iterator;

    iterator begin() { return devices.begin(); }
    const_iterator begin() const { return devices.begin(); }
    iterator end() { return devices.end(); }
    const_iterator end() const { return devices.end(); }
};

struct devices {
    device_list sources{};
    device_list sinks{};
};

static void pulseaudio_mainloop(std::shared_ptr<devices> all_devices,
                                std::shared_ptr<mpsc<command>> channel) {
    pulse_api api{};
    bool subscribe_set{false};
    bool devices_init{false};
    std::chrono::microseconds timeout{500000};
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
            api.default_iterate();
            continue;
        }
        auto cmd = channel->recv_or_timeout(timeout);
        if (cmd) {
            switch (cmd->type()) {
            case command_type::list_sources:
                std::cout << "Sources:\n";
                for (const auto &device : all_devices->sources)
                    std::cout << device << "\n";
                break;
            case command_type::list_sinks:
                std::cout << "Sinks:\n";
                for (const auto &device : all_devices->sinks)
                    std::cout << device << "\n";
                break;
            }
        }
        api.nonblocking_iterate();
    }
}

int main() {
    auto all_devices = std::make_shared<devices>();
    auto channel = mpsc<command>::unbounded();
    std::thread pa_thread(pulseaudio_mainloop, all_devices, channel);
    for (;;) {
        std::cout << "Enter command\n";
        std::string comm;
        std::cin >> comm;
        if (comm == "sources") {
            channel->send(command(command_type::list_sources,
                                  std::make_unique<command_data>()));
        } else if (comm == "sinks") {
            channel->send(command(command_type::list_sinks,
                                  std::make_unique<command_data>()));
        }
        sleep(1);
    }
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
