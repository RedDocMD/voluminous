#include "device.h"

std::ostream &operator<<(std::ostream &os, const device_info &info) {
    char buf[100];
    os << "(" << info.index << ")"
       << " " << info.name << ": " << info.description << " => "
       << pa_cvolume_snprint(buf, sizeof(buf), &info.volume) << "\n";
    return os;
}
