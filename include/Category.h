#ifndef CATEGORY_H
#define CATEGORY_H

#include <string>

class Category {
    public:
        Category(int id, const std::string& name, bool isIncome);

        int getId() const;
        std::string getName() const;
        bool getIsIncome() const;

        std::string toString() const;

    private:
        int id;
        std::string name;
        bool isIncome;
};

#endif