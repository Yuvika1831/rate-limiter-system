#include <iostream>
#include <thread>
#include <vector>
#include <atomic>
#include <chrono>
#include <iomanip>
#include <unordered_map>

#include "fixed_window.h"
#include "token_bucket.h"

using namespace std;

atomic<int> total_requests(0);
atomic<int> allowed_requests(0);
atomic<int> rejected_requests(0);

mutex stats_mutex;

unordered_map<string, pair<int,int>> client_stats;

string timestamp() {

    auto now = chrono::system_clock::now();
    time_t t = chrono::system_clock::to_time_t(now);

    string s = ctime(&t);
    s.pop_back();

    return s;
}

void simulateClient(string client_id, RateLimiter &limiter, int requests) {

    for (int i = 0; i < requests; i++) {

        bool allowed = limiter.allowRequest(client_id);

        total_requests++;

        {
            lock_guard<mutex> lock(stats_mutex);

            if (allowed) {
                allowed_requests++;
                client_stats[client_id].first++;
            }
            else {
                rejected_requests++;
                client_stats[client_id].second++;
            }
        }

        cout << timestamp() << " | "
             << client_id << " | "
             << limiter.getName() << " | "
             << (allowed ? "ALLOWED" : "RATE_LIMITED")
             << endl;

        this_thread::sleep_for(chrono::milliseconds(50));
    }
}

int main() {

    int max_requests = 10;
    int window_seconds = 5;

    int bucket_size = 10;
    int refill_rate = 2;

    FixedWindowLimiter fixedLimiter(max_requests, window_seconds);

    vector<string> clients = {
        "client1",
        "client2",
        "client3",
        "client4",
        "client5"
    };

    vector<thread> threads;

    for (auto &c : clients) {
        threads.push_back(thread(simulateClient, c, ref(fixedLimiter), 25));
    }

    for (auto &t : threads) {
        t.join();
    }

    cout << "\n===== SUMMARY =====\n";

    cout << "Total Requests: " << total_requests << endl;
    cout << "Allowed: " << allowed_requests << endl;
    cout << "Rejected: " << rejected_requests << endl;

    cout << "\nPer Client Stats\n";

    for (auto &p : client_stats) {

        cout << p.first
             << " -> allowed: "
             << p.second.first
             << " rejected: "
             << p.second.second
             << endl;
    }

    return 0;
}