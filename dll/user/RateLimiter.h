#pragma once
#include <chrono>
#include <thread>
#include <unordered_map>
#include <mutex>

using namespace std;

class RateLimiter {
public:
    // Rate limiter for user connection requests (DOS attack detection)
    RateLimiter(int rate, chrono::seconds window_size)
        : rate_(rate), window_size_(window_size), running_(true) {
        // Start the cleanup thread
        cleanup_thread_ = thread(&RateLimiter::cleanupExpiredEntries, this);
    }

    ~RateLimiter() {
        // Stop the cleanup thread
        running_ = false;
        cleanup_thread_.join();
    }

    bool allowRequest(long long player_id) {
        lock_guard<mutex> lock(mutex_);

        // Get player entry or create a new one
        auto it = entries_.find(player_id);

        // If the entry was not found, create one
        if (it == entries_.end()) {
            auto result = entries_.insert({ player_id, { 0, chrono::steady_clock::now() } });
            it = result.first;
        }

        // Update counter and timestamp if window has moved
        auto& entry = it->second;
        auto now = chrono::steady_clock::now();
        if (now - entry.second > window_size_) {
            entry.first = 0;
            entry.second = now;
        }

        // Check remaining quota and update counter
        if (entry.first < rate_) {
            entry.first++;
            return true;
        }

        return false;
    }

private:
    int rate_;
    chrono::seconds window_size_;
    chrono::seconds clean_after_ = std::chrono::seconds(10);
    unordered_map<long long, pair<int, chrono::steady_clock::time_point>> entries_;
    thread cleanup_thread_;
    mutex mutex_;
    bool running_;

    // Cleanup map in intervals to prevent it from getting too large
    void cleanupExpiredEntries() {
        while (running_) {
            // Sleep for the cleanup interval
            this_thread::sleep_for(clean_after_);

            // Lock the map for cleanup
            lock_guard<mutex> lock(mutex_);

            // Get the current time
            auto now = chrono::steady_clock::now();

            // Iterate through entries and remove expired ones
            for (auto it = entries_.begin(); it != entries_.end();) {
                if (now - it->second.second > window_size_) {
                    it = entries_.erase(it);
                }
                else {
                    ++it;
                }
            }
        }
    }
};