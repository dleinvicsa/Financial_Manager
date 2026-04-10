FM.setupShell("categories");

function renderCategoryList(items, targetId) {
  const target = document.getElementById(targetId);
  if (!items.length) {
    target.innerHTML = "<li>Empty</li>";
    return;
  }

  target.innerHTML = items.map((c) => `
    <li>
      <strong>${c.name}</strong> (id ${c.id})
      <button class="danger right" data-category-id="${c.id}">Delete</button>
    </li>
  `).join("");

  target.querySelectorAll("button[data-category-id]").forEach((btn) => {
    btn.addEventListener("click", async () => {
      try {
        await FM.api(`/api/categories/${btn.dataset.categoryId}`, { method: "DELETE" });
        FM.setMessage("categoryMessage", "Category deleted");
        await refreshCategories();
      } catch (error) {
        FM.setMessage("categoryMessage", error.message, true);
      }
    });
  });
}

async function refreshCategories() {
  const categories = await FM.api("/api/categories");
  renderCategoryList(categories.filter((c) => c.isIncome), "incomeCategories");
  renderCategoryList(categories.filter((c) => !c.isIncome), "expenseCategories");
}

document.getElementById("createCategoryForm").addEventListener("submit", async (event) => {
  event.preventDefault();
  try {
    await FM.api("/api/categories", {
      method: "POST",
      body: JSON.stringify({
        name: document.getElementById("categoryName").value.trim(),
        isIncome: document.getElementById("categoryType").value === "income"
      })
    });

    FM.setMessage("categoryMessage", "Category created");
    event.target.reset();
    await refreshCategories();
  } catch (error) {
    FM.setMessage("categoryMessage", error.message, true);
  }
});

refreshCategories().catch((error) => FM.setMessage("categoryMessage", error.message, true));
