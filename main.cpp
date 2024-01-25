#include <chrono>
#include <iostream>
#include <mutex>
#include <thread>
#include <vector>

#define LIMIT 10000000

/*
This function checks if an integer n is prime.

Parameters:
n : int - integer to check

Returns true if n is prime, and false otherwise.
*/
bool check_prime(const int &n);

int main() {
  std::vector<int> primes;
  int limit = LIMIT;
  int num_threads = 1;
  // Mutex used for locking prime list
  std::mutex primes_mutex;

  // Get inputs
  std::cout << "Enter the upper bound of integers to check: ";
  std::cin >> limit;
  std::cout << "Enter the number of threads to use: ";
  std::cin >> num_threads;
  // Timer starts after input is collected
  auto start_time = std::chrono::high_resolution_clock::now();

  // This lambda function is what each thread calls to check for prime. Lambda
  // function is used so we don't have to pass the shared variables manually.
  // [&] is a lambda capture by reference any variable in the scope of the
  // thread. We use this to be able to access the shared variable `primes` and
  // `primes_mutex`
  auto thread_function = [&](int start, int end) {
    // [start, end] inclusive check for prime
    for (int current_num = start; current_num <= end; current_num++) {
      if (check_prime(current_num)) {
        std::lock_guard<std::mutex> guard(primes_mutex);
        primes.push_back(current_num);
      }
    }
  };

  // Create threads
  // Make sure the number of threads is less than the number of primes to check
  // or else the calculation for splitting of work will not work.
  // We should be able to assign at least one prime for every thread.
  if (num_threads > limit)
    num_threads = limit;
  // Create a variable `threads` that is a vector of size num_threads
  // contatining std::thread objects
  std::vector<std::thread> threads(num_threads);

  int split_size = limit / num_threads;
  for (int i = 0; i < num_threads; i++) {
    int start =
        i * split_size + 2; // +2 because we don't check if 0 or 1 is prime
    // if last thread, check till limit; else check split_size -1. The -1 is to
    // remove overlap between previous iteration. This assumes start and end are
    // inclusive (ex. if start=2 and end=4, [2, 3, 4] is the range of numbers to
    // check).
    int end = (i == num_threads - 1) ? limit : start + split_size - 1;
    // Create and start threads, arg1:function to execute, other_args:paramters
    // to pass to arg1 function.
    threads[i] = std::thread(
        thread_function, start,
        end); // Replace existing threads in vector with new threads. The old
              // threads were just there to initialize the vector size.
  }

  // Join threads
  for (auto &th : threads) {
    // check if a thread has finished executing code
    if (th.joinable()) {
      // join if done
      th.join();
    }
  }

  // Timer ends befpre output is printed
  auto end_time = std::chrono::high_resolution_clock::now();
  auto duration = std::chrono::duration_cast<std::chrono::microseconds>(end_time - start_time).count();

  std::cout << primes.size() << " primes were found." << std::endl;

  std::cout << "Runtime is " << duration << " microseconds." << std::endl;

  return 0;
}

// This function checks if a specific number is prime
bool check_prime(const int &n) {
  for (int i = 2; i * i <= n; i++) {
    if (n % i == 0) {
      return false;
    }
  }
  return true;
}