#ifndef TRANSACTION_H
#define TRANSACTION_H

#include <string>
#include "Category.h"

class Transaction {
    public:
        Transaction();
        Transaction(int id, double amount, int categoryId, int walletId, bool isIncome, const std::string& date, const std::string& description);
        
        int getId() const;
        double getAmount() const;
        int getCategoryId() const;
        int getWalletId() const;
        bool getIsIncome() const;
        std::string getDate() const;
        std::string getDescription() const;

        std::string toString() const;

    private:
        int id;
        double amount;
        int categoryId;
        int walletId;
        bool isIncome;
        std::string date;
        std::string description;
};

#endif