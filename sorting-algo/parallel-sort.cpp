#include <algorithm>
#include <iostream>
#include <thread>
#include <vector>

using namespace std;

/*
Complexity:
- Work / total CPU time: O(n log n), same as normal merge sort.
- Best parallel wall-clock time: O((n log n) / p + n), where p is the number
  of worker threads. The final merge levels still require linear work.
- Extra space: O(n) for merge buffers across recursive calls.
- Thread count: O(p), capped by hardware_concurrency().
*/

void merge_ranges(vector<int>& nums, int left, int mid, int right) {
    vector<int> temp;
    temp.reserve(right - left + 1);

    int i = left;
    int j = mid + 1;

    while (i <= mid && j <= right) {
        if (nums[i] <= nums[j]) {
            temp.push_back(nums[i++]);
        } else {
            temp.push_back(nums[j++]);
        }
    }

    while (i <= mid) {
        temp.push_back(nums[i++]);
    }

    while (j <= right) {
        temp.push_back(nums[j++]);
    }

    copy(temp.begin(), temp.end(), nums.begin() + left);
}

void merge_ranges_inplace(vector<int>& nums, int left, int mid, int right) {
    int i = left;
    int j = mid + 1;

    while (i <= mid && j <= right) {
        if (nums[i] <= nums[j]) {
            i++;
            continue;
        }

        int value = nums[j];
        int index = j;

        while (index > i) {
            nums[index] = nums[index - 1];
            index--;
        }

        nums[i] = value;
        i++;
        mid++;
        j++;
    }
}

void parallel_merge_sort(vector<int>& nums, int left, int right, int depth) {
    if (left >= right) {
        return;
    }

    int mid = left + (right - left) / 2;

    if (depth > 0) {
        thread left_thread(parallel_merge_sort, ref(nums), left, mid, depth - 1);
        parallel_merge_sort(nums, mid + 1, right, depth - 1);
        left_thread.join();
    } else {
        parallel_merge_sort(nums, left, mid, 0);
        parallel_merge_sort(nums, mid + 1, right, 0);
    }

    merge_ranges(nums, left, mid, right);
}

void parallel_sort(vector<int>& nums) {
    if (nums.empty()) {
        return;
    }

    unsigned int thread_count = thread::hardware_concurrency();
    if (thread_count == 0) {
        thread_count = 2;
    }

    int depth = 0;
    while ((1u << depth) < thread_count) {
        depth++;
    }

    parallel_merge_sort(nums, 0, static_cast<int>(nums.size()) - 1, depth);
}

int main() {
    vector<int> nums = {9, 4, 7, 3, 10, 1, 8, 2, 6, 5};

    parallel_sort(nums);

    for (int num : nums) {
        cout << num << ' ';
    }
    cout << '\n';

    return 0;
}
