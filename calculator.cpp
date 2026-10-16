#include <iostream>
#include <string>
#include <limits>

int main() {
    std::string input;

    while (true) {
        std::cout << "Enter expression (e.g., 5 + 3) or 'q' to quit: ";
        if (!(std::cin >> input)) break;
        if (input == "q" || input == "Q") break;

        try {
            size_t pos;
            double num1 = std::stod(input, &pos);
            
            char op;
            if (!(std::cin >> op)) {
                std::cout << "Error: Missing operator." << std::endl;
                std::cin.clear();
                std::cin.ignore(std::numeric_limits<std::streamsize>::max(), '\n');
                continue;
            }

            double num2;
            if (!(std::cin >> num2)) {
                std::cout << "Error: Missing second operand." << std::endl;
                std::cin.clear();
                std::cin.ignore(std::numeric_limits<std::streamsize>::max(), '\n');
                continue;
            }

            if (op == '+') {
                std::cout << "Result: " << num1 + num2 << std::endl;
            } else if (op == '-') {
                std::cout << "Result: " << num1 - num2 << std::endl;
            } else if (op == '*') {
                std::cout << "Result: " << num1 * num2 << std::endl;
            } else if (op == '/') {
                if (num2 == 0) {
                    std::cout << "Error: Division by zero!" << std::endl;
                } else {
                    std::cout << "Result: " << num1 / num2 << std::endl;
                }
            } else {
                std::cout << "Error: Unsupported operator '" << op << "'." << std::endl;
            }
        } catch (const std::invalid_argument&) {
            std::cout << "Error: Invalid numeric input." << std::endl;
            std::cin.clear();
            std::cin.ignore(std::numeric_limits<std::streamsize>::max(), '\n');
        } catch (const std::out_of_range&) {
            std::cout << "Error: Number out of range." << std::endl;
            std::cin.clear();
            std::cin.ignore(std::numeric_limits<std::streamsize>::max(), '\n');
        }
        std::cin.ignore(std::numeric_limits<std::streamsize>::max(), '\n');
    }
    return 0;
}
