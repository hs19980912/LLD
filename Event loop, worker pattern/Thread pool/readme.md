# Thread pool

Perfect 👍 Let’s go step-by-step with a **concrete example** of how a **thread pool with multiple event loops** works — where **each thread runs its own event loop** to process tasks independently.

---

## 🧩 Problem Context

We want to:

* Have **multiple threads**, each running its **own event loop**.
* Distribute tasks among these event loops.
* Make it **thread-safe**, so no data races occur when multiple threads push tasks.

---

## ✅ Step 1: What’s an Event Loop?

Each event loop:

* Waits for tasks in a queue.
* When a task arrives, executes it.
* Repeats indefinitely until stopped.

Think of it as:

```cpp
void eventLoop() {
    while (!stop) {
        Task task = getNextTask();  // Wait for a task
        task();                     // Run it
    }
}
```

---

## ✅ Step 2: Where Thread-Safety Issues Arise

In the earlier single-thread version, we had something like this:

```cpp
std::queue<std::function<void()>> tasks;
```

But if multiple threads **push tasks** to this same queue concurrently, it’s **not thread safe**.

So we need a **mutex** to protect access.

---

## ✅ Step 3: Thread Pool with Multiple Event Loops

Let’s design:

* A central **ThreadPool** that owns multiple **EventLoops**.
* Each **EventLoop** runs on its **own thread**.
* When a new task comes, ThreadPool assigns it to one of the loops (e.g., round-robin).

---

### Example Code

```cpp
#include <iostream>
#include <vector>
#include <thread>
#include <queue>
#include <mutex>
#include <condition_variable>
#include <functional>
#include <atomic>

class EventLoop {
public:
    EventLoop() : stop(false) {}

    void start() {
        thread = std::thread([this]() { this->run(); });
    }

    void run() {
        while (!stop) {
            std::function<void()> task;

            // Wait for task
            {
                std::unique_lock<std::mutex> lock(mtx);
                cv.wait(lock, [this]() { return stop || !tasks.empty(); });

                if (stop && tasks.empty())
                    return;

                task = std::move(tasks.front());
                tasks.pop();
            }

            // Execute outside lock
            task();
        }
    }

    void post(std::function<void()> task) {
        {
            std::lock_guard<std::mutex> lock(mtx);
            tasks.push(std::move(task));
        }
        cv.notify_one();
    }

    void stopLoop() {
        {
            std::lock_guard<std::mutex> lock(mtx);
            stop = true;
        }
        cv.notify_all();
        if (thread.joinable()) thread.join();
    }

private:
    std::queue<std::function<void()>> tasks;
    std::mutex mtx;
    std::condition_variable cv;
    std::thread thread;
    bool stop;
};
```

---

## ✅ Step 4: ThreadPool Managing Multiple EventLoops

```cpp
class ThreadPool {
public:
    ThreadPool(size_t numThreads)
        : nextLoop(0)
    {
        for (size_t i = 0; i < numThreads; ++i) {
            loops.emplace_back(new EventLoop());
            loops.back()->start();
        }
    }

    void post(std::function<void()> task) {
        loops[nextLoop]->post(std::move(task));
        nextLoop = (nextLoop + 1) % loops.size(); // round-robin
    }

    void stop() {
        for (auto& loop : loops)
            loop->stopLoop();
    }

private:
    std::vector<std::unique_ptr<EventLoop>> loops;
    size_t nextLoop;
};
```

---

## ✅ Step 5: Using It

```cpp
int main() {
    ThreadPool pool(4);  // 4 threads, 4 event loops

    for (int i = 0; i < 10; ++i) {
        pool.post([i]() {
            std::cout << "Task " << i
                      << " executed in thread " 
                      << std::this_thread::get_id() << "\n";
        });
    }

    std::this_thread::sleep_for(std::chrono::seconds(1));
    pool.stop();
}
```

---

## ✅ Step 6: What’s Thread Safe Here?

* Each event loop’s **task queue** is **protected by its own mutex**.
* `ThreadPool::post()` distributes work safely.
* No shared data structure is modified by multiple threads at the same time.

---

## ✅ Step 7: Summary — Design Conceptually

| Component        | Responsibility                         | Thread Safety                 |
| ---------------- | -------------------------------------- | ----------------------------- |
| **EventLoop**    | Runs on one thread, owns queue         | Protected by mutex            |
| **ThreadPool**   | Distributes tasks to loops             | Uses round-robin index safely |
| **Task posting** | Multiple threads can safely post tasks | Protected per loop            |

---

Would you like me to extend this to a **network server** example next — where each event loop handles its own set of connections (like in `libevent` or `boost::asio`)?
