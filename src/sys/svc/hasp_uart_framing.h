// MIT License. Generic newline transport; no application/widget mappings.
#pragma once
#include <stddef.h>
#include <stdint.h>
#include <string.h>

namespace hasp_uart {
class Receiver {
public:
    // 1022 command bytes + optional CR + NUL. LF is not stored.
    static const size_t limit = 1023;
    bool feed(uint8_t c) {
        if(c == '\n') {
            bool valid = !dropping_;
            if(size_ && data_[size_ - 1] == '\r') --size_;
            valid = valid && size_ > 0 && size_ <= 1022 && !memchr(data_, '\r', size_);
            data_[size_] = 0;
            size_ = 0;
            dropping_ = false;
            return valid;
        }
        if(c == 0 || (c < 32 && c != '\r' && c != '\t') || size_ == limit) dropping_ = true;
        if(!dropping_) data_[size_++] = static_cast<char>(c);
        return false;
    }
    const char* line() const { return data_; }
private:
    char data_[limit + 1] = {};
    size_t size_ = 0;
    bool dropping_ = false;
};

// Callers serialize access. Writes can be drained in small nonblocking chunks.
class Outbox {
public:
    static const size_t depth = 8;
    static const size_t limit = 512; // Maximum frame length before the terminating LF.
    void ready() {
        head_ = count_ = offset_ = 0;
        static const char marker[] = "\nready 1\n";
        memcpy(frames_[0], marker, sizeof(marker) - 1);
        lengths_[0] = sizeof(marker) - 1;
        count_ = 1;
    }
    bool state(const char* topic, const char* payload) { return frame(topic, payload, false); }
    bool event(const char* topic, const char* name) { return frame(topic, name, true); }
    bool frame(const char* topic, const char* payload, bool event) {
        const size_t prefix = event ? 6 : 0;
        size_t tn = bounded(topic, limit), pn = bounded(payload, limit);
        if(!tn || tn > limit || pn > limit || prefix + tn + 1 + pn > limit ||
           strpbrk(topic, " \t\r\n") || strpbrk(payload, event ? " \t\r\n" : "\r\n")) { ++dropped; return false; }
        // Recover a lost release by cancelling the peer's active touch and
        // requesting a refresh. Never transmit a suffix as a separate frame.
        if(count_ == depth) { ++dropped; ready(); return false; }
        size_t tail = (head_ + count_) % depth;
        char* line = frames_[tail];
        if(event) memcpy(line, "event ", prefix);
        memcpy(line + prefix, topic, tn);
        line[prefix + tn] = ' ';
        memcpy(line + prefix + tn + 1, payload, pn);
        line[prefix + tn + 1 + pn] = '\n';
        lengths_[tail] = prefix + tn + 2 + pn;
        ++count_;
        return true;
    }
    const char* data() const { return count_ ? frames_[head_] + offset_ : nullptr; }
    size_t size() const { return count_ ? lengths_[head_] - offset_ : 0; }
    void consume(size_t n) {
        if(n > size()) n = size();
        offset_ += n;
        if(count_ && offset_ == lengths_[head_]) { head_ = (head_ + 1) % depth; --count_; offset_ = 0; }
    }
    uint32_t dropped = 0;
private:
    static size_t bounded(const char* p, size_t max) {
        size_t n = 0; while(n <= max && p[n]) ++n; return n;
    }
    char frames_[depth][limit + 1] = {};
    size_t lengths_[depth] = {};
    size_t head_ = 0, count_ = 0, offset_ = 0;
};
} // namespace hasp_uart
