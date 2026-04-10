#ifndef WALLET_H
#define WALLET_H

#include <string>

class Wallet {
    public:
        Wallet();
        Wallet(int id, const std::string& name, double balance);

        int getId() const;
        std::string getName() const;
        double getBalance() const;

        void setName(const std::string& newName);
        void addMoney(double amount);
        bool spendMoney(double amount);
        std::string toString() const;

    private:
        int id;
        std::string name;
        double balance;
};

#endif