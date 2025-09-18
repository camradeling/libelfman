/*
 * Auto-added header
 * File: memory_helpers.h
 * Author: camradeling
 * Email: camradeling@gmail.com
 * 2025
 */
//------------------------------------------------------------------------------------------------------------------------------
/*
 * Copyright (c) 2017, Matias Fontanini
 * All rights reserved.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions are
 * met:
 * 
 * * Redistributions of source code must retain the above copyright
 *   notice, this list of conditions and the following disclaimer.
 * * Redistributions in binary form must reproduce the above
 *   copyright notice, this list of conditions and the following disclaimer
 *   in the documentation and/or other materials provided with the
 *   distribution.
 * 
 * THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS
 * "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT
 * LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR
 * A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT
 * OWNER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL,
 * SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT
 * LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE,
 * DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY
 * THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
 * (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE
 * OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
 *
 */
//------------------------------------------------------------------------------------------------------------------------------
#ifndef ELFMAN_MEMORY_HELPERS_H
#define ELFMAN_MEMORY_HELPERS_H
//------------------------------------------------------------------------------------------------------------------------------
#include <stdint.h>
#include <cstring>
#include <vector>
#include "exceptions.h"
//------------------------------------------------------------------------------------------------------------------------------
#ifdef _MSC_VER
    // This is Visual Studio
    #define LIKELY(x) (x)
    #define UNLIKELY(x) (x)
#else
    // Not Visual Studio. Assume this is gcc compatible
    #define LIKELY(x) __builtin_expect((x),1)
    #define UNLIKELY(x) __builtin_expect((x),0)
#endif // _MSC_VER
//------------------------------------------------------------------------------------------------------------------------------
namespace ElfMan {
//------------------------------------------------------------------------------------------------------------------------------
namespace Memory {
//------------------------------------------------------------------------------------------------------------------------------
inline void read_data(const uint8_t* buffer, uint8_t* output_buffer, size_t size) {
    std::memcpy(output_buffer, buffer, size);
}
//------------------------------------------------------------------------------------------------------------------------------
template <typename T>
void read_value(const uint8_t* buffer, T& value) {
    std::memcpy(&value, buffer, sizeof(value));
}
//------------------------------------------------------------------------------------------------------------------------------
inline void write_data(uint8_t* buffer, const uint8_t* ptr, size_t size) {
    std::memcpy(buffer, ptr, size);
}
//------------------------------------------------------------------------------------------------------------------------------
template <typename T>
void write_value(uint8_t* buffer, const T& value) {
    std::memcpy(buffer, &value, sizeof(value));
}
//------------------------------------------------------------------------------------------------------------------------------
class InputMemoryStream {
public:
    InputMemoryStream(const uint8_t* buffer, size_t total_sz)
    : buffer_(buffer), size_(total_sz) {
    }

    InputMemoryStream(const std::vector<uint8_t>& data) : buffer_(&data[0]), size_(data.size()) {
    }
 
    template <typename T>
    T read() {
        T output;
        read(output);
        return output;
    }

    template <typename T>
    void read(T& value) {
        if (!can_read(sizeof(value))) {
            throw malformed_object();
        }
        read_value(buffer_, value);
        skip(sizeof(value));
    }

    void skip(size_t size) {
        if (UNLIKELY(size > size_)) {
            throw malformed_object();
        }
        buffer_ += size;
        size_ -= size;
    }

    bool can_read(size_t byte_count) const {
        return LIKELY(size_ >= byte_count);
    }

    void read(void* output_buffer, size_t output_buffer_size) {
        if (!can_read(output_buffer_size)) {
            throw malformed_object();
        }
        read_data(buffer_, (uint8_t*)output_buffer, output_buffer_size);
        skip(output_buffer_size);
    }

    const uint8_t* pointer() const {
        return buffer_;
    }

    size_t size() const {
        return size_;
    }

    void size(size_t new_size) {
        size_ = new_size;
    }

    operator bool() const {
        return size_ > 0;
    }

    void read(std::vector<uint8_t>& value, size_t count);
private:
    const uint8_t* buffer_;
    size_t size_;
};
//------------------------------------------------------------------------------------------------------------------------------
class OutputMemoryStream {
public:
    OutputMemoryStream(uint8_t* buffer, size_t total_sz)
    : buffer_(buffer), size_(total_sz) {
    }

    OutputMemoryStream(std::vector<uint8_t>& buffer)
    : buffer_(&buffer[0]), size_(buffer.size()) {
    }

    template <typename T>
    void write(const T& value) {
        if (UNLIKELY(size_ < sizeof(value))) {
            throw serialization_error();
        }
        write_value(buffer_, value);
        skip(sizeof(value));
    }

    template <typename ForwardIterator>
    void write(ForwardIterator start, ForwardIterator end) {
        const size_t length = std::distance(start, end); 
        if (UNLIKELY(size_ < length)) {
            throw serialization_error();
        }
        // VC doesn't like dereferencing empty vector's iterators so check this here
        if (UNLIKELY(length == 0)) {
            return;
        }
        std::memcpy(buffer_, &*start, length);
        skip(length);
    }

    void skip(size_t size) {
        if (UNLIKELY(size > size_)) {
            throw malformed_object();
        }
        buffer_ += size;
        size_ -= size;
    }

    void write(const uint8_t* ptr, size_t length) {
        write(ptr, ptr + length);
    }

    void fill(size_t size, uint8_t value) {
        if (UNLIKELY(size_ < size)) {
            throw serialization_error();
        }
        std::memset(buffer_, value, size);
        skip(size);
    }

    uint8_t* pointer() {
        return buffer_;
    }

    size_t size() const {
        return size_;
    }

private:
    uint8_t* buffer_;
    size_t size_;
};
//------------------------------------------------------------------------------------------------------------------------------
} // Memory
//------------------------------------------------------------------------------------------------------------------------------
} // ElfMan
//------------------------------------------------------------------------------------------------------------------------------
#endif // ELFMAN_MEMORY_HELPERS_H
