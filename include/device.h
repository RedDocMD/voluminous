#pragma once

#include <ostream>
#include <pulse/pulseaudio.h>
#include <string>
#include <vector>

struct device_info {
    std::string name;
    std::string description;
    std::uint32_t index;
    pa_cvolume volume;

    device_info(const std::string &name, const std::string &description,
                std::uint32_t index, pa_cvolume volume)
        : name{name}, description{description}, index{index}, volume{volume} {}

    device_info(const pa_source_info *info)
        : name{info->name}, description{info->description}, index{info->index},
          volume{info->volume} {}
    device_info(const pa_sink_info *info)
        : name{info->name}, description{info->description}, index{info->index},
          volume{info->volume} {}
};

std::ostream &operator<<(std::ostream &os, const device_info &info);

