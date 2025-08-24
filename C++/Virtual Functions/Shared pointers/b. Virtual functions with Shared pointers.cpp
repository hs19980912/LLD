#include <memory>

class IKeyboard {
public:
    virtual ~IKeyboard() = default;
    virtual void type() const = 0; // Pure virtual function
};

class IMouse {
public:
    virtual ~IMouse() = default;
    virtual void click() const = 0; // Pure virtual function
};


class MacBook {
public:
    MacBook(std::shared_ptr<IKeyboard> keyboard, std::shared_ptr<IMouse> mouse)
        : keyboard(keyboard), mouse(mouse) {}

private:
    std::shared_ptr<IKeyboard> keyboard;
    std::shared_ptr<IMouse> mouse;
};

class MechanicalKeyboard : public IKeyboard {
public:
    void type() const override {

    }
};

class OpticalMouse : public IMouse {
public:
    void click() const override {
        
    }
};

int main() {
    auto keyboard = std::make_shared<MechanicalKeyboard>();
    auto mouse = std::make_shared<OpticalMouse>();

    MacBook myMacBook(keyboard, mouse);

    return 0;
}