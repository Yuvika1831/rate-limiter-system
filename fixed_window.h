#ifndef FIXED_WINDOW_H
#define FIXED_WINDOW_H

#include "rate_limiter.h"
#include <unordered_map>
#include <mutex>
#include <chrono>

class FixedWindowLimiter : public RateLimiter {

private:
    int max_requests;
    int window_seconds;

    struct ClientData {
        int count;
        std::chrono::steady_clock::time_point window_start;
    };

    std::unordered_map<std::string, ClientData> clients;
    std::mutex mtx;

public:
    FixedWindowLimiter(int max_req, int window_sec)
        : max_requests(max_req), window_seconds(window_sec) {}

    bool allowRequest(const std::string& client_id) override {

        std::lock_guard<std::mutex> lock(mtx);

        auto now = std::chrono::steady_clock::now();

        if (clients.find(client_id) == clients.end()) {
            clients[client_id] = {1, now};
            return true;
        }

        auto &data = clients[client_id];

        auto elapsed = std::chrono::duration_cast<std::chrono::seconds>(
            now - data.window_start).count();

        if (elapsed >= window_seconds) {
            data.count = 1;
            data.window_start = now;
            return true;
        }

        if (data.count < max_requests) {
            data.count++;
            return true;
        }

        return false;
    }

    std::string getName() const override {
        return "FixedWindow";
    }
};

#endif