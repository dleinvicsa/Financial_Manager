const state = {
  profile: null,
  wallets: [],
  categories: [],
  transactions: [],
  stats: null
};

function money(value) {
  return Number(value || 0).toFixed(2);
}

async function api(path, options = {}) {
  const response = await fetch(path, {
    headers: { "Content-Type": "application/json" },
    ...options
  });

  const data = await response.json();
  if (!response.ok) {
    throw new Error(data.error || "Request failed");
  }
  return data;
}

function walletNameById(id) {
  const wallet = state.wallets.find((w) => w.id === id);
  return wallet ? wallet.name : "Unknown wallet";
}

function categoryNameById(id) {
  const category = state.categories.find((c) => c.id === id);
  return category ? category.name : "Unknown category";
}

function renderCards() {
  const cards = document.getElementById("cards");
  const totals = state.stats?.totals || {
    transactionCount: 0,
    totalIncome: 0,
    totalExpense: 0,
    net: 0
  };

  cards.innerHTML = `
    <article class="card">
      <p class="label">Transactions</p>
      <p class="value">${totals.transactionCount}</p>
    </article>
    <article class="card">
      <p class="label">Income</p>
      <p class="value type-income">+${money(totals.totalIncome)}</p>
    </article>
    <article class="card">
      <p class="label">Expense</p>
      <p class="value type-expense">-${money(totals.totalExpense)}</p>
    </article>
    <article class="card">
      <p class="label">Net</p>
      <p class="value">${money(totals.net)}</p>
    </article>
  `;
}

function renderProfile() {
  const line = document.getElementById("profileLine");
  if (!state.profile) {
    line.textContent = "Profile is not set";
    return;
  }

  line.textContent = `${state.profile.ownerName || "Unknown user"} • ${state.profile.email || "No email"}`;
}

function renderSelectors() {
  const walletSelect = document.getElementById("txWallet");
  const categorySelect = document.getElementById("txCategory");
  const txType = document.getElementById("txType");

  walletSelect.innerHTML = state.wallets
    .map((w) => `<option value="${w.id}">${w.name} (${money(w.balance)})</option>`)
    .join("");

  const desiredIncome = txType.value === "income";
  const allowedCategories = state.categories.filter((c) => c.isIncome === desiredIncome);
  categorySelect.innerHTML = allowedCategories
    .map((c) => `<option value="${c.id}">${c.name}</option>`)
    .join("");
}

function renderTransactions() {
  const body = document.getElementById("txBody");
  if (!state.transactions.length) {
    body.innerHTML = `<tr><td colspan="6">No transactions yet</td></tr>`;
    return;
  }

  const rows = [...state.transactions].sort((a, b) => b.id - a.id).map((tx) => {
    const typeClass = tx.isIncome ? "type-income" : "type-expense";
    const typeLabel = tx.isIncome ? "Income" : "Expense";
    return `
      <tr>
        <td>${tx.date}</td>
        <td class="${typeClass}">${typeLabel}</td>
        <td>${walletNameById(tx.walletId)}</td>
        <td>${categoryNameById(tx.categoryId)}</td>
        <td>${money(tx.amount)}</td>
        <td>${tx.description || "-"}</td>
      </tr>
    `;
  });

  body.innerHTML = rows.join("");
}

function renderWalletStats() {
  const list = document.getElementById("walletStats");
  const rows = state.stats?.byWallet || [];

  if (!rows.length) {
    list.innerHTML = "<li>No wallet data</li>";
    return;
  }

  list.innerHTML = rows.map((row) => `
    <li>
      <strong>${row.walletName}</strong><br />
      Income: +${money(row.income)} | Expense: -${money(row.expense)} | Net: ${money(row.net)} | Balance: ${money(row.balance)}
    </li>
  `).join("");
}

function renderCategoryStats() {
  const list = document.getElementById("categoryStats");
  const rows = state.stats?.byCategory || [];
  const active = rows.filter((r) => r.income > 0 || r.expense > 0);

  if (!active.length) {
    list.innerHTML = "<li>No category activity yet</li>";
    return;
  }

  list.innerHTML = active.map((row) => `
    <li>
      <strong>${row.categoryName}</strong><br />
      Income: +${money(row.income)} | Expense: -${money(row.expense)} | Net: ${money(row.net)}
    </li>
  `).join("");
}

function setMessage(id, text, isError = false) {
  const el = document.getElementById(id);
  el.textContent = text;
  el.style.color = isError ? "#b91c1c" : "#0f766e";
}

async function refresh() {
  const [profile, wallets, categories, transactions, stats] = await Promise.all([
    api("/api/profile"),
    api("/api/wallets"),
    api("/api/categories"),
    api("/api/transactions"),
    api("/api/stats")
  ]);

  state.profile = profile;
  state.wallets = wallets;
  state.categories = categories;
  state.transactions = transactions;
  state.stats = stats;

  renderProfile();
  renderCards();
  renderSelectors();
  renderTransactions();
  renderWalletStats();
  renderCategoryStats();
}

function bindEvents() {
  document.getElementById("txType").addEventListener("change", renderSelectors);

  document.getElementById("txForm").addEventListener("submit", async (event) => {
    event.preventDefault();
    try {
      const txType = document.getElementById("txType").value;
      await api("/api/transactions", {
        method: "POST",
        body: JSON.stringify({
          isIncome: txType === "income",
          amount: Number(document.getElementById("txAmount").value),
          walletId: Number(document.getElementById("txWallet").value),
          categoryId: Number(document.getElementById("txCategory").value),
          date: document.getElementById("txDate").value,
          description: document.getElementById("txDescription").value.trim()
        })
      });

      setMessage("txMessage", "Transaction saved");
      event.target.reset();
      document.getElementById("txDate").value = new Date().toISOString().slice(0, 10);
      await refresh();
    } catch (error) {
      setMessage("txMessage", error.message, true);
    }
  });

  document.getElementById("walletForm").addEventListener("submit", async (event) => {
    event.preventDefault();
    try {
      await api("/api/wallets", {
        method: "POST",
        body: JSON.stringify({
          name: document.getElementById("walletName").value.trim(),
          balance: Number(document.getElementById("walletBalance").value)
        })
      });

      setMessage("walletMessage", "Wallet created");
      event.target.reset();
      await refresh();
    } catch (error) {
      setMessage("walletMessage", error.message, true);
    }
  });

  document.getElementById("categoryForm").addEventListener("submit", async (event) => {
    event.preventDefault();
    try {
      await api("/api/categories", {
        method: "POST",
        body: JSON.stringify({
          name: document.getElementById("categoryName").value.trim(),
          isIncome: document.getElementById("categoryType").value === "income"
        })
      });

      setMessage("categoryMessage", "Category created");
      event.target.reset();
      await refresh();
    } catch (error) {
      setMessage("categoryMessage", error.message, true);
    }
  });
}

async function init() {
  bindEvents();
  document.getElementById("txDate").value = new Date().toISOString().slice(0, 10);

  try {
    await refresh();
  } catch (error) {
    setMessage("txMessage", `Failed to load data: ${error.message}`, true);
  }
}

init();
