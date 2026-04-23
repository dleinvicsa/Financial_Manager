const http = require("http");
const fs = require("fs");
const path = require("path");
const crypto = require("crypto");
const cbor = require("cbor");
const { URL } = require("url");

const PORT = process.env.PORT || 8080;
const ROOT_DIR = path.resolve(__dirname, "..");
const DATA_DIR = path.join(ROOT_DIR, "data");
const PUBLIC_DIR = path.join(__dirname, "public");

const FILES = {
  accounts: path.join(DATA_DIR, "accounts.bin"),
  categories: path.join(DATA_DIR, "categories.bin"),
  wallets: path.join(DATA_DIR, "wallets.bin"),
  transactions: path.join(DATA_DIR, "transactions.bin"),
  profile: path.join(DATA_DIR, "profile.bin")
};

const LEGACY_JSON_FILES = {
  accounts: path.join(DATA_DIR, "accounts.json"),
  categories: path.join(DATA_DIR, "categories.json"),
  wallets: path.join(DATA_DIR, "wallets.json"),
  transactions: path.join(DATA_DIR, "transactions.json"),
  profile: path.join(DATA_DIR, "profile.json")
};

function sendJson(res, statusCode, payload) {
  res.writeHead(statusCode, {
    "Content-Type": "application/json; charset=utf-8",
    "Cache-Control": "no-store"
  });
  res.end(JSON.stringify(payload, null, 2));
}

function sendText(res, statusCode, payload, contentType = "text/plain; charset=utf-8") {
  res.writeHead(statusCode, { "Content-Type": contentType });
  res.end(payload);
}

function writeBinary(filePath, payload) {
  fs.writeFileSync(filePath, cbor.encode(payload));
}

function readBinary(filePath, fallback, legacyJsonPath) {
  try {
    if (!fs.existsSync(filePath)) {
      if (legacyJsonPath && fs.existsSync(legacyJsonPath)) {
        const legacyRaw = fs.readFileSync(legacyJsonPath, "utf8");
        const legacyParsed = JSON.parse(legacyRaw);
        writeBinary(filePath, legacyParsed);
        return legacyParsed;
      }

      writeBinary(filePath, fallback);
      return JSON.parse(JSON.stringify(fallback));
    }

    const raw = fs.readFileSync(filePath);
    if (!raw || raw.length === 0) {
      writeBinary(filePath, fallback);
      return JSON.parse(JSON.stringify(fallback));
    }

    try {
      return cbor.decodeFirstSync(raw);
    } catch {
      // Recovery path for accidentally JSON-encoded binary files.
      const textPayload = JSON.parse(raw.toString("utf8"));
      writeBinary(filePath, textPayload);
      return textPayload;
    }
  } catch {
    writeBinary(filePath, fallback);
    return JSON.parse(JSON.stringify(fallback));
  }
}

function getDb() {
  return {
    accounts: readBinary(FILES.accounts, { accounts: [] }, LEGACY_JSON_FILES.accounts),
    categories: readBinary(FILES.categories, { categories: [] }, LEGACY_JSON_FILES.categories),
    wallets: readBinary(FILES.wallets, { wallets: [] }, LEGACY_JSON_FILES.wallets),
    transactions: readBinary(FILES.transactions, { transactions: [] }, LEGACY_JSON_FILES.transactions),
    profile: readBinary(FILES.profile, { ownerName: "", email: "", createdAt: "" }, LEGACY_JSON_FILES.profile)
  };
}

function saveDb(db) {
  writeBinary(FILES.accounts, db.accounts);
  writeBinary(FILES.categories, db.categories);
  writeBinary(FILES.wallets, db.wallets);
  writeBinary(FILES.transactions, db.transactions);
  writeBinary(FILES.profile, db.profile);
}

function nextId(items) {
  let max = 0;
  for (const item of items) {
    if (item.id > max) {
      max = item.id;
    }
  }
  return max + 1;
}

function today() {
  return new Date().toISOString().slice(0, 10);
}

function hashPassword(password) {
  return crypto.createHash("sha256").update(password).digest("hex");
}

function sanitizeAccount(account) {
  return {
    username: account.username,
    email: account.email,
    createdAt: account.createdAt
  };
}

function getStats(categories, wallets, transactions) {

  let totalIncome = 0;
  let totalExpense = 0;

  const byWallet = wallets.map((w) => ({
    walletId: w.id,
    walletName: w.name,
    income: 0,
    expense: 0,
    net: 0,
    balance: w.balance
  }));

  const byCategory = categories.map((c) => ({
    categoryId: c.id,
    categoryName: c.name,
    isIncomeCategory: c.isIncome,
    income: 0,
    expense: 0,
    net: 0
  }));

  for (const tx of transactions) {
    if (tx.isIncome) {
      totalIncome += tx.amount;
    } else {
      totalExpense += tx.amount;
    }

    const walletRow = byWallet.find((w) => w.walletId === tx.walletId);
    if (walletRow) {
      if (tx.isIncome) {
        walletRow.income += tx.amount;
      } else {
        walletRow.expense += tx.amount;
      }
      walletRow.net = walletRow.income - walletRow.expense;
    }

    const categoryRow = byCategory.find((c) => c.categoryId === tx.categoryId);
    if (categoryRow) {
      if (tx.isIncome) {
        categoryRow.income += tx.amount;
      } else {
        categoryRow.expense += tx.amount;
      }
      categoryRow.net = categoryRow.income - categoryRow.expense;
    }
  }

  return {
    totals: {
      transactionCount: transactions.length,
      totalIncome,
      totalExpense,
      net: totalIncome - totalExpense
    },
    byWallet,
    byCategory
  };
}

function stripUserField(item) {
  const { username, ...cleanItem } = item;
  return cleanItem;
}

function nextIdForUser(items, username) {
  let max = 0;
  for (const item of items) {
    if (item.username === username && item.id > max) {
      max = item.id;
    }
  }
  return max + 1;
}

function ensureUserScopedDb(db) {
  let changed = false;
  const accounts = db.accounts.accounts;

  if (!Array.isArray(db.profile.profiles)) {
    const legacyProfile = db.profile || {};
    db.profile = { profiles: [] };
    changed = true;

    if (accounts.length === 1 && (legacyProfile.ownerName || legacyProfile.email || legacyProfile.createdAt)) {
      db.profile.profiles.push({
        username: accounts[0].username,
        ownerName: String(legacyProfile.ownerName || ""),
        email: String(legacyProfile.email || ""),
        createdAt: String(legacyProfile.createdAt || "")
      });
    }
  }

  if (accounts.length === 1) {
    const onlyUser = accounts[0].username;

    for (const category of db.categories.categories) {
      if (!category.username) {
        category.username = onlyUser;
        changed = true;
      }
    }

    for (const wallet of db.wallets.wallets) {
      if (!wallet.username) {
        wallet.username = onlyUser;
        changed = true;
      }
    }

    for (const tx of db.transactions.transactions) {
      if (!tx.username) {
        tx.username = onlyUser;
        changed = true;
      }
    }
  }

  return changed;
}

function getRequestUsername(req) {
  return String(req.headers["x-username"] || "").trim();
}

function requireUser(req, res, db) {
  const username = getRequestUsername(req);
  if (!username) {
    sendJson(res, 401, { error: "Authentication required" });
    return null;
  }

  const exists = db.accounts.accounts.some((account) => account.username === username);
  if (!exists) {
    sendJson(res, 401, { error: "Unknown user" });
    return null;
  }

  return username;
}

function parseBody(req) {
  return new Promise((resolve, reject) => {
    let raw = "";

    req.on("data", (chunk) => {
      raw += chunk;
      if (raw.length > 1_000_000) {
        reject(new Error("Payload too large"));
      }
    });

    req.on("end", () => {
      if (!raw) {
        resolve({});
        return;
      }

      try {
        resolve(JSON.parse(raw));
      } catch {
        reject(new Error("Invalid JSON body"));
      }
    });

    req.on("error", () => reject(new Error("Failed to read request body")));
  });
}

function sanitizeStaticPath(inputPath) {
  if (inputPath === "/") {
    return path.join(PUBLIC_DIR, "login.html");
  }

  const safePath = path.normalize(inputPath).replace(/^\/+/, "");
  return path.join(PUBLIC_DIR, safePath);
}

function isInsidePublic(filePath) {
  const rel = path.relative(PUBLIC_DIR, filePath);
  return (rel === "" || (!rel.startsWith("..") && !path.isAbsolute(rel)));
}

async function handleApi(req, res, pathname) {
  const db = getDb();
  if (ensureUserScopedDb(db)) {
    saveDb(db);
  }

  if (req.method === "GET" && pathname === "/api/health") {
    sendJson(res, 200, { ok: true });
    return;
  }

  if (req.method === "POST" && pathname === "/api/auth/login") {
    const body = await parseBody(req);
    const username = String(body.username || "").trim();
    const password = String(body.password || "").trim();

    if (!username || !password) {
      sendJson(res, 400, { error: "Username and password are required" });
      return;
    }

    const account = db.accounts.accounts.find((a) => a.username === username);
    if (!account) {
      sendJson(res, 401, { error: "Invalid username or password" });
      return;
    }

    const validPassword =
      account.passwordHash === password ||
      account.passwordHash === hashPassword(password) ||
      account.passwordHash === String(body.passwordHash || "");

    if (!validPassword) {
      sendJson(res, 401, { error: "Invalid username or password" });
      return;
    }

    sendJson(res, 200, { user: sanitizeAccount(account) });
    return;
  }

  if (req.method === "POST" && pathname === "/api/auth/register") {
    const body = await parseBody(req);
    const username = String(body.username || "").trim();
    const password = String(body.password || "").trim();

    if (!username || !password) {
      sendJson(res, 400, { error: "Username and password are required" });
      return;
    }

    const exists = db.accounts.accounts.some((a) => a.username === username);
    if (exists) {
      sendJson(res, 409, { error: "Username already exists" });
      return;
    }

    const account = {
      username,
      email: "",
      passwordHash: hashPassword(password),
      createdAt: today()
    };

    db.accounts.accounts.push(account);
    saveDb(db);
    sendJson(res, 201, { user: sanitizeAccount(account) });
    return;
  }

  const username = requireUser(req, res, db);
  if (!username) {
    return;
  }

  if (req.method === "GET" && pathname === "/api/profile") {
    const profile = db.profile.profiles.find((p) => p.username === username) || {
      username,
      ownerName: "",
      email: "",
      createdAt: ""
    };

    sendJson(res, 200, stripUserField(profile));
    return;
  }

  if (req.method === "PUT" && pathname === "/api/profile") {
    const body = await parseBody(req);
    const index = db.profile.profiles.findIndex((p) => p.username === username);
    const prev = index >= 0 ? db.profile.profiles[index] : { createdAt: "" };
    const nextProfile = {
      username,
      ownerName: String(body.ownerName || "").trim(),
      email: String(body.email || "").trim(),
      createdAt: String(body.createdAt || prev.createdAt || "").trim()
    };

    if (index >= 0) {
      db.profile.profiles[index] = nextProfile;
    } else {
      db.profile.profiles.push(nextProfile);
    }

    saveDb(db);
    sendJson(res, 200, stripUserField(nextProfile));
    return;
  }

  if (req.method === "GET" && pathname === "/api/categories") {
    const categories = db.categories.categories
      .filter((c) => c.username === username)
      .map(stripUserField);
    sendJson(res, 200, categories);
    return;
  }

  if (req.method === "POST" && pathname === "/api/categories") {
    const body = await parseBody(req);
    const name = String(body.name || "").trim();
    const isIncome = Boolean(body.isIncome);

    if (!name) {
      sendJson(res, 400, { error: "Category name is required" });
      return;
    }

    const newCategory = {
      id: nextIdForUser(db.categories.categories, username),
      name,
      isIncome,
      username
    };

    db.categories.categories.push(newCategory);
    saveDb(db);
    sendJson(res, 201, stripUserField(newCategory));
    return;
  }

  const categoryDeleteMatch = pathname.match(/^\/api\/categories\/(\d+)$/);
  if (req.method === "DELETE" && categoryDeleteMatch) {
    const categoryId = Number(categoryDeleteMatch[1]);
    const index = db.categories.categories.findIndex((c) => c.id === categoryId && c.username === username);
    if (index < 0) {
      sendJson(res, 404, { error: "Category not found" });
      return;
    }

    const hasTransactions = db.transactions.transactions.some((t) => t.username === username && t.categoryId === categoryId);
    if (hasTransactions) {
      sendJson(res, 400, { error: "Cannot delete category with transactions" });
      return;
    }

    db.categories.categories.splice(index, 1);
    saveDb(db);
    sendJson(res, 200, { ok: true });
    return;
  }

  if (req.method === "GET" && pathname === "/api/wallets") {
    const wallets = db.wallets.wallets
      .filter((w) => w.username === username)
      .map(stripUserField);
    sendJson(res, 200, wallets);
    return;
  }

  if (req.method === "POST" && pathname === "/api/wallets") {
    const body = await parseBody(req);
    const name = String(body.name || "").trim();
    const balance = Number(body.balance || 0);

    if (!name || Number.isNaN(balance) || balance < 0) {
      sendJson(res, 400, { error: "Valid wallet name and non-negative balance are required" });
      return;
    }

    const newWallet = {
      id: nextIdForUser(db.wallets.wallets, username),
      name,
      balance,
      username
    };

    db.wallets.wallets.push(newWallet);
    saveDb(db);
    sendJson(res, 201, stripUserField(newWallet));
    return;
  }

  const walletTopupMatch = pathname.match(/^\/api\/wallets\/(\d+)\/topup$/);
  if (req.method === "POST" && walletTopupMatch) {
    const walletId = Number(walletTopupMatch[1]);
    const body = await parseBody(req);
    const amount = Number(body.amount || 0);

    if (Number.isNaN(amount) || amount <= 0) {
      sendJson(res, 400, { error: "Top up amount must be positive" });
      return;
    }

    const wallet = db.wallets.wallets.find((w) => w.id === walletId && w.username === username);
    if (!wallet) {
      sendJson(res, 404, { error: "Wallet not found" });
      return;
    }

    wallet.balance += amount;
    saveDb(db);
    sendJson(res, 200, stripUserField(wallet));
    return;
  }

  const walletDeleteMatch = pathname.match(/^\/api\/wallets\/(\d+)$/);
  if (req.method === "DELETE" && walletDeleteMatch) {
    const walletId = Number(walletDeleteMatch[1]);
    const index = db.wallets.wallets.findIndex((w) => w.id === walletId && w.username === username);
    if (index < 0) {
      sendJson(res, 404, { error: "Wallet not found" });
      return;
    }

    const hasTransactions = db.transactions.transactions.some((t) => t.username === username && t.walletId === walletId);
    if (hasTransactions) {
      sendJson(res, 400, { error: "Cannot delete wallet with transactions" });
      return;
    }

    db.wallets.wallets.splice(index, 1);
    saveDb(db);
    sendJson(res, 200, { ok: true });
    return;
  }

  if (req.method === "GET" && pathname === "/api/transactions") {
    const transactions = db.transactions.transactions
      .filter((t) => t.username === username)
      .map(stripUserField);
    sendJson(res, 200, transactions);
    return;
  }

  if (req.method === "POST" && pathname === "/api/transactions") {
    const body = await parseBody(req);
    const amount = Number(body.amount);
    const categoryId = Number(body.categoryId);
    const walletId = Number(body.walletId);
    const isIncome = Boolean(body.isIncome);
    const date = String(body.date || "").trim();
    const description = String(body.description || "").trim();

    if (Number.isNaN(amount) || amount <= 0 || Number.isNaN(categoryId) || Number.isNaN(walletId) || !date) {
      sendJson(res, 400, { error: "Invalid transaction payload" });
      return;
    }

    const category = db.categories.categories.find((c) => c.id === categoryId && c.username === username);
    const wallet = db.wallets.wallets.find((w) => w.id === walletId && w.username === username);

    if (!category || !wallet) {
      sendJson(res, 400, { error: "Category or wallet not found" });
      return;
    }

    if (category.isIncome !== isIncome) {
      sendJson(res, 400, { error: "Category type does not match transaction type" });
      return;
    }

    if (!isIncome && wallet.balance < amount) {
      sendJson(res, 400, { error: "Insufficient wallet balance" });
      return;
    }

    if (isIncome) {
      wallet.balance += amount;
    } else {
      wallet.balance -= amount;
    }

    const newTransaction = {
      id: nextIdForUser(db.transactions.transactions, username),
      amount,
      categoryId,
      walletId,
      isIncome,
      date,
      description,
      username
    };

    db.transactions.transactions.push(newTransaction);
    saveDb(db);
    sendJson(res, 201, stripUserField(newTransaction));
    return;
  }

  if (req.method === "GET" && pathname === "/api/stats") {
    const categories = db.categories.categories
      .filter((c) => c.username === username)
      .map(stripUserField);
    const wallets = db.wallets.wallets
      .filter((w) => w.username === username)
      .map(stripUserField);
    const transactions = db.transactions.transactions
      .filter((t) => t.username === username)
      .map(stripUserField);
    sendJson(res, 200, getStats(categories, wallets, transactions));
    return;
  }

  sendJson(res, 404, { error: "Not found" });
}

const server = http.createServer(async (req, res) => {
  try {
    const parsed = new URL(req.url, `http://${req.headers.host}`);
    const pathname = parsed.pathname;

    if (pathname.startsWith("/api/")) {
      await handleApi(req, res, pathname);
      return;
    }

    const staticPath = sanitizeStaticPath(pathname);
    if (!isInsidePublic(staticPath) || !fs.existsSync(staticPath)) {
      sendText(res, 404, "Not found");
      return;
    }

    const ext = path.extname(staticPath);
    const contentType = {
      ".html": "text/html; charset=utf-8",
      ".css": "text/css; charset=utf-8",
      ".js": "application/javascript; charset=utf-8",
      ".json": "application/json; charset=utf-8"
    }[ext] || "application/octet-stream";

    sendText(res, 200, fs.readFileSync(staticPath), contentType);
  } catch (error) {
    sendJson(res, 500, { error: error.message || "Internal server error" });
  }
});

server.listen(PORT, () => {
  console.log(`Web app is running at http://localhost:${PORT}`);
});
