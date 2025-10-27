```cpp
#include <iostream>
#include <thread>
#include <mutex>
#include <condition_variable>
#include <queue>
#include <functional>
#include <vector>
#include <chrono>
#include <atomic>

// ===============================
// Task Scheduler Implementation
// ===============================
class TaskScheduler {
public:
    using Task = std::function<void()>;

    TaskScheduler(size_t numThreads = std::thread::hardware_concurrency())
        : stopFlag(false) {
        for (size_t i = 0; i < numThreads; ++i) {
            workers.emplace_back(&TaskScheduler::workerLoop, this);
        }
    }

    ~TaskScheduler() {
        {
            std::unique_lock<std::mutex> lock(mtx);
            stopFlag = true;
        }
        cv.notify_all();

        for (auto& w : workers)
            if (w.joinable())
                w.join();
    }

    // Schedule a task for immediate execution
    void schedule(Task task) {
        scheduleAt(std::move(task), std::chrono::steady_clock::now());
    }

    // Schedule a task for delayed execution
    void scheduleAfter(Task task, std::chrono::milliseconds delay) {
        scheduleAt(std::move(task), std::chrono::steady_clock::now() + delay);
    }

private:
    struct ScheduledTask {
        std::chrono::steady_clock::time_point time;
        Task task;

        bool operator>(const ScheduledTask& other) const {
            return time > other.time;
        }
    };

    std::priority_queue<
        ScheduledTask,
        std::vector<ScheduledTask>,
        std::greater<>
    > taskQueue;  // min-heap ordered by time

    std::vector<std::thread> workers;
    std::mutex mtx;
    std::condition_variable cv;
    std::atomic<bool> stopFlag;

    // Core scheduling function
    void scheduleAt(Task task, std::chrono::steady_clock::time_point time) {
        bool wakeAll = false;
        {
            std::unique_lock<std::mutex> lock(mtx);
            // If the new task is earlier than the current top, we must wake everyone
            if (taskQueue.empty() || time < taskQueue.top().time)
                wakeAll = true;

            taskQueue.push({time, std::move(task)});
        }

        if (wakeAll)
            cv.notify_all();
        else
            cv.notify_one();
    }

    // Each worker runs its own event loop
    void workerLoop() {
        while (true) {
            Task currentTask;

            {
                std::unique_lock<std::mutex> lock(mtx);

                // Wait until there is a task or stop is requested
                cv.wait(lock, [this]() {
                    return stopFlag || !taskQueue.empty();
                });

                if (stopFlag && taskQueue.empty())
                    return;

                // Wait until the next task's time has arrived
                auto now = std::chrono::steady_clock::now();
                while (!taskQueue.empty() && taskQueue.top().time > now) {
                    cv.wait_until(lock, taskQueue.top().time, [this]() {
                        return stopFlag;
                    });

                    if (stopFlag)
                        return;
                    now = std::chrono::steady_clock::now();
                }

                if (!taskQueue.empty()) {
                    currentTask = std::move(taskQueue.top().task);
                    taskQueue.pop();
                }
            }

            if (currentTask)
                currentTask();  // Execute outside lock
        }
    }
};

// ===============================
// Example Usage
// ===============================
int main() {
    TaskScheduler scheduler(3);

    scheduler.schedule([] {
        std::cout << "Immediate task on thread "
                  << std::this_thread::get_id() << "\n";
    });

    scheduler.scheduleAfter([] {
        std::cout << "Task after 1 second on thread "
                  << std::this_thread::get_id() << "\n";
    }, std::chrono::seconds(1));

    scheduler.scheduleAfter([] {
        std::cout << "Task after 2 seconds on thread "
                  << std::this_thread::get_id() << "\n";
    }, std::chrono::seconds(2));

    std::this_thread::sleep_for(std::chrono::seconds(3));
    std::cout << "Main exiting\n";
}
```

---

### Adding support for periodic tasks

    What if in our Task scheduler design we wanted to add the support of scheduling a task after every t time? What would change?

🔥 Excellent follow-up — this is one of the most **common interview extensions** once you’ve built a solid Task Scheduler.

When an interviewer says:

> “Now let’s say you want to support *repeated* or *periodic* tasks — how would you modify your scheduler?”

They’re testing if you can reason about **recurring scheduling** and **state management** without breaking your existing concurrency guarantees.

Let’s break it down carefully 👇

---

## 🧩 Step 1: What “Periodic Task” Means

You currently support:

```cpp
scheduleAfter([]{ ... }, std::chrono::seconds(5));
```

✅ → Executes *once* after 5 seconds.

Now you want:

```cpp
scheduleEvery([]{ ... }, std::chrono::seconds(5));
```

✅ → Executes *every 5 seconds* until stopped (or until scheduler shuts down).

So we’re introducing **repetition**.

---

## 🧠 Step 2: How This Fits Into the Current Model

Our scheduler already has:

* A **priority queue** of `ScheduledTask`s ordered by next execution time.
* Worker threads that pop the earliest task, run it, and continue.

So the natural way to support repetition is:

1. When a **periodic task** is executed,
2. After execution, re-schedule it with a new execution time:

   ```
   new_time = now + period
   ```

That’s it — the *worker thread itself* can reinsert the task after running it.

---

## 🧱 Step 3: Extend the `ScheduledTask` Struct

Let’s add an optional **interval** field to track repeat frequency:

```cpp
struct ScheduledTask {
    std::chrono::steady_clock::time_point time;
    std::chrono::milliseconds interval; // 0 = one-time
    Task task;
    int priority = 0;

    bool operator>(const ScheduledTask& other) const {
        if (time == other.time)
            return priority < other.priority;
        return time > other.time;
    }

    bool isRepeating() const {
        return interval.count() > 0;
    }
};
```

---

## ⚙️ Step 4: Modify the Worker Loop

In the event loop, after executing the task,
check if it’s a *repeating* task and reinsert it:

```cpp
void workerLoop() {
    while (true) {
        Task currentTask;
        std::chrono::milliseconds repeatInterval{0};

        {
            std::unique_lock<std::mutex> lock(mtx);
            cv.wait(lock, [this] {
                return stopFlag || !taskQueue.empty();
            });

            if (stopFlag && taskQueue.empty())
                return;

            auto now = std::chrono::steady_clock::now();
            while (!taskQueue.empty() && taskQueue.top().time > now) {
                cv.wait_until(lock, taskQueue.top().time, [this]() {
                    return stopFlag;
                });
                if (stopFlag) return;
                now = std::chrono::steady_clock::now();
            }

            if (!taskQueue.empty()) {
                auto scheduled = std::move(taskQueue.top());
                taskQueue.pop();
                currentTask = std::move(scheduled.task);
                repeatInterval = scheduled.interval;

                // If periodic, reinsert before unlocking
                if (scheduled.isRepeating()) {
                    scheduled.time = std::chrono::steady_clock::now() + scheduled.interval;
                    taskQueue.push(std::move(scheduled));
                    cv.notify_all(); // because new top might have changed
                }
            }
        }

        if (currentTask)
            currentTask();
    }
}
```

---

## 🧩 Step 5: Add New API for Repeating Tasks

```cpp
void scheduleEvery(Task task, std::chrono::milliseconds interval, int priority = 0) {
    scheduleAt(std::move(task), std::chrono::steady_clock::now() + interval, priority, interval);
}
```

and extend your internal `scheduleAt` to accept that interval:

```cpp
void scheduleAt(Task task,
                std::chrono::steady_clock::time_point time,
                int priority = 0,
                std::chrono::milliseconds interval = std::chrono::milliseconds(0)) {
    bool wakeAll = false;
    {
        std::unique_lock<std::mutex> lock(mtx);
        if (taskQueue.empty() || time < taskQueue.top().time ||
            (time == taskQueue.top().time && priority > taskQueue.top().priority))
            wakeAll = true;

        taskQueue.push({time, interval, std::move(task), priority});
    }

    if (wakeAll)
        cv.notify_all();
    else
        cv.notify_one();
}
```

---

## 🧪 Step 6: Example Usage

```cpp
TaskScheduler scheduler(3);

scheduler.scheduleEvery([] {
    std::cout << "Heartbeat every 1 second: " << std::this_thread::get_id() << "\n";
}, std::chrono::seconds(1));

scheduler.scheduleAfter([] {
    std::cout << "Run once after 3 seconds\n";
}, std::chrono::seconds(3));

std::this_thread::sleep_for(std::chrono::seconds(6));
std::cout << "Main exiting\n";
```

Output:

```
Heartbeat every 1 second: 139972998727424
Heartbeat every 1 second: 139972998727424
Run once after 3 seconds
Heartbeat every 1 second: 139972998727424
...
Main exiting
```

