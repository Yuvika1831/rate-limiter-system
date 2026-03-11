#ifndef TOKEN_BUCKET_H
#define TOKEN_BUCKET_H

#include "rate_limiter.h"
#include <unordered_map>
#include <mutex>
#include <chrono>

class TokenBucketLimiter : public RateLimiter {

private:
    int bucket_size;
    int refill_rate;

    struct ClientData {
        double tokens;
        std::chrono::steady_clock::time_point last_refill;
    };

    std::unordered_map<std::string, ClientData> clients;
    std::mutex mtx;

public:
    TokenBucketLimiter(int size, int rate)
        : bucket_size(size), refill_rate(rate) {}

    bool allowRequest(const std::string& client_id) override {

        std::lock_guard<std::mutex> lock(mtx);

        auto now = std::chrono::steady_clock::now();

        if (clients.find(client_id) == clients.end()) {
            clients[client_id] = {bucket_size - 1, now};
            return true;
        }

        auto &data = clients[client_id];

        double elapsed = std::chrono::duration<double>(now - data.last_refill).count();

        double new_tokens = elapsed * refill_rate;

        data.tokens = std::min((double)bucket_size, data.tokens + new_tokens);

        data.last_refill = now;

        if (data.tokens >= 1) {
            data.tokens -= 1;
            return true;
        }

        return false;
    }

    std::string getName() const override {
        return "TokenBucket";
    }
};

#endif