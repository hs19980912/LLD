S - Single Responsibility Principle

A class should have only 1 reason to change.

Yes, that’s correct! In the SOLID principles of object-oriented design, this concept is referred to as the Single Responsibility Principle (SRP). According to the SRP, a class should have only one reason to change, meaning it should have only one responsibility or job.
In practical terms, this means that a class should focus on a single part of the functionality provided by the software, and this part should be encapsulated within that class. If a class has more than one responsibility, changes in one area might affect or introduce bugs in another, making the code more difficult to maintain and understand.
By adhering to the Single Responsibility Principle, you make your code more modular, easier to test, and more resilient to changes.

=====================================================================================

Violating SRP:

class Marker{
    int colour;
    string brand;
};


class Invoice{
    Marker marker;
    int quantity;

    Invoice(Marker marker, int q) : marker(maker), quantity(q) {}

    void generateBill(){

    }

    void printInvoice(){

    }

    void savetoDB(){

    }
};

=========================================================================================

Adhering to single repsonsibility principle

class Marker{
    int colour;
    string brand;
};

class InvoiceBill{
    Marker marker;
    InvoiceBill(Marker marker) : marker(marker){}

    void generateBill(){

    }
};

class InvoicePrint{
    Marker marker;
    InvoiceBill(Marker marker) : marker(marker){}

    void printInvoice(){
        
    }
};

class InvoiceSaveDB{
    Marker marker;
    InvoiceBill(Marker marker) : marker(marker){}

    void saveInvoicetoDB(){
        
    }
};