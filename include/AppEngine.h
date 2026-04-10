#ifndef APPENGINE_H
#define APPENGINE_H

#include <vector>
#include "Category.h"
#include "Wallet.h"
#include "Transaction.h"
#include "DataManager.h"

class AppEngine {
	public:
		AppEngine();
		void run();

	private:
		DataManager dataManager;
		Account activeAccount;
		Profile profile;
		std::vector<Category> categories;
		std::vector<Wallet> wallets;
		std::vector<Transaction> transactions;
		int nextCategoryId;
		int nextWalletId;
		int nextTransactionId;

		bool authenticationMenu();
		bool registerAccount();
		bool loginAccount();
		std::string hashPassword(const std::string& password) const;

		void loadData();
		void saveAll() const;
		void initializeDefaults();

		void showMainMenu() const;
		void personalCabinetMenu();
		void showCabinetOverview() const;
		double totalWalletBalance() const;
		void setupProfileIfNeeded();

		void listWallets() const;
		void createWallet();
		void deleteWallet();
		void topUpWallet();

		void listCategories(bool incomeType) const;
		void createCategory(bool incomeType);
		void deleteCategory(bool incomeType);

		void addTransaction(bool incomeType);
		void listTransactions() const;
		void showStatistics() const;

		int chooseWalletId() const;
		int chooseCategoryId(bool incomeType) const;
		int walletIndexById(int walletId) const;
		int categoryIndexById(int categoryId) const;
};

#endif
