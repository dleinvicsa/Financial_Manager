#ifndef DATAMANAGER_H
#define DATAMANAGER_H

#include <string>
#include <vector>
#include "Category.h"
#include "Wallet.h"
#include "Transaction.h"

struct Profile {
	std::string ownerName;
	std::string email;
	std::string createdAt;
};

struct Account {
	std::string username;
	std::string passwordHash;
	std::string email;
	std::string createdAt;
};

class DataManager {
	public:
		explicit DataManager(const std::string& dataPath = "data");

		std::vector<Account> loadAccounts() const;
		bool accountExists(const std::string& username) const;
		bool createAccount(const Account& account) const;
		bool validateLogin(const std::string& username, const std::string& passwordHash) const;
		void setActiveUser(const std::string& username);
		std::string getActiveUser() const;

		std::vector<Category> loadCategories() const;
		std::vector<Wallet> loadWallets() const;
		std::vector<Transaction> loadTransactions() const;
		Profile loadProfile() const;

		void saveCategories(const std::vector<Category>& categories) const;
		void saveWallets(const std::vector<Wallet>& wallets) const;
		void saveTransactions(const std::vector<Transaction>& transactions) const;
		void saveProfile(const Profile& profile) const;

	private:
		std::string dataRoot;
		std::string accountsFile;
		std::string activeUser;
		std::string categoriesFile;
		std::string walletsFile;
		std::string transactionsFile;
		std::string profileFile;

		void ensureFileExists(const std::string& path) const;
		std::string sanitizeUsername(const std::string& username) const;
		void refreshUserFiles();
};

#endif
