#include <iostream>

template <typename T>
class UniquePtr {
private:
    T* ptr;

public:
    // Constructor
    explicit UniquePtr(T* p = nullptr) : ptr(p) {}

    // Delete copy constructor (no copying allowed)
    UniquePtr(const UniquePtr&) = delete;

    // Delete copy assignment (no copying allowed)
    UniquePtr& operator=(const UniquePtr&) = delete;

    // Move constructor (transfer ownership)
    UniquePtr(UniquePtr&& other) noexcept : ptr(other.ptr) {
        other.ptr = nullptr;
    }

    // Move assignment operator (transfer ownership)
    UniquePtr& operator=(UniquePtr&& other) noexcept {
        if (this != &other) {
            delete ptr;        // free current resource
            ptr = other.ptr;   // transfer ownership
            other.ptr = nullptr;
        }
        return *this;
    }

    // Destructor
    ~UniquePtr() {
        delete ptr;
    }

    // Accessors
    T& operator*() const { return *ptr; }
    T* operator->() const { return ptr; }
    T* get() const { return ptr; }

    // Release ownership manually
    T* release() {
        T* temp = ptr;
        ptr = nullptr;
        return temp;
    }

    // Reset the pointer with a new object
    void reset(T* p = nullptr) {
        delete ptr;
        ptr = p;
    }
};
