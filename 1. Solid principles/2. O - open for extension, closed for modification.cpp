The Open/Closed Principle (OCP) is one of the five SOLID principles of object-oriented design. It states that:

"Software entities (classes, modules, functions, etc.) should be open for extension, but closed for modification."
Explanation

    Open for Extension: The behavior of a module (e.g., class) can be extended. This means you can add new functionality or behaviors to the module.

    Closed for Modification: The existing code of the module should not be modified when you extend it. This helps to prevent introducing bugs into existing, well-tested code.

In practice, the Open/Closed Principle is often achieved through abstraction and polymorphism. By using interfaces, abstract classes, or inheritance, you can extend the behavior of existing code without changing its source.
=============================================================================================================

Not following open for extension and closed for modification

class InvoiceDao { 
    Invoice invoice; 
    public InvoiceDao (Invoice invoice) { 
        this.invoice = invoice; 
    } 
    
    public void saveToDB() { 
        // Save Invoice into DB 
    } 
    
    public void saveToFile(String filename) { 
        // Save Invoice in the File with the given name 
    }
}

==========================================================================

Adheres to Open/Close principle

interface InvoiceDao { 
    public void save (Invoice invoice);
} 

class Database implements InvoiceDao { 
    @Override 
    public void save (Invoice invoice) { 
        // Save to DB 
    } 
} 

class FileInvoiceDao implements InvoiceDao { 
    @Override 
    public void save (Invoice invoice) { 
        // Save to file
    }
}

===========================================================================
// Forward declaration of Invoice class
class Invoice;

// Abstract base class (equivalent to Java interface)
class InvoiceDao {
public:
    virtual void save(Invoice* invoice) = 0; // Pure virtual function
    virtual ~InvoiceDao() = default; // Virtual destructor
};

// Concrete class for saving to a database
class Database : public InvoiceDao {
public:
    void save(Invoice* invoice) override {
        // Save to DB
        std::cout << "Saving invoice to the database." << std::endl;
    }
};

// Concrete class for saving to a file
class FileInvoiceDao : public InvoiceDao {
public:
    void save(Invoice* invoice) override {
        // Save to file
        std::cout << "Saving invoice to a file." << std::endl;
    }
};

// Example Invoice class
class Invoice {
    // Invoice details
};

// Example usage
int main() {
    Invoice invoice;
    
    // Using Database implementation
    InvoiceDao* databaseDao = new Database();
    databaseDao->save(&invoice);
    
    // Using File implementation
    InvoiceDao* fileDao = new FileInvoiceDao();
    fileDao->save(&invoice);
    
    // Clean up
    delete databaseDao;
    delete fileDao;

    return 0;
}
Benefits:

    Maintainability: Since existing code is not modified, the risk of introducing new bugs is minimized.
    Scalability: New features can be added easily by extending the existing system without altering its core functionality.
    Readability: The separation of concerns makes the code easier to understand and manage.