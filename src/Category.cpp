#include "../include/Category.h"

Category::Category(int id, const std::string& name, bool isIncome)
    : id(id), name(name), isIncome(isIncome) {}

int Category::getId() const {
    return id;
}

std::string Category::getName() const {
    return name;
}

bool Category::getIsIncome() const {
    return isIncome;
}

std::string Category::toString() const {
    std::string type = isIncome ? "Income" : "Expense";
    return std::to_string(id) + ": " + name + " (" + type + ")";
}