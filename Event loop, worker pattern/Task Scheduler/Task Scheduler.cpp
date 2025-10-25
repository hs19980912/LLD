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
        {
            std::unique_lock<std::mutex> lock(mtx);
            taskQueue.push({time, std::move(task)});
        }
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
