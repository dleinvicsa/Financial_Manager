FM.redirectIfAuthed();

const tabLogin = document.getElementById("tabLogin");
const tabRegister = document.getElementById("tabRegister");
const loginForm = document.getElementById("loginForm");
const registerForm = document.getElementById("registerForm");

function switchTab(mode) {
  const isLogin = mode === "login";
  loginForm.classList.toggle("hidden", !isLogin);
  registerForm.classList.toggle("hidden", isLogin);
  tabLogin.classList.toggle("tab-btn-active", isLogin);
  tabRegister.classList.toggle("tab-btn-active", !isLogin);
  FM.setMessage("authMessage", "");
}

tabLogin.addEventListener("click", () => switchTab("login"));
tabRegister.addEventListener("click", () => switchTab("register"));

loginForm.addEventListener("submit", async (event) => {
  event.preventDefault();
  try {
    const result = await FM.api("/api/auth/login", {
      method: "POST",
      body: JSON.stringify({
        username: document.getElementById("loginUsername").value.trim(),
        password: document.getElementById("loginPassword").value
      })
    });

    FM.setCurrentUser(result.user);
    window.location.href = "/dashboard.html";
  } catch (error) {
    FM.setMessage("authMessage", error.message, true);
  }
});

registerForm.addEventListener("submit", async (event) => {
  event.preventDefault();
  try {
    const result = await FM.api("/api/auth/register", {
      method: "POST",
      body: JSON.stringify({
        username: document.getElementById("regUsername").value.trim(),
        password: document.getElementById("regPassword").value
      })
    });

    FM.setCurrentUser(result.user);
    window.location.href = "/dashboard.html";
  } catch (error) {
    FM.setMessage("authMessage", error.message, true);
  }
});
