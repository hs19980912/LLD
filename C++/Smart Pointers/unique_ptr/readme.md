- Constructors
    1. Default constructor/parameterized constructor
    2. copy constructor = delete
    3. copy assignment operator = delete
    4. move consturctor
    5. move assignemnt operator
    - use `explicit` in default constuctor.
    - use `noexcept` in move ctor and = operator.


- Accessors
    1. get() returns ptr of type T*
    2. operator -> : returns ptr of type T*
    3. operator * : dereferencing, returns *ptr of type T&  
    - make all the accessors `const` member functions.
        - These functions promises not to modify any member variables of the object. ptr in our case.
        - These functions can only be called on non-const objects.


- **Note: use `explicit` in default constuctor**:
    - Ownership Clarity: Only _prevent_ issues like below, does not fully eliminate double delete:
        ```cpp
        int* rawPtr = new int(42);

        // Without explicit - unclear who owns the memory
        UniquePtr<int> ptr1 = rawPtr;  // Now ptr1 owns it
        UniquePtr<int> ptr2 = rawPtr;  // Now ptr2 ALSO owns it → DOUBLE DELETE! 💥
        ```
    - 
- **Note**: Never manually manage raw pointers - use `std::make_unique` instead:
    -  ```cpp
        int* rawPtr = new int(42);
        UniquePtr<int> ptr1(rawPtr);  // ✅ Direct initialization works with 'explicit'
        UniquePtr<int> ptr2(rawPtr);  // ✅ Also works with 'explicit'
        // 💥 Still causes DOUBLE DELETE! ```
    - Deleted copy constructor → prevents UniquePtr-to-UniquePtr copying
    - explicit → prevents raw-pointer-to-UniquePtr implicit conversion (but can still cause double delete)
    - Hence, best practice is not to use raw pointers itself and use make_unique.
    

