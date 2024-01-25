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
double run_experiment(int num_threads, int limit);


int main() {
  std::vector<double> runtime_tracker;
  int custom_limit = 1000000;
  int iter_cnt = 6;
  // Experiment with different thread counts
  std::vector<int> thread_counts = {1, 2, 4, 8, 16, 32, 64, 128, 256, 512, 1024};
  for (int threads : thread_counts) {
    double avg_runtime = 0.0;
    for (int i = 0; i < iter_cnt; i++) {
      double time = run_experiment(threads, custom_limit);
      avg_runtime += time;
      std::cout  << " | Test " << (i+1) << " | Speed: " << time << " microseconds\n";
    }
    avg_runtime /= iter_cnt; // Calculate average runtime
    runtime_tracker.push_back(avg_runtime);
    std::cout << "No. of Threads: " << threads << " | Avg. Runtime: " << avg_runtime << " microseconds\n\n" << std::endl;
  }
  // Print the results
  std::cout << "Average Runtime Array: \n" << std::endl;
  for (const auto& value : runtime_tracker) {
    std::cout << value << ", ";
  }
  std::cout << std::endl;

  return 0;
}

double run_experiment(int num_threads=1, int limit=LIMIT) {
  std::vector<int> primes;
  // Mutex used for locking prime list
  std::mutex primes_mutex;

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

  return duration;
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
