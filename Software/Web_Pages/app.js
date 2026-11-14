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
	var GRID_COLS = 4;
	var GRID_ROWS = 6;
	// the picture the lamp draws (/api/disp), asked once a second. Three failed answers in a row switch back to the measurements
	var DISP_POLL_MS = 1000;
	var DISP_FAIL_LIMIT = 3;
	// after DISP_FAIL_LIMIT failures in a row the lamp is asked this rarely, until one answer comes through
	var DISP_BACKOFF_MS = 15000;
	// the page restarts a picture when its clock and the lamp's age differ by more than this (the lamp restarted, or the page slept)
	var DISP_RESYNC_MS = 2000;
	// the longest time in a picture the lamp can send: a snake or random step at the lamp's slowest pause (65535 ms) is
	// (109 * 65535 + 100) / 24 = 297642 ms, a growing step 2 * 65535, a fading level 65935; all fit in 300 s
	var DISP_MAX_MS = 300000;
	var DISP_SOURCES = ['none', 'effect', 'meas', 'error', 'event'];
	// the error words of /api/disp; index 0 means no error
	var DISP_ERRORS = ['none', 'wifi', 'ap', 'fs', 'sensors', 'ntp'];
	var DISP_LAYOUTS = ['standard', 'multi', 'night'];
	var DISP_KEYS = ['total', 'temp', 'humid', 'eco2', 'tvoc', 'pm25'];
	// the readings the lamp averages for the "total" picture
	var TOTAL_KEYS = ['temp', 'humid', 'eco2', 'tvoc', 'pm25'];
	// colour of an unlit cell, read from --led-off of the current theme when first needed (null: read again)
	var offRgb = null;
	// the mediaquery is live: matches always tells the current setting
	var reduceMotion = window.matchMedia ? window.matchMedia('(prefers-reduced-motion: reduce)') : null;

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
			fxEffect: 'The lamp is showing an effect, not readings', fxError: 'The lamp is signalling a problem, not showing readings',
			fxConnecting: 'Starting or connecting to WiFi…', fxConnected: 'Connected to WiFi', fxLost: 'WiFi connection lost',
			fxAp: 'Access point mode: add your WiFi',
			errWifi: 'Can’t connect to WiFi', errAp: 'Can’t start the access point', errFs: 'Storage error', errSensors: 'Sensor error',
			errNtp: 'Can’t get the time',
			cta: 'Add WiFi network', noData: 'no data', close: 'Close',
			lastFail: function (reason, n, cur) {
				var why = 'no answer in time';
				if (reason === 'wrong_password') {
					why = 'the password was not accepted';
				} else if (reason === 'no_network') {
					why = 'the network was not found (the lamp works only with 2.4 GHz WiFi)';
				} else if (reason === 'save_failed') {
					why = 'the lamp could not save it';
				} else if (reason === 'scan_failed') {
					why = 'the lamp could not search for networks';
				}
				return 'Could not join “' + n + '”: ' + why + '.' + (cur ? ' The lamp is back on “' + cur + '”.' : '');
			},
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
			fxEffect: 'Лампа показывает эффект, а не показания', fxError: 'Лампа сигнализирует о проблеме, а не показывает показания',
			fxConnecting: 'Запуск или подключение к WiFi…', fxConnected: 'Подключена к WiFi', fxLost: 'Связь с WiFi потеряна',
			fxAp: 'Режим точки доступа: добавьте свою WiFi-сеть',
			errWifi: 'Не удаётся подключиться к WiFi', errAp: 'Не удаётся создать точку доступа', errFs: 'Ошибка хранилища', errSensors: 'Ошибка датчиков',
			errNtp: 'Не удаётся получить время',
			cta: 'Добавить WiFi-сеть', noData: 'нет данных', close: 'Закрыть',
			lastFail: function (reason, n, cur) {
				var why = 'нет ответа вовремя';
				if (reason === 'wrong_password') {
					why = 'пароль не принят';
				} else if (reason === 'no_network') {
					why = 'сеть не найдена (лампа работает только с WiFi 2,4 ГГц)';
				} else if (reason === 'save_failed') {
					why = 'лампа не смогла сохранить сеть';
				} else if (reason === 'scan_failed') {
					why = 'лампа не смогла искать сети';
				}
				return 'Не удалось подключиться к «' + n + '»: ' + why + '.' + (cur ? ' Лампа снова в сети «' + cur + '».' : '');
			},
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
	// disp: the picture of the lamp. pic is null until /api/disp answers (and after it failed): the matrix then shows the readings.
	// t0: page time at which the picture started, frozenAt: page time of a "hold" that stopped the animation, or null
	var disp = { pic: null, seq: -1, t0: 0, frozenAt: null, fails: 0, busy: false, timer: 0, raf: 0, sig: '', rnd: null };
	// the outcome of a network switch that failed and restarted the lamp: { ssid, reason }, or null.
	// lastAsked: /api/join has answered since the lamp last came back, so the outcome is known
	var lastJoin = null;
	var lastAsked = false;

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

	// the names come from the lamp: text only
	function renderLast(d) {
		var cur = d && d.mode === 'sta' && typeof d.ssid === 'string' ? d.ssid : '';
		byId('lastBanner').hidden = !lastJoin;
		byId('lastBannerText').textContent = lastJoin ? T.lastFail(lastJoin.reason, lastJoin.ssid, cur) : '';
	}

	function now() {
		return window.performance && performance.now ? performance.now() : Date.now();
	}

	function intIn(v, lo, hi) {
		return typeof v === 'number' && isFinite(v) && Math.floor(v) === v && v >= lo && v <= hi;
	}

	// everything below comes from the lamp: only whitelisted words, plain #rrggbb colours and range-checked numbers get through.
	// Returns { seq, age, pic } or null when the reply is not as expected. pic.kind: static, fading, snake, growing, random, meas, hold
	function parseDisp(d) {
		if (!d || typeof d !== 'object' || !intIn(d.seq, 0, 4294967295) || !intIn(d.age, 0, 2147483647) || DISP_SOURCES.indexOf(d.src) < 0) {
			return null;
		}
		// err: which error the lamp signals (only with src "error"), '' for an unknown word
		var pic = { kind: '', src: d.src, err: DISP_ERRORS.indexOf(d.err) > 0 ? d.err : '' };
		var i;
		if (d.fx === 'hold') {
			pic.kind = 'hold';
		} else if (d.fx === 'random') {
			// the lamp's step is the pause of a pixel scaled to a page cell: 0 for a pause of 0
			if (!intIn(d.step, 0, DISP_MAX_MS) || !intIn(d.end, 0, DISP_MAX_MS) || (d.dark !== undefined && !intIn(d.dark, 0, DISP_MAX_MS))) {
				return null;
			}
			pic.kind = 'random';
			pic.step = d.step;
			pic.end = d.end;
			pic.dark = d.dark === undefined ? 0 : d.dark;
		} else if (d.fx === 'snake' || d.fx === 'growing') {
			// the lamp's snake lap always has the 100 ms pause, so its step is never 0; a growing step is 0 for a pause of 0
			if (!validColor(d.color) || !intIn(d.step, d.fx === 'snake' ? 1 : 0, DISP_MAX_MS) || (d.dark !== undefined && !intIn(d.dark, 0, DISP_MAX_MS))) {
				return null;
			}
			if (d.fx === 'snake' ? !intIn(d.tail, 1, GRID_COLS * GRID_ROWS) : !intIn(d.end, 0, DISP_MAX_MS)) {
				return null;
			}
			pic.kind = d.fx;
			pic.color = d.color;
			pic.step = d.step;
			pic.dark = d.dark === undefined ? 0 : d.dark;
			pic.tail = d.fx === 'snake' ? d.tail : 0;
			pic.end = d.fx === 'growing' ? d.end : 0;
		} else if (d.fx === 'static') {
			if (!validColor(d.color)) {
				return null;
			}
			pic.kind = 'static';
			pic.color = d.color;
		} else if (d.fx === 'fading') {
			if (!validColor(d.color) || !intIn(d.from, 0, 100) || !intIn(d.to, 0, 100) || !intIn(d.steps, 1, 100) || !intIn(d.step, 1, DISP_MAX_MS) || !intIn(d.end, 0, DISP_MAX_MS)) {
				return null;
			}
			// dark: dark time at the start of the cycle, first: time of the first level; a reply without them (older firmware) has
			// no dark part and a first level as long as the others
			if ((d.dark !== undefined && !intIn(d.dark, 0, DISP_MAX_MS)) || (d.first !== undefined && !intIn(d.first, 1, DISP_MAX_MS))) {
				return null;
			}
			pic.kind = 'fading';
			pic.color = d.color;
			pic.from = d.from;
			pic.to = d.to;
			pic.steps = d.steps;
			pic.step = d.step;
			pic.end = d.end;
			pic.dark = d.dark === undefined ? 0 : d.dark;
			pic.first = d.first === undefined ? d.step : d.first;
		} else if (d.fx === 'meas') {
			var need = d.layout === 'multi' ? 3 : 1;
			if (DISP_LAYOUTS.indexOf(d.layout) < 0 || !Array.isArray(d.keys) || d.keys.length < need) {
				return null;
			}
			pic.kind = 'meas';
			pic.layout = d.layout;
			pic.keys = [];
			for (i = 0; i < need; i++) {
				if (DISP_KEYS.indexOf(d.keys[i]) < 0) {
					return null;
				}
				pic.keys.push(d.keys[i]);
			}
			// the colours the lamp drew, one per key: the grid shows them, not the current readings
			if (!Array.isArray(d.colors) || d.colors.length !== need) {
				return null;
			}
			pic.colors = [];
			for (i = 0; i < need; i++) {
				if (!validColor(d.colors[i])) {
					return null;
				}
				pic.colors.push(d.colors[i]);
			}
			pic.white = 0;
			pic.whiteColor = '';
			if (d.layout === 'night') {
				if (!intIn(d.white, 0, GRID_ROWS) || !validColor(d.whiteColor)) {
					return null;
				}
				pic.white = d.white;
				pic.whiteColor = d.whiteColor;
			}
		} else {
			return null;
		}
		return { seq: d.seq, age: d.age, pic: pic };
	}

	function hex2(v) {
		var n = Math.round(v);
		return (n < 16 ? '0' : '') + n.toString(16);
	}

	// the colour of one reading, or of the average of the readings the lamp averages for "total"; '' when there is none.
	// The average is taken over the readings that have a colour, so one failed sensor does not blank the picture
	function measColor(meas, key) {
		var keys = key === 'total' ? TOTAL_KEYS : [key];
		var sum = [0, 0, 0];
		var n = 0;
		var last = '';
		keys.forEach(function (k) {
			var m = meas[k];
			if (validMeas(m) && validColor(m.c)) {
				var v = parseInt(m.c.slice(1), 16);
				sum[0] += (v >> 16) & 255;
				sum[1] += (v >> 8) & 255;
				sum[2] += v & 255;
				n++;
				last = m.c;
			}
		});
		if (n <= 1) {
			return last;
		}
		return '#' + hex2(sum[0] / n) + hex2(sum[1] / n) + hex2(sum[2] / n);
	}

	function fillGrid(color) {
		var a = [];
		for (var i = 0; i < GRID_COLS * GRID_ROWS; i++) {
			a.push(color);
		}
		return a;
	}

	// Row 0 is the top. The lamp's strip starts at its bottom end: multi draws keys[0] nearest to it and the night bar grows from it,
	// so keys[0] takes the two bottom rows and the white bar counts rows from the bottom.
	function measFrame(pic) {
		var colors = fillGrid('');
		for (var i = 0; i < colors.length; i++) {
			var col = i % GRID_COLS;
			var fromBottom = GRID_ROWS - 1 - Math.floor(i / GRID_COLS);
			if (pic.layout === 'standard') {
				colors[i] = pic.colors[0];
			} else if (pic.layout === 'multi') {
				colors[i] = pic.colors[Math.floor(fromBottom / 2)];
			} else if (col % 2 === 0) {
				colors[i] = pic.colors[0];
			} else if (fromBottom < pic.white) {
				colors[i] = pic.whiteColor;
			}
		}
		return colors;
	}

	// the brightness the lamp shows at page time t (ms since the picture started), as a share of the full colour; 0 is dark.
	// A cycle: pic.dark of dark, the first level for pic.first, every further level for pic.step (pic.steps levels in all),
	// the last one pic.end longer. Then the cycle repeats
	function fadeAlpha(pic, t) {
		var p = t % (pic.dark + pic.first + (pic.steps - 1) * pic.step + pic.end);
		if (p < pic.dark) {
			return 0;
		}
		p -= pic.dark;
		var k = p < pic.first ? 0 : Math.min(pic.steps - 1, 1 + Math.floor((p - pic.first) / pic.step));
		return Math.round(pic.from + ((pic.to - pic.from) * k) / pic.steps) / 100;
	}

	// colour of an unlit cell in the current theme, as [r, g, b]
	function ledOff() {
		if (!offRgb) {
			var v = '';
			try {
				v = getComputedStyle(root).getPropertyValue('--led-off').trim();
			} catch (e) {}
			var n = parseInt((validColor(v) ? v : '#1f2937').slice(1), 16);
			offRgb = [(n >> 16) & 255, (n >> 8) & 255, n & 255];
		}
		return offRgb;
	}

	// the colour at brightness alpha (0..1): a lit cell fades toward the unlit one, not toward the page background
	function fadeColor(color, alpha) {
		if (alpha <= 0) {
			return '';
		}
		if (alpha >= 1) {
			return color;
		}
		var off = ledOff();
		var n = parseInt(color.slice(1), 16);
		return '#' + hex2(off[0] + (((n >> 16) & 255) - off[0]) * alpha) + hex2(off[1] + (((n >> 8) & 255) - off[1]) * alpha) + hex2(off[2] + ((n & 255) - off[2]) * alpha);
	}

	// Index in the grid (row from the top times columns, plus column) of step n of the snake's path. The lamp's strip is a column
	// serpentine that starts at the bottom: column 0 runs bottom to top, column 1 top to bottom, and so on.
	function snakeCell(n) {
		var col = Math.floor(n / GRID_ROWS);
		var r = n % GRID_ROWS;
		return (col % 2 === 0 ? GRID_ROWS - 1 - r : r) * GRID_COLS + col;
	}

	// The snake at page time t. A cycle: pic.dark of dark, then one step per cell of the path. At step k the lamp lights the cells
	// k .. k + tail - 1 of the strip (cut at its end) and puts out cell k afterwards: the full tail is there from the first step,
	// and at the end of the lap it runs off the strip and shortens. Colour '' is an unlit cell.
	function snakeFrame(pic, t) {
		var cellsNum = GRID_COLS * GRID_ROWS;
		var colors = fillGrid('');
		var p = t % (pic.dark + cellsNum * pic.step);
		if (p >= pic.dark) {
			var k = Math.min(cellsNum - 1, Math.floor((p - pic.dark) / pic.step));
			for (var n = k; n < Math.min(cellsNum, k + pic.tail); n++) {
				colors[snakeCell(n)] = pic.color;
			}
		}
		return colors;
	}

	// Growing at page time t. A cycle: pic.dark of dark, then one row per step from the top down, earlier rows stay lit, the last
	// step is pic.end longer. With step 0 the whole grid is lit at once after the dark part
	function growingFrame(pic, t) {
		var colors = fillGrid('');
		var cycle = pic.dark + GRID_ROWS * pic.step + pic.end;
		var p = cycle > 0 ? t % cycle : 0;
		if (p >= pic.dark) {
			var rows = pic.step > 0 ? Math.min(GRID_ROWS, 1 + Math.floor((p - pic.dark) / pic.step)) : GRID_ROWS;
			for (var i = 0; i < rows * GRID_COLS; i++) {
				colors[i] = pic.color;
			}
		}
		return colors;
	}

	// a random channel value like the lamp's (0..254), but the brightest channel of a colour is lifted to at least 100 so that no
	// cell looks unlit
	function randomColor() {
		var ch = [0, 0, 0].map(function () {
			return Math.floor(Math.random() * 255);
		});
		var top = Math.max(ch[0], ch[1], ch[2]);
		if (top < 100) {
			ch[Math.floor(Math.random() * 3)] = 100 + Math.floor(Math.random() * 155);
		}
		return '#' + hex2(ch[0]) + hex2(ch[1]) + hex2(ch[2]);
	}

	// the order the cells light up in and their colours for one cycle of the random effect. The page's own randomness, made once per
	// cycle and kept, so every repaint inside the cycle draws the same picture (the frame signature does not change)
	function randomCycle(n) {
		if (disp.rnd && disp.rnd.n === n) {
			return disp.rnd;
		}
		var cellsNum = GRID_COLS * GRID_ROWS;
		var order = [];
		var colors = [];
		var i;
		for (i = 0; i < cellsNum; i++) {
			order.push(i);
			colors.push(randomColor());
		}
		for (i = cellsNum - 1; i > 0; i--) {
			var j = Math.floor(Math.random() * (i + 1));
			var swap = order[i];
			order[i] = order[j];
			order[j] = swap;
		}
		disp.rnd = { n: n, order: order, colors: colors };
		return disp.rnd;
	}

	// Random at page time t. A cycle: pic.dark of dark, then one cell per step in a random order with a random colour, the full grid
	// stays for pic.end more. With step 0 the whole grid is lit at once after the dark part. full: the finished grid of the first
	// cycle, for reduced motion
	function randomFrame(pic, t, full) {
		var cellsNum = GRID_COLS * GRID_ROWS;
		var colors = fillGrid('');
		var cycle = pic.dark + cellsNum * pic.step + pic.end;
		var n = full || cycle <= 0 ? 0 : Math.floor(t / cycle);
		var p = full ? cycle : (cycle > 0 ? t % cycle : 0);
		if (p >= pic.dark) {
			var lit = pic.step > 0 ? Math.min(cellsNum, 1 + Math.floor((p - pic.dark) / pic.step)) : cellsNum;
			var rnd = randomCycle(n);
			for (var k = 0; k < lit; k++) {
				colors[rnd.order[k]] = rnd.colors[k];
			}
		}
		return colors;
	}

	// the theme changed: the colour of an unlit cell must be read again
	function themeChanged() {
		offRgb = null;
		disp.sig = '';
		paintGrid((state.data && state.data.meas) || {}, state.down);
	}

	// draws the grid; down shows it dark, as before. While a fading, snake, growing or random picture plays, requestAnimationFrame
	// calls it again, the DOM is touched only when the frame changed. With reduced motion the picture stays still, without flashing:
	// fading at its first level, snake at its first step (the tail at the start of the path), growing and random as the full grid
	function paintGrid(meas, down) {
		var pic = down ? null : disp.pic;
		var colors;
		var animated = !!pic && (pic.kind === 'fading' || pic.kind === 'snake' || pic.kind === 'growing' || pic.kind === 'random');
		var still = !!reduceMotion && reduceMotion.matches;
		var t = disp.frozenAt === null ? now() : disp.frozenAt;
		if (down) {
			colors = fillGrid('');
		} else if (pic && pic.kind === 'static') {
			colors = fillGrid(pic.color);
		} else if (pic && pic.kind === 'fading') {
			colors = fillGrid(fadeColor(pic.color, still ? pic.from / 100 : fadeAlpha(pic, t - disp.t0)));
		} else if (pic && pic.kind === 'snake') {
			colors = snakeFrame(pic, still ? pic.dark : t - disp.t0);
		} else if (pic && pic.kind === 'growing') {
			colors = still ? fillGrid(pic.color) : growingFrame(pic, t - disp.t0);
		} else if (pic && pic.kind === 'random') {
			colors = randomFrame(pic, t - disp.t0, still);
		} else if (pic && pic.kind === 'meas') {
			colors = measFrame(pic);
		} else {
			colors = fillGrid('');
			for (var i = 0; i < colors.length; i++) {
				colors[i] = measColor(meas, MATRIX_ROWS[Math.floor(i / GRID_COLS)]);
			}
		}
		var sig = colors.join();
		if (sig !== disp.sig) {
			disp.sig = sig;
			cells.forEach(function (cell, i) {
				var c = colors[i];
				cell.style.background = c;
				cell.style.boxShadow = c ? '0 0 10px ' + c + '80' : '';
			});
		}
		var animating = animated && !still && disp.frozenAt === null && !document.hidden;
		byId('matrix').classList.toggle('anim', animated);
		if (animating && !disp.raf) {
			disp.raf = requestAnimationFrame(function () {
				disp.raf = 0;
				paintGrid((state.data && state.data.meas) || {}, state.down);
			});
		}
	}

	// the legend lists readings: while the grid shows an effect or an error signal it gives a caption instead.
	// The lamp's "event" and "none" sources arrive as "hold": applyDisp keeps disp.pic then (its kind is never "hold"), so the grid
	// keeps its last frame and this caption stays.
	// An error names itself (err). An effect has no reason attached, so it is told by what the firmware does: a snake is the
	// start and the reconnect, a fading effect in access point mode is the access point, a green static colour is "connected",
	// a red one "connection lost" (the dark one between them gets no caption); anything else gets the generic text
	function fxNoteKey() {
		var p = disp.pic;
		if (state.down || !p || ['static', 'fading', 'snake', 'growing', 'random'].indexOf(p.kind) < 0) {
			return '';
		}
		if (p.src === 'error') {
			return { wifi: 'errWifi', ap: 'errAp', fs: 'errFs', sensors: 'errSensors', ntp: 'errNtp' }[p.err] || 'fxError';
		}
		if (p.src !== 'effect') {
			return '';
		}
		if (p.kind === 'snake') {
			return 'fxConnecting';
		}
		if (p.kind === 'fading' && state.data && state.data.mode === 'ap') {
			return 'fxAp';
		}
		if (p.kind === 'static') {
			var c = p.color.toLowerCase();
			return c === '#01ff00' ? 'fxConnected' : (c === '#ff0000' ? 'fxLost' : (c === '#000000' ? '' : 'fxEffect'));
		}
		return 'fxEffect';
	}

	function renderMatrix(meas, down) {
		paintGrid(meas, down);
		var note = fxNoteKey();
		byId('legend').classList.toggle('fx', !!note);
		byId('legendNote').textContent = note ? T[note] : '';
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
		renderLast(d);
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
		themeChanged();
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

	// asks once whether the last network switch failed; asked again after the lamp has been away (it restarts after a switch)
	function loadLast() {
		fetchTimed('/api/join', { cache: 'no-store' }, function (r) {
			if (!r.ok) {
				throw new Error(r.status);
			}
			return r.json();
		})
			.then(function (d) {
				var l = d && d.last;
				lastAsked = true;
				lastJoin = l && l.result === 'fail' && typeof l.ssid === 'string' && l.ssid ? { ssid: l.ssid, reason: typeof l.reason === 'string' ? l.reason : '' } : null;
				render();
			})
			.catch(function () {});
	}

	// the user has read the outcome: the lamp forgets it
	function dismissLast() {
		lastJoin = null;
		render();
		fetch('/api/join/dismiss', { method: 'POST' }).catch(function () {});
	}

	// p: parseDisp result, or null when the request or the reply failed
	function applyDisp(p) {
		if (!p) {
			disp.fails++;
			if (disp.fails >= DISP_FAIL_LIMIT) {
				disp.pic = null;
			}
			return;
		}
		disp.fails = 0;
		var t = now();
		if (p.pic.kind === 'hold') {
			// the lamp keeps its last frame: so does the grid, with the animation stopped
			if (disp.pic && disp.frozenAt === null) {
				disp.frozenAt = t;
			}
			disp.seq = p.seq;
			return;
		}
		// a new picture starts the animation in phase with the lamp; the same one keeps running on the page clock
		// unless the two clocks have drifted apart (the lamp restarted, or the page was suspended)
		if (p.seq !== disp.seq || !disp.pic || disp.frozenAt !== null || Math.abs(p.age - (t - disp.t0)) > DISP_RESYNC_MS) {
			disp.t0 = t - p.age;
			disp.frozenAt = null;
		}
		// a new picture (or another kind) starts the random effect over: it must not replay the order and colours of an earlier cycle 0
		if (p.seq !== disp.seq || !disp.pic || disp.pic.kind !== p.pic.kind) {
			disp.rnd = null;
		}
		disp.seq = p.seq;
		disp.pic = p.pic;
	}

	// a separate chain from poll(): once a second, one request at a time, stopped while the page is hidden.
	// The next request is scheduled before the page is redrawn, so a drawing error cannot stop it
	function pollDisp() {
		disp.timer = 0;
		if (document.hidden || disp.busy) {
			return;
		}
		disp.busy = true;
		fetchTimed('/api/disp', { cache: 'no-store' }, function (r) {
			if (!r.ok) {
				throw new Error(r.status);
			}
			return r.json();
		})
			.then(function (d) {
				applyDisp(parseDisp(d));
			})
			.catch(function () {
				applyDisp(null);
			})
			.then(function () {
				disp.busy = false;
				disp.timer = setTimeout(pollDisp, disp.fails >= DISP_FAIL_LIMIT ? DISP_BACKOFF_MS : DISP_POLL_MS);
				renderMatrix((state.data && state.data.meas) || {}, state.down);
			});
	}

	function onVisible() {
		if (document.hidden) {
			return;
		}
		if (!disp.busy) {
			clearTimeout(disp.timer);
			pollDisp();
		}
		paintGrid((state.data && state.data.meas) || {}, state.down);
	}

	// one request at a time: the next poll is scheduled before the page is redrawn, so a drawing error cannot stop polling
	function poll() {
		fetchTimed('/api/status', { cache: 'no-store' }, function (r) {
			if (r.ok) {
				return r.json().then(function (d) {
					if (state.down) {
						lastAsked = false;
					}
					state.data = d;
					state.down = false;
					state.err = '';
					state.lastOk = clock();
					if (!lastAsked) {
						loadLast();
					}
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
	byId('lastClose').addEventListener('click', dismissLast);
	Array.prototype.forEach.call(document.querySelectorAll('[data-lang]'), function (b) {
		b.addEventListener('click', function () {
			setLang(b.getAttribute('data-lang'));
		});
	});
	document.addEventListener('visibilitychange', onVisible);
	// the setting can change while the page is open: repaint at once, the frame set is different
	if (reduceMotion) {
		if (reduceMotion.addEventListener) {
			reduceMotion.addEventListener('change', themeChanged);
		} else if (reduceMotion.addListener) {
			reduceMotion.addListener(themeChanged);
		}
	}
	if (window.matchMedia && window.matchMedia('(prefers-color-scheme: dark)').addEventListener) {
		window.matchMedia('(prefers-color-scheme: dark)').addEventListener('change', themeChanged);
	}
	setLang(saved);
	poll();
	pollDisp();
})();
