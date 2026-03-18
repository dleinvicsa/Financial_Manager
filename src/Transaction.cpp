#include "../include/Transaction.h"

Transaction::Transaction(int id, double amount, int categoryId, const std::string& date, const std::string& description)
    : id(id), amount(amount), categoryId(categoryId), date(date), description(description) {}

int Transaction::getId() const {
    return id;
}

double Transaction::getAmount() const {
    return amount;
}

int Transaction::getCategoryId() const {
    return categoryId;
}

std::string Transaction::getDate() const {
    return date;
}

std::string Transaction::getDescription() const {
    return description;
}

std::string Transaction::toString() const {
    return date + " | " + std::to_string(amount) + " | " + description;  
}