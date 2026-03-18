#include <iostream>
#include <vector>
#include "include/Category.h"
#include "include/Transaction.h"
#include "include/Wallet.h"

int main() {

    Wallet cash("Cash", 500.0);
    std::vector <Transaction> transaction;
    std::vector <Category> category;

    int choice;
    while (true) {
        std::cout << "========= Financial Manager =========" << std::endl;
        std::cout << "Balance: " << cash.getBalance() << std::endl;
        std::cout << "1. Transactions List" << std::endl;
        std::cout << "2. Categories List" << std::endl;
        std::cout << "3. Add Income" << std::endl;
        std::cout << "4. Add Expense" << std::endl;
        std::cout << "0. Exit " << std::endl;
        std::cout << "Select an option: ";
        std::cin >> choice;

        if (choice == 0) break;

        if (choice == 1) {
            std::cout << "Transactions: " << std::endl;
            for (const auto& t : transaction) {
                std::cout << t.toString() << std::endl;
            }
        } 

        else if (choice == 2) {
            std::cout << "Categories: " << std::endl;
            for (const auto& c : category) 
            {
                std::cout << c.toString() << std::endl;
            }
        }
        else if (choice == 3) {
            double amount;
            std::cout << "Enter amount: ";
            std::cin >> amount;
            cash.addMoney(amount);

            transaction.push_back(Transaction(transaction.size() + 1, amount, 0, "2026-03-18", "Income"));

        }
        else if (choice == 4) {
            double amount;
            std::cout << "Enter amount: ";
            std::cin >> amount;
            if (cash.spendMoney(amount)) {
                transaction.push_back(Transaction(transaction.size() + 1, -amount, 0, "2026-03-18", "Expense"));
            } else {
                std::cout << "Not enough balance!" << std::endl;
            }
        }

    }
    return 0;
}