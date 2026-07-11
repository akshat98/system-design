#include <chrono>
#include <iostream>
#include <string>

using namespace std;

/*
Question:
Design and implement a Leaky Bucket rate limiter.

The limiter should:
1. Allow a request if the bucket has available capacity.
2. Reject a request if the bucket is full.
3. Leak requests from the bucket at a fixed rate over time.

Implement the LeakyBucket class below.

Example:
LeakyBucket limiter(5, 2);
// capacity = 5 requests
// leakRatePerSecond = 2 requests per second

limiter.allowRequest("user-1");
*/

class LeakyBucket {
public:
    LeakyBucket(int capacity, int leakRatePerSecond) {
        // TODO: initialize bucket capacity, leak rate, current water level,
        // and last leak timestamp.
    }

    bool allowRequest(const string& userId) {
        // TODO:
        // 1. Leak old requests based on elapsed time.
        // 2. If bucket has capacity, add this request and return true.
        // 3. Otherwise return false.
        return false;
    }

private:
    void leak() {
        // TODO: reduce current bucket level based on elapsed time and leak rate.
    }

    int capacity;
    int leakRatePerSecond;
    int currentLevel;
    chrono::steady_clock::time_point lastLeakTime;
};

int main() {
    LeakyBucket limiter(5, 2);

    // TODO: Add test calls after implementing the class.
    cout << boolalpha;
    cout << limiter.allowRequest("user-1") << '\n';

    return 0;
}
