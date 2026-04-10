FM.setupShell("transactions");

const cache = {
  wallets: [],
  categories: [],
  transactions: []
};

function walletName(id) {
  const wallet = cache.wallets.find((w) => w.id === id);
  return wallet ? wallet.name : "Unknown";
}

function categoryName(id) {
  const category = cache.categories.find((c) => c.id === id);
  return category ? category.name : "Unknown";
}

function renderTransactionTable() {
  const body = document.getElementById("txBody");
  if (!cache.transactions.length) {
    body.innerHTML = "<tr><td colspan='7'>No transactions</td></tr>";
    return;
  }

  body.innerHTML = [...cache.transactions].sort((a, b) => b.id - a.id).map((tx) => `
    <tr>
      <td>${tx.id}</td>
      <td>${tx.date}</td>
      <td class="${tx.isIncome ? "type-income" : "type-expense"}">${tx.isIncome ? "Income" : "Expense"}</td>
      <td>${walletName(tx.walletId)}</td>
      <td>${categoryName(tx.categoryId)}</td>
      <td>${FM.money(tx.amount)}</td>
      <td>${tx.description || "-"}</td>
    </tr>
  `).join("");
}

function renderFormSelectors() {
  document.getElementById("txWallet").innerHTML = cache.wallets
    .map((w) => `<option value="${w.id}">${w.name} (${FM.money(w.balance)})</option>`)
    .join("");

  const isIncome = document.getElementById("txType").value === "income";
  const allowed = cache.categories.filter((c) => c.isIncome === isIncome);
  document.getElementById("txCategory").innerHTML = allowed
    .map((c) => `<option value="${c.id}">${c.name}</option>`)
    .join("");
}

async function refresh() {
  const [wallets, categories, transactions] = await Promise.all([
    FM.api("/api/wallets"),
    FM.api("/api/categories"),
    FM.api("/api/transactions")
  ]);

  cache.wallets = wallets;
  cache.categories = categories;
  cache.transactions = transactions;

  renderFormSelectors();
  renderTransactionTable();
}

document.getElementById("txType").addEventListener("change", renderFormSelectors);

document.getElementById("txForm").addEventListener("submit", async (event) => {
  event.preventDefault();
  try {
    await FM.api("/api/transactions", {
      method: "POST",
      body: JSON.stringify({
        isIncome: document.getElementById("txType").value === "income",
        amount: Number(document.getElementById("txAmount").value),
        walletId: Number(document.getElementById("txWallet").value),
        categoryId: Number(document.getElementById("txCategory").value),
        date: document.getElementById("txDate").value,
        description: document.getElementById("txDescription").value.trim()
      })
    });

    FM.setMessage("txMessage", "Transaction saved");
    event.target.reset();
    document.getElementById("txDate").value = new Date().toISOString().slice(0, 10);
    await refresh();
  } catch (error) {
    FM.setMessage("txMessage", error.message, true);
  }
});

document.getElementById("txDate").value = new Date().toISOString().slice(0, 10);
refresh().catch((error) => FM.setMessage("txMessage", error.message, true));
