#include "../include/Transaction.h"
#include <sstream>

Transaction::Transaction()
    : id(0), amount(0.0), categoryId(0), walletId(0), isIncome(false), date(""), description("") {}

Transaction::Transaction(int id, double amount, int categoryId, int walletId, bool isIncome, const std::string& date, const std::string& description)
    : id(id), amount(amount), categoryId(categoryId), walletId(walletId), isIncome(isIncome), date(date), description(description) {}

int Transaction::getId() const {
    return id;
}

double Transaction::getAmount() const {
    return amount;
}

int Transaction::getCategoryId() const {
    return categoryId;
}

int Transaction::getWalletId() const {
    return walletId;
}

bool Transaction::getIsIncome() const {
    return isIncome;
}

std::string Transaction::getDate() const {
    return date;
}

std::string Transaction::getDescription() const {
    return description;
}

std::string Transaction::toString() const {
    std::ostringstream out;
    out << id << " | " << date << " | " << (isIncome ? "IN" : "OUT") << " | " << amount
        << " | wallet=" << walletId << " | category=" << categoryId << " | " << description;
    return out.str();
}