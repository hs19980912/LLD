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
    // Constructor accepts raw pointers to the interface types
    MacBook(IKeyboard* keyboard, IMouse* mouse)
        : keyboard(keyboard), mouse(mouse) {}

private:
    IKeyboard* keyboard; // Raw pointer to IKeyboard
    IMouse* mouse;       // Raw pointer to IMouse
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
    // Create objects on the stack or heap
    MechanicalKeyboard keyboard;
    OpticalMouse mouse;

    // Pass raw pointers to the MacBook constructor
    MacBook myMacBook(&keyboard, &mouse);

    return 0;
}
