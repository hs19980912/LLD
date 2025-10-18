# Factory pattern
---

## 🧭 Summary Table (with Analogies)

| Pattern              | Analogy            | Code Essence                             | Number of Factories      | Key Benefit                     |
| -------------------- | ------------------ | ---------------------------------------- | ------------------------ | ------------------------------- |
| **Static Factory**   | Vending Machine    | One static `getSnack()`                  | 1                        | Simple, centralized creation    |
| **Factory Method**   | Different Bakeries | Virtual `createBread()`                  | One per product type     | Extensible via subclassing      |
| **Abstract Factory** | Furniture Factory  | Virtual `createChair()` + `createSofa()` | One per product *family* | Creates related object families |

## 1️⃣ Static Factory Pattern — *“Vending Machine” Analogy*

### 🧠 Analogy:

* You **don’t** create snacks yourself.
* You ask the **VendingMachine** to give you one by specifying a type.
* The machine decides *which* snack to dispense.

```cpp
#include <iostream>
#include <memory>
#include <string>
using namespace std;

// ---------- Product Interface ----------
class Snack {
public:
    virtual void eat() = 0;
    virtual ~Snack() = default;
};

// ---------- Concrete Products ----------
class Chips : public Snack {
public:
    void eat() override { cout << "Eating Chips \n"; }
};

class Chocolate : public Snack {
public:
    void eat() override { cout << "Eating Chocolate \n"; }
};

// ---------- Static Factory ----------
class VendingMachine {
public:
    static unique_ptr<Snack> getSnack(const string& type) {
        if (type == "chips")
            return make_unique<Chips>();
        else if (type == "chocolate")
            return make_unique<Chocolate>();
        else
            return nullptr;
    }
};

// ---------- Client ----------
int main() {
    auto snack1 = VendingMachine::getSnack("chips");
    auto snack2 = VendingMachine::getSnack("chocolate");

    snack1->eat();
    snack2->eat();
}
```

### 🧩 Notes

* Centralized logic.
* One static method.
* Simple, but adding new snacks requires modifying `VendingMachine`.

---

## 2️⃣ Factory Method Pattern — *“Different Bakeries” Analogy*

### 🧠 Analogy:

* Each bakery knows how to bake its own type of bread.
* The **client** just asks the bakery to “bake bread”, not caring which bread it is.

```cpp
#include <iostream>
#include <memory>
using namespace std;

// ---------- Product Interface ----------
class Bread {
public:
    virtual void bake() = 0;
    virtual ~Bread() = default;
};

// ---------- Concrete Products ----------
class WhiteBread : public Bread {
public:
    void bake() override { cout << "Baking White Bread\n"; }
};

class BrownBread : public Bread {
public:
    void bake() override { cout << "Baking Brown Bread\n"; }
};

// ---------- Creator (Factory Base Class) ----------
class Bakery {
public:
    virtual unique_ptr<Bread> createBread() = 0;  // Factory Method
    virtual ~Bakery() = default;
};

// ---------- Concrete Factories ----------
class WhiteBreadBakery : public Bakery {
public:
    unique_ptr<Bread> createBread() override {
        return make_unique<WhiteBread>();
    }
};

class BrownBreadBakery : public Bakery {
public:
    unique_ptr<Bread> createBread() override {
        return make_unique<BrownBread>();
    }
};

// ---------- Client ----------
int main() {
    unique_ptr<Bakery> bakery1 = make_unique<WhiteBreadBakery>();
    unique_ptr<Bakery> bakery2 = make_unique<BrownBreadBakery>();

    auto bread1 = bakery1->createBread();
    auto bread2 = bakery2->createBread();

    bread1->bake();
    bread2->bake();
}
```

### 🧩 Notes

* The **factory method** is *virtual and non-static*.
* Each subclass decides which bread to bake.
* Easy to add new bakeries (new subclasses) → follows **Open/Closed Principle**.

---

## 3️⃣ Abstract Factory Pattern — *“Furniture Factory” Analogy*

### 🧠 Analogy:

* Each furniture factory produces a **family** of matching products (Chair + Sofa + Table).
* Client can build a **consistent furniture set** without worrying about details.

```cpp
#include <iostream>
#include <memory>
using namespace std;

// ---------- Product Interfaces ----------
class Chair {
public:
    virtual void sit() = 0;
    virtual ~Chair() = default;
};

class Sofa {
public:
    virtual void relax() = 0;
    virtual ~Sofa() = default;
};

// ---------- Concrete Products (Modern Family) ----------
class ModernChair : public Chair {
public:
    void sit() override { cout << "Sitting on Modern Chair 🪑\n"; }
};

class ModernSofa : public Sofa {
public:
    void relax() override { cout << "Relaxing on Modern Sofa\n"; }
};

// ---------- Concrete Products (Victorian Family) ----------
class VictorianChair : public Chair {
public:
    void sit() override { cout << "Sitting on Victorian Chair\n"; }
};

class VictorianSofa : public Sofa {
public:
    void relax() override { cout << "Relaxing on Victorian Sofa\n"; }
};

// ---------- Abstract Factory ----------
class FurnitureFactory {
public:
    virtual unique_ptr<Chair> createChair() = 0;
    virtual unique_ptr<Sofa> createSofa() = 0;
    virtual ~FurnitureFactory() = default;
};

// ---------- Concrete Factories ----------
class ModernFurnitureFactory : public FurnitureFactory {
public:
    unique_ptr<Chair> createChair() override {
        return make_unique<ModernChair>();
    }
    unique_ptr<Sofa> createSofa() override {
        return make_unique<ModernSofa>();
    }
};

class VictorianFurnitureFactory : public FurnitureFactory {
public:
    unique_ptr<Chair> createChair() override {
        return make_unique<VictorianChair>();
    }
    unique_ptr<Sofa> createSofa() override {
        return make_unique<VictorianSofa>();
    }
};

// ---------- Client ----------
void furnishRoom(FurnitureFactory& factory) {
    auto chair = factory.createChair();
    auto sofa = factory.createSofa();

    chair->sit();
    sofa->relax();
}

int main() {
    cout << "Using Modern Furniture Factory:\n";
    ModernFurnitureFactory modern;
    furnishRoom(modern);

    cout << "\nUsing Victorian Furniture Factory:\n";
    VictorianFurnitureFactory victorian;
    furnishRoom(victorian);
}
```

### 🧩 Notes

* Each concrete factory creates **related products** (Chair + Sofa) that match in style.
* The client uses only `FurnitureFactory` interface → no dependency on concrete classes.
* Adding a new furniture *family* is easy — add one new factory class.

---


---

Would you like me to extend this series next with **Builder Pattern** (which is the next in creational design patterns — focuses on *step-by-step construction* instead of selection)?
