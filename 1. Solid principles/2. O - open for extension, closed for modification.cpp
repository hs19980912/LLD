The Open/Closed Principle (OCP) is one of the five SOLID principles of object-oriented design. It states that:

"Software entities (classes, modules, functions, etc.) should be open for extension, but closed for modification."
Explanation

    Open for Extension: The behavior of a module (e.g., class) can be extended. This means you can add new functionality or behaviors to the module.

    Closed for Modification: The existing code of the module should not be modified when you extend it. This helps to prevent introducing bugs into existing, well-tested code.

In practice, the Open/Closed Principle is often achieved through abstraction and polymorphism. By using interfaces, abstract classes, or inheritance, you can extend the behavior of existing code without changing its source.
=============================================================================================================

Example Without OCP

Consider a class that processes different types of shapes. Initially, the class is designed to handle just circles, but over time, it needs to support rectangles, triangles, and more. Each time a new shape is added, the original class must be modified, violating the OCP.


#include <iostream>
#include <cmath>

class ShapeProcessor {
public:
    double calculateArea(const std::string& shapeType, double dimension1, double dimension2 = 0) {
        if (shapeType == "circle") {
            return M_PI * dimension1 * dimension1;
        } else if (shapeType == "rectangle") {
            return dimension1 * dimension2;
        }
        // More shapes will require adding more conditions here
        return 0.0;
    }
};

int main() {
    ShapeProcessor processor;
    std::cout << "Circle area: " << processor.calculateArea("circle", 5) << std::endl;
    std::cout << "Rectangle area: " << processor.calculateArea("rectangle", 4, 6) << std::endl;
    return 0;
}

Problems:

    Every time a new shape is added, the ShapeProcessor class needs to be modified.
    This makes the class fragile and prone to errors, especially as the complexity increases.
====================================================================================================================
Example with OCP Applied

We can refactor the code to follow the Open/Closed Principle by using polymorphism. We'll create a base Shape class and derive specific shapes from it. This way, we can extend the system by adding new shape types without modifying existing code.

#include <iostream>
#include <cmath>
#include <vector>
#include <memory>

// Abstract base class
class Shape {
public:
    virtual ~Shape() = default;
    virtual double calculateArea() const = 0; // Pure virtual function
};

// Derived class for Circle
class Circle : public Shape {
public:
    Circle(double radius) : radius(radius) {}

    double calculateArea() const override {
        return M_PI * radius * radius;
    }

private:
    double radius;
};

// Derived class for Rectangle
class Rectangle : public Shape {
public:
    Rectangle(double width, double height) : width(width), height(height) {}

    double calculateArea() const override {
        return width * height;
    }

private:
    double width;
    double height;
};

// Processor class that is closed for modification but open for extension
class ShapeProcessor {
public:
    void addShape(std::shared_ptr<Shape> shape) {
        shapes.push_back(shape);
    }

    void processShapes() const {
        for (const auto& shape : shapes) {
            std::cout << "Area: " << shape->calculateArea() << std::endl;
        }
    }

private:
    std::vector<std::shared_ptr<Shape>> shapes;
};

int main() {
    ShapeProcessor processor;
    
    processor.addShape(std::make_shared<Circle>(5));
    processor.addShape(std::make_shared<Rectangle>(4, 6));
    
    processor.processShapes();

    return 0;
}

Explanation:

    Open for Extension: You can add new shapes (e.g., Triangle, Square, etc.) by creating new classes that derive from the Shape class. This requires no modification to the ShapeProcessor class.

    Closed for Modification: The ShapeProcessor class does not need to be modified when new shapes are introduced. It only relies on the abstract interface provided by the Shape base class.

Benefits:

    Maintainability: Since existing code is not modified, the risk of introducing new bugs is minimized.
    Scalability: New features can be added easily by extending the existing system without altering its core functionality.
    Readability: The separation of concerns makes the code easier to understand and manage.