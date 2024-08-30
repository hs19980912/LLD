#include <iostream>
#include <string>
using namespace std; 
class Bird{
public:
    virtual void fly() = 0;
};

class Sparrow : public Bird{
public:
    void fly() override {
        cout << "Sparrow is flying" << endl;
    }
};

class Parrot : public Bird{
public:
    void fly() override{
        cout << "Parrot is flying" << endl;
    }
};

void flyingBirds(Bird &bird){
    bird.fly();
}

int main() {
    Parrot parrot;
    flyingBirds(parrot);
}