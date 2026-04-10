#include "../include/Statistics.h"

double Statistics::totalIncome(const std::vector<Transaction>& transactions) {
	double total = 0.0;
	for (const Transaction& transaction : transactions) {
		if (transaction.getIsIncome()) {
			total += transaction.getAmount();
		}
	}
	return total;
}

double Statistics::totalExpense(const std::vector<Transaction>& transactions) {
	double total = 0.0;
	for (const Transaction& transaction : transactions) {
		if (!transaction.getIsIncome()) {
			total += transaction.getAmount();
		}
	}
	return total;
}

double Statistics::totalIncomeByWallet(const std::vector<Transaction>& transactions, int walletId) {
	double total = 0.0;
	for (const Transaction& transaction : transactions) {
		if (transaction.getWalletId() == walletId && transaction.getIsIncome()) {
			total += transaction.getAmount();
		}
	}
	return total;
}

double Statistics::totalExpenseByWallet(const std::vector<Transaction>& transactions, int walletId) {
	double total = 0.0;
	for (const Transaction& transaction : transactions) {
		if (transaction.getWalletId() == walletId && !transaction.getIsIncome()) {
			total += transaction.getAmount();
		}
	}
	return total;
}
