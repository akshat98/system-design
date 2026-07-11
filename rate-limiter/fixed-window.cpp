#include <bits/stdc++.h>
using namespace std;

class TokenLimiter {
    struct Usage {
        long long bucket; // Current time window
        long long used;   // Tokens used in this window
    };

    long long limit;
    long long windowSize;
    unordered_map<string, Usage> mp;

public:
    TokenLimiter(long long tokenLimit, long long window) {
        limit = tokenLimit;
        windowSize = window;
    }

    bool allow(string key, long long cost, long long time) {
        long long bucket = time / windowSize;
        auto it = mp.find(key);

        if (it == mp.end() || it->second.bucket != bucket) {
            mp[key] = {bucket, 0};
            it = mp.find(key);
        }

        if (it->second.used + cost > limit)
            return false;

        it->second.used += cost;
        return true;
    }

    void reset(string key) {
        mp.erase(key);
    }
};
