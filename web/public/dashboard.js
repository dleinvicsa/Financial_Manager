FM.setupShell("dashboard");

async function init() {
  try {
    const [profile, stats] = await Promise.all([
      FM.api("/api/profile"),
      FM.api("/api/stats")
    ]);

    document.getElementById("welcomeLine").textContent = `${profile.ownerName || "User"} • ${profile.email || "no email"}`;

    const cards = document.getElementById("summaryCards");
    cards.innerHTML = `
      <article class="card">
        <p class="label">Transactions</p>
        <p class="value">${stats.totals.transactionCount}</p>
      </article>
      <article class="card">
        <p class="label">Income</p>
        <p class="value type-income">+${FM.money(stats.totals.totalIncome)}</p>
      </article>
      <article class="card">
        <p class="label">Expense</p>
        <p class="value type-expense">-${FM.money(stats.totals.totalExpense)}</p>
      </article>
      <article class="card">
        <p class="label">Net</p>
        <p class="value">${FM.money(stats.totals.net)}</p>
      </article>
    `;
  } catch (error) {
    document.getElementById("welcomeLine").textContent = `Error: ${error.message}`;
  }
}

init();
