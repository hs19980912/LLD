Lets come to this code:
#include <iostream>

template <typename T>
class SharedPtr {
private:
    T* ptr;             // Pointer to the managed object
    int* refCount;      // Pointer to the shared reference count

public:
    // Constructor
    explicit SharedPtr(T* p = nullptr)
        : ptr(p), refCount(new int(1)) {
        if (p == nullptr)
            *refCount = 0;
    }

    // Copy constructor
    SharedPtr(const SharedPtr& other)
        : ptr(other.ptr), refCount(other.refCount) {
        if (ptr) (*refCount)++;
    }

    // Copy assignment operator
    SharedPtr& operator=(const SharedPtr& other) {
        if (this != &other) {
            release(); // Release current resource
            ptr = other.ptr;
            refCount = other.refCount;
            if (ptr) (*refCount)++;
        }
        return *this;
    }

    // Destructor
    ~SharedPtr() {
        release();
    }

    // Release helper
    void release() {
        if (refCount && ptr) {
            (*refCount)--;
            if (*refCount == 0) {
                delete ptr;
                delete refCount;
                ptr = nullptr;
                refCount = nullptr;
            }
        }
    }

    // Accessors
    T& operator*() const { return *ptr; }
    T* operator->() const { return ptr; }
    int use_count() const { return refCount ? *refCount : 0; }
    T* get() const { return ptr; }
};
