#include "../include/DataManager.h"

#include <cctype>
#include <filesystem>
#include <fstream>
#include <functional>
#include <iterator>
#include <sstream>
#include <string>
#include <vector>

#include "../include/json.hpp"

namespace fs = std::filesystem;
using json = nlohmann::json;

namespace {
	bool fileExists(const std::string& path) {
		std::ifstream in(path);
		return in.good();
	}

	void ensureDirectoryExists(const std::string& path) {
		std::error_code error;
		fs::create_directories(path, error);
	}

	std::string trim(const std::string& value) {
		std::size_t start = 0;
		while (start < value.size() && std::isspace(static_cast<unsigned char>(value[start]))) {
			++start;
		}
		if (start >= value.size()) {
			return "";
		}

		std::size_t end = value.size() - 1;
		while (end > start && std::isspace(static_cast<unsigned char>(value[end]))) {
			--end;
		}
		return value.substr(start, end - start + 1);
	}

	std::vector<std::string> splitByTab(const std::string& line) {
		std::vector<std::string> fields;
		std::string field;
		std::istringstream stream(line);
		while (std::getline(stream, field, '\t')) {
			fields.push_back(field);
		}
		return fields;
	}

	bool parseBool(const std::string& value, bool fallback = false) {
		std::string normalized;
		normalized.reserve(value.size());
		for (char ch : value) {
			normalized.push_back(static_cast<char>(std::tolower(static_cast<unsigned char>(ch))));
		}

		if (normalized == "1" || normalized == "true" || normalized == "yes") {
			return true;
		}
		if (normalized == "0" || normalized == "false" || normalized == "no") {
			return false;
		}
		return fallback;
	}

	json defaultAccountsJson() {
		return json{{"accounts", json::array()}};
	}

	json defaultCategoriesJson() {
		return json{{"categories", json::array()}};
	}

	json defaultWalletsJson() {
		return json{{"wallets", json::array()}};
	}

	json defaultTransactionsJson() {
		return json{{"transactions", json::array()}};
	}

	json defaultProfileJson() {
		return json{
			{"ownerName", ""},
			{"email", ""},
			{"createdAt", ""}
		};
	}

	json loadJsonOrDefault(const std::string& path, const json& fallback) {
		if (!fileExists(path)) {
			std::ofstream out(path, std::ios::binary | std::ios::trunc);
			std::vector<std::uint8_t> payload = json::to_cbor(fallback);
			out.write(reinterpret_cast<const char*>(payload.data()), static_cast<std::streamsize>(payload.size()));
			return fallback;
		}

		std::ifstream in(path, std::ios::binary);
		if (!in.good()) {
			return fallback;
		}

		std::vector<std::uint8_t> bytes((std::istreambuf_iterator<char>(in)), std::istreambuf_iterator<char>());
		if (bytes.empty()) {
			std::ofstream out(path, std::ios::binary | std::ios::trunc);
			std::vector<std::uint8_t> payload = json::to_cbor(fallback);
			out.write(reinterpret_cast<const char*>(payload.data()), static_cast<std::streamsize>(payload.size()));
			return fallback;
		}

		try {
			return json::from_cbor(bytes);
		} catch (...) {
			try {
				std::string raw(bytes.begin(), bytes.end());
				json parsed = json::parse(raw);
				std::ofstream out(path, std::ios::binary | std::ios::trunc);
				std::vector<std::uint8_t> payload = json::to_cbor(parsed);
				out.write(reinterpret_cast<const char*>(payload.data()), static_cast<std::streamsize>(payload.size()));
				return parsed;
			} catch (...) {
				std::ofstream out(path, std::ios::binary | std::ios::trunc);
				std::vector<std::uint8_t> payload = json::to_cbor(fallback);
				out.write(reinterpret_cast<const char*>(payload.data()), static_cast<std::streamsize>(payload.size()));
				return fallback;
			}
		}
	}

	void saveJson(const std::string& path, const json& value) {
		std::ofstream out(path, std::ios::binary | std::ios::trunc);
		std::vector<std::uint8_t> payload = json::to_cbor(value);
		out.write(reinterpret_cast<const char*>(payload.data()), static_cast<std::streamsize>(payload.size()));
	}

	json jsonFromAccountsDat(const std::string& content) {
		json accounts = json::array();
		std::istringstream input(content);
		std::string line;
		while (std::getline(input, line)) {
			line = trim(line);
			if (line.empty()) {
				continue;
			}

			std::vector<std::string> fields = splitByTab(line);
			if (fields.size() < 4) {
				continue;
			}

			std::string username = trim(fields[0]);
			std::string passwordHash = trim(fields[1]);
			if (username.empty() || passwordHash.empty()) {
				continue;
			}

			accounts.push_back({
				{"username", username},
				{"passwordHash", passwordHash},
				{"email", trim(fields[2])},
				{"createdAt", trim(fields[3])}
			});
		}
		return json{{"accounts", accounts}};
	}

	json jsonFromCategoriesDat(const std::string& content) {
		json categories = json::array();
		std::istringstream input(content);
		std::string line;
		while (std::getline(input, line)) {
			line = trim(line);
			if (line.empty()) {
				continue;
			}

			std::vector<std::string> fields = splitByTab(line);
			if (fields.size() < 3) {
				continue;
			}

			int id = 0;
			try {
				id = std::stoi(trim(fields[0]));
			} catch (...) {
				continue;
			}

			std::string name = trim(fields[1]);
			if (id <= 0 || name.empty()) {
				continue;
			}

			categories.push_back({
				{"id", id},
				{"name", name},
				{"isIncome", parseBool(trim(fields[2]), false)}
			});
		}
		return json{{"categories", categories}};
	}

	json jsonFromWalletsDat(const std::string& content) {
		json wallets = json::array();
		std::istringstream input(content);
		std::string line;
		while (std::getline(input, line)) {
			line = trim(line);
			if (line.empty()) {
				continue;
			}

			std::vector<std::string> fields = splitByTab(line);
			if (fields.size() < 3) {
				continue;
			}

			int id = 0;
			double balance = 0.0;
			try {
				id = std::stoi(trim(fields[0]));
				balance = std::stod(trim(fields[2]));
			} catch (...) {
				continue;
			}

			std::string name = trim(fields[1]);
			if (id <= 0 || name.empty()) {
				continue;
			}

			wallets.push_back({
				{"id", id},
				{"name", name},
				{"balance", balance}
			});
		}
		return json{{"wallets", wallets}};
	}

	json jsonFromTransactionsDat(const std::string& content) {
		json transactions = json::array();
		std::istringstream input(content);
		std::string line;
		while (std::getline(input, line)) {
			line = trim(line);
			if (line.empty()) {
				continue;
			}

			std::vector<std::string> fields = splitByTab(line);
			if (fields.size() < 7) {
				continue;
			}

			int id = 0;
			double amount = 0.0;
			int categoryId = 0;
			int walletId = 0;
			try {
				id = std::stoi(trim(fields[0]));
				amount = std::stod(trim(fields[1]));
				categoryId = std::stoi(trim(fields[2]));
				walletId = std::stoi(trim(fields[3]));
			} catch (...) {
				continue;
			}

			std::string date = trim(fields[5]);
			if (id <= 0 || categoryId <= 0 || walletId <= 0 || date.empty()) {
				continue;
			}

			transactions.push_back({
				{"id", id},
				{"amount", amount},
				{"categoryId", categoryId},
				{"walletId", walletId},
				{"isIncome", parseBool(trim(fields[4]), false)},
				{"date", date},
				{"description", trim(fields[6])}
			});
		}
		return json{{"transactions", transactions}};
	}

	json jsonFromProfileDat(const std::string& content) {
		std::istringstream input(content);
		std::string line;
		if (!std::getline(input, line)) {
			return defaultProfileJson();
		}

		line = trim(line);
		if (line.empty()) {
			return defaultProfileJson();
		}

		std::vector<std::string> fields = splitByTab(line);
		std::string ownerName = fields.size() > 0 ? trim(fields[0]) : "";
		std::string email = fields.size() > 1 ? trim(fields[1]) : "";
		std::string createdAt = fields.size() > 2 ? trim(fields[2]) : "";
		return json{
			{"ownerName", ownerName},
			{"email", email},
			{"createdAt", createdAt}
		};
	}

	void createJsonWithFallback(
		const std::string& jsonPath,
		const std::string& legacyJsonPath,
		const std::string& legacyDatPath,
		const json& defaultValue,
		const std::function<json(const std::string&)>& migrateFromDat) {
		if (fileExists(jsonPath)) {
			return;
		}

		if (fileExists(legacyJsonPath)) {
			std::ifstream in(legacyJsonPath);
			json parsed = defaultValue;
			try {
				in >> parsed;
			} catch (...) {
				parsed = defaultValue;
			}
			saveJson(jsonPath, parsed);
			return;
		}

		if (fileExists(legacyDatPath)) {
			std::ifstream in(legacyDatPath);
			std::ostringstream content;
			content << in.rdbuf();
			saveJson(jsonPath, migrateFromDat(content.str()));
			return;
		}

		saveJson(jsonPath, defaultValue);
	}
}

DataManager::DataManager(const std::string& dataPath)
	: dataRoot(dataPath),
	  accountsFile(dataPath + "/accounts.bin"),
	  activeUser(""),
	  categoriesFile(dataPath + "/categories.bin"),
	  walletsFile(dataPath + "/wallets.bin"),
	  transactionsFile(dataPath + "/transactions.bin"),
	  profileFile(dataPath + "/profile.bin") {
	ensureDirectoryExists(dataRoot);
	createJsonWithFallback(accountsFile, dataRoot + "/accounts.json", dataRoot + "/accounts.dat", defaultAccountsJson(), jsonFromAccountsDat);
	refreshUserFiles();
}

std::vector<Account> DataManager::loadAccounts() const {
	std::vector<Account> result;
	json root = loadJsonOrDefault(accountsFile, defaultAccountsJson());
	if (!root.contains("accounts") || !root["accounts"].is_array()) {
		return result;
	}

	for (const auto& item : root["accounts"]) {
		if (!item.is_object()) {
			continue;
		}
		Account account;
		account.username = item.value("username", "");
		account.passwordHash = item.value("passwordHash", "");
		account.email = item.value("email", "");
		account.createdAt = item.value("createdAt", "");
		if (!account.username.empty() && !account.passwordHash.empty()) {
			result.push_back(account);
		}
	}
	return result;
}

bool DataManager::accountExists(const std::string& username) const {
	if (username.empty()) {
		return false;
	}
	std::vector<Account> accounts = loadAccounts();
	for (const Account& account : accounts) {
		if (account.username == username) {
			return true;
		}
	}
	return false;
}

bool DataManager::createAccount(const Account& account) const {
	if (account.username.empty() || account.passwordHash.empty()) {
		return false;
	}
	if (accountExists(account.username)) {
		return false;
	}

	json root = loadJsonOrDefault(accountsFile, defaultAccountsJson());
	if (!root.contains("accounts") || !root["accounts"].is_array()) {
		root["accounts"] = json::array();
	}

	root["accounts"].push_back({
		{"username", account.username},
		{"passwordHash", account.passwordHash},
		{"email", account.email},
		{"createdAt", account.createdAt}
	});
	saveJson(accountsFile, root);
	return true;
}

bool DataManager::validateLogin(const std::string& username, const std::string& passwordHash) const {
	if (username.empty() || passwordHash.empty()) {
		return false;
	}

	std::vector<Account> accounts = loadAccounts();
	for (const Account& account : accounts) {
		if (account.username == username && account.passwordHash == passwordHash) {
			return true;
		}
	}
	return false;
}

void DataManager::setActiveUser(const std::string& username) {
	activeUser = username;
	refreshUserFiles();
	ensureDirectoryExists(dataRoot);
	createJsonWithFallback(categoriesFile, dataRoot + "/categories.json", dataRoot + "/categories.dat", defaultCategoriesJson(), jsonFromCategoriesDat);
	createJsonWithFallback(walletsFile, dataRoot + "/wallets.json", dataRoot + "/wallets.dat", defaultWalletsJson(), jsonFromWalletsDat);
	createJsonWithFallback(transactionsFile, dataRoot + "/transactions.json", dataRoot + "/transactions.dat", defaultTransactionsJson(), jsonFromTransactionsDat);
	createJsonWithFallback(profileFile, dataRoot + "/profile.json", dataRoot + "/profile.dat", defaultProfileJson(), jsonFromProfileDat);
}

std::string DataManager::getActiveUser() const {
	return activeUser;
}

void DataManager::ensureFileExists(const std::string& path) const {
	if (!fileExists(path)) {
		std::ofstream out(path);
		(void)out;
	}
}

std::string DataManager::sanitizeUsername(const std::string& username) const {
	if (username.empty()) {
		return "default";
	}

	static const char* digits = "0123456789abcdef";
	std::string key;
	key.reserve(username.size() * 2);
	for (unsigned char ch : username) {
		key.push_back(digits[(ch >> 4) & 0x0F]);
		key.push_back(digits[ch & 0x0F]);
	}
	return key;
}

void DataManager::refreshUserFiles() {
	// Always use the shared binary files in data/.
	categoriesFile = dataRoot + "/categories.bin";
	walletsFile = dataRoot + "/wallets.bin";
	transactionsFile = dataRoot + "/transactions.bin";
	profileFile = dataRoot + "/profile.bin";
}

std::vector<Category> DataManager::loadCategories() const {
	std::vector<Category> result;
	json root = loadJsonOrDefault(categoriesFile, defaultCategoriesJson());
	if (!root.contains("categories") || !root["categories"].is_array()) {
		return result;
	}

	for (const auto& item : root["categories"]) {
		if (!item.is_object()) {
			continue;
		}
		int id = item.value("id", 0);
		std::string name = item.value("name", "");
		bool isIncome = item.value("isIncome", false);
		if (id > 0 && !name.empty()) {
			result.emplace_back(id, name, isIncome);
		}
	}
	return result;
}

std::vector<Wallet> DataManager::loadWallets() const {
	std::vector<Wallet> result;
	json root = loadJsonOrDefault(walletsFile, defaultWalletsJson());
	if (!root.contains("wallets") || !root["wallets"].is_array()) {
		return result;
	}

	for (const auto& item : root["wallets"]) {
		if (!item.is_object()) {
			continue;
		}
		int id = item.value("id", 0);
		std::string name = item.value("name", "");
		double balance = item.value("balance", 0.0);
		if (id > 0 && !name.empty()) {
			result.emplace_back(id, name, balance);
		}
	}
	return result;
}

std::vector<Transaction> DataManager::loadTransactions() const {
	std::vector<Transaction> result;
	json root = loadJsonOrDefault(transactionsFile, defaultTransactionsJson());
	if (!root.contains("transactions") || !root["transactions"].is_array()) {
		return result;
	}

	for (const auto& item : root["transactions"]) {
		if (!item.is_object()) {
			continue;
		}
		int id = item.value("id", 0);
		double amount = item.value("amount", 0.0);
		int categoryId = item.value("categoryId", 0);
		int walletId = item.value("walletId", 0);
		bool isIncome = item.value("isIncome", false);
		std::string date = item.value("date", "");
		std::string description = item.value("description", "");
		if (id > 0 && categoryId > 0 && walletId > 0 && !date.empty()) {
			result.emplace_back(id, amount, categoryId, walletId, isIncome, date, description);
		}
	}
	return result;
}

Profile DataManager::loadProfile() const {
	Profile profile;
	json root = loadJsonOrDefault(profileFile, defaultProfileJson());
	profile.ownerName = root.value("ownerName", "");
	profile.email = root.value("email", "");
	profile.createdAt = root.value("createdAt", "");
	return profile;
}

void DataManager::saveCategories(const std::vector<Category>& categories) const {
	json root = defaultCategoriesJson();
	for (const Category& category : categories) {
		root["categories"].push_back({
			{"id", category.getId()},
			{"name", category.getName()},
			{"isIncome", category.getIsIncome()}
		});
	}
	saveJson(categoriesFile, root);
}

void DataManager::saveWallets(const std::vector<Wallet>& wallets) const {
	json root = defaultWalletsJson();
	for (const Wallet& wallet : wallets) {
		root["wallets"].push_back({
			{"id", wallet.getId()},
			{"name", wallet.getName()},
			{"balance", wallet.getBalance()}
		});
	}
	saveJson(walletsFile, root);
}

void DataManager::saveTransactions(const std::vector<Transaction>& transactions) const {
	json root = defaultTransactionsJson();
	for (const Transaction& transaction : transactions) {
		root["transactions"].push_back({
			{"id", transaction.getId()},
			{"amount", transaction.getAmount()},
			{"categoryId", transaction.getCategoryId()},
			{"walletId", transaction.getWalletId()},
			{"isIncome", transaction.getIsIncome()},
			{"date", transaction.getDate()},
			{"description", transaction.getDescription()}
		});
	}
	saveJson(transactionsFile, root);
}

void DataManager::saveProfile(const Profile& profile) const {
	json root = {
		{"ownerName", profile.ownerName},
		{"email", profile.email},
		{"createdAt", profile.createdAt}
	};
	saveJson(profileFile, root);
}
