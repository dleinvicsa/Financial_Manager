const FM = (() => {
  const SESSION_KEY = "fm_current_user";
  let quickBalanceTimerId = null;
  const EUR_FORMAT = new Intl.NumberFormat("en-IE", {
    style: "currency",
    currency: "EUR",
    minimumFractionDigits: 2,
    maximumFractionDigits: 2
  });

  function money(value) {
    return EUR_FORMAT.format(Number(value || 0));
  }

  async function api(path, options = {}) {
    const currentUser = getCurrentUser();
    const userHeaders = currentUser?.username ? { "X-Username": currentUser.username } : {};
    const headers = {
      "Content-Type": "application/json",
      ...userHeaders,
      ...(options.headers || {})
    };

    const response = await fetch(path, {
      headers,
      ...options
    });

    const data = await response.json();
    if (!response.ok) {
      throw new Error(data.error || "Request failed");
    }

    const method = (options.method || "GET").toUpperCase();
    if (method !== "GET" && method !== "HEAD") {
      window.dispatchEvent(new CustomEvent("fm:data-changed"));
    }

    return data;
  }

  function getCurrentUser() {
    const raw = localStorage.getItem(SESSION_KEY);
    if (!raw) {
      return null;
    }

    try {
      return JSON.parse(raw);
    } catch {
      localStorage.removeItem(SESSION_KEY);
      return null;
    }
  }

  function setCurrentUser(user) {
    localStorage.setItem(SESSION_KEY, JSON.stringify(user));
  }

  function logout() {
    localStorage.removeItem(SESSION_KEY);
    window.location.href = "/login.html";
  }

  function requireAuth() {
    const user = getCurrentUser();
    if (!user) {
      window.location.href = "/login.html";
      return null;
    }
    return user;
  }

  function redirectIfAuthed() {
    if (getCurrentUser()) {
      window.location.href = "/dashboard.html";
    }
  }

  function setMessage(id, text, isError = false) {
    const el = document.getElementById(id);
    if (!el) {
      return;
    }

    el.textContent = text;
    el.style.color = isError ? "#b91c1c" : "#0f766e";
  }

  function setupShell(activePage) {
    const user = requireAuth();
    if (!user) {
      return null;
    }

    const header = document.getElementById("shellHeader");
    if (header) {
      header.innerHTML = `
        <div class="panel shell">
          <div class="shell-top">
            <div class="brand">
              <img class="brand-logo" src="/CashFlow.png" alt="CashFlow logo" />
              <div class="brand-text">
                <p class="eyebrow">CashFlow</p>
                <p class="subtle">Signed in as <strong>${user.username}</strong></p>
              </div>
            </div>
            <div class="shell-actions">
              <div class="quick-balance" title="Total balance across all wallets">
                <span class="quick-balance-label">Balance</span>
                <strong id="quickBalanceValue">...</strong>
              </div>
              <button id="logoutBtn" class="ghost" type="button">Log out</button>
            </div>
          </div>
          <nav class="menu">
            <a href="/dashboard.html" class="${activePage === "dashboard" ? "active" : ""}">Home</a>
            <a href="/cabinet.html" class="${activePage === "cabinet" ? "active" : ""}">Personal Cabinet</a>
            <a href="/wallets.html" class="${activePage === "wallets" ? "active" : ""}">Wallets</a>
            <a href="/categories.html" class="${activePage === "categories" ? "active" : ""}">Categories</a>
            <a href="/transactions.html" class="${activePage === "transactions" ? "active" : ""}">Transactions</a>
            <a href="/statistics.html" class="${activePage === "statistics" ? "active" : ""}">Statistics</a>
          </nav>
        </div>
      `;

      document.getElementById("logoutBtn").addEventListener("click", logout);

      const refreshQuickBalance = async () => {
        const valueEl = document.getElementById("quickBalanceValue");
        if (!valueEl) {
          return;
        }

        try {
          const wallets = await api("/api/wallets");
          const total = wallets.reduce((sum, wallet) => sum + Number(wallet.balance || 0), 0);
          valueEl.textContent = money(total);
          valueEl.classList.remove("quick-balance-error");
        } catch {
          valueEl.textContent = "--";
          valueEl.classList.add("quick-balance-error");
        }
      };

      window.addEventListener("fm:data-changed", refreshQuickBalance);
      if (quickBalanceTimerId) {
        window.clearInterval(quickBalanceTimerId);
      }
      quickBalanceTimerId = window.setInterval(refreshQuickBalance, 30000);
      void refreshQuickBalance();
    }

    return user;
  }

  return {
    api,
    money,
    getCurrentUser,
    setCurrentUser,
    requireAuth,
    redirectIfAuthed,
    logout,
    setMessage,
    setupShell
  };
})();
