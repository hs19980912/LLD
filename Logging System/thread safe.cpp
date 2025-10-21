// logging_threadsafe.cpp
#include <atomic>
#include <chrono>
#include <condition_variable>
#include <fstream>
#include <iomanip>
#include <iostream>
#include <memory>
#include <mutex>
#include <queue>
#include <sstream>
#include <string>
#include <thread>

//
// Basic Logger interface
//
class Logger {
public:
    virtual ~Logger() = default;
    virtual void log(const std::string& msg) = 0;
    // optional: flush() virtual
};

//
// ConsoleLogger - simple thread-safe (mutex-protected) console writer
//
class ConsoleLogger : public Logger {
    std::mutex cout_mtx;
public:
    void log(const std::string& msg) override {
        std::lock_guard<std::mutex> lk(cout_mtx);
        std::cout << "[Console] " << msg << std::endl;
    }
};

//
// RotationPolicy interface (keeps it simple)
//
class RotationPolicy {
public:
    virtual ~RotationPolicy() = default;
    virtual bool shouldRotate() = 0;
    virtual std::string nextFileName() = 0;
};

//
// Time-based rotation (rotate every N seconds). The policy will be used
// exclusively by the background worker thread, so it doesn't need internal locking.
//
class TimeBasedRotationPolicy : public RotationPolicy {
    std::chrono::seconds interval;
    std::chrono::system_clock::time_point lastRotation;
public:
    TimeBasedRotationPolicy(std::chrono::seconds secs)
        : interval(secs), lastRotation(std::chrono::system_clock::now()) {}

    bool shouldRotate() override {
        auto now = std::chrono::system_clock::now();
        if (now - lastRotation >= interval) {
            lastRotation = now;
            return true;
        }
        return false;
    }

    std::string nextFileName() override {
        auto t = std::chrono::system_clock::to_time_t(std::chrono::system_clock::now());
        std::tm tm;
        localtime_r(&t, &tm);
        char buf[64];
        strftime(buf, sizeof(buf), "log_%Y%m%d_%H%M%S.txt", &tm);
        return std::string(buf);
    }
};

//
// AsyncFileLogger - non-blocking log() that enqueues and returns.
// A background thread does actual file I/O and rotation.
//
class AsyncFileLogger : public Logger {
    struct Item {
        std::string message;
        std::chrono::system_clock::time_point ts;
    };

    std::unique_ptr<RotationPolicy> rotationPolicy;
    std::ofstream outFile;
    std::string currentFile;

    std::mutex queue_mtx;
    std::condition_variable queue_cv;
    std::queue<Item> q;
    bool stopping = false;
    std::thread worker;

    // configuration
    size_t maxQueueSize = 10000; // bounded queue for safety

    // internal helper: open file using rotation policy
    void rotateNow() {
        if (outFile.is_open()) outFile.close();
        currentFile = rotationPolicy->nextFileName();
        outFile.open(currentFile, std::ios::app);
        // Could add error handling if open fails
    }

    // background worker function
    void run() {
        while (true) {
            std::unique_lock<std::mutex> lk(queue_mtx);
            queue_cv.wait(lk, [this] { return stopping || !q.empty(); });   // Always blocks until predicate is true

            // drain queue (batching)
            while (!q.empty()) {
                Item it = std::move(q.front());
                q.pop();
                lk.unlock();

                // rotation decision is made in worker thread only
                if (rotationPolicy->shouldRotate()) {
                    rotateNow();
                }

                if (!outFile.is_open()) {
                    // try opening if not open
                    rotateNow();
                }

                // write with timestamp
                auto t = std::chrono::system_clock::to_time_t(it.ts);
                std::tm tm; localtime_r(&t, &tm);
                char timebuf[32];
                strftime(timebuf, sizeof(timebuf), "%Y-%m-%d %H:%M:%S", &tm);

                outFile << "[" << timebuf << "] " << it.message << "\n";

                // optional: flush periodically or based on message count
                // outFile.flush();

                lk.lock();
            }

            if (stopping && q.empty()) {
                // flush & close file before exit
                if (outFile.is_open()) {
                    outFile.flush();
                    outFile.close();
                }
                return;  // NOTE: This return statement will only be called once in the enitre program lifetime.
            }
        }
    }

public:
    AsyncFileLogger(std::unique_ptr<RotationPolicy> policy)
        : rotationPolicy(std::move(policy)), worker(&AsyncFileLogger::run, this)  // NOTE: thread's lifetime is directly tied to the execution of its entry function (run() in this case, When a thread function returns, the thread terminates/exits) 
    {
        // open initial file
        currentFile = rotationPolicy->nextFileName();
        outFile.open(currentFile, std::ios::app);
    }

    ~AsyncFileLogger() override {
        // stop worker gracefully
        {
            std::lock_guard<std::mutex> lk(queue_mtx);
            stopping = true;
        }
        queue_cv.notify_one();
        if (worker.joinable()) worker.join();   // Always blocks until thread returns
    }

    // non-blocking (pushes to queue)
    void log(const std::string& msg) override {
        std::lock_guard<std::mutex> lk(queue_mtx);  // Blocks if another thread holds it
        if (q.size() >= maxQueueSize) {
            // simple drop policy: drop oldest or newest
            // here: drop the newest (ignore this message)
            // alternatives: pop oldest, block, or expand queue
            return;
        }
        q.push(Item{msg, std::chrono::system_clock::now()});
        queue_cv.notify_one();
    }
};

//
// LogManager (Singleton) - global entrypoint
// Uses atomic_load/store for the shared_ptr so swapping is thread-safe.
//
class LogManager {
private:
    std::shared_ptr<Logger> logger; // protected by atomic operations
    // If atomic_load/store for shared_ptr are not available, use a mutex here.

    LogManager() {
        // default to console or file as default
        std::shared_ptr<Logger> def = std::make_shared<ConsoleLogger>();
        std::atomic_store(&logger, def);
    }
public:
    LogManager(const LogManager&) = delete;
    LogManager& operator=(const LogManager&) = delete;

    static LogManager& getInstance() {
        static LogManager instance;
        return instance;
    }

    // atomically swap logger
    void setLogger(std::shared_ptr<Logger> newLogger) {
        std::atomic_store(&logger, newLogger);
    }

    // one-line log API: make a snapshot copy atomically then call it
    void log(const std::string& msg) {
        auto snap = std::atomic_load(&logger);
        if (snap) snap->log(msg);
    }
};

//
// Example usage
//
int main() {
    // choose a time based rotation every 5 seconds
    auto fileLogger = std::make_shared<AsyncFileLogger>(
        std::make_unique<TimeBasedRotationPolicy>(std::chrono::seconds(5))
    );

    LogManager::getInstance().setLogger(fileLogger);

    // multiple threads logging concurrently
    std::thread t1([] {
        for (int i = 0; i < 50; ++i) {
            LogManager::getInstance().log("thread1 msg " + std::to_string(i));
            std::this_thread::sleep_for(std::chrono::milliseconds(120));
        }
    });

    std::thread t2([] {
        for (int i = 0; i < 50; ++i) {
            LogManager::getInstance().log("thread2 msg " + std::to_string(i));
            std::this_thread::sleep_for(std::chrono::milliseconds(90));
        }
    });

    t1.join();  // Always blocks until thread returns
    t2.join();  // Always blocks until thread returns

    // destructor will flush and join worker
    return 0;
}
