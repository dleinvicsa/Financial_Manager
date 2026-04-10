FM.setupShell("wallets");

async function refreshWallets() {
  const wallets = await FM.api("/api/wallets");

  const topupSelect = document.getElementById("topupWallet");
  topupSelect.innerHTML = wallets.map((w) => `<option value="${w.id}">${w.name} (${FM.money(w.balance)})</option>`).join("");

  const body = document.getElementById("walletBody");
  if (!wallets.length) {
    body.innerHTML = "<tr><td colspan='4'>No wallets</td></tr>";
    return;
  }

  body.innerHTML = wallets.map((w) => `
    <tr>
      <td>${w.id}</td>
      <td>${w.name}</td>
      <td>${FM.money(w.balance)}</td>
      <td><button class="danger" data-wallet-id="${w.id}">Delete</button></td>
    </tr>
  `).join("");

  body.querySelectorAll("button[data-wallet-id]").forEach((btn) => {
    btn.addEventListener("click", async () => {
      try {
        await FM.api(`/api/wallets/${btn.dataset.walletId}`, { method: "DELETE" });
        FM.setMessage("walletMessage", "Wallet deleted");
        await refreshWallets();
      } catch (error) {
        FM.setMessage("walletMessage", error.message, true);
      }
    });
  });
}

document.getElementById("createWalletForm").addEventListener("submit", async (event) => {
  event.preventDefault();
  try {
    await FM.api("/api/wallets", {
      method: "POST",
      body: JSON.stringify({
        name: document.getElementById("walletName").value.trim(),
        balance: Number(document.getElementById("walletBalance").value)
      })
    });

    FM.setMessage("walletMessage", "Wallet created");
    event.target.reset();
    await refreshWallets();
  } catch (error) {
    FM.setMessage("walletMessage", error.message, true);
  }
});

document.getElementById("topupForm").addEventListener("submit", async (event) => {
  event.preventDefault();
  try {
    const walletId = Number(document.getElementById("topupWallet").value);
    const amount = Number(document.getElementById("topupAmount").value);
    await FM.api(`/api/wallets/${walletId}/topup`, {
      method: "POST",
      body: JSON.stringify({ amount })
    });

    FM.setMessage("walletMessage", "Wallet topped up");
    event.target.reset();
    await refreshWallets();
  } catch (error) {
    FM.setMessage("walletMessage", error.message, true);
  }
});

refreshWallets().catch((error) => FM.setMessage("walletMessage", error.message, true));
