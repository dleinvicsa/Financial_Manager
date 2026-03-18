#ifndef WALLET_H
#define WALLET_H

#include <string>

class Wallet {
    public:
        Wallet(const std::string& name, double balance);

        std::string getName() const;
        double getBalance() const;

        void addMoney(double amount);
        bool spendMoney(double amount);

    private:
        std::string name;
        double balance;
};

#endif