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