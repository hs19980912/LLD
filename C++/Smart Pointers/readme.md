- **C++ guidelines say** :  
Use `unique_ptr` by default, `shared_ptr` only when you truly need shared ownership.


- unique_ptr wins in practice at many places
    - Performance: Zero overhead, as fast as raw pointers.
    - Clearer ownership semantics: Forces you to think who ows what. 
    - Most relationships aren't truly shared
        - Usually there is a clear parent/owner.
        - "Observing" relations can use raw pointers or references.

- shared_ptr are used when:
    - Caching systems - Multiple parts of code share cached data
    - Resource pools - Thread pools, connection pools
    - Callback systems - Callbacks might outlive the object that created them
    - Plugin architectures - Unclear who should own the plugin
    - Asynchronous operations - Tasks running on different threads need shared access
    - Complex object graphs - When lifetime dependencies are hard to model