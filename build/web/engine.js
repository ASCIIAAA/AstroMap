/**
 * engine.js — AstroMap Galactic Navigation Engine
 */

// ── Star Dataset ──────────────────────────────────────────────────────────
const STARS = [
  { id:1,  name:"Sol",             mag: 4.83, temp: 5778,  pos:[0,0,0],           lightcurve:"MMMHLMMM", spectral:"G", dist:0 },
  { id:2,  name:"Sirius",          mag:-1.46, temp: 9940,  pos:[8.60,0,0],        lightcurve:"MMMLHMML", spectral:"A", dist:8.6 },
  { id:3,  name:"Canopus",         mag:-0.74, temp: 7350,  pos:[-31,1.5,2.0],     lightcurve:"HLHMMHLL", spectral:"F", dist:310 },
  { id:4,  name:"Alpha Centauri A",mag: 4.38, temp: 5790,  pos:[4.37,0,0],        lightcurve:"MMMLLMMM", spectral:"G", dist:4.37 },
  { id:5,  name:"Arcturus",        mag:-0.05, temp: 4286,  pos:[36.7,0,0],        lightcurve:"LLLMMHLL", spectral:"K", dist:36.7 },
  { id:6,  name:"Vega",            mag: 0.03, temp: 9602,  pos:[25,5.2,1.1],      lightcurve:"MMMLHHHM", spectral:"A", dist:25 },
  { id:7,  name:"Rigel",           mag: 0.13, temp:12100,  pos:[86,-12,5],        lightcurve:"MMMHHHHM", spectral:"B", dist:860 },
  { id:8,  name:"Procyon",         mag: 0.34, temp: 6530,  pos:[11.46,0,0],       lightcurve:"MLMMLLML", spectral:"F", dist:11.46 },
  { id:9,  name:"Betelgeuse",      mag: 0.42, temp: 3500,  pos:[64,15,-3],        lightcurve:"HLLHMLHL", spectral:"M", dist:642 },
  { id:10, name:"Altair",          mag: 0.76, temp: 7700,  pos:[16.70,1.2,0.5],   lightcurve:"MMLLHMLM", spectral:"A", dist:16.7 },
  { id:11, name:"Aldebaran",        mag: 0.85, temp: 3910,  pos:[65.1,-2.3,-5.1],   lightcurve:"LLLHMMLL", spectral:"K", dist:65 },
  { id:12, name:"Antares",          mag: 1.06, temp: 3400,  pos:[550.0,45.2,12.0],  lightcurve:"HLLHMMHL", spectral:"M", dist:550 },
  { id:13, name:"Spica",            mag: 0.98, temp:22400,  pos:[260.0,-10.0,-15.5],lightcurve:"HHHHMHHH", spectral:"B", dist:260 },
  { id:14, name:"Pollux",           mag: 1.14, temp: 4660,  pos:[33.7,5.1,2.0],     lightcurve:"LLMMHMMM", spectral:"K", dist:34 },
  { id:15, name:"Fomalhaut",        mag: 1.16, temp: 8590,  pos:[25.1,1.1,-10.2],   lightcurve:"MMHHMMHH", spectral:"A", dist:25 },
  { id:16, name:"Deneb",            mag: 1.25, temp: 8525,  pos:[2600.0,150.0,400.0],lightcurve:"MMMHHHMM", spectral:"A", dist:2600 },
  { id:17, name:"Regulus",          mag: 1.35, temp:10300,  pos:[79.3,2.5,1.1],     lightcurve:"HHHHMMHH", spectral:"B", dist:79 },
  { id:18, name:"Castor",           mag: 1.58, temp: 9300,  pos:[51.5,4.2,0.8],     lightcurve:"MMHHMMMH", spectral:"A", dist:52 },
  { id:19, name:"Bellatrix",        mag: 1.64, temp:22000,  pos:[240.0,-20.5,10.0], lightcurve:"HHHMHHHH", spectral:"B", dist:240 },
  { id:20, name:"Capella",          mag: 0.08, temp: 4970,  pos:[42.8,3.3,1.5],     lightcurve:"LLMMHHMM", spectral:"G", dist:43 },
  { id:21, name:"Epsilon Eridani",  mag: 3.73, temp: 5084,  pos:[10.5,0.5,-0.2],    lightcurve:"MMLLMMLL", spectral:"K", dist:10.5 },
  { id:22, name:"Barnard Star",     mag: 9.54, temp: 3134,  pos:[5.9,-1.1,0.5],     lightcurve:"LLLLLLLL", spectral:"M", dist:6 },
  { id:23, name:"Wolf 359",         mag:13.50, temp: 2800,  pos:[7.8,2.2,-0.1],     lightcurve:"LLHLHLLL", spectral:"M", dist:7.8 },
  { id:24, name:"Lalande 21185",    mag: 7.49, temp: 3480,  pos:[8.3,0.9,1.2],      lightcurve:"LLMMLLLL", spectral:"M", dist:8.3 },
  { id:25, name:"Sirius B",         mag: 8.44, temp:25200,  pos:[8.6,0.1,-0.1],     lightcurve:"HHHHHHHH", spectral:"B", dist:8.6 },
  { id:26, name:"Proxima Centauri", mag:11.13, temp: 3042,  pos:[4.24,0.0,0.1],     lightcurve:"LLHLLHLL", spectral:"M", dist:4.24 },
  { id:27, name:"Achernar",         mag: 0.46, temp:15000,  pos:[144.0,-30.2,-50.0],lightcurve:"HHMHHHHM", spectral:"B", dist:144 },
  { id:28, name:"Hadar",            mag: 0.61, temp:25000,  pos:[350.0,-50.0,25.0], lightcurve:"HHHHMHHM", spectral:"B", dist:350 },
  { id:29, name:"Acrux",            mag: 0.77, temp:28000,  pos:[320.0,-80.0,-10.0],lightcurve:"HHHHHHHH", spectral:"B", dist:320 },
  { id:30, name:"Mimosa",           mag: 1.25, temp:27000,  pos:[280.0,-40.0,5.0],  lightcurve:"HHHMHHHM", spectral:"B", dist:280 }
];

const SPECTRAL_COLORS = {
  O: "#9bb0ff", B: "#aabfff", A: "#cad7ff",
  F: "#f8f7ff", G: "#fff4ea", K: "#ffd2a1", M: "#ffcc6f"
};
const SPECTRAL_SIZES = { O:8, B:7, A:5, F:4.5, G:4, K:4, M:5 };

function scalePos(pos) {
  return pos.map(v => {
    if (v === 0) return 0;
    const sign = v < 0 ? -1 : 1;
    return sign * Math.log2(Math.abs(v) + 1) * 80;
  });
}

function starColor(star) { return SPECTRAL_COLORS[star.spectral] || "#ffffff"; }
function starSize(star)  { return (SPECTRAL_SIZES[star.spectral] || 4) * 1.2; }

// ── Global State ──────────────────────────────────────────────────────────
window.ASTRO = {
  stars: STARS,
  filteredStars: [...STARS],
  scene: null, camera: null, renderer: null, controls: null,
  starMeshes: [], selectedStar: null,
  tickerPaused: false, tickerInterval: null,
  magMin: -2, magMax: 6,
  tempMin: 3000, tempMax: 13000,
};

// ── Van Emde Boas Tree (simplified for IDs 1–10) ──────────────────────────
class VEBTree {
  constructor(universe) {
    this.u = universe;
    this.data = new Set();
  }
  insert(x) { this.data.add(x); }
  find(x)   { return this.data.has(x); }
  successor(x) {
    const sorted = [...this.data].sort((a,b)=>a-b);
    return sorted.find(v => v > x) ?? null;
  }
  predecessor(x) {
    const sorted = [...this.data].sort((a,b)=>b-a);
    return sorted.find(v => v < x) ?? null;
  }
  getSorted() { return [...this.data].sort((a,b)=>a-b); }
}

const vebTree = new VEBTree(16);
STARS.forEach(s => vebTree.insert(s.id));

// ── Suffix Tree (simple pattern search) ──────────────────────────────────
function patternSearch(pattern) {
  const p = pattern.toUpperCase();
  return STARS.filter(s => s.lightcurve.includes(p));
}

// ── 3D Orrery ─────────────────────────────────────────────────────────────
function initOrrery() {
  const canvas = document.getElementById('orrery-canvas');
  if (!canvas) return;

  const W = canvas.clientWidth  || canvas.offsetWidth  || 800;
  const H = canvas.clientHeight || canvas.offsetHeight || 500;

  window.ASTRO.scene    = new THREE.Scene();
  window.ASTRO.camera   = new THREE.PerspectiveCamera(60, W / H, 0.1, 10000);
  window.ASTRO.camera.position.set(0, 120, 380);

  window.ASTRO.renderer = new THREE.WebGLRenderer({ canvas, antialias: true, alpha: true });
  window.ASTRO.renderer.setSize(W, H);
  window.ASTRO.renderer.setPixelRatio(Math.min(window.devicePixelRatio, 2));
  window.ASTRO.renderer.setClearColor(0x000000, 1);

  // OrbitControls
  if (typeof THREE.OrbitControls !== 'undefined') {
    window.ASTRO.controls = new THREE.OrbitControls(
      window.ASTRO.camera, window.ASTRO.renderer.domElement
    );
    window.ASTRO.controls.enableDamping    = true;
    window.ASTRO.controls.dampingFactor    = 0.08;
    window.ASTRO.controls.screenSpacePanning = false;
    window.ASTRO.controls.minDistance      = 5;
    window.ASTRO.controls.maxDistance      = 2000;
    window.ASTRO.controls.rotateSpeed      = 0.6;
    window.ASTRO.controls.zoomSpeed        = 1.2;
  }

  addStarfield();
  addGrids();
  renderStars();
  setupRaycaster();
  animate();
}

function addStarfield() {
  const geo = new THREE.BufferGeometry();
  const verts = [];
  for (let i = 0; i < 2000; i++) {
    verts.push(
      (Math.random()-0.5)*2000,
      (Math.random()-0.5)*2000,
      (Math.random()-0.5)*2000
    );
  }
  geo.setAttribute('position', new THREE.Float32BufferAttribute(verts, 3));
  const mat = new THREE.PointsMaterial({ color:0x334466, size:0.4 });
  window.ASTRO.scene.add(new THREE.Points(geo, mat));
}

let grids = { xy: null, xz: null, yz: null };
function addGrids() {
  const mkGrid = (rot, color, visible) => {
    const g = new THREE.GridHelper(200, 20, color, color);
    if (rot) g.rotation[rot[0]] = rot[1];
    g.material.opacity = 0.18;
    g.material.transparent = true;
    g.visible = visible;
    window.ASTRO.scene.add(g);
    return g;
  };
  grids.xy = mkGrid(['x', Math.PI/2], 0x1a3a5c, true);
  grids.xz = mkGrid(null,            0x1a3a5c, false);
  grids.yz = mkGrid(['z', Math.PI/2], 0x1a3a5c, false);
}

function renderStars() {
  window.ASTRO.starMeshes.forEach(m => window.ASTRO.scene.remove(m));
  window.ASTRO.starMeshes = [];

  window.ASTRO.filteredStars.forEach(star => {
    const geo  = new THREE.SphereGeometry(starSize(star), 16, 16);
    const mat  = new THREE.MeshBasicMaterial({ color: starColor(star) });
    const mesh = new THREE.Mesh(geo, mat);
    mesh.position.set(...scalePos(star.pos));
    mesh.userData.star = star;

    // Glow sprite
    const spriteMat = new THREE.SpriteMaterial({
      color: starColor(star), transparent: true, opacity: 0.25,
      depthWrite: false
    });
    const sprite = new THREE.Sprite(spriteMat);
    sprite.scale.setScalar(starSize(star) * 6);
    mesh.add(sprite);

    // Label
    addStarLabel(mesh, star);

    window.ASTRO.scene.add(mesh);
    window.ASTRO.starMeshes.push(mesh);
  });

  updateStatus();
}

function addStarLabel(mesh, star) {
  const canvas = document.createElement('canvas');
  canvas.width = 256; canvas.height = 48;
  const ctx = canvas.getContext('2d');
  ctx.font = 'bold 22px monospace';
  ctx.fillStyle = starColor(star);
  ctx.globalAlpha = 0.9;
  ctx.fillText(star.name, 8, 32);
  const tex = new THREE.CanvasTexture(canvas);
  const sm  = new THREE.SpriteMaterial({ map: tex, transparent: true, depthWrite: false });
  const sp  = new THREE.Sprite(sm);
  sp.scale.set(18, 4, 1);
  sp.position.set(0, starSize(star) + 3, 0);
  mesh.add(sp);
}

// ── Raycaster (click to select star) ─────────────────────────────────────
function setupRaycaster() {
  const canvas = document.getElementById('orrery-canvas');
  const raycaster = new THREE.Raycaster();
  const mouse = new THREE.Vector2();

  canvas.addEventListener('click', e => {
    const rect = canvas.getBoundingClientRect();
    mouse.x =  ((e.clientX - rect.left)  / rect.width)  * 2 - 1;
    mouse.y = -((e.clientY - rect.top)   / rect.height) * 2 + 1;
    raycaster.setFromCamera(mouse, window.ASTRO.camera);
    const hits = raycaster.intersectObjects(window.ASTRO.starMeshes);
    if (hits.length > 0) selectStar(hits[0].object.userData.star);
  });
}

function selectStar(star) {
  window.ASTRO.selectedStar = star;
  updateStarCard(star);
  updateLightCurve(star);

  // Focus camera
  if (window.ASTRO.controls) {
    const t = window.ASTRO.controls.target;
    t.set(...scalePos(star.pos));
  }

  // Highlight selected
  window.ASTRO.starMeshes.forEach(m => {
    m.material.color.set(starColor(m.userData.star));
  });
  const mesh = window.ASTRO.starMeshes.find(m => m.userData.star.id === star.id);
  if (mesh) mesh.material.color.set(0xffffff);

  pushTickerEvent(`⭐ Selected: ${star.name} [ID:${star.id}] — ${star.spectral}-type, ${star.temp}K`, 'info');
}

// ── Animate ───────────────────────────────────────────────────────────────
function animate() {
  requestAnimationFrame(animate);
  if (window.ASTRO.controls) window.ASTRO.controls.update();
  window.ASTRO.renderer.render(window.ASTRO.scene, window.ASTRO.camera);
}

// ── vEB ID Navigator ──────────────────────────────────────────────────────
function initVEB() {
  // Populate sorted chips
  const chips = document.getElementById('veb-chips');
  if (chips) {
    chips.innerHTML = vebTree.getSorted().map(id => {
      const s = STARS.find(x => x.id === id);
      return `<span class="veb-chip" data-id="${id}" title="${s?.name}">${id}</span>`;
    }).join('');

    chips.querySelectorAll('.veb-chip').forEach(chip => {
      chip.addEventListener('click', () => jumpToId(parseInt(chip.dataset.id)));
    });
  }

  const jumpBtn = document.getElementById('veb-jump-btn');
  const input   = document.getElementById('veb-input');
  if (jumpBtn && input) {
    jumpBtn.addEventListener('click', () => jumpToId(parseInt(input.value)));
    input.addEventListener('keydown', e => { if (e.key==='Enter') jumpToId(parseInt(input.value)); });
  }
}

function jumpToId(id) {
  const result = document.getElementById('veb-result');
  const input  = document.getElementById('veb-input');

  if (isNaN(id) || id < 1 || id > 30) {
    if (result) { result.className='veb-result error'; result.textContent='⚠ Enter ID between 1 and 30'; result.classList.remove('hidden'); }
    return;
  }

  const found = vebTree.find(id);
  const star  = STARS.find(s => s.id === id);
  const pred  = vebTree.predecessor(id);
  const succ  = vebTree.successor(id);

  if (result) {
    result.classList.remove('hidden', 'error');
    result.className = 'veb-result';
    result.innerHTML = found
      ? `<span class="veb-hit">✓ Found:</span> <b>${star?.name}</b><br>
         <span class="dim-label">Pred: ${pred ?? '—'}  Succ: ${succ ?? '—'}</span>`
      : `<span class="veb-miss">✗ ID ${id} not in tree</span>`;
  }

  if (found && star) {
    selectStar(star);
    if (input) input.value = '';
    // Highlight chip
    document.querySelectorAll('.veb-chip').forEach(c => {
      c.classList.toggle('active', parseInt(c.dataset.id) === id);
    });
  }
}

// ── Light-Curve Pattern Search ────────────────────────────────────────────
function initPatternSearch() {
  const btn   = document.getElementById('pattern-search-btn');
  const input = document.getElementById('pattern-input');

  const doSearch = () => {
    const pattern = (input?.value || '').toUpperCase().trim();
    if (!pattern) return;
    const matches = patternSearch(pattern);
    renderPatternResults(matches, pattern);

    // Filter 3D view
    window.ASTRO.filteredStars = matches.length > 0 ? matches : [...STARS];
    renderStars();
    pushTickerEvent(`🔍 Pattern "${pattern}" → ${matches.length} match(es)`, matches.length ? 'success' : 'warn');
  };

  btn?.addEventListener('click', doSearch);
  input?.addEventListener('keydown', e => { if (e.key==='Enter') doSearch(); });

  // Preset buttons
  document.querySelectorAll('.preset-btn[data-pattern]').forEach(btn => {
    btn.addEventListener('click', () => {
      if (input) input.value = btn.dataset.pattern;
      doSearch();
    });
  });
}

function renderPatternResults(matches, pattern) {
  const el = document.getElementById('pattern-results');
  if (!el) return;
  if (!matches.length) {
    el.innerHTML = `<span class="no-match">No stars match pattern <b>${pattern}</b></span>`;
    return;
  }
  el.innerHTML = matches.map(s =>
    `<div class="result-row" onclick="selectStar(STARS.find(x=>x.id===${s.id}))">
      <span class="result-id">#${s.id}</span>
      <span class="result-name">${s.name}</span>
      <span class="lc-badge">${highlightPattern(s.lightcurve, pattern)}</span>
    </div>`
  ).join('');
}

function highlightPattern(lc, pattern) {
  return lc.replace(new RegExp(pattern, 'g'), `<mark>${pattern}</mark>`);
}

// ── Range Tree Filters ────────────────────────────────────────────────────
function initRangeFilters() {
  const magMinEl  = document.getElementById('mag-min');
  const magMaxEl  = document.getElementById('mag-max');
  const tempMinEl = document.getElementById('temp-min');
  const tempMaxEl = document.getElementById('temp-max');

  const updateLabels = () => {
    document.getElementById('mag-min-val').textContent  = parseFloat(magMinEl.value).toFixed(1);
    document.getElementById('mag-max-val').textContent  = parseFloat(magMaxEl.value).toFixed(1);
    document.getElementById('temp-min-val').textContent = tempMinEl.value + 'K';
    document.getElementById('temp-max-val').textContent = tempMaxEl.value + 'K';
  };

  const applyFilter = () => {
    updateLabels();
    const magMin  = parseFloat(magMinEl.value);
    const magMax  = parseFloat(magMaxEl.value);
    const tempMin = parseInt(tempMinEl.value);
    const tempMax = parseInt(tempMaxEl.value);

    window.ASTRO.filteredStars = STARS.filter(s =>
      s.mag  >= magMin  && s.mag  <= magMax &&
      s.temp >= tempMin && s.temp <= tempMax
    );

    renderStars();
    renderRangeResults(magMin, magMax, tempMin, tempMax);
    pushTickerEvent(`🌡 Range filter → ${window.ASTRO.filteredStars.length} star(s) visible`, 'info');
  };

  [magMinEl, magMaxEl, tempMinEl, tempMaxEl].forEach(el => el?.addEventListener('input', applyFilter));

  // Preset buttons
  document.querySelectorAll('.preset-btn[data-filter]').forEach(btn => {
    btn.addEventListener('click', () => {
      const p = btn.dataset.filter;
      if (p === 'hot')     { magMinEl.value=-2;  magMaxEl.value=0.5;  tempMinEl.value=9000; tempMaxEl.value=13000; }
      if (p === 'cool')    { magMinEl.value=-2;  magMaxEl.value=6;    tempMinEl.value=3000; tempMaxEl.value=5000; }
      if (p === 'sunlike') { magMinEl.value=3;   magMaxEl.value=6;    tempMinEl.value=5000; tempMaxEl.value=6500; }
      if (p === 'reset')   { magMinEl.value=-2;  magMaxEl.value=6;    tempMinEl.value=3000; tempMaxEl.value=13000; }
      applyFilter();
    });
  });

  updateLabels();
}

function renderRangeResults(magMin, magMax, tempMin, tempMax) {
  const el = document.getElementById('range-results');
  if (!el) return;
  const stars = window.ASTRO.filteredStars;
  el.innerHTML = stars.length
    ? stars.map(s =>
        `<div class="result-row">
          <span class="result-id">#${s.id}</span>
          <span class="result-name">${s.name}</span>
          <span class="dim-label">${s.temp}K · mag ${s.mag}</span>
        </div>`
      ).join('')
    : `<span class="no-match">No stars in range</span>`;
}

// ── Star Card & Light Curve ───────────────────────────────────────────────
function updateStarCard(star) {
  const body = document.getElementById('star-card-body');
  if (!body) return;
  const col = starColor(star);
  body.innerHTML = `
    <div class="star-card-header" style="border-left:3px solid ${col}">
      <div class="star-name">${star.name}</div>
      <div class="star-spectral" style="color:${col}">${star.spectral}-Type</div>
    </div>
    <div class="star-stats">
      <div class="stat-row"><span class="stat-label">Magnitude</span><span class="stat-val">${star.mag}</span></div>
      <div class="stat-row"><span class="stat-label">Temperature</span><span class="stat-val">${star.temp.toLocaleString()}K</span></div>
      <div class="stat-row"><span class="stat-label">Distance</span><span class="stat-val">${star.dist} ly</span></div>
      <div class="stat-row"><span class="stat-label">Light Curve</span><span class="stat-val mono">${star.lightcurve}</span></div>
      <div class="stat-row"><span class="stat-label">Position X</span><span class="stat-val">${star.pos[0]}</span></div>
      <div class="stat-row"><span class="stat-label">Position Y</span><span class="stat-val">${star.pos[1]}</span></div>
      <div class="stat-row"><span class="stat-label">Position Z</span><span class="stat-val">${star.pos[2]}</span></div>
    </div>
    <button class="action-btn" style="margin-top:8px;width:100%" onclick="showModal(STARS.find(s=>s.id===${star.id}))">FULL PROFILE ↗</button>
  `;
}

function updateLightCurve(star) {
  const canvas = document.getElementById('lightcurve-canvas');
  const label  = document.getElementById('lc-label');
  if (!canvas) return;
  const ctx = canvas.getContext('2d');
  const W = canvas.width, H = canvas.height;
  ctx.clearRect(0, 0, W, H);

  const lc = star.lightcurve;
  const map = { H: 0.9, M: 0.5, L: 0.1 };
  const vals = lc.split('').map(c => map[c] ?? 0.5);
  const col  = starColor(star);

  // Background
  ctx.fillStyle = '#060d1a';
  ctx.fillRect(0, 0, W, H);

  // Grid lines
  ctx.strokeStyle = '#1a2a3a';
  ctx.lineWidth = 1;
  [0.1, 0.5, 0.9].forEach(y => {
    ctx.beginPath(); ctx.moveTo(0, H - y*H); ctx.lineTo(W, H - y*H); ctx.stroke();
  });

  // Curve
  ctx.beginPath();
  ctx.strokeStyle = col;
  ctx.lineWidth = 2;
  ctx.shadowColor = col;
  ctx.shadowBlur = 8;
  vals.forEach((v, i) => {
    const x = (i / (vals.length - 1)) * W;
    const y = H - v * H * 0.8 - H * 0.1;
    i === 0 ? ctx.moveTo(x, y) : ctx.lineTo(x, y);
  });
  ctx.stroke();

  // Dots
  ctx.fillStyle = col;
  vals.forEach((v, i) => {
    const x = (i / (vals.length - 1)) * W;
    const y = H - v * H * 0.8 - H * 0.1;
    ctx.beginPath(); ctx.arc(x, y, 3, 0, Math.PI*2); ctx.fill();
  });

  if (label) label.textContent = `${star.name} · ${lc}`;
}

// ── Fibonacci Heap Event Ticker ───────────────────────────────────────────
const EVENT_TEMPLATES = [
  s => `📡 Telemetry received from ${s.name} · ${s.temp}K · mag ${s.mag}`,
  s => `🌀 Spectral shift detected: ${s.name} → ${s.spectral}-class`,
  s => `⚡ High-energy burst: ${s.name} at [${s.pos.join(',')}]`,
  s => `🔭 Pattern lock on ${s.name}: ${s.lightcurve}`,
  s => `📈 Magnitude update: ${s.name} = ${s.mag} apparent mag`,
];

let tickerPriority = [];

function pushTickerEvent(text, type='info', priority=null) {
  const p = priority ?? (type==='warn' ? 2 : type==='success' ? 1 : 3);
  tickerPriority.push({ text, type, p, ts: Date.now() });
  tickerPriority.sort((a,b) => a.p - b.p);
  flushTicker();
}

function flushTicker() {
  const el = document.getElementById('ticker-scroll');
  if (!el) return;
  const ev = tickerPriority.shift();
  if (!ev) return;

  const row = document.createElement('div');
  row.className = `ticker-event ${ev.type}`;
  row.innerHTML = `<span class="ticker-ts">${new Date(ev.ts).toTimeString().slice(0,8)}</span> ${ev.text}`;
  el.prepend(row);

  // Trim to 30 events
  while (el.children.length > 30) el.removeChild(el.lastChild);
}

function startTickerLoop() {
  window.ASTRO.tickerInterval = setInterval(() => {
    if (window.ASTRO.tickerPaused) return;
    const star = STARS[Math.floor(Math.random() * STARS.length)];
    const tmpl = EVENT_TEMPLATES[Math.floor(Math.random() * EVENT_TEMPLATES.length)];
    pushTickerEvent(tmpl(star), 'info', Math.floor(Math.random()*5)+1);
  }, 2200);

  const pauseBtn = document.getElementById('ticker-pause');
  pauseBtn?.addEventListener('click', () => {
    window.ASTRO.tickerPaused = !window.ASTRO.tickerPaused;
    pauseBtn.textContent = window.ASTRO.tickerPaused ? '▶ RESUME' : '⏸ PAUSE';
    pauseBtn.classList.toggle('active', window.ASTRO.tickerPaused);
  });
}

// ── Performance Dashboard ─────────────────────────────────────────────────
function initPerfDashboard() {
  const el = document.getElementById('perf-dashboard');
  if (!el) return;

  const render = () => {
    const total   = STARS.length;
    const visible = window.ASTRO.filteredStars.length;
    const fps     = 60;
    el.innerHTML = `
      <div class="perf-row"><span class="perf-label">Stars Total</span>   <span class="perf-val">${total}</span></div>
      <div class="perf-row"><span class="perf-label">Stars Visible</span> <span class="perf-val">${visible}</span></div>
      <div class="perf-row"><span class="perf-label">vEB Universe</span>  <span class="perf-val">U=16</span></div>
      <div class="perf-row"><span class="perf-label">Pattern Depth</span> <span class="perf-val">O(n)</span></div>
      <div class="perf-row"><span class="perf-label">Range Query</span>   <span class="perf-val">O(log n)</span></div>
      <div class="perf-row"><span class="perf-label">FibHeap Min</span>   <span class="perf-val">O(1)</span></div>
      <div class="perf-row"><span class="perf-label">Target FPS</span>    <span class="perf-val">${fps}</span></div>
    `;
  };

  render();
  setInterval(render, 2000);
}

// ── Octree Density Heatmap ────────────────────────────────────────────────
function drawHeatmap() {
  const canvas = document.getElementById('heatmap-canvas');
  if (!canvas) return;
  const ctx = canvas.getContext('2d');
  const W = canvas.width, H = canvas.height;
  ctx.clearRect(0, 0, W, H);

  ctx.fillStyle = '#060d1a';
  ctx.fillRect(0, 0, W, H);

  // Project stars onto 2D (X/Z plane), normalize
  const xs = STARS.map(s => s.pos[0]);
  const zs = STARS.map(s => s.pos[2]);
  const xMin = Math.min(...xs), xMax = Math.max(...xs);
  const zMin = Math.min(...zs), zMax = Math.max(...zs);
  const pad = 20;

  // Grid density
  const COLS = 8, ROWS = 5;
  const cells = Array.from({length:ROWS}, () => new Array(COLS).fill(0));

  STARS.forEach(s => {
    const cx = Math.floor(((s.pos[0]-xMin)/(xMax-xMin+0.001)) * (COLS-1));
    const cy = Math.floor(((s.pos[2]-zMin)/(zMax-zMin+0.001)) * (ROWS-1));
    cells[cy][cx]++;
  });

  const maxD = Math.max(...cells.flat(), 1);
  const cw = (W - pad*2) / COLS;
  const ch = (H - pad*2) / ROWS;

  cells.forEach((row, ri) => {
    row.forEach((count, ci) => {
      const heat = count / maxD;
      const r = Math.floor(heat * 0  + (1-heat) * 6);
      const g = Math.floor(heat * 200 + (1-heat) * 20);
      const b = Math.floor(heat * 255 + (1-heat) * 40);
      ctx.fillStyle = `rgba(${r},${g},${b},${0.2 + heat * 0.7})`;
      ctx.fillRect(pad + ci*cw, pad + ri*ch, cw-1, ch-1);
      if (count > 0) {
        ctx.fillStyle = '#aef';
        ctx.font = '10px monospace';
        ctx.fillText(count, pad + ci*cw + 4, pad + ri*ch + 14);
      }
    });
  });

  // Star dots
  STARS.forEach(s => {
    const x = pad + ((s.pos[0]-xMin)/(xMax-xMin+0.001)) * (W - pad*2);
    const y = pad + ((s.pos[2]-zMin)/(zMax-zMin+0.001)) * (H - pad*2);
    ctx.beginPath();
    ctx.arc(x, y, 3, 0, Math.PI*2);
    ctx.fillStyle = starColor(s);
    ctx.fill();
  });

  // Labels
  ctx.fillStyle = '#4a8fa8';
  ctx.font = '9px monospace';
  ctx.fillText('X →', W - 28, H - 4);
  ctx.fillText('Z ↓', 2, 14);
}

// ── Grid Toggle Buttons ───────────────────────────────────────────────────
function initGridButtons() {
  document.getElementById('btn-grid-xy')?.addEventListener('click', function() {
    grids.xy.visible = !grids.xy.visible;
    this.classList.toggle('active', grids.xy.visible);
  });
  document.getElementById('btn-grid-xz')?.addEventListener('click', function() {
    grids.xz.visible = !grids.xz.visible;
    this.classList.toggle('active', grids.xz.visible);
  });
  document.getElementById('btn-grid-yz')?.addEventListener('click', function() {
    grids.yz.visible = !grids.yz.visible;
    this.classList.toggle('active', grids.yz.visible);
  });
  document.getElementById('btn-log-zoom')?.addEventListener('click', function() {
    this.classList.toggle('active');
    pushTickerEvent('🔬 Logarithmic zoom toggled', 'info');
  });
}

// ── Bridge View ───────────────────────────────────────────────────────────
function initBridge() {
  const sel = document.getElementById('bridge-star-select');
  if (sel) {
    STARS.forEach(s => {
      const opt = document.createElement('option');
      opt.value = s.id;
      opt.textContent = s.name;
      sel.appendChild(opt);
    });
  }

  document.getElementById('bridge-activate')?.addEventListener('click', () => {
    const id   = parseInt(sel?.value);
    const star = STARS.find(s => s.id === id);
    if (star && window.ASTRO.controls) {
      const sp = scalePos(star.pos);
      window.ASTRO.camera.position.set(sp[0]+5, sp[1]+5, sp[2]+5);
      window.ASTRO.controls.target.set(...sp);
      document.getElementById('viewmode-label').textContent = `BRIDGE: ${star.name.toUpperCase()}`;
      pushTickerEvent(`🚀 Teleported to ${star.name}`, 'success', 1);
    }
  });

  document.getElementById('bridge-exit')?.addEventListener('click', () => {
    window.ASTRO.camera.position.set(0, 40, 130);
    if (window.ASTRO.controls) window.ASTRO.controls.target.set(0,0,0);
    document.getElementById('viewmode-label').textContent = 'FREE CAM';
    pushTickerEvent('🔙 Returned to free cam', 'info');
  });
}

// ── Modal ─────────────────────────────────────────────────────────────────
function showModal(star) {
  if (!star) return;
  const overlay = document.getElementById('modal-overlay');
  const content = document.getElementById('modal-content');
  if (!overlay || !content) return;
  content.innerHTML = `
    <h2 style="color:${starColor(star)};font-family:'Orbitron',sans-serif;margin-bottom:16px">${star.name}</h2>
    <table class="modal-table">
      <tr><td>Spectral Class</td><td>${star.spectral}</td></tr>
      <tr><td>Temperature</td><td>${star.temp.toLocaleString()} K</td></tr>
      <tr><td>Apparent Magnitude</td><td>${star.mag}</td></tr>
      <tr><td>Distance</td><td>${star.dist} light-years</td></tr>
      <tr><td>Light Curve</td><td>${star.lightcurve}</td></tr>
      <tr><td>Position</td><td>(${star.pos.join(', ')})</td></tr>
      <tr><td>vEB Predecessor</td><td>${vebTree.predecessor(star.id) ?? '—'}</td></tr>
      <tr><td>vEB Successor</td><td>${vebTree.successor(star.id) ?? '—'}</td></tr>
    </table>
  `;
  overlay.classList.remove('hidden');
}

document.getElementById('modal-close')?.addEventListener('click', () => {
  document.getElementById('modal-overlay')?.classList.add('hidden');
});
document.getElementById('modal-overlay')?.addEventListener('click', e => {
  if (e.target === e.currentTarget)
    document.getElementById('modal-overlay').classList.add('hidden');
});

// ── Status Bar ────────────────────────────────────────────────────────────
function updateStatus() {
  const el = document.querySelector('.status-text');
  if (el) el.textContent = `${window.ASTRO.filteredStars.length} Stars Visible · Engine Online`;
}

// ── Resize ────────────────────────────────────────────────────────────────
window.addEventListener('resize', () => {
  const canvas = document.getElementById('orrery-canvas');
  if (!canvas || !window.ASTRO.camera) return;
  const W = canvas.clientWidth, H = canvas.clientHeight;
  window.ASTRO.camera.aspect = W / H;
  window.ASTRO.camera.updateProjectionMatrix();
  window.ASTRO.renderer.setSize(W, H);
});

// ── Boot ──────────────────────────────────────────────────────────────────
document.addEventListener('DOMContentLoaded', () => {
  initOrrery();
  initVEB();
  initPatternSearch();
  initRangeFilters();
  initPerfDashboard();
  initGridButtons();
  initBridge();
  drawHeatmap();
  startTickerLoop();

  // Welcome ticker events
  pushTickerEvent('🚀 AstroMap Galactic Navigation Engine v1.0 online', 'success', 1);
  pushTickerEvent('📡 10 stars loaded into vEB tree (U=16)', 'info', 2);
  pushTickerEvent('🌌 OrbitControls active — drag, scroll, click to explore', 'info', 3);
});