// app.js - the browser only DRAWS things. All event logic (store, search by name/date,
// sort, add, edit, delete) is done by the C++ code in main.cpp (engine.wasm).

const $ = id => document.getElementById(id);
const S = "string", N = "number";
let engine = null;           // the loaded C++ module
let isAdmin = false;
let saved = new Set(JSON.parse(localStorage.getItem("college_saved") || "[]"));

// first-run sample events: [name, category, date, time, venue, organizer, description, link]
const demo = [
  ["CodeFest 2026", "Hackathon", "2026-10-15", "10:00", "Seminar Hall", "Coding Club",
   "A day of creative problem-solving and building useful technology.", "https://example.com/register/codefest"],
  ["Design thinking workshop", "Workshop", "2026-10-20", "14:00", "Innovation Lab", "Design Society",
   "Explore user-first design and rapid ideation.", "https://example.com/register/design"]
];

// ---------- thin wrappers around the C++ functions ----------
const api = {
  load: text => engine.ccall("load_data", null, [S], [text]),
  exportData: () => engine.ccall("export_data", S, [], []),
  search: (q, cat, day, sortBy) =>
    JSON.parse(engine.ccall("search_events", S, [S, S, S, S], [q, cat, day, sortBy])),
  get: id => JSON.parse(engine.ccall("get_event", S, [N], [id])),
  add: f => engine.ccall("add_event", N, [S, S, S, S, S, S, S, S],
    [f.name, f.category, f.date, f.time, f.venue, f.organizer, f.description, f.link]),
  update: (id, f) => engine.ccall("update_event", N, [N, S, S, S, S, S, S, S, S],
    [id, f.name, f.category, f.date, f.time, f.venue, f.organizer, f.description, f.link]),
  remove: id => engine.ccall("delete_event", N, [N], [id])
};

function persist() {
  localStorage.setItem("college_events_v2", api.exportData());
  localStorage.setItem("college_saved", JSON.stringify([...saved]));
}

// ---------- display helpers ----------
const safe = s => String(s || "").replace(/[&<>"']/g, c => ({"&":"&amp;","<":"&lt;",">":"&gt;",'"':"&quot;","'":"&#39;"}[c]));
const fmtDate = s => s ? new Date(s + "T12:00:00").toLocaleDateString("en-IN", {day: "numeric", month: "short", year: "numeric"}) : "Date TBA";
const fmtTime = s => s ? new Date("2000-01-01T" + s).toLocaleTimeString("en-IN", {hour: "numeric", minute: "2-digit"}) : "Time TBA";
const sym = {Workshop: "✳", Hackathon: "⌘", Cultural: "♫", Talk: "◉", Sports: "↗"};

function canDelete(e) {
  if (!e.date) return true;
  return new Date() < new Date(`${e.date}T${e.time || "23:59"}:00`);
}

function card(e) {
  let deleteBtn = "";
  if (isAdmin && canDelete(e)) {
    deleteBtn = `<button class="delete-btn" data-delete="${e.id}" style="background:#ef4444; color:white; border:none; padding:4px 8px; border-radius:4px; cursor:pointer; margin-top:8px; font-size:0.8rem;">Delete Event</button>`;
  }
  return `<article class="event-card">
    <div class="visual ${safe(e.category)}">
      <span class="pill">${safe(e.category)}</span>
      <button class="heart ${saved.has(e.id) ? "saved" : ""}" data-save="${e.id}">${saved.has(e.id) ? "♥" : "♡"}</button>
      <span class="symbol">${sym[e.category] || "✦"}</span>
    </div>
    <div class="content">
      <div class="date">${fmtDate(e.date)}</div>
      <h3>${safe(e.name)}</h3>
      <p>${safe(e.description)}</p>
      <div class="meta"><span>◷ ${fmtTime(e.time)}</span><span>⌖ ${safe(e.venue)}</span></div>
      ${deleteBtn}
    </div>
    <button class="open" data-open="${e.id}">View event details <span style="float:right">↗</span></button>
  </article>`;
}

// ---------- drawing ----------
function renderAdminEventList() {
  const box = $("admin-event-list");
  if (!box) return;
  if (!isAdmin) { box.innerHTML = ""; return; }
  const term = ($("admin-search-delete")?.value || "");
  const list = api.search(term, "All categories", "", "date");
  if (list.length === 0) {
    box.innerHTML = `<p style="color:#94a3b8; font-size:0.9rem; text-align:center; padding:10px;">No events found.</p>`;
    return;
  }
  box.innerHTML = list.map(e => `
    <div style="display:flex; justify-content:space-between; align-items:center; padding:10px; background:#f8fafc; border:1px solid #e2e8f0; border-radius:6px;">
      <div>
        <b style="font-size:0.95rem; color:#1e293b;">${safe(e.name)}</b>
        <span style="font-size:0.8rem; color:#64748b; margin-left:8px;">(${safe(e.category)} · ${safe(e.date) || "No date"})</span>
      </div>
      <button class="admin-delete-item" data-delete-id="${e.id}" style="background:#ef4444; color:white; border:none; padding:6px 12px; border-radius:4px; cursor:pointer; font-size:0.8rem; font-weight:500;">Delete</button>
    </div>`).join("");
}

function updateAdminUI() {
  const add = $("add-event-section"), manage = $("admin-manage-section"), btn = $("admin-btn");
  if (add) add.style.display = isAdmin ? "flex" : "none";
  if (manage) manage.style.display = isAdmin ? "block" : "none";
  if (btn) btn.textContent = isAdmin ? "🔓 Admin Logout" : "🔐 Admin Login";
}

function render() {
  const day = $("datefilter") ? $("datefilter").value : "";
  const list = api.search($("search").value, $("category").value, day, $("sort").value); // <-- C++ does the search
  $("grid").innerHTML = list.length
    ? list.map(card).join("")
    : `<p class="empty" style="grid-column:1/-1">⌕<br><b>No events found</b><br><small>Try a different name, date or category.</small></p>`;
  $("count").textContent = list.length;

  const everything = api.search("", "All categories", "", "date");
  const mine = everything.filter(e => saved.has(e.id));
  $("saved-grid").innerHTML = mine.map(card).join("");
  $("saved-empty").classList.toggle("hidden", mine.length > 0);

  updateAdminUI();
  renderAdminEventList();
}

// ---------- clicks ----------
document.addEventListener("click", ev => {
  const s = ev.target.closest("[data-save]"),
        o = ev.target.closest("[data-open]"),
        d = ev.target.closest("[data-delete]"),
        adminDel = ev.target.closest("[data-delete-id]");

  if (s) {
    const id = +s.dataset.save;
    saved.has(id) ? saved.delete(id) : saved.add(id);
    persist(); render();
  }
  if (o) openEvent(o.dataset.open);
  if (d && isAdmin) {
    const id = +d.dataset.delete, e = api.get(id);
    if (e && canDelete(e)) {
      if (confirm("Are you sure you want to delete this upcoming event?")) {
        api.remove(id); saved.delete(id); persist(); render();
      }
    } else alert("Cannot delete: This event has already taken place or started!");
  }
  if (adminDel && isAdmin) {
    const id = +adminDel.dataset.deleteId, e = api.get(id);
    if (e && confirm(`Are you sure you want to delete "${e.name}"?`)) {
      api.remove(id); saved.delete(id); persist(); render();
    }
  }
});

document.addEventListener("input", e => {
  if (e.target && e.target.id === "admin-search-delete") renderAdminEventList();
});

window.toggleAdminMode = function () {
  if (!isAdmin) {
    const pass = prompt("Enter Admin Password:");
    if (pass === "admin123") { isAdmin = true; alert("Admin mode activated!"); }
    else { alert("Incorrect password!"); return; }
  } else { isAdmin = false; alert("Logged out of admin mode."); }
  render();
};

function openEvent(id) {
  const e = api.get(+id);
  if (!e) return;
  $("modal-name").textContent = e.name;
  $("modal-cat").textContent = e.category;
  $("modal-desc").textContent = e.description;
  $("modal-date").textContent = fmtDate(e.date);
  $("modal-time").textContent = fmtTime(e.time);
  $("modal-venue").textContent = e.venue;
  $("modal-organizer").textContent = e.organizer || "College organizer";
  $("modal-art").textContent = sym[e.category] || "✦";
  $("modal-art").className = "modal-art " + e.category;
  $("register").href = e.link;
  $("modal").classList.remove("hidden");
  document.body.style.overflow = "hidden";
}

function closeModal() {
  $("modal").classList.add("hidden");
  document.body.style.overflow = "";
}

// ---------- wiring ----------
function wireUp() {
  ["search", "category", "sort", "datefilter"].forEach(id => {
    const el = $(id);
    if (el) el.addEventListener(id === "search" || id === "datefilter" ? "input" : "change", render);
  });

  $("clear").onclick = () => {
    $("search").value = "";
    $("category").value = "All categories";
    $("sort").value = "date";
    if ($("datefilter")) $("datefilter").value = "";
    render();
  };

  $("close").onclick = closeModal;
  $("modal").onclick = e => { if (e.target === $("modal")) closeModal(); };
  document.addEventListener("keydown", e => { if (e.key === "Escape") closeModal(); });

  document.querySelectorAll(".nav").forEach(b => b.onclick = () => {
    document.querySelectorAll(".nav").forEach(n => n.classList.toggle("active", n === b));
    document.querySelectorAll(".page").forEach(p => p.classList.add("hidden"));
    $(b.dataset.page).classList.remove("hidden");
    $("crumb").textContent = b.textContent.trim().replace(/^[▦♡＋]\s*/, "");
    window.scrollTo({top: 0, behavior: "smooth"});
  });

  $("event-form").onsubmit = e => {
    e.preventDefault();
    if (!isAdmin) { alert("Unauthorized: Only admins can add events."); return; }
    const f = new FormData(e.target);
    const id = api.add({
      name: f.get("name").trim(),
      category: f.get("category"),
      date: f.get("date"),
      time: f.get("time"),
      venue: f.get("venue").trim(),
      organizer: f.get("organizer").trim() || "College organizer",
      description: f.get("description").trim() || "More details will be shared by the organizer.",
      link: f.get("link").trim()
    });
    if (id < 0) { alert("Event name is required."); return; }
    persist(); render();
    e.target.reset();
    $("message").textContent = "Event published successfully!";
  };
}

// ---------- start: load the C++ engine, then show the page ----------
createEngine().then(m => {
  engine = m;
  const stored = localStorage.getItem("college_events_v2");
  if (stored) api.load(stored);
  else { demo.forEach(d => api.add({name: d[0], category: d[1], date: d[2], time: d[3], venue: d[4], organizer: d[5], description: d[6], link: d[7]})); persist(); }
  wireUp();
  render();
}).catch(err => {
  document.body.insertAdjacentHTML("afterbegin",
    `<p style="padding:20px;color:#b00">Could not load engine.wasm. Open this page through a web server (not by double-clicking the file). ${safe(err)}</p>`);
});
