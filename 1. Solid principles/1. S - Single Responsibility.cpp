S - Single Responsibility Principle

A class should have only 1 reason to change.

Yes, that’s correct! In the SOLID principles of object-oriented design, this concept is referred to as the Single Responsibility Principle (SRP). According to the SRP, a class should have only one reason to change, meaning it should have only one responsibility or job.
In practical terms, this means that a class should focus on a single part of the functionality provided by the software, and this part should be encapsulated within that class. If a class has more than one responsibility, changes in one area might affect or introduce bugs in another, making the code more difficult to maintain and understand.
By adhering to the Single Responsibility Principle, you make your code more modular, easier to test, and more resilient to changes.

=====================================================================================

Example without SRP

    class Employee {
    public:
        Employee(const std::string& name, double hourlyRate)
            : name(name), hourlyRate(hourlyRate) {}

        double calculateSalary(int hoursWorked) const {
            return hoursWorked * hourlyRate;
        }

        void generateReport(int hoursWorked) const {
            double salary = calculateSalary(hoursWorked);
            std::cout << "Employee Report\n";
            std::cout << "Name: " << name << "\n";
            std::cout << "Hours Worked: " << hoursWorked << "\n";
            std::cout << "Salary: $" << salary << "\n";
        }

    private:
        std::string name;
        double hourlyRate;
    };

    int main() {
        Employee employee("Alice Johnson", 50.0);
        employee.generateReport(40); // Generates a report for 40 hours of work
        return 0;
    }

---------------------------------------------------------------------------------------

Example with SRP 

    // Class responsible for managing employee data and calculating salary
    class Employee {
    public:
        Employee(const std::string& name, double hourlyRate)
            : name(name), hourlyRate(hourlyRate) {}

        double calculateSalary(int hoursWorked) const {
            return hoursWorked * hourlyRate;
        }

        std::string getName() const {
            return name;
        }

    private:
        std::string name;
        double hourlyRate;
    };

    // Class responsible for generating reports for employees
    class EmployeeReport {
    public:
        void generateReport(const Employee& employee, int hoursWorked) const {
            double salary = employee.calculateSalary(hoursWorked);
            std::cout << "Employee Report\n";
            std::cout << "Name: " << employee.getName() << "\n";
            std::cout << "Hours Worked: " << hoursWorked << "\n";
            std::cout << "Salary: $" << salary << "\n";
        }
    };

    int main() {
        Employee employee("Alice Johnson", 50.0);

        EmployeeReport report;
        report.generateReport(employee, 40); // Generates a report for 40 hours of work

        return 0;
    }
