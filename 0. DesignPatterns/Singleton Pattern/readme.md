## **Singleton Design Pattern**

The **Singleton pattern** ensures that **only one instance** of a class exists throughout the program **and** provides a **global access point** to it.

So, two main goals:

1. **Single instance** — Prevent multiple objects of the class.
2. **Global access** — Provide a static function to get that single instance.

---

## 🧩 **Common Use Cases**

* Logging systems (single global logger)
* Configuration manager (one config shared across system)
* Connection pool manager
* Thread pool
* Cache manager

---


## ⚙️ **Thread-Safe Singleton (Modern C++ Way)**

Since C++11, **static local variables** are initialized in a thread-safe manner.
This is the cleanest and safest Singleton implementation:

```cpp
class Singleton {
private:
    Singleton() { std::cout << "Instance created\n"; }
    ~Singleton() = default;

    Singleton(const Singleton&) = delete;
    Singleton& operator=(const Singleton&) = delete;

public:
    static Singleton& getInstance() {
        static Singleton instance; // Created once, thread-safe in C++11+
        return instance;
    }
};
```

### ✅ Advantages:

* No dynamic memory allocation (no `new`/`delete`)
* Thread-safe initialization
* Simpler and exception-safe

Usage:

```cpp
int main() {
    Singleton& s1 = Singleton::getInstance();
    Singleton& s2 = Singleton::getInstance();
    std::cout << (&s1 == &s2) << std::endl; // prints 1
}
```

---

## ⚠️ **Common Interview Discussion Points**

### 1. **Why private constructor?**

To prevent clients from creating multiple objects directly using `new`.

### 2. **Why delete copy constructor and assignment?**

To prevent copying the singleton instance and creating duplicates.

### 3. **When to use reference vs pointer?**

* Returning a **reference** is safer (avoids dealing with `nullptr`).
* Returning a **pointer** is used when lazy allocation or manual deletion control is needed.

### 4. **Is Singleton an Anti-Pattern?**

Sometimes. Overusing it can lead to **global state**, **tight coupling**, and **testing difficulties**.

---

## 🧠 **Summary**

| Feature          | Implementation                 |
| ---------------- | ------------------------------ |
| Instance storage | static variable                |
| Creation control | private constructor            |
| Access control   | static function                |
| Thread safety    | static local variable (C++11+) |
| Copy prevention  | delete copy/move constructors  |

---

Would you like me to show you a **lazy-loaded thread-safe version** using **`std::mutex`**, which is the typical “double-checked locking” implementation interviewers love to discuss?
