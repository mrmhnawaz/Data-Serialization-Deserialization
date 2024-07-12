#include <iostream>
#include <fstream>
#include <vector>
#include <string>
#include <limits>

class Employee {
public:
    std::string name;
    float salary;

    Employee() : name(""), salary(0) {}
    Employee(const std::string& n, float s) : name(n), salary(s) {}

    void serialize(std::ofstream& out) const {
        size_t nameLength = name.length();
        out.write(reinterpret_cast<const char*>(&nameLength), sizeof(nameLength));
        out.write(name.c_str(), nameLength);
        out.write(reinterpret_cast<const char*>(&salary), sizeof(salary));
    }

    static Employee deserialize(std::ifstream& in) {
        size_t nameLength;
        in.read(reinterpret_cast<char*>(&nameLength), sizeof(nameLength));
        std::string name(nameLength, ' ');
        in.read(&name[0], nameLength);
        float salary;
        in.read(reinterpret_cast<char*>(&salary), sizeof(salary));
        return Employee(name, salary);
    }
};

class Customer {
public:
    std::string name;
    std::string address;

    Customer() : name(""), address("") {}
    Customer(const std::string& n, const std::string& a) : name(n), address(a) {}

    void serialize(std::ofstream& out) const {
        size_t nameLength = name.length();
        size_t addressLength = address.length();
        out.write(reinterpret_cast<const char*>(&nameLength), sizeof(nameLength));
        out.write(name.c_str(), nameLength);
        out.write(reinterpret_cast<const char*>(&addressLength), sizeof(addressLength));
        out.write(address.c_str(), addressLength);
    }

    static Customer deserialize(std::ifstream& in) {
        size_t nameLength, addressLength;
        in.read(reinterpret_cast<char*>(&nameLength), sizeof(nameLength));
        std::string name(nameLength, ' ');
        in.read(&name[0], nameLength);
        in.read(reinterpret_cast<char*>(&addressLength), sizeof(addressLength));
        std::string address(addressLength, ' ');
        in.read(&address[0], addressLength);
        return Customer(name, address);
    }
};

class Sale {
public:
    Customer customer;
    Employee salerep;
    float amount;
    std::string date;

    Sale() : customer(), salerep(), amount(0), date("") {}
    Sale(const Customer& c, const Employee& e, float a, const std::string& d) : customer(c), salerep(e), amount(a), date(d) {}

    void serialize(std::ofstream& out) const {
        customer.serialize(out);
        salerep.serialize(out);
        out.write(reinterpret_cast<const char*>(&amount), sizeof(amount));
        size_t dateLength = date.length();
        out.write(reinterpret_cast<const char*>(&dateLength), sizeof(dateLength));
        out.write(date.c_str(), dateLength);
    }

    static Sale deserialize(std::ifstream& in) {
        Customer customer = Customer::deserialize(in);
        Employee salerep = Employee::deserialize(in);
        float amount;
        in.read(reinterpret_cast<char*>(&amount), sizeof(amount));
        size_t dateLength;
        in.read(reinterpret_cast<char*>(&dateLength), sizeof(dateLength));
        std::string date(dateLength, ' ');
        in.read(&date[0], dateLength);
        return Sale(customer, salerep, amount, date);
    }
};

std::vector<Employee> employees;
std::vector<Customer> customers;
std::vector<Sale> sales;

void addEmployee() {
    std::string name;
    float salary;
    std::cout << "Enter employee name: ";
    std::getline(std::cin >> std::ws, name); // Read input with spaces
    std::cout << "Enter employee salary: ";
    std::cin >> salary;
    employees.emplace_back(name, salary);
    std::cout << "Employee " << name << " with salary " << salary << " added." << std::endl;
}

void addCustomer() {
    std::string name, address;
    std::cout << "Enter customer name: ";
    std::getline(std::cin >> std::ws, name); // Read input with spaces
    std::cout << "Enter customer address: ";
    std::getline(std::cin, address);
    customers.emplace_back(name, address);
    std::cout << "Customer " << name << " with address " << address << " added." << std::endl;
}

void addSale() {
    std::string customerName, employeeName, date;
    float amount;
    std::cout << "Enter customer name: ";
    std::getline(std::cin >> std::ws, customerName); // Read input with spaces
    std::cout << "Enter sales rep name: ";
    std::getline(std::cin, employeeName); // Read input with spaces
    std::cout << "Enter sale amount: ";
    std::cin >> amount;
    std::cout << "Enter sale date: ";
    std::getline(std::cin >> std::ws, date); // Read input with spaces

    Customer* customer = nullptr;
    Employee* employee = nullptr;

    for (auto& c : customers) {
        if (c.name == customerName) {
            customer = &c;
            break;
        }
    }

    for (auto& e : employees) {
        if (e.name == employeeName) {
            employee = &e;
            break;
        }
    }

    if (customer && employee) {
        sales.emplace_back(*customer, *employee, amount, date);
        std::cout << "Sale added: Customer " << customerName << ", Employee " << employeeName << ", Amount " << amount << ", Date " << date << std::endl;
    } else {
        std::cout << "Invalid customer or employee name." << std::endl;
    }
}

std::vector<char> serialize() {
    std::vector<char> byteArray;
    std::ofstream out("Data.bin", std::ios::binary);

    size_t employeeCount = employees.size();
    out.write(reinterpret_cast<const char*>(&employeeCount), sizeof(employeeCount));
    for (const auto& e : employees) {
        e.serialize(out);
    }

    size_t customerCount = customers.size();
    out.write(reinterpret_cast<const char*>(&customerCount), sizeof(customerCount));
    for (const auto& c : customers) {
        c.serialize(out);
    }

    size_t saleCount = sales.size();
    out.write(reinterpret_cast<const char*>(&saleCount), sizeof(saleCount));
    for (const auto& s : sales) {
        s.serialize(out);
    }

    std::ifstream in("Data.bin", std::ios::binary);
    byteArray = std::vector<char>((std::istreambuf_iterator<char>(in)), std::istreambuf_iterator<char>());

    return byteArray;
}

void deserialize(const std::vector<char>& byteArray) {
    std::ofstream out("Temp.bin", std::ios::binary);
    out.write(byteArray.data(), byteArray.size());
    out.close();

    std::ifstream in("Temp.bin", std::ios::binary);

    size_t employeeCount, customerCount, saleCount;
    in.read(reinterpret_cast<char*>(&employeeCount), sizeof(employeeCount));
    employees.clear();
    for (size_t i = 0; i < employeeCount; ++i) {
        employees.push_back(Employee::deserialize(in));
    }

    in.read(reinterpret_cast<char*>(&customerCount), sizeof(customerCount));
    customers.clear();
    for (size_t i = 0; i < customerCount; ++i) {
        customers.push_back(Customer::deserialize(in));
    }

    in.read(reinterpret_cast<char*>(&saleCount), sizeof(saleCount));
    sales.clear();
    for (size_t i = 0; i < saleCount; ++i) {
        sales.push_back(Sale::deserialize(in));
    }

    std::remove("Temp.bin");
}

void loadFile() {
    std::ifstream in("Data.bin", std::ios::binary);

    if (!in) {
        std::cout << "Data.bin not found. Starting with an empty dataset." << std::endl;
        return;
    }

    size_t employeeCount, customerCount, saleCount;
    in.read(reinterpret_cast<char*>(&employeeCount), sizeof(employeeCount));
    employees.clear();
    for (size_t i = 0; i < employeeCount; ++i) {
        employees.push_back(Employee::deserialize(in));
    }

    in.read(reinterpret_cast<char*>(&customerCount), sizeof(customerCount));
    customers.clear();
    for (size_t i = 0; i < customerCount; ++i) {
        customers.push_back(Customer::deserialize(in));
    }

    in.read(reinterpret_cast<char*>(&saleCount), sizeof(saleCount));
    sales.clear();
    for (size_t i = 0; i < saleCount; ++i) {
        sales.push_back(Sale::deserialize(in));
    }
}

void writeFile() {
    serialize();
}

int main() {
    loadFile();

    int choice;
    do {
        std::cout << "1. Add Employee\n2. Add Customer\n3. Add Sale\n4. Exit\nEnter your choice: ";
        std::cin >> choice;

        switch (choice) {
        case 1:
            addEmployee();
            break;
        case 2:
            addCustomer();
            break;
        case 3:
            addSale();
            break;
        case 4:
            writeFile();
            std::cout << "Data saved to Data.bin. Exiting..." << std::endl;
            break;
        default:
            std::cout << "Invalid choice. Try again." << std::endl;
            std::cin.clear();
            std::cin.ignore(std::numeric_limits<std::streamsize>::max(), '\n');
        }
    } while (choice != 4);

    return 0;
}
