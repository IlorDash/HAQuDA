(function () {
	'use strict';

	var POLL_MS = 5000;
	var FETCH_TIMEOUT_MS = 4000;
	var KEYS = ['temp', 'humid', 'eco2', 'tvoc', 'pm1', 'pm25', 'pm10'];
	// PM1.0 and PM10 are not in /api/status of the current firmware: their tiles appear only when the field is present
	var OPTIONAL = { pm1: true, pm10: true };
	var UNITS = { temp: '°C', humid: '%', eco2: 'ppm', tvoc: 'ppb', pm1: 'µg/m³', pm25: 'µg/m³', pm10: 'µg/m³' };
	// matrix rows from the top: the lamp shows temperature at the bottom, then eCO2, then dust
	var MATRIX_ROWS = ['pm25', 'pm25', 'eco2', 'eco2', 'temp', 'temp'];
	var LEGEND_KEYS = ['pm25', 'eco2', 'temp'];

	var TEXT = {
		en: {
			good: 'Good air', fair: 'Moderate', poor: 'Poor air', none: 'No data',
			allOk: 'All readings are normal', worst: 'Worst: ', last: 'Last values from ', sensorFail: 'Sensor readings failed: ',
			connecting: 'Connecting to the lamp…', sta: 'Connected', dbm: 'dBm',
			staTo: function (n) {
				return 'Connected to ' + n;
			},
			ap: 'Access point · no internet', off: 'WiFi connection lost', down: 'Lamp not responding · retry in 5 s',
			bAp: 'The lamp couldn’t find a saved network and started its own. Add your home WiFi.',
			bDown: 'No response from the lamp. It may be rebooting, switched off, or your phone changed networks. Showing the last values; data returns on its own once the lamp answers.',
			cta: 'Add WiFi network', noData: 'no data',
			labels: { temp: 'Temperature', humid: 'Humidity', eco2: 'eCO₂', tvoc: 'TVOC', pm1: 'PM1.0', pm25: 'PM2.5', pm10: 'PM10' },
			lDust: 'Dust PM2.5', lTemp: 'Temperature',
			wifi: 'WiFi networks', log: 'Live log', guide: 'User guide ↗', offNote: 'needs internet',
			fw: 'Firmware', up: 'Uptime', d: 'd', h: 'h', m: 'm'
		},
		ru: {
			good: 'Воздух хороший', fair: 'Умеренно', poor: 'Воздух плохой', none: 'Нет данных',
			allOk: 'Все показатели в норме', worst: 'Хуже всего: ', last: 'Последние значения в ', sensorFail: 'Нет данных датчиков: ',
			connecting: 'Подключаемся к лампе…', sta: 'Подключена', dbm: 'дБм',
			staTo: function (n) {
				return 'Подключена к ' + n;
			},
			ap: 'Точка доступа · без интернета', off: 'Нет связи с WiFi', down: 'Лампа не отвечает · повтор через 5 с',
			bAp: 'Лампа не нашла сохранённую сеть и создала свою. Добавьте домашнюю сеть.',
			bDown: 'Нет ответа от лампы. Возможно, она перезагружается, выключена или телефон сменил сеть. Показаны последние значения; данные вернутся сами, как только лампа ответит.',
			cta: 'Добавить WiFi-сеть', noData: 'нет данных',
			labels: { temp: 'Температура', humid: 'Влажность', eco2: 'eCO₂', tvoc: 'Летучие вещества', pm1: 'PM1.0', pm25: 'Частицы PM2.5', pm10: 'PM10' },
			lDust: 'Пыль PM2.5', lTemp: 'Температура',
			wifi: 'WiFi-сети', log: 'Живой лог', guide: 'Руководство ↗', offNote: 'нужен интернет',
			fw: 'Прошивка', up: 'Работает', d: 'д', h: 'ч', m: 'мин'
		}
	};

	var root = document.documentElement;
	var lang = 'en';
	var T = TEXT.en;
	// data: the last good /api/status reply; down: the last poll failed; err: reply text of a non-2xx answer
	var state = { data: null, down: false, err: '', lastOk: '' };
	var cells = [];
	var tileEls = {};

	function byId(id) {
		return document.getElementById(id);
	}

	function el(tag, cls, text) {
		var e = document.createElement(tag);
		if (cls) {
			e.className = cls;
		}
		if (text !== undefined) {
			e.textContent = text;
		}
		return e;
	}

	// the captive portal browser may refuse storage: the choice then lasts until the page closes
	function store(key, value) {
		try {
			localStorage.setItem(key, value);
		} catch (e) {}
	}

	function load(key) {
		try {
			return localStorage.getItem(key);
		} catch (e) {
			return null;
		}
	}

	function pad2(n) {
		return (n < 10 ? '0' : '') + n;
	}

	function clock() {
		var n = new Date();
		return pad2(n.getHours()) + ':' + pad2(n.getMinutes());
	}

	function uptimeText(s) {
		return Math.floor(s / 86400) + T.d + ' ' + Math.floor((s % 86400) / 3600) + T.h + ' ' + pad2(Math.floor((s % 3600) / 60)) + T.m;
	}

	function validMeas(m) {
		return !!m && typeof m.v === 'number' && isFinite(m.v);
	}

	// colours come from the lamp: only a plain #rrggbb reaches a style
	function validColor(c) {
		return typeof c === 'string' && /^#[0-9a-f]{6}$/i.test(c);
	}

	function fmt(k, v) {
		var s = (k === 'temp' || k === 'humid') ? v.toFixed(1) : String(Math.round(v));
		return lang === 'ru' ? s.replace('.', ',') : s;
	}

	function hue(hex) {
		var n = parseInt(hex.slice(1), 16);
		var r = ((n >> 16) & 255) / 255;
		var g = ((n >> 8) & 255) / 255;
		var b = (n & 255) / 255;
		var mx = Math.max(r, g, b);
		var d = mx - Math.min(r, g, b);
		if (!d) {
			return 120;
		}
		var h = (mx === r ? ((g - b) / d) % 6 : (mx === g ? (b - r) / d + 2 : (r - g) / d + 4)) * 60;
		return h < 0 ? h + 360 : h;
	}

	// 0 normal (green to blue), 1 moderate (yellow), 2 bad (orange to red)
	function rank(hex) {
		var h = hue(hex);
		return h >= 75 && h <= 260 ? 0 : (h >= 35 && h < 75 ? 1 : 2);
	}

	function buildMatrix() {
		var box = byId('matrix');
		MATRIX_ROWS.forEach(function () {
			for (var c = 0; c < 4; c++) {
				var i = el('i');
				box.appendChild(i);
				cells.push(i);
			}
		});
	}

	function buildTiles() {
		var box = byId('tiles');
		KEYS.forEach(function (k) {
			var tile = el('div', 'tile');
			var tl = el('div', 'tl');
			var sq = el('i', 'sq');
			var lbl = el('span');
			var tv = el('div', 'tv');
			var val = el('span', 'val', '--');
			var unit = el('span', 'unit', UNITS[k]);
			tl.appendChild(sq);
			tl.appendChild(lbl);
			tv.appendChild(val);
			tv.appendChild(unit);
			tile.appendChild(tl);
			tile.appendChild(tv);
			box.appendChild(tile);
			tileEls[k] = { box: tile, sq: sq, lbl: lbl, val: val, unit: unit };
		});
	}

	function renderHeader(d, down) {
		var dot = '';
		var text = T.connecting;
		var level = 0;
		if (down) {
			dot = 'bad';
			text = T.down;
		} else if (d) {
			if (d.mode === 'ap') {
				dot = 'warn';
				text = T.ap;
			} else if (d.rssi === null) {
				dot = 'bad';
				text = T.off;
			} else {
				dot = 'ok';
				// the network name comes from the lamp: text only
				text = (typeof d.ssid === 'string' && d.ssid ? T.staTo(d.ssid) : T.sta) + ' · ' + d.rssi + ' ' + T.dbm;
				level = d.rssi > -55 ? 4 : (d.rssi > -65 ? 3 : (d.rssi > -75 ? 2 : 1));
			}
		}
		byId('sdot').className = 'sdot ' + dot;
		byId('stext').textContent = text;
		Array.prototype.forEach.call(byId('bars').children, function (bar, i) {
			bar.className = i < level ? 'on' : '';
		});
	}

	function renderBanner(d, down) {
		var text = '';
		var cta = false;
		if (down) {
			// a non-2xx answer carries the lamp's own text, shown as it is
			text = state.err || T.bDown;
		} else if (d && d.mode === 'ap') {
			text = T.bAp;
			cta = true;
		}
		byId('banner').hidden = !text;
		byId('bannerText').textContent = text;
		byId('bannerCta').hidden = !cta;
	}

	function renderMatrix(meas, down) {
		cells.forEach(function (cell, i) {
			var m = meas[MATRIX_ROWS[Math.floor(i / 4)]];
			if (!down && validMeas(m) && validColor(m.c)) {
				cell.style.background = m.c;
				cell.style.boxShadow = '0 0 10px ' + m.c + '80';
			} else {
				cell.style.background = '';
				cell.style.boxShadow = '';
			}
		});
		LEGEND_KEYS.forEach(function (k) {
			var m = meas[k];
			byId('lv-' + k).textContent = validMeas(m) ? fmt(k, m.v) + ' ' + UNITS[k] : (state.data ? T.noData : '--');
		});
	}

	function renderIndex(d, meas, down) {
		var word = '—';
		var sub = '';
		var worst = null;
		var worstRank = -1;
		if (down) {
			sub = d ? T.last + state.lastOk : '';
		} else if (d) {
			var failed = [];
			KEYS.forEach(function (k) {
				var m = meas[k];
				// PM1.0 and PM10 count only when the lamp sends them
				if (OPTIONAL[k] && !meas.hasOwnProperty(k)) {
					return;
				}
				if (!validMeas(m)) {
					failed.push(T.labels[k]);
				}
				if (validMeas(m) && validColor(m.c) && rank(m.c) > worstRank) {
					worst = k;
					worstRank = rank(m.c);
				}
			});
			word = worst ? [T.good, T.fair, T.poor][worstRank] : T.none;
			// a failed sensor must not be reported as "all normal": the failure line wins over the worst-reading line
			if (failed.length) {
				sub = T.sensorFail + failed.join(', ');
			} else {
				sub = worstRank === 0 ? T.allOk : (worst ? T.worst + T.labels[worst] : '');
			}
		}
		byId('idxWord').textContent = word;
		byId('idxSub').textContent = sub;
	}

	function renderTiles(meas) {
		var shown = KEYS.filter(function (k) {
			return !OPTIONAL[k] || meas.hasOwnProperty(k);
		});
		var pmCount = shown.filter(function (k) {
			return k.indexOf('pm') === 0;
		}).length;
		KEYS.forEach(function (k) {
			var t = tileEls[k];
			var m = meas[k];
			var ok = validMeas(m);
			var span = k.indexOf('pm') === 0 ? 6 / pmCount : 3;
			t.box.hidden = shown.indexOf(k) < 0;
			t.box.style.gridColumn = 'span ' + span;
			t.box.className = 'tile' + (span <= 2 ? ' t2' : '') + (ok ? '' : ' nodata');
			t.lbl.textContent = T.labels[k];
			t.sq.style.background = ok && validColor(m.c) ? m.c : '';
			// before the first reply the tiles show a placeholder, not "no data"
			t.val.textContent = ok ? fmt(k, m.v) : (state.data ? T.noData : '--');
			t.unit.textContent = ok ? UNITS[k] : '';
		});
	}

	function renderMeta(d) {
		byId('mfw').textContent = d ? T.fw + ' ' + d.fw : '';
		byId('mip').textContent = d ? 'IP ' + d.ip : '';
		byId('mup').textContent = d ? T.up + ' ' + uptimeText(d.uptime) : '';
	}

	function render() {
		var d = state.data;
		var meas = (d && d.meas) || {};
		renderHeader(d, state.down);
		renderBanner(d, state.down);
		renderMatrix(meas, state.down);
		renderIndex(d, meas, state.down);
		renderTiles(meas);
		renderMeta(d);
		byId('lamp').classList.toggle('dim', state.down);
		byId('tiles').classList.toggle('dim', state.down);
		document.body.classList.toggle('ap', !!d && d.mode === 'ap');
	}

	function setLang(code) {
		lang = code;
		T = TEXT[code];
		store('lang', code);
		root.lang = code;
		Array.prototype.forEach.call(document.querySelectorAll('[data-i]'), function (e) {
			e.textContent = T[e.getAttribute('data-i')];
		});
		Array.prototype.forEach.call(document.querySelectorAll('[data-lang]'), function (b) {
			b.setAttribute('aria-pressed', b.getAttribute('data-lang') === code ? 'true' : 'false');
		});
		render();
	}

	function toggleTheme() {
		var cur = root.getAttribute('data-theme');
		if (!cur) {
			cur = window.matchMedia && window.matchMedia('(prefers-color-scheme: dark)').matches ? 'dark' : 'light';
		}
		var next = cur === 'dark' ? 'light' : 'dark';
		root.setAttribute('data-theme', next);
		store('theme', next);
	}

	// a hung request would stop the 5 s chain: abort it after FETCH_TIMEOUT_MS where the browser can.
	// handle(response) reads the body; the timer runs until that read is done or the request failed
	function fetchTimed(url, opts, handle) {
		var timer = null;
		function stop() {
			clearTimeout(timer);
		}
		if (window.AbortController) {
			var ctl = new AbortController();
			opts.signal = ctl.signal;
			timer = setTimeout(function () {
				ctl.abort();
			}, FETCH_TIMEOUT_MS);
		}
		return fetch(url, opts).then(handle).then(function (v) {
			stop();
			return v;
		}, function (e) {
			stop();
			throw e;
		});
	}

	// one request at a time: the next poll is scheduled before the page is redrawn, so a drawing error cannot stop polling
	function poll() {
		fetchTimed('/api/status', { cache: 'no-store' }, function (r) {
			if (r.ok) {
				return r.json().then(function (d) {
					state.data = d;
					state.down = false;
					state.err = '';
					state.lastOk = clock();
				});
			}
			return r.text().then(function (t) {
				state.down = true;
				state.err = t;
			});
		})
			.catch(function () {
				state.down = true;
				state.err = '';
			})
			.then(function () {
				setTimeout(poll, POLL_MS);
				render();
			});
	}

	var saved = load('lang');
	if (saved !== 'en' && saved !== 'ru') {
		saved = /^ru/i.test(navigator.language || '') ? 'ru' : 'en';
	}

	buildMatrix();
	buildTiles();
	byId('theme').addEventListener('click', toggleTheme);
	Array.prototype.forEach.call(document.querySelectorAll('[data-lang]'), function (b) {
		b.addEventListener('click', function () {
			setLang(b.getAttribute('data-lang'));
		});
	});
	setLang(saved);
	poll();
})();
