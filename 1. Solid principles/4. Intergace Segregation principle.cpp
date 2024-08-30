Interface segmented principle

"Clients should not be forced to depend on interfaces they do not use."

=======================================================================


Example Without ISP

Consider a situation where we have an interface that combines multiple unrelated methods, forcing classes to implement methods they dont need:

cpp

#include <iostream>

class IWorker {
public:
    virtual void work() = 0;
    virtual void eat() = 0;
    virtual ~IWorker() = default;
};

class HumanWorker : public IWorker {
public:
    void work() override {
        std::cout << "Human working..." << std::endl;
    }

    void eat() override {
        std::cout << "Human eating..." << std::endl;
    }
};

class RobotWorker : public IWorker {
public:
    void work() override {
        std::cout << "Robot working..." << std::endl;
    }

    void eat() override {
        // Robots do not eat, but we must provide an implementation
        // to satisfy the interface, which leads to a violation of ISP
    }
};

int main() {
    HumanWorker human;
    RobotWorker robot;

    human.work();
    human.eat();

    robot.work();
    // robot.eat(); // Meaningless for Robot, violating ISP

    return 0;
}

Problems:

    The IWorker interface forces RobotWorker to implement the eat() method, even though it doesn't make sense for robots.
    This violates the Interface Segregation Principle because RobotWorker is depending on an interface it doesn't fully utilize.


--------------------------------------------------------------------------------------------------------------------------------

Example with ISP Applied

To adhere to the Interface Segregation Principle, we can split the IWorker interface into smaller, more specific interfaces:

cpp

#include <iostream>

// Interface for work-related methods
class IWorkable {
public:
    virtual void work() = 0;
    virtual ~IWorkable() = default;
};

// Interface for eat-related methods
class IEatable {
public:
    virtual void eat() = 0;
    virtual ~IEatable() = default;
};

class HumanWorker : public IWorkable, public IEatable {
public:
    void work() override {
        std::cout << "Human working..." << std::endl;
    }

    void eat() override {
        std::cout << "Human eating..." << std::endl;
    }
};

class RobotWorker : public IWorkable {
public:
    void work() override {
        std::cout << "Robot working..." << std::endl;
    }
    // No need to implement IEatable
};

int main() {
    HumanWorker human;
    RobotWorker robot;

    human.work();
    human.eat();

    robot.work();
    // No meaningless "eat" method call for RobotWorker

    return 0;
}