- **Constructors:**
    1. Default constructor/parameterized constructor
    2. copy constructor = delete
    3. copy assignment operator = delete
    4. move consturctor
    5. move assignemnt operator
    - use `explicit` in default constuctor.
    - use `noexcept` in move ctor and = operator.


- **Accessors:**
    1. get() returns ptr of type T*
    2. operator -> : returns ptr of type T*
    3. operator * : dereferencing, returns *ptr of type T&  
    - make all the accessors `const` member functions.
        - These functions promises not to modify any member variables of the object. ptr in our case.
        - These functions can only be called on non-const objects.


- **Note: use `explicit` in default constuctor**
    - explicit → safer ownership transfer.  
      we want to prevent: 
      ```cpp
      std::unique_ptr<int> p = new int(5); // illegal
      ```
    - Ownership Clarity: Only _prevent_ issues like below, does not fully eliminate double delete:
        ```cpp
        int* rawPtr = new int(42);

        // Without explicit - unclear who owns the memory
        UniquePtr<int> ptr1 = rawPtr;  // Now ptr1 owns it
        UniquePtr<int> ptr2 = rawPtr;  // Now ptr2 ALSO owns it → DOUBLE DELETE! 💥
        ```
- **Note: Never manually manage raw pointers - use `std::make_unique` instead**
    -  ```cpp
        int* rawPtr = new int(42);
        UniquePtr<int> ptr1(rawPtr);  // ✅ Direct initialization works with 'explicit'
        UniquePtr<int> ptr2(rawPtr);  // ✅ Also works with 'explicit'
        // 💥 Still causes DOUBLE DELETE! ```
    - Deleted copy constructor → prevents UniquePtr-to-UniquePtr copying
    - explicit → prevents raw-pointer-to-UniquePtr implicit conversion (but can still cause double delete)
    - Hence, best practice is not to use raw pointers itself and use make_unique.

- **Note: use `noexcept` in move ctor and = operator**
    - They are guaranteed not to throw (pointer copy + nulling is safe).
    - Standard containers (like std::vector) depend on this guarantee to safely and efficiently move elements during reallocation.
    - Without noexcept, containers may fall back to copying (which is deleted for UniquePtr) → compilation errors or performance penalties.
        - If we dont use noexcept → 
        - std::vector says: “Move might throw” →
        - Falls back to copy constructor →
        - But copy constructor is deleted →
        - Compilation error or container becomes unusable 
    - **further reading:**
        - Containers promise `"Either the operation succeeds, or the container remains unchanged".` 
        - This is the strong exception guarantee. If move can throw, containers cannot uphold this guarantee during reallocation. So they require: `std::is_nothrow_move_constructible<T>::value == true` for T to be able to use the container.
        - When you write: `UniquePtr(UniquePtr&&) noexcept;` you are making a strong promise:
            - ❗ If this function throws at runtime → the program will call std::terminate.
            - There is no stack unwinding, no catching, no recovery.
            - STL does not check whether your function actually throws. It assumes noexcept == truth and does its working.
            - If we lie, STL code will silently become unsafe. noexcept is a strict contract.
        - So the best way(a more idiomatic C++ way) is to use `contional noexcept`. 
        - ``` cpp
          template<typename T>
          struct Wrapper {
              T value;

              Wrapper(Wrapper&& other) noexcept(std::is_nothrow_move_constructible_v<T>)
                  : value(std::move(other.value)) {}
          };
          ```

- **Note: `std::make_unique` is NOT a member function of `std::unique_ptr`**
    - Seperation of concerns: `unique_ptr` manages ownership; `make_unique` manages construction.
    - It is a free (standalone) function template provided by the C++ standard library.
    - Think of it as a **factory function** that creates and returns a std::unique_ptr<T> safely.
    - Allocation + construction happen inside make_unique
    - Either:
        - Object fully created → returned in unique_ptr
        - Or exception → no memory leaked
    - What make_unique actually does internally?
        - ```cpp 
          template<typename T, typename... Args>
          std::unique_ptr<T> make_unique(Args&&... args) {
          return std::unique_ptr<T>(new T(std::forward<Args>(args)...));
          }
          ```
