# **Logging Framework**

Design a **Logging Framework** that:

* Supports multiple logging types:  
    - Console  
    - Disk (with rotation policy)  
    - Database (can be added later)
* Provides a **single global access point** to logging (→ Singleton)
* Lets the system **create different loggers** easily (→ Factory)
* Extensible (e.g., rotation based on time or file size)

---

## 🧱 **Step 1 — Define the Abstraction (Interface)**

Start with a **base class `Logger`**, defining the common interface:

```cpp
class Logger {
public:
    virtual ~Logger() = default;
    virtual void log(const std::string& message) = 0;
};
```

All loggers (Console, File, DB) will implement this.

---

## ⚙️ **Step 2 — Concrete Implementations**

### 1️⃣ Console Logger

```cpp
#include <iostream>

class ConsoleLogger : public Logger {
public:
    void log(const std::string& message) override {
        std::cout << "[Console] " << message << std::endl;
    }
};
```

---

### 2️⃣ File Logger with Rotation Policy

We’ll make this extensible — rotation can be **time-based** or **size-based**.

```cpp
#include <fstream>
#include <chrono>
#include <ctime>
#include <filesystem>

class RotationPolicy {
public:
    virtual ~RotationPolicy() = default;
    virtual bool shouldRotate() = 0;
    virtual std::string nextFileName() = 0;
};
```

#### Example — Time-Based Policy

```cpp
class TimeBasedRotationPolicy : public RotationPolicy {
    std::chrono::system_clock::time_point lastRotation;
    int intervalSeconds;
public:
    TimeBasedRotationPolicy(int seconds) : intervalSeconds(seconds) {
        lastRotation = std::chrono::system_clock::now();
    }

    bool shouldRotate() override {
        auto now = std::chrono::system_clock::now();
        auto diff = std::chrono::duration_cast<std::chrono::seconds>(now - lastRotation).count();
        if (diff >= intervalSeconds) {
            lastRotation = now;
            return true;
        }
        return false;
    }

    std::string nextFileName() override {
        auto now = std::time(nullptr);
        char buf[64];
        strftime(buf, sizeof(buf), "log_%Y%m%d_%H%M%S.txt", std::localtime(&now));
        return buf;
    }
};
```

#### File Logger using Rotation Policy

```cpp
class FileLogger : public Logger {
    std::ofstream outFile;
    std::unique_ptr<RotationPolicy> rotationPolicy;
    std::string currentFile;

public:
    FileLogger(std::unique_ptr<RotationPolicy> policy)
        : rotationPolicy(std::move(policy)) {
        currentFile = rotationPolicy->nextFileName();
        outFile.open(currentFile, std::ios::app);
    }

    void log(const std::string& message) override {
        if (rotationPolicy->shouldRotate()) {
            outFile.close();
            currentFile = rotationPolicy->nextFileName();
            outFile.open(currentFile, std::ios::app);
        }
        outFile << "[File] " << message << std::endl;
    }
};
```

---

### 3️⃣ Database Logger (stub)

```cpp
class DatabaseLogger : public Logger {
public:
    void log(const std::string& message) override {
        std::cout << "[Database] " << message << std::endl;
        // In real systems, send to DB connection
    }
};
```

---

## 🏭 **Step 3 — Add the Factory Pattern**

We’ll use a **factory** to create the right logger at runtime.

```cpp
enum class LoggerType {
    CONSOLE,
    FILE,
    DATABASE
};

class LoggerFactory {
public:
    static std::unique_ptr<Logger> createLogger(LoggerType type) {
        switch (type) {
        case LoggerType::CONSOLE:
            return std::make_unique<ConsoleLogger>();
        case LoggerType::FILE:
            return std::make_unique<FileLogger>(
                std::make_unique<TimeBasedRotationPolicy>(5)); // rotate every 5 seconds
        case LoggerType::DATABASE:
            return std::make_unique<DatabaseLogger>();
        default:
            throw std::runtime_error("Unknown logger type");
        }
    }
};
```

✅ **Factory Pattern**:

* Centralizes object creation logic
* Keeps `main()` clean and decoupled from concrete classes

---

## 🧍‍♂️ **Step 4 — Make Logging Globally Accessible (Singleton)**

You generally want **a single global logging manager**.
That manager holds a single `Logger` instance.

```cpp
class LogManager {
private:
    std::unique_ptr<Logger> logger;

    LogManager() {
        // default logger
        logger = LoggerFactory::createLogger(LoggerType::FILE);
    }

    LogManager(const LogManager&) = delete;
    LogManager& operator=(const LogManager&) = delete;

public:
    static LogManager& getInstance() {
        static LogManager instance;
        return instance;
    }

    void setLogger(std::unique_ptr<Logger> newLogger) {
        logger = std::move(newLogger);
    }

    void log(const std::string& message) {
        logger->log(message);
    }
};
```

---

## 🧩 **Step 5 — Client Usage**

```cpp
int main() {
    auto& manager = LogManager::getInstance();

    manager.log("App started");
    std::this_thread::sleep_for(std::chrono::seconds(2));
    manager.log("Running some task...");
    std::this_thread::sleep_for(std::chrono::seconds(4));
    manager.log("Rotation should have occurred now!");
}
```

Output:

```
[File] App started
[File] Running some task...
[File] Rotation should have occurred now!
```

(and you’ll see new log files created every 5 seconds)

---

## 🧠 **Design Pattern Summary**

| Pattern       | Used For                                 | Example                         |
| ------------- | ---------------------------------------- | ------------------------------- |
| **Factory**   | To create different logger types         | `LoggerFactory::createLogger()` |
| **Singleton** | To manage a single global access point   | `LogManager::getInstance()`     |
| **Strategy**  | To plug in rotation policies dynamically | `RotationPolicy` interface      |

---

## 🚀 How to Explain This in the Interview

When asked “How would you begin?” — say this:

> I’ll start by identifying key responsibilities:
>
> * Define a base `Logger` interface.
> * Use **Factory Pattern** to create different types of loggers.
> * Use **Singleton Pattern** for global logging manager.
> * Use **Strategy Pattern** for flexible rotation policy.
>
> This allows my framework to be **extensible**, **thread-safe**, and **easily configurable**.

---

# **Thread Safe Framework**

Great — this is exactly the kind of careful, structured answer that wins LLD interviews. I’ll first **point out the race / thread-safety problems** in the design you already have, then give a **step-by-step plan** to make it thread-safe, and finally provide **clean C++ code** for a production-style, thread-safe solution focused on *disk logging with time-based rotation*. I’ll also explain trade-offs and how you can talk about them in the interview.

---

# 1) Which portions are **not thread-safe** in the earlier design

From your earlier design, these are the *unsafe* places:

1. **Concrete logger implementations**

   * `FileLogger::log()` manipulates `std::ofstream outFile`, checks `rotationPolicy->shouldRotate()` and does `outFile.close()` / `open()` — **not safe** for concurrent writers.
   * `RotationPolicy` (e.g. `lastRotation`) is mutable state updated in `shouldRotate()` — race if called by multiple threads.
   * `ConsoleLogger` writes to `std::cout` concurrently — `std::cout` is not guaranteed to be atomic per call; partial interleaves are possible.
   * `DatabaseLogger` might reuse a DB connection concurrently — requires its own synchronization/connection pooling.

2. **LogManager::setLogger / getInstance / logger pointer**

   * Changing the underlying logger while other threads call `LogManager::log()` can cause use-after-free or inconsistency if swaps are not atomic.

3. **LoggerFactory creation** is OK as long as factory calls happen during init; but if created concurrently without synchronization you could see race conditions.

4. **File rotation logic**: closing and re-opening a file concurrently with writes causes corruption if not coordinated.

---

# 2) High-level strategy (stepwise) to make it thread-safe

I’ll give steps in order of increasing robustness and complexity. For each step I’ll explain why and the trade-offs.

## Step A — Make `LogManager`’s logger pointer safe to swap/read

* **Goal:** Let callers call `LogManager::log()` without blocking on setting a new logger.
* **Approach:** Store the logger as a `std::shared_ptr<Logger>` and use `std::atomic_load` / `std::atomic_store` for atomic pointer swap (C++11 has `std::atomic_load` overloads for `shared_ptr` in `<memory>`).
* **Tradeoffs:** cheap snapshot copy for each log call, safe concurrent reads/writes. If not available on your target, protect setter with a mutex and have `log()` copy under a short mutex or copy a `shared_ptr` under the same mutex.

## Step B — Make **each concrete logger** thread-safe

Two options:

**B1 (Simple):** internal mutex per logger

* Add a `std::mutex mtx;` to `FileLogger` and `ConsoleLogger`, lock in `log()` while touching file or console.
* **Pro:** Easy to implement, correct.
* **Con:** Blocking I/O inside `log()` will block caller threads and hurt throughput.

**B2 (Recommended for Disk Logging):** **Asynchronous logging** with a background worker thread

* `log()` must be non-blocking (or minimally blocking): push message into a thread-safe queue and return.
* Background thread dequeues messages and performs file I/O and rotation (so all file ops and rotation policy are single-threaded inside the worker — no extra locking on rotation or file).
* **Pro:** High throughput, safe rotation, deterministic file I/O location.
* **Con:** More code; need graceful shutdown, backpressure handling, bounded queue, possible message loss if not flushed on crash.

For disk logging (rotation), **B2** is usually preferred.

## Step C — File rotation safety

* Let **only the background writer thread** perform rotation and file open/close.
* Rotation state (`lastRotation`) then becomes local to the writer thread and needs no synchronization.

## Step D — Console / Database Loggers

* Console: either a mutex around `std::cout` or use the same asynchronous queue approach (console writer thread).
* Database: use a worker thread or a DB connection pool that is itself thread-safe.

## Step E — Shutdown & flush

* Provide `LogManager::shutdown()` / destructor that:

  * Atomically stop accepting new entries,
  * Signal background worker(s) to flush the queue,
  * Join worker threads.
* Important to demonstrate in interview you considered process exit & graceful flush.

## Step F — Performance & backpressure

* Use bounded queue and drop policy or blocking push when queue is full. Consider batching writes for disk to reduce syscalls.
* Metrics: queue length, dropped messages, write failures.

---

# 3) Concrete C++ implementation (focused on async file logger + safe LogManager)

Below is a compact, interview-worthy implementation that:

* Uses a `LogManager` singleton.
* Stores logger as `std::shared_ptr<Logger>` and uses `std::atomic_load`/`std::atomic_store`.
* Implements an `AsyncFileLogger` (background writer + time-based rotation).
* Provides a safe `ConsoleLogger` with a small mutex (simple).

> This code targets modern C++ (C++11). I use standard library concurrency primitives and `std::atomic_load`/`std::atomic_store` for `shared_ptr`.

```cpp
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
            queue_cv.wait(lk, [this] { return stopping || !q.empty(); });

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
                return;
            }
        }
    }

public:
    AsyncFileLogger(std::unique_ptr<RotationPolicy> policy)
        : rotationPolicy(std::move(policy)), worker(&AsyncFileLogger::run, this)
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
        if (worker.joinable()) worker.join();
    }

    // non-blocking (pushes to queue)
    void log(const std::string& msg) override {
        std::lock_guard<std::mutex> lk(queue_mtx);
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

    t1.join();
    t2.join();

    // destructor will flush and join worker
    return 0;
}
```

---

# 4) Explanations & interview talking points

* **Why use async worker for disk logging?** Disk I/O is slow — by pushing to an in-memory queue we avoid blocking caller threads. Also rotation logic is centralized and single-threaded, avoiding race conditions on file handles.
* **Why use atomic shared_ptr?** It gives safe, lock-free swapping of the active logger. Each log call atomically snapshots a shared_ptr and uses it; setting a new logger is atomic and safe.
* **Backpressure & safety:** We used a bounded queue with a drop policy. In an interview mention alternatives: block producers, drop oldest, write to secondary store, or apply rate-limiting.
* **Durability:** If crash happens, in-memory queue may lose messages. For stronger durability: synchronous writes or write-ahead buffer persisted before enqueue (trade-off latency).
* **Testing:** Unit-test concurrency and rotation: set small rotation interval and high log rate, verify multiple files created and no corruption.
* **Extension:** To add `DatabaseLogger`, implement its own async worker or make a thread-safe connection pool. Reuse same queue design.
* **Graceful shutdown:** Destructors must flush queue and join worker to avoid lost logs — show that in `~AsyncFileLogger()`.

---

# 5) Variations you can mention in an interview

* Use `std::atomic<std::shared_ptr<Logger>>` (C++20) for more direct atomic shared_ptr semantics.
* Use lock-free MPSC queue (multiple producers, single consumer) for even better performance; there are classic implementations (boost/folly).
* Batch writes (collect N messages or wait T ms) before writing to disk to reduce syscalls.
* Add severity levels and filters (INFO/WARN/ERROR) at the `LogManager` level or as filter objects (strategy).
* Use memory-mapped files for very high write rates (advanced).
* Provide config-driven rotation (time/size) via strategy pattern.

---

# 6) Short checklist to show in the interview when you code

1. Define `Logger` interface.
2. Implement `ConsoleLogger` (mutex-protected) and `AsyncFileLogger` (queue + worker + rotation).
3. Implement `LogManager` singleton — atomic pointer to logger and `log()` method.
4. Demonstrate swap of logger via `setLogger()` (atomic).
5. Show concurrent threads logging and file rotation working.
6. Handle graceful shutdown & error handling.
7. Discuss trade-offs (latency vs durability, queues vs blocking, backpressure).

---

If you want, I can now:

* Turn the provided code into a runnable file and test it (I can run Python tooling to compile? — I cannot compile here but I can simulate or format unit tests).
* Replace the bounded queue with a simple lock-free MPSC queue (I can sketch one).
* Add severity levels, filtering, and a configuration loader.
  Which of those would you like next?
