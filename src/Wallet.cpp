#include "../include/Wallet.h"
#include <sstream>

Wallet::Wallet() : id(0), name(""), balance(0.0) {}

Wallet::Wallet(int id, const std::string& name, double balance) : id(id), name(name), balance(balance) {}

int Wallet::getId() const {
    return id;
}

std::string Wallet::getName() const {
    return name;
}

double Wallet::getBalance() const {
    return balance;
}

void Wallet::setName(const std::string& newName) {
    name = newName;
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

std::string Wallet::toString() const {
    std::ostringstream out;
    out << id << " | " << name << " | " << balance;
    return out.str();
}