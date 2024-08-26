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