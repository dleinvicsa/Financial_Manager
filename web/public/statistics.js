FM.setupShell("statistics");

const PIE_COLORS = [
  "#0f766e",
  "#2563eb",
  "#d97706",
  "#ef4444",
  "#7c3aed",
  "#0891b2",
  "#65a30d",
  "#f59e0b",
  "#be185d",
  "#4f46e5"
];

function getCurrentMonthKey() {
  return new Date().toISOString().slice(0, 7);
}

function monthTitle() {
  return new Intl.DateTimeFormat("en-US", { month: "long", year: "numeric" }).format(new Date());
}

function renderPieChart(rows) {
  const pie = document.getElementById("categoryPie");
  const legend = document.getElementById("pieLegend");

  if (!pie || !legend) {
    return;
  }

  if (!rows.length) {
    pie.style.background = "rgba(18, 32, 31, 0.08)";
    pie.innerHTML = "<span class='subtle'>No data</span>";
    legend.innerHTML = "<li class='subtle'>No category activity this month</li>";
    return;
  }

  const total = rows.reduce((sum, row) => sum + row.total, 0);
  let cursor = 0;
  const segments = rows.map((row, index) => {
    const percent = (row.total / total) * 100;
    const start = cursor;
    cursor += percent;
    return `${PIE_COLORS[index % PIE_COLORS.length]} ${start}% ${cursor}%`;
  });

  pie.innerHTML = "";
  pie.style.background = `
    radial-gradient(circle at center, rgba(255,255,255,0.95) 0 39%, transparent 40%),
    conic-gradient(${segments.join(",")})
  `;

  legend.innerHTML = rows.map((row, index) => {
    const percent = ((row.total / total) * 100).toFixed(1);
    const color = PIE_COLORS[index % PIE_COLORS.length];
    return `
      <li>
        <span class="legend-swatch" style="background:${color};"></span>
        <span class="legend-label">${row.categoryName}</span>
        <span class="legend-value">${percent}%</span>
      </li>
    `;
  }).join("");
}

async function init() {
  try {
    const [categories, transactions] = await Promise.all([
      FM.api("/api/categories"),
      FM.api("/api/transactions")
    ]);

    document.getElementById("statsPeriod").textContent = `Current month: ${monthTitle()}`;

    const currentMonth = getCurrentMonthKey();
    const monthTx = transactions.filter((tx) => String(tx.date || "").slice(0, 7) === currentMonth);

    let totalIncome = 0;
    let totalExpense = 0;
    const categoryMap = new Map();

    categories.forEach((category) => {
      categoryMap.set(category.id, {
        categoryName: category.name,
        income: 0,
        expense: 0,
        total: 0
      });
    });

    monthTx.forEach((tx) => {
      if (tx.isIncome) {
        totalIncome += tx.amount;
      } else {
        totalExpense += tx.amount;
      }

      const current = categoryMap.get(tx.categoryId) || {
        categoryName: `Category #${tx.categoryId}`,
        income: 0,
        expense: 0,
        total: 0
      };

      if (tx.isIncome) {
        current.income += tx.amount;
      } else {
        current.expense += tx.amount;
      }
      current.total = current.income + current.expense;
      categoryMap.set(tx.categoryId, current);
    });

    const categoryRows = [...categoryMap.values()]
      .filter((row) => row.total > 0)
      .sort((a, b) => b.total - a.total);

    document.getElementById("summaryCards").innerHTML = `
      <article class="card">
        <p class="label">Earned</p>
        <p class="value type-income">+${FM.money(totalIncome)}</p>
      </article>
      <article class="card">
        <p class="label">Spent</p>
        <p class="value type-expense">-${FM.money(totalExpense)}</p>
      </article>
    `;

    renderPieChart(categoryRows);

    const detailTotal = categoryRows.reduce((sum, row) => sum + row.total, 0);
    const details = document.getElementById("categoryDetails");
    details.innerHTML = categoryRows.map((row) => {
      const share = detailTotal > 0 ? ((row.total / detailTotal) * 100).toFixed(1) : "0.0";
      return `
      <li>
        <strong>${row.categoryName}</strong><br />
        Earned: +${FM.money(row.income)} | Spent: -${FM.money(row.expense)} | Share: ${share}%
      </li>
    `;
    }).join("") || "<li>No category activity this month</li>";
  } catch (error) {
    document.getElementById("summaryCards").innerHTML = `<article class='card'><p class='value'>Error: ${error.message}</p></article>`;
  }
}

init();
