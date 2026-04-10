#include "../include/AppEngine.h"

#include <functional>
#include <iomanip>
#include <iostream>
#include "../include/Statistics.h"
#include "../include/Utils.h"

AppEngine::AppEngine() : nextCategoryId(1), nextWalletId(1), nextTransactionId(1) {
}

void AppEngine::run() {
	if (!authenticationMenu()) {
		return;
	}
	loadData();
	initializeDefaults();
	setupProfileIfNeeded();

	while (true) {
		showMainMenu();
		int choice = Utils::readInt("Choose an option: ");

		if (choice == 0) {
			saveAll();
			break;
		}

		if (choice == 1) {
			personalCabinetMenu();
		} else if (choice == 2) {
			topUpWallet();
		} else if (choice == 3) {
			addTransaction(true);
		} else if (choice == 4) {
			addTransaction(false);
		} else if (choice == 5) {
			listTransactions();
		} else if (choice == 6) {
			showStatistics();
		} else {
			std::cout << "Unknown option.\n";
		}
	}
}

bool AppEngine::authenticationMenu() {
	while (true) {
		std::cout << "\n===== Authorization =====\n";
		std::cout << "1. Register\n";
		std::cout << "2. Login\n";
		std::cout << "0. Exit\n";

		int choice = Utils::readInt("Choose an option: ");
		if (choice == 0) {
			return false;
		}
		if (choice == 1) {
			if (registerAccount()) {
				return true;
			}
		} else if (choice == 2) {
			if (loginAccount()) {
				return true;
			}
		} else {
			std::cout << "Unknown option.\n";
		}
	}
}

bool AppEngine::registerAccount() {
	std::cout << "\n===== Register =====\n";
	std::string username = Utils::readLine("Username: ");
	std::string email = Utils::readLine("Email: ");
	std::string password = Utils::readLine("Password: ");
	std::string passwordRepeat = Utils::readLine("Repeat password: ");

	if (username.empty() || email.empty() || password.empty()) {
		std::cout << "All fields are required.\n";
		return false;
	}
	if (password != passwordRepeat) {
		std::cout << "Passwords do not match.\n";
		return false;
	}
	if (dataManager.accountExists(username)) {
		std::cout << "Username already exists.\n";
		return false;
	}

	Account account;
	account.username = username;
	account.passwordHash = hashPassword(password);
	account.email = email;
	account.createdAt = Utils::getCurrentDate();

	if (!dataManager.createAccount(account)) {
		std::cout << "Failed to create account.\n";
		return false;
	}

	activeAccount = account;
	dataManager.setActiveUser(activeAccount.username);

	profile.ownerName = username;
	profile.email = email;
	profile.createdAt = account.createdAt;
	dataManager.saveProfile(profile);

	std::cout << "Registration successful.\n";
	return true;
}

bool AppEngine::loginAccount() {
	std::cout << "\n===== Login =====\n";
	std::string username = Utils::readLine("Username: ");
	std::string password = Utils::readLine("Password: ");

	if (!dataManager.validateLogin(username, hashPassword(password))) {
		std::cout << "Invalid username or password.\n";
		return false;
	}

	std::vector<Account> accounts = dataManager.loadAccounts();
	for (const Account& account : accounts) {
		if (account.username == username) {
			activeAccount = account;
			break;
		}
	}

	dataManager.setActiveUser(username);
	std::cout << "Login successful.\n";
	return true;
}

std::string AppEngine::hashPassword(const std::string& password) const {
	std::size_t value = std::hash<std::string>{}(password);
	return std::to_string(value);
}

void AppEngine::loadData() {
	profile = dataManager.loadProfile();
	categories = dataManager.loadCategories();
	wallets = dataManager.loadWallets();
	transactions = dataManager.loadTransactions();
	nextCategoryId = 1;
	nextWalletId = 1;
	nextTransactionId = 1;

	for (const Category& category : categories) {
		if (category.getId() >= nextCategoryId) {
			nextCategoryId = category.getId() + 1;
		}
	}
	for (const Wallet& wallet : wallets) {
		if (wallet.getId() >= nextWalletId) {
			nextWalletId = wallet.getId() + 1;
		}
	}
	for (const Transaction& transaction : transactions) {
		if (transaction.getId() >= nextTransactionId) {
			nextTransactionId = transaction.getId() + 1;
		}
	}
}

void AppEngine::saveAll() const {
	dataManager.saveProfile(profile);
	dataManager.saveCategories(categories);
	dataManager.saveWallets(wallets);
	dataManager.saveTransactions(transactions);
}

void AppEngine::initializeDefaults() {
	bool hasIncome = false;
	bool hasExpense = false;
	for (const Category& category : categories) {
		if (category.getIsIncome()) {
			hasIncome = true;
		} else {
			hasExpense = true;
		}
	}

	if (!hasIncome) {
		categories.emplace_back(nextCategoryId++, "Salary", true);
		categories.emplace_back(nextCategoryId++, "Freelance", true);
	}

	if (!hasExpense) {
		categories.emplace_back(nextCategoryId++, "Food", false);
		categories.emplace_back(nextCategoryId++, "Transport", false);
	}

	if (wallets.empty()) {
		wallets.emplace_back(nextWalletId++, "Cash", 0.0);
	}

	saveAll();
}

void AppEngine::showMainMenu() const {
	std::cout << "\n===== Financial Manager =====\n";
	std::cout << "User: " << activeAccount.username << "\n";
	std::cout << "1. Personal cabinet\n";
	std::cout << "2. Top up wallet\n";
	std::cout << "3. Add income\n";
	std::cout << "4. Add expense\n";
	std::cout << "5. Transactions\n";
	std::cout << "6. Statistics\n";
	std::cout << "0. Exit\n";
}

void AppEngine::personalCabinetMenu() {
	while (true) {
		showCabinetOverview();
		std::cout << "1. Create wallet\n";
		std::cout << "2. Delete wallet\n";
		std::cout << "3. Show categories\n";
		std::cout << "4. Add income category\n";
		std::cout << "5. Add expense category\n";
		std::cout << "6. Delete income category\n";
		std::cout << "7. Delete expense category\n";
		std::cout << "0. Back\n";

		int choice = Utils::readInt("Choose an option: ");
		if (choice == 0) {
			return;
		}
		if (choice == 1) {
			createWallet();
		} else if (choice == 2) {
			deleteWallet();
		} else if (choice == 3) {
			listCategories(true);
			listCategories(false);
		} else if (choice == 4) {
			createCategory(true);
		} else if (choice == 5) {
			createCategory(false);
		} else if (choice == 6) {
			deleteCategory(true);
		} else if (choice == 7) {
			deleteCategory(false);
		} else {
			std::cout << "Unknown option.\n";
		}
	}
}

void AppEngine::showCabinetOverview() const {
	std::cout << "\n===== Personal Cabinet =====\n";
	std::cout << "Name: " << profile.ownerName << "\n";
	std::cout << "Email: " << profile.email << "\n";
	std::cout << "Created: " << profile.createdAt << "\n";
	std::cout << "Total balance: " << totalWalletBalance() << "\n";
	listWallets();
}

double AppEngine::totalWalletBalance() const {
	double total = 0.0;
	for (const Wallet& wallet : wallets) {
		total += wallet.getBalance();
	}
	return total;
}

void AppEngine::setupProfileIfNeeded() {
	if (!profile.ownerName.empty()) {
		return;
	}
	profile.ownerName = activeAccount.username;
	profile.email = activeAccount.email;
	profile.createdAt = activeAccount.createdAt.empty() ? Utils::getCurrentDate() : activeAccount.createdAt;
	saveAll();
}

void AppEngine::listWallets() const {
	std::cout << "\n===== Wallets =====\n";
	for (const Wallet& wallet : wallets) {
		std::cout << wallet.toString() << "\n";
	}
}

void AppEngine::createWallet() {
	std::string name = Utils::readLine("Wallet name: ");
	double balance = Utils::readDouble("Initial balance: ");
	if (name.empty() || balance < 0.0) {
		std::cout << "Invalid wallet data.\n";
		return;
	}

	wallets.emplace_back(nextWalletId++, name, balance);
	saveAll();
	std::cout << "Wallet created.\n";
}

void AppEngine::deleteWallet() {
	if (wallets.empty()) {
		std::cout << "No wallets available.\n";
		return;
	}

	int walletId = chooseWalletId();
	if (walletId < 0) {
		return;
	}

	int index = walletIndexById(walletId);
	if (index < 0) {
		std::cout << "Wallet not found.\n";
		return;
	}

	for (const Transaction& transaction : transactions) {
		if (transaction.getWalletId() == walletId) {
			std::cout << "Cannot delete wallet with transactions history.\n";
			return;
		}
	}

	wallets.erase(wallets.begin() + index);
	saveAll();
	std::cout << "Wallet deleted.\n";
}

void AppEngine::topUpWallet() {
	int walletId = chooseWalletId();
	if (walletId < 0) {
		return;
	}

	double amount = Utils::readDouble("Amount to top up: ");
	if (amount <= 0.0) {
		std::cout << "Amount must be positive.\n";
		return;
	}

	int index = walletIndexById(walletId);
	if (index < 0) {
		std::cout << "Wallet not found.\n";
		return;
	}

	wallets[index].addMoney(amount);
	saveAll();
	std::cout << "Wallet topped up.\n";
}

void AppEngine::listCategories(bool incomeType) const {
	std::cout << (incomeType ? "\n===== Income Categories =====\n" : "\n===== Expense Categories =====\n");
	for (const Category& category : categories) {
		if (category.getIsIncome() == incomeType) {
			std::cout << category.toString() << "\n";
		}
	}
}

void AppEngine::createCategory(bool incomeType) {
	std::string name = Utils::readLine("Category name: ");
	if (name.empty()) {
		std::cout << "Category name cannot be empty.\n";
		return;
	}

	categories.emplace_back(nextCategoryId++, name, incomeType);
	saveAll();
	std::cout << "Category created.\n";
}

void AppEngine::deleteCategory(bool incomeType) {
	int categoryId = chooseCategoryId(incomeType);
	if (categoryId < 0) {
		return;
	}

	for (const Transaction& transaction : transactions) {
		if (transaction.getCategoryId() == categoryId) {
			std::cout << "Cannot delete category with transactions history.\n";
			return;
		}
	}

	int index = categoryIndexById(categoryId);
	if (index < 0) {
		std::cout << "Category not found.\n";
		return;
	}

	categories.erase(categories.begin() + index);
	saveAll();
	std::cout << "Category deleted.\n";
}

void AppEngine::addTransaction(bool incomeType) {
	int walletId = chooseWalletId();
	if (walletId < 0) {
		return;
	}

	int categoryId = chooseCategoryId(incomeType);
	if (categoryId < 0) {
		return;
	}

	double amount = Utils::readDouble("Amount: ");
	if (amount <= 0.0) {
		std::cout << "Amount must be positive.\n";
		return;
	}

	std::string description = Utils::readLine("Description: ");

	int walletIndex = walletIndexById(walletId);
	if (walletIndex < 0) {
		std::cout << "Wallet not found.\n";
		return;
	}

	if (incomeType) {
		wallets[walletIndex].addMoney(amount);
	} else {
		if (!wallets[walletIndex].spendMoney(amount)) {
			std::cout << "Not enough balance.\n";
			return;
		}
	}

	transactions.emplace_back(
		nextTransactionId++,
		amount,
		categoryId,
		walletId,
		incomeType,
		Utils::getCurrentDate(),
		description
	);

	saveAll();
	std::cout << "Transaction saved.\n";
}

void AppEngine::listTransactions() const {
	std::cout << "\n===== Transactions =====\n";
	if (transactions.empty()) {
		std::cout << "No transactions yet.\n";
		return;
	}

	for (const Transaction& transaction : transactions) {
		std::string walletName = "Unknown wallet";
		int walletIndex = walletIndexById(transaction.getWalletId());
		if (walletIndex >= 0) {
			walletName = wallets[walletIndex].getName();
		}

		std::string categoryName = "Unknown category";
		int categoryIndex = categoryIndexById(transaction.getCategoryId());
		if (categoryIndex >= 0) {
			categoryName = categories[categoryIndex].getName();
		}

		std::cout << transaction.getId() << " | "
				  << transaction.getDate() << " | "
				  << (transaction.getIsIncome() ? "IN" : "OUT") << " | "
				  << transaction.getAmount() << " | "
				  << "wallet=" << walletName << " | "
				  << "category=" << categoryName << " | "
				  << transaction.getDescription() << "\n";
	}
}

void AppEngine::showStatistics() const {
	double income = Statistics::totalIncome(transactions);
	double expense = Statistics::totalExpense(transactions);
	double net = income - expense;
	int incomeCount = 0;
	int expenseCount = 0;

	std::vector<double> incomeByCategory(categories.size(), 0.0);
	std::vector<double> expenseByCategory(categories.size(), 0.0);

	for (const Transaction& transaction : transactions) {
		if (transaction.getIsIncome()) {
			++incomeCount;
		} else {
			++expenseCount;
		}

		int categoryIndex = categoryIndexById(transaction.getCategoryId());
		if (categoryIndex < 0) {
			continue;
		}

		if (transaction.getIsIncome()) {
			incomeByCategory[categoryIndex] += transaction.getAmount();
		} else {
			expenseByCategory[categoryIndex] += transaction.getAmount();
		}
	}

	std::cout << "\n===== Statistics =====\n";
	std::cout << std::fixed << std::setprecision(2);
	std::cout << "Total transactions: " << transactions.size() << "\n";
	std::cout << "Income transactions: " << incomeCount << "\n";
	std::cout << "Expense transactions: " << expenseCount << "\n";
	std::cout << "Total income: " << income << "\n";
	std::cout << "Total expense: " << expense << "\n";
	std::cout << "Net: " << net << "\n";

	std::cout << "\nBy wallet:\n";
	for (const Wallet& wallet : wallets) {
		double walletIncome = Statistics::totalIncomeByWallet(transactions, wallet.getId());
		double walletExpense = Statistics::totalExpenseByWallet(transactions, wallet.getId());
		std::cout << wallet.getName() << " | income=" << walletIncome
				  << " | expense=" << walletExpense
				  << " | balance=" << wallet.getBalance() << "\n";
	}

	std::cout << "\nBy category:\n";
	bool hasCategoryData = false;
	for (std::size_t i = 0; i < categories.size(); ++i) {
		double catIncome = incomeByCategory[i];
		double catExpense = expenseByCategory[i];
		if (catIncome <= 0.0 && catExpense <= 0.0) {
			continue;
		}

		hasCategoryData = true;
		std::cout << categories[i].getName()
				  << " | income=" << catIncome
				  << " | expense=" << catExpense
				  << " | net=" << (catIncome - catExpense) << "\n";
	}

	if (!hasCategoryData) {
		std::cout << "No category activity yet.\n";
	}

	std::cout << std::defaultfloat;
}

int AppEngine::chooseWalletId() const {
	if (wallets.empty()) {
		std::cout << "No wallets available.\n";
		return -1;
	}
	listWallets();
	int walletId = Utils::readInt("Enter wallet id: ");
	if (walletIndexById(walletId) < 0) {
		std::cout << "Wallet id is invalid.\n";
		return -1;
	}
	return walletId;
}

int AppEngine::chooseCategoryId(bool incomeType) const {
	listCategories(incomeType);
	int categoryId = Utils::readInt("Enter category id: ");
	int index = categoryIndexById(categoryId);
	if (index < 0 || categories[index].getIsIncome() != incomeType) {
		std::cout << "Category id is invalid.\n";
		return -1;
	}
	return categoryId;
}

int AppEngine::walletIndexById(int walletId) const {
	for (std::size_t i = 0; i < wallets.size(); ++i) {
		if (wallets[i].getId() == walletId) {
			return static_cast<int>(i);
		}
	}
	return -1;
}

int AppEngine::categoryIndexById(int categoryId) const {
	for (std::size_t i = 0; i < categories.size(); ++i) {
		if (categories[i].getId() == categoryId) {
			return static_cast<int>(i);
		}
	}
	return -1;
}
