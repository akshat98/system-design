#include <algorithm>
#include <chrono>
#include <iostream>
#include <optional>
#include <string>
#include <unordered_map>

using namespace std;

/*
Question:
Design and implement a Token Bucket rate limiter backed by a distributed cache.

DistributedCache contract:
- get(key): returns the stored object, or empty if the key is missing.
- put(key, value): stores the value for the key.

TokenBucket fields:
- tokens: tokens currently available.
- last_refill_time: unix timestamp of last refill.
- capacity: max tokens the bucket can hold.
- refill_rate: tokens added per second.

Implement two functions:

1. refill_token_bucket(bucket, current_time)
   Add elapsed * refill_rate tokens, capped at capacity, and advance
   last_refill_time.

   If current_time <= last_refill_time due to clock skew, return the bucket
   unchanged.

2. allow_request(cache, user_id, tokens_requested = 1, capacity = 100,
                 refill_rate = 10.0, current_time = now)
   Key = "rate_limit:" + user_id.

   A brand-new user starts with a FULL bucket.

   Refill on read, then:
   - if tokens >= tokens_requested, deduct tokens_requested, save, return true.
   - otherwise, save the refilled bucket anyway so credit keeps accruing,
     return false.

Expected examples:
allow_request(cache, "user_123") // true: new bucket starts full with 100 tokens
// spend the remaining 99 quickly ...
allow_request(cache, "user_123") // false: 0 tokens left
// wait 1s at refill_rate = 10.0 -> +10 tokens
allow_request(cache, "user_123") // true

Refill cases:
refill: tokens = 50, rate = 10, +1s -> 60
refill: tokens = 95, rate = 10, +1s -> 100 (capped, not 105)
refill: tokens = 0, rate = 10, +100s -> 100 (capped)
refill: same timestamp -> unchanged
*/

constexpr int DEFAULT_CAPACITY = 100;
constexpr double DEFAULT_REFILL_RATE = 10.0;

struct TokenBucket {
    double tokens;
    double last_refill_time;
    int capacity;
    double refill_rate;
};

class DistributedCache {
public:
    optional<TokenBucket> get(const string& key) {
        auto it = data.find(key);
        if (it == data.end()) {
            return nullopt;
        }
        return it->second;
    }

    void put(const string& key, const TokenBucket& value) {
        data[key] = value;
    }

private:
    unordered_map<string, TokenBucket> data;
};

double current_unix_time() {
    auto now = chrono::system_clock::now();
    auto duration = now.time_since_epoch();
    return chrono::duration<double>(duration).count();
}

TokenBucket refill_token_bucket(TokenBucket bucket, double current_time) {
    if (current_time <= bucket.last_refill_time) {
        return bucket;
    }

    double elapsed = current_time - bucket.last_refill_time;
    double tokens_to_add = elapsed * bucket.refill_rate;

    bucket.tokens = min(static_cast<double>(bucket.capacity), bucket.tokens + tokens_to_add);
    bucket.last_refill_time = current_time;

    return bucket;
}

bool allow_request(
    DistributedCache& cache,
    const string& user_id,
    int tokens_requested = 1,
    int capacity = DEFAULT_CAPACITY,
    double refill_rate = DEFAULT_REFILL_RATE,
    double current_time = current_unix_time()
) {
    string key = "rate_limit:" + user_id;
    optional<TokenBucket> cached_bucket = cache.get(key);

    TokenBucket bucket = cached_bucket.value_or(TokenBucket{
        static_cast<double>(capacity),
        current_time,
        capacity,
        refill_rate
    });

    bucket = refill_token_bucket(bucket, current_time);

    if (bucket.tokens >= tokens_requested) {
        bucket.tokens -= tokens_requested;
        cache.put(key, bucket);
        return true;
    }

    cache.put(key, bucket);
    return false;
}

int main() {
    DistributedCache cache;

    cout << boolalpha;

    /*
    Test cases to validate after implementation:

    1. New user starts with a full bucket.
       allow_request(cache, "user_123") -> true

    2. Same user can spend the remaining 99 tokens immediately.
       The next request after 100 total allowed requests should return false.

    3. Refill after time passes.
       If refill_rate = 10.0 and 1 second passes, the bucket gains 10 tokens.
       allow_request(cache, "user_123") -> true

    4. Refill is capped by capacity.
       tokens = 50, rate = 10, +1s -> 60
       tokens = 95, rate = 10, +1s -> 100, not 105
       tokens = 0, rate = 10, +100s -> 100

    5. Clock skew does not change the bucket.
       If current_time <= last_refill_time, return the bucket unchanged.
    */

    // Example calls:
    cout << allow_request(cache, "user_123") << '\n';
    cout << allow_request(cache, "user_123") << '\n';

    return 0;
}
