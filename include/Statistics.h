#ifndef STATISTICS_H
#define STATISTICS_H

#include <vector>
#include "Transaction.h"

class Statistics {
	public:
		static double totalIncome(const std::vector<Transaction>& transactions);
		static double totalExpense(const std::vector<Transaction>& transactions);
		static double totalIncomeByWallet(const std::vector<Transaction>& transactions, int walletId);
		static double totalExpenseByWallet(const std::vector<Transaction>& transactions, int walletId);
};

#endif
