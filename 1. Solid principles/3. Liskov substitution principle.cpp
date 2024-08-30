Liskov substitution principle

If class B is a subtype of class A, then we should be able to replace object of class A with B without breaking the behaviour of the program.

Subclass should extend the capability of the parent class and NOT narrow it down.

==================================================================================

Example Without LSP

Consider a base class Bird and a derived class Penguin. The Bird class has a method fly(), but not all birds can fly (e.g., penguins). If we define a Penguin class that overrides the fly() method, it can lead to incorrect behavior if the client code expects all Bird objects to fly.

cpp

#include <iostream>
#include <string>

class Bird {
public:
    virtual void fly() const {
        std::cout << "Flying high in the sky!" << std::endl;
    }
};

class Sparrow : public Bird {
    // Inherits fly() behavior correctly
};

class Penguin : public Bird {
public:
    void fly() const override {
        throw std::logic_error("Penguins can't fly!");
    }
};

void letBirdFly(const Bird& bird) {
    bird.fly();
}

int main() {
    Sparrow sparrow;
    Penguin penguin;

    letBirdFly(sparrow); // Works as expected
    letBirdFly(penguin); // Throws an exception, violating LSP

    return 0;
}

Problems:

    The Penguin class violates the Liskov Substitution Principle because it cannot be used as a Bird without causing problems (e.g., an exception when trying to fly).
    The letBirdFly function expects any Bird to be able to fly, which is not true for Penguin.

Example with LSP Applied

To adhere to LSP, we should redesign the class hierarchy so that the Bird class does not make assumptions about flying. We could introduce a separate interface or base class for flying birds.

cpp

#include <iostream>
#include <string>

class Bird {
public:
    virtual void makeSound() const {
        std::cout << "Chirp chirp!" << std::endl;
    }
};

class Flyable {
public:
    virtual void fly() const = 0; // Pure virtual function for flying
};

class Sparrow : public Bird, public Flyable {
public:
    void fly() const override {
        std::cout << "Flying high in the sky!" << std::endl;
    }
};

class Penguin : public Bird {
    // No fly() method since penguins cannot fly
};

void letBirdFly(const Flyable& bird) {
    bird.fly();
}

int main() {
    Sparrow sparrow;
    Penguin penguin;

    letBirdFly(sparrow); // Works as expected
    // letBirdFly(penguin); // Compile-time error, since Penguin is not Flyable

    return 0;
}

Explanation:

    Adherence to LSP:
        Bird now represents general bird behavior, like making sounds, without assuming that all birds can fly.
        Flyable is an interface that only birds that can fly implement, ensuring that only those birds are passed to the letBirdFly function.

    Safety:
        The Penguin class no longer overrides fly() because it doesn’t implement Flyable. This prevents the misuse of Penguin objects where flying is expected, ensuring that the program behaves correctly and predictably.

Benefits:

    Correctness: The program adheres to expectations, and substituting subclasses does not lead to unexpected behaviors.
    Flexibility: The class hierarchy is now more flexible, accommodating different kinds of birds without making incorrect assumptions.
    Maintainability: The separation of concerns (flying vs. non-flying birds) makes the code easier to understand and extend.