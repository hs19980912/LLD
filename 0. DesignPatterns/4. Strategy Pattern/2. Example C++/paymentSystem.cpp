#include <iostream>
#include <string>
using namespace std;

class IPayment{
private:
	virtual string makePayment(int) = 0;
public:
	string processPayment(int amount){
		return makePayment(amount);
	}
};

class UPIPayment : public IPayment {
private:
	string makePayment(int amount){
		return "upi payment done: Rs" + to_string(amount);
	}
};

class NEFTPayment : public IPayment{
private:
	string makePayment(int amount){
		return "NEFT Payment done: Rs" + to_string(amount);
	}
};

class PaymentSystem{
	IPayment* curMode = nullptr;

public:
	PaymentSystem(){
		curMode = nullptr;
	}

	bool makePayment(string mode, int amount){
		if(mode == "upi"){
			UPIPayment upiMode;
			curMode = &upiMode;
		}else if(mode == "neft"){
			NEFTPayment neftMode;
			curMode = &neftMode;
		}else{
			cout << "Invalid mode" << endl;
			return false;
		}
		cout << curMode->processPayment(amount);
		return true;
	}
};

class Application{
	int amount;

public:
	Application(){
		amount = 0;
	}

	void makePayment(){
		PaymentSystem paymentSystem;
		string mode = "upi"; // Take from user
		amount = 200;	// amount is calculated;
		bool success = false;
		do{
			success = paymentSystem.makePayment(mode, amount);
		}while(success!=true);
	}
};

int main(){
	Application app;
	app.makePayment();
}