#pragma once

#include <memory>

enum class command_type {
    list_sources,
    list_sinks,
};

class command_data;

class command {
    command_type type_;
    std::unique_ptr<command_data> data_;

public:
    command(command_type type, std::unique_ptr<command_data> &&data)
        : type_{type}, data_{std::move(data)} {}
    command_type type() const { return type_; }
    command_data *data() const { return data_.get(); }
};

class command_data {
public:
    virtual ~command_data() {}
};
