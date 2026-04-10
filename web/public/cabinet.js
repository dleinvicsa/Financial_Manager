FM.setupShell("cabinet");

async function init() {
  try {
    const [profile, wallets, categories, stats] = await Promise.all([
      FM.api("/api/profile"),
      FM.api("/api/wallets"),
      FM.api("/api/categories"),
      FM.api("/api/stats")
    ]);

    document.getElementById("ownerName").value = profile.ownerName || "";
    document.getElementById("ownerEmail").value = profile.email || "";
    document.getElementById("createdAt").value = profile.createdAt || new Date().toISOString().slice(0, 10);

    document.getElementById("cabinetCards").innerHTML = `
      <article class="card">
        <p class="label">Wallets</p>
        <p class="value">${wallets.length}</p>
      </article>
      <article class="card">
        <p class="label">Categories</p>
        <p class="value">${categories.length}</p>
      </article>
      <article class="card">
        <p class="label">Transactions</p>
        <p class="value">${stats.totals.transactionCount}</p>
      </article>
      <article class="card">
        <p class="label">Total Balance</p>
        <p class="value">${FM.money(wallets.reduce((sum, w) => sum + w.balance, 0))}</p>
      </article>
    `;
  } catch (error) {
    FM.setMessage("profileMessage", error.message, true);
  }
}

document.getElementById("profileForm").addEventListener("submit", async (event) => {
  event.preventDefault();

  try {
    await FM.api("/api/profile", {
      method: "PUT",
      body: JSON.stringify({
        ownerName: document.getElementById("ownerName").value.trim(),
        email: document.getElementById("ownerEmail").value.trim(),
        createdAt: document.getElementById("createdAt").value
      })
    });

    FM.setMessage("profileMessage", "Profile saved");
    await init();
  } catch (error) {
    FM.setMessage("profileMessage", error.message, true);
  }
});

init();
