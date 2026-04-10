CashFlow (Financial Manager)

CashFlow is a personal finance manager with:
- a C++ console application
- a lightweight Node.js web interface
- JSON-based storage in the data directory

The web API now stores and returns data per user account, so each user sees only their own profile, wallets, categories, transactions, and statistics.

Requirements
- macOS, Linux, or Windows
- C++ compiler with C++17 support (tested with g++)
- Node.js 18+ (or any modern Node.js version)

Project Structure
- main.cpp: Console app entry point
- src/: C++ business logic
- include/: C++ headers (+ json.hpp)
- data/: JSON database files
- web/server.js: Web API + static file server
- web/public/: Frontend pages, scripts, and styles

Build and Run (Console App)
1. Open terminal in the project root (FinanceManager).
2. Build:
	make
3. Run:
	./finance_manager

Build and Run on Windows (Console App)
Option A: MSYS2 / MinGW (with make + g++)
1. Open MSYS2 MinGW terminal in the project root.
2. Build:
	make
3. Run:
	./finance_manager.exe

Option B: PowerShell with g++ only (without make)
1. Open PowerShell in the project root.
2. Build:
	g++ -Wall -Wextra -std=c++17 -g -o finance_manager.exe main.cpp src/AppEngine.cpp src/Category.cpp src/DataManager.cpp src/Statistics.cpp src/Transaction.cpp src/Utils.cpp src/Wallet.cpp
3. Run:
	.\finance_manager.exe

Run Web Version
1. Open terminal in the project root (FinanceManager).
2. Start server:
	make run-web
	or
	node web/server.js
3. Open in browser:
	http://localhost:8080

Run Web Version on Windows
PowerShell:
1. Open PowerShell in the project root (FinanceManager).
2. Start server:
	node web/server.js
3. Open in browser:
	http://localhost:8080

If you use Git Bash or MSYS2 and have make installed, you can also run:
	make run-web

Stop Web Server
- Press Ctrl + C in the terminal where the server is running.

Stop Web Server on Windows
- Press Ctrl + C in PowerShell/CMD/Git Bash where the server is running.

Available Make Targets
- make: Build console application
- make clean: Remove build artifacts
- make run-web: Start Node.js web server

Data Storage
- accounts.json: Registered users
- profile.json: Per-user profile records
- wallets.json: Per-user wallets
- categories.json: Per-user categories
- transactions.json: Per-user transactions

Notes
- Frontend authentication state is stored in browser localStorage.
- API requests include the current username and are scoped on the server.
- If port 8080 is busy, stop the existing process and restart the server.
