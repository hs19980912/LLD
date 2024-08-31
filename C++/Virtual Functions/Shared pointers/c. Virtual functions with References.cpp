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
    // Constructor accepts references to the interface types
    MacBook(IKeyboard& keyboard, IMouse& mouse)
        : keyboard(keyboard), mouse(mouse) {}

private:
    IKeyboard& keyboard; // Reference to IKeyboard
    IMouse& mouse;       // Reference to IMouse
};

class MechanicalKeyboard : public IKeyboard {
public:
    void type() const override {
        // Implementation for typing
    }
};

class OpticalMouse : public IMouse {
public:
    void click() const override {
        // Implementation for clicking
    }
};

int main() {
    // Create objects on the stack
    MechanicalKeyboard keyboard;
    OpticalMouse mouse;

    // Pass references to the MacBook constructor
    MacBook myMacBook(keyboard, mouse);

    return 0;
}
