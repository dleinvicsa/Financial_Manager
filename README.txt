CashFlow (Financial Manager)

CashFlow is a personal finance manager with:
- a C++ console application
- a lightweight Node.js web interface
- binary .bin storage in the data directory

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

Run Web Version
1. Open terminal in the project root (FinanceManager).
2. Install web dependencies (first run only):
	npm install --prefix web
3. Start server:
	make run-web
	or
	node web/server.js
4. Open in browser:
	http://localhost:8080

Stop Web Server
- Press Ctrl + C in the terminal where the server is running.

Run Web Version on Windows
PowerShell:
1. Open PowerShell in the project root (FinanceManager).
2. If Node.js is not installed, install Node.js 18+ from https://nodejs.org and reopen PowerShell.
3. Check that Node.js is available:
	node -v
4. Start server:
	node web/server.js
5. Open in browser:
	http://localhost:8080

Stop Web Server on Windows
- Press Ctrl + C in PowerShell/CMD/Git Bash where the server is running.

Available Make Targets
- make: Build console application
- make clean: Remove build artifacts
- make run-web: Start Node.js web server

Data Storage
- accounts.bin: Registered users
- profile.bin: Per-user profile records
- wallets.bin: Per-user wallets
- categories.bin: Per-user categories
- transactions.bin: Per-user transactions

Legacy migration
- If old .json files exist, they are migrated to .bin automatically on first run.

Notes
- Frontend authentication state is stored in browser localStorage.
- API requests include the current username and are scoped on the server.
- If port 8080 is busy, stop the existing process and restart the server.
