#include "../include/Wallet.h"
#include <iostream>

Wallet::Wallet(const std::string& name, double balance) : name(name), balance(balance) {}

std::string Wallet::getName() const {
    return name;
}

double Wallet::getBalance() const {
    return balance;
}

void Wallet::addMoney(double amount) {
    balance += amount;
}

bool Wallet::spendMoney(double amount) {
    if (amount > balance) {
        return false;
    }
    balance -= amount;
    return true;
}