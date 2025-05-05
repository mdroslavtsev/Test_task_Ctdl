#include <iostream>
#include <fstream>
#include <sstream>
#include <vector>
#include <string>
#include <iomanip>
#include <algorithm>
#include <limits>

class Product
{
public:
    std::string name;
    std::string barcode;
    double price;
};

class ProductDB
{
private:
    std::vector<Product> products;

public:
    bool loadFromCSV(const std::string &filename)
    {
        std::ifstream file(filename);
        if (!file.is_open())
            return false;

        std::string line;
        std::getline(file, line); // Пропускаем заголовок

        while (std::getline(file, line))
        {
            std::istringstream ss(line);
            std::string name, barcode, priceStr;

            if (std::getline(ss, name, ';') &&
                std::getline(ss, barcode, ';') &&
                std::getline(ss, priceStr))
            {

                // Убираем пробелы
                auto trim = [](std::string &s)
                {
                    s.erase(s.find_last_not_of(" \t\n\r\f\v") + 1);
                    s.erase(0, s.find_first_not_of(" \t\n\r\f\v"));
                };

                trim(name);
                trim(barcode);
                trim(priceStr);

                try
                {
                    double price = std::stod(priceStr);
                    products.push_back(Product{name, barcode, price});
                }
                catch (...)
                {
                    continue;
                }
            }
        }
        return true;
    }

    Product* findProductByBarcode(const std::string& barcode) {
        for (auto& product : products) {
            if (product.barcode == barcode) {
                return &product;
            }
        }
        return nullptr;
    }

    std::vector<Product *> findProductsByName(const std::string &name)
    {
        std::vector<Product *> matches;
        std::string lowerName = toLower(name);
        for (auto &product : products)
        {
            if (toLower(product.name).find(lowerName) != std::string::npos)
            {
                matches.push_back(&product);
            }
        }
        return matches;
    }

private:
    std::string toLower(const std::string& str) {
        std::string lowerStr = str;
        std::transform(lowerStr.begin(), lowerStr.end(), lowerStr.begin(), ::tolower);
        return lowerStr;
}
};

struct ReceiptItem
{
    Product product;
    int quantity;
    double total() const { return product.price * quantity; }
};

class Receipt
{
private:
    std::vector<ReceiptItem> items;
    double totalAmount = 0.0;
    std::string paymentMethod;
    double paymentAmount = 0.0;
    double change = 0.0;

public:
    void addItem(const Product &product, int quantity)
    {
        for (auto &item : items)
        {
            if (item.product.barcode == product.barcode)
            {
                item.quantity += quantity;
                totalAmount += product.price * quantity;
                return;
            }
        }
        items.push_back({product, quantity});
        totalAmount += product.price * quantity;
    }

    double getTotal() const { return totalAmount; }

    bool setPayment(const std::string &method, double amount, double initialCash)
    {
        paymentMethod = method;
        paymentAmount = amount;
        if (method == "cash")
        {
            if (amount < totalAmount)
                return false;
            change = amount - totalAmount;
            if (initialCash + totalAmount - change < 0)
                return false;
        }
        return true;
    }

    void print() const
    {
        std::cout << "\n---------------- Receipt ----------------\n";
        for (const auto &item : items)
        {
            std::cout << std::left << std::setw(20) << item.product.name
                      << " x" << item.quantity
                      << " @ " << std::fixed << std::setprecision(2) << item.product.price
                      << " = " << item.total() << "\n";
        }
        std::cout << "-----------------------------------------\n";
        std::cout << "Total: " << std::fixed << std::setprecision(2) << totalAmount << "\n";
        std::cout << "Payment Method: " << paymentMethod << "\n";
        std::cout << "Amount Received: " << paymentAmount << "\n";
        if (paymentMethod == "cash")
        {
            std::cout << "Change: " << change << "\n";
        }
        std::cout << "-----------------------------------------\n\n";
    }

    void clear()
    {
        items.clear();
        totalAmount = 0.0;
        paymentMethod.clear();
        paymentAmount = 0.0;
        change = 0.0;
    }
};

class Shift
{
private:
    std::string cashierName;
    double initialCash;
    double totalCashSales = 0.0;
    double totalCardSales = 0.0;
    double totalChange = 0.0;
    bool isOpenFlag = false;

public:
    void open(const std::string &name, double cash)
    {
        cashierName = name;
        initialCash = cash;
        isOpenFlag = true;
    }

    void close()
    {
        isOpenFlag = false;
        printShiftReport();
    }

    bool isOpen() const { return isOpenFlag; }

    void addCashSale(double amount, double change)
    {
        totalCashSales += amount;
        totalChange += change;
    }

    void addCardSale(double amount)
    {
        totalCardSales += amount;
    }

    void printShiftReport() const
    {
        std::cout << "\n=============== Shift Report ===============\n";
        std::cout << "Cashier: " << cashierName << "\n";
        std::cout << "Initial Cash: " << std::fixed << std::setprecision(2) << initialCash << "\n";
        std::cout << "Total Cash Sales: " << totalCashSales << "\n";
        std::cout << "Total Card Sales: " << totalCardSales << "\n";
        std::cout << "Total Sales: " << (totalCashSales + totalCardSales) << "\n";
        std::cout << "Total Change Given: " << totalChange << "\n";
        std::cout << "Final Cash: " << (initialCash + totalCashSales - totalChange) << "\n";
        std::cout << "==========================================\n\n";
    }

    double getAvailableCash() const
    {
        return initialCash + totalCashSales - totalChange;
    }
};

double readDouble(const std::string &prompt)
{
    double value;
    while (true)
    {
        std::cout << prompt;
        std::cin >> value;
        if (std::cin.fail())
        {
            std::cin.clear();
            std::cin.ignore(std::numeric_limits<std::streamsize>::max(), '\n');
            std::cout << "Invalid input. Please enter a number.\n";
        }
        else
        {
            std::cin.ignore(std::numeric_limits<std::streamsize>::max(), '\n');
            return value >= 0 ? value : (std::cout << "Value cannot be negative. ", -1.0);
        }
    }
}

int readInt(const std::string &prompt)
{
    int value;
    while (true)
    {
        std::cout << prompt;
        std::cin >> value;
        if (std::cin.fail())
        {
            std::cin.clear();
            std::cin.ignore(std::numeric_limits<std::streamsize>::max(), '\n');
            std::cout << "Invalid input. Please enter an integer.\n";
        }
        else
        {
            std::cin.ignore(std::numeric_limits<std::streamsize>::max(), '\n');
            if (value > 0)
                return value;
            std::cout << "Value must be positive. Please try again.\n";
        }
    }
}

void handleReceipt(Receipt &receipt, ProductDB &db, Shift &shift)
{
    while (true)
    {
        std::cout << "Enter barcode/name or 'done' to finish: ";
        std::string input;
        std::getline(std::cin, input);

        if (input == "done")
            break;

        Product *product = db.findProductByBarcode(input);
        if (!product)
        {
            auto matches = db.findProductsByName(input);
            if (matches.empty())
            {
                std::cout << "Product not found.\n";
                continue;
            }
            if (matches.size() > 1)
            {
                std::cout << "Multiple products found:\n";
                for (size_t i = 0; i < matches.size(); ++i)
                {
                    std::cout << " " << i + 1 << ". " << matches[i]->name
                              << " (" << matches[i]->barcode << ") "
                              << matches[i]->price << "$\n";
                }
                int choice = readInt("Select product: ");
                if (choice < 1 || choice > matches.size())
                {
                    std::cout << "Invalid selection.\n";
                    continue;
                }
                product = matches[choice - 1];
            }
            else
            {
                product = matches[0];
            }
        }

        int qty = readInt("Enter quantity: ");
        receipt.addItem(*product, qty);
        std::cout << "Added " << qty << " x " << product->name << "\n";
    }

    std::cout << "\nCurrent receipt total: "
              << std::fixed << std::setprecision(2)
              << receipt.getTotal() << "$\n\n";
    if (receipt.getTotal() == 0.0)
    {
        std::cout << "No products in receipt." << std::endl;
        return;
    }
    while (true)
    {
        std::cout << "Payment method (cash/card): ";
        std::string method;
        std::getline(std::cin, method);
        if (method != "cash" && method != "card")
        {
            std::cout << "Invalid method.\n";
            continue;
        }

        double amount = 0.0;
        if (method == "cash")
        {
            amount = readDouble("Enter cash amount: ");
            if (!receipt.setPayment(method, amount, shift.getAvailableCash()))
            {
                std::cout << "Insufficient payment or change unavailable.\n";
                continue;
            }
            shift.addCashSale(receipt.getTotal(), amount - receipt.getTotal());
        }
        else
        {
            amount = receipt.getTotal();
            receipt.setPayment(method, amount, 0);
            shift.addCardSale(amount);
        }
        break;
    }

    receipt.print();
    receipt.clear();
}

int main(int argc, char *argv[])
{
    std::string filename = "products.csv";
    if (argc > 1)
        filename = argv[1];

    ProductDB db;
    if (!db.loadFromCSV(filename))
    {
        std::cerr << "Error: Failed to load product database from '"
                  << filename << "'\n";
        return 1;
    }

    Shift shift;
    Receipt currentReceipt;

    while (true)
    {
        if (!shift.isOpen())
        {
            std::cout << "1. Open Shift\n2. Exit\nChoose: ";
            int choice = readInt("");
            if (choice == 1)
            {
                std::cout << "Cashier name: ";
                std::string name;
                std::getline(std::cin, name);
                double cash = readDouble("Initial cash: ");
                shift.open(name, cash);
            }
            else if (choice == 2)
            {
                break;
            }
            else
            {
                std::cout << "Invalid choice.\n";
            }
        }
        else
        {
            std::cout << "1. New Receipt\n2. Close Shift\nChoose: ";
            int choice = readInt("");
            if (choice == 1)
            {
                handleReceipt(currentReceipt, db, shift);
            }
            else if (choice == 2)
            {
                shift.close();
            }
            else
            {
                std::cout << "Invalid choice.\n";
            }
        }
    }

    return 0;
}