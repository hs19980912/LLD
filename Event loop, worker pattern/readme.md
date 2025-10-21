## Event loop/Worker pattern in Async programming

**We used a event loop in File logger design to make it Asynchronous. It means that this type of code paradigmn of using while(true) in the thread function should be very common in Async programming?**

**Absolutely! YES!** 🎯

The `while(true)` pattern in thread functions is **extremely common** in async/concurrent programming. It's a fundamental pattern called the **"Event Loop"** or **"Worker Loop"** pattern.

## Why `while(true)` is Common in Async Programming

The pattern is:
```cpp
void workerThread() {
    while (true) {
        // 1. Wait for work
        // 2. Do work
        // 3. Check if should exit
        // 4. Loop back
    }
}
```

This creates a **long-lived worker** that continuously processes tasks.

## Common Use Cases

### 1. **Producer-Consumer Queue** (Your Code!)

```cpp
109:115:rough/rough.cpp
// background worker function
void run() {
    while (true) {
        std::unique_lock<std::mutex> lk(queue_mtx);
        queue_cv.wait(lk, [this] { return stopping || !q.empty(); });

        // drain queue (batching)
```

**Pattern:**
- Producer threads add work to queue
- Consumer thread (with `while(true)`) continuously processes queue
- Super common in logging, job processing, message handling

### 2. **Thread Pool**

```cpp
class ThreadPool {
    std::vector<std::thread> workers;
    std::queue<std::function<void()>> tasks;
    
    void workerThread() {
        while (true) {  // ← Same pattern!
            std::unique_lock<std::mutex> lock(queue_mutex);
            condition.wait(lock, [this] { 
                return stop || !tasks.empty(); 
            });
            
            if (stop && tasks.empty()) return;
            
            auto task = std::move(tasks.front());
            tasks.pop();
            lock.unlock();
            
            task();  // Execute the task
        }
    }
    
public:
    ThreadPool(size_t threads) {
        for (size_t i = 0; i < threads; ++i) {
            workers.emplace_back([this] { workerThread(); });
        }
    }
};
```

**Used in:** Web servers, parallel processing libraries, task schedulers

### 3. **Network Server**

```cpp
void serverThread() {
    while (true) {  // Accept connections forever
        auto client = acceptConnection();
        if (shouldShutdown) break;
        handleClient(client);
    }
}
```

**Used in:** HTTP servers, database servers, game servers

### 4. **Event Loop** (GUI, Node.js style)

```cpp
void eventLoop() {
    while (true) {  // Process events forever
        Event event = waitForEvent();
        if (event.type == QUIT) break;
        handleEvent(event);
    }
}
```

**Used in:** GUI frameworks (Qt, GTK), Node.js, game engines

### 5. **Message Pump** (Windows-style)

```cpp
void messagePump() {
    MSG msg;
    while (true) {  // Process messages forever
        if (GetMessage(&msg, NULL, 0, 0) <= 0) break;
        TranslateMessage(&msg);
        DispatchMessage(&msg);
    }
}
```

**Used in:** Windows applications, message-based systems

### 6. **Monitoring/Polling Thread**

```cpp
void monitorThread() {
    while (true) {
        checkSystemHealth();
        std::this_thread::sleep_for(std::chrono::seconds(5));
        if (shouldStop) break;
    }
}
```

**Used in:** System monitors, health checks, watchdogs

## The Standard Pattern Structure

Almost all use this structure:

```cpp
void workerFunction() {
    // 1. Setup (once)
    initialize();
    
    // 2. Main loop (runs until shutdown)
    while (true) {
        // Wait for work (efficient - uses condition variable, select(), etc.)
        waitForWork();
        
        // Check shutdown condition FIRST
        if (shouldStop && noMoreWork()) {
            break;  // or return
        }
        
        // Do the work
        processWork();
    }
    
    // 3. Cleanup (once)
    cleanup();
}
```

## Real-World Examples

### Example 1: spdlog (Popular C++ Logging Library)

```cpp
// Simplified from spdlog's async logger
void thread_pool::worker_loop_() {
    while (true) {
        async_msg incoming_async_msg;
        
        // Wait for message
        q_.dequeue(incoming_async_msg);
        
        if (incoming_async_msg.msg_type == async_msg_type::terminate) {
            break;  // Exit loop
        }
        
        // Process log message
        incoming_async_msg.worker_ptr->backend_sink_it_(incoming_async_msg);
    }
}
```

### Example 2: Boost.Asio (Networking Library)

```cpp
// io_context runs an event loop
void io_context::run() {
    while (true) {  // Event loop
        // Wait for I/O operations
        auto result = wait_for_io();
        
        if (stopped_) break;
        
        // Execute handlers
        execute_handlers();
    }
}
```

### Example 3: Redis (In-Memory Database)

```cpp
// Simplified from Redis source
void aeMain(aeEventLoop *eventLoop) {
    eventLoop->stop = 0;
    while (!eventLoop->stop) {  // Event loop
        aeProcessEvents(eventLoop, 
            AE_ALL_EVENTS | AE_CALL_BEFORE_SLEEP);
    }
}
```

## Alternatives to `while(true)` in Async Programming

Not all async programming uses `while(true)`. There are other paradigms:

### 1. **Callbacks** (One-shot tasks)

```cpp
void doAsyncWork(std::function<void(Result)> callback) {
    std::thread([callback] {
        // Do work once
        Result result = compute();
        callback(result);
        // Thread exits (no loop!)
    }).detach();
}
```

### 2. **Futures/Promises** (One-shot results)

```cpp
std::future<int> asyncCompute() {
    return std::async(std::launch::async, [] {
        // Compute once and return
        return 42;
        // Thread exits (no loop!)
    });
}
```

### 3. **async/await** (C++20 Coroutines)

```cpp
Task<int> asyncOperation() {
    int result = co_await someAsyncWork();
    co_return result;
    // No explicit thread or loop!
}
```

### 4. **Fork-Join** (Parallel algorithms)

```cpp
std::vector<int> data = {1, 2, 3, 4, 5};
std::for_each(std::execution::par, data.begin(), data.end(), 
    [](int& x) { x *= 2; });
// Threads created and destroyed automatically
// No explicit while(true)
```

## When to Use `while(true)` Pattern

| Use Case | Pattern | Why |
|----------|---------|-----|
| **Long-lived workers** | `while(true)` | Thread lives entire program lifetime |
| **Continuous processing** | `while(true)` | Always ready to handle work |
| **Event loops** | `while(true)` | Process events indefinitely |
| **Server connections** | `while(true)` | Accept connections forever |
| **One-time tasks** | No loop | Task completes and thread exits |
| **Parallel computation** | No loop | Compute result and return |

## Pattern Comparison

### Your Logger Pattern (Worker Loop)

```cpp
void run() {
    while (true) {  // Long-lived worker
        wait_for_messages();
        process_all_messages();
        if (shutdown) return;
    }
}
```

**Characteristics:**
- ✅ Thread lives entire program lifetime
- ✅ Efficient (sleeps when no work)
- ✅ Processes many tasks
- ✅ One thread handles stream of work

### One-Shot Pattern (No Loop)

```cpp
void compute_once(Data data) {
    // Process once
    auto result = heavy_computation(data);
    save_result(result);
    // Thread exits
}

std::thread t(compute_once, my_data);
```

**Characteristics:**
- ✅ Thread created for one task
- ✅ Thread exits after task completes
- ❌ Overhead of creating/destroying thread
- ❌ Not suitable for many small tasks

## The Key Insight

The `while(true)` pattern is used when you want a **long-lived worker** that:

1. **Waits efficiently** (not busy-waiting)
2. **Processes multiple tasks** over its lifetime
3. **Stays alive** until explicitly told to stop
4. **Reuses thread resources** (no create/destroy overhead)

**Your code is a textbook example** of this pattern! It's exactly how professional async systems are built.

## Summary

| Question | Answer |
|----------|--------|
| **Is `while(true)` common in async programming?** | ✅ YES! Extremely common! |
| **What's it called?** | Event loop, worker loop, message pump |
| **When is it used?** | Long-lived workers, servers, thread pools, event systems |
| **Real-world examples?** | Logging libraries, web servers, GUI frameworks, databases |
| **Is it the only async pattern?** | ❌ No, but it's very common for continuous processing |
| **Is my code using best practices?** | ✅ YES! Your code follows industry-standard patterns |

**Bottom line:** The `while(true)` worker loop pattern is a cornerstone of async/concurrent programming. You'll see it everywhere in production code! 🚀

Your intuition is spot-on - this is exactly how async systems are built in the real world!