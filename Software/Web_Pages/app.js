(function () {
	'use strict';

	var POLL_MS = 5000;
	var FETCH_TIMEOUT_MS = 4000;
	var KEYS = ['temp', 'humid', 'eco2', 'tvoc', 'pm1', 'pm25', 'pm10'];
	// PM1.0 and PM10 are not in /api/status of the current firmware: their tiles appear only when the field is present
	var OPTIONAL = { pm1: true, pm10: true };
	var UNITS = { temp: '°C', humid: '%', eco2: 'ppm', tvoc: 'ppb', pm1: 'µg/m³', pm25: 'µg/m³', pm10: 'µg/m³' };
	var LEGEND_KEYS = ['pm25', 'eco2', 'temp'];
	var GRID_COLS = 4;
	var GRID_ROWS = 6;
	var LAMP_COLUMNS = 9;
	var LAMP_ROWS = 12;
	var LAMP_PIXELS = LAMP_COLUMNS * LAMP_ROWS;
	// the lamp card: what the lamp can show and which keys it takes (the readings of /api/status, "total" for standard only)
	var LAMP_MODES = ['standard', 'multi', 'night', 'light'];
	var STD_KEYS = ['total', 'temp', 'humid', 'eco2', 'tvoc', 'pm25'];
	var SEL_KEYS = STD_KEYS.slice(1);
	// the lamp light colours, the index is what POST /api/lamp takes
	var LAMP_COLORS = ['#ffcf8f', '#fff1dc', '#dce9ff', '#ff9f43', '#8f8cff', '#36c9b8'];
	var NIGHT_CELLS = 18;
	// the picture the lamp draws (/api/disp), asked once a second. Three failed answers in a row switch back to the measurements
	var DISP_POLL_MS = 1000;
	var DISP_FAIL_LIMIT = 3;
	// after DISP_FAIL_LIMIT failures in a row the lamp is asked this rarely, until one answer comes through
	var DISP_BACKOFF_MS = 15000;
	// the page restarts a picture when its clock and the lamp's age differ by more than this (the lamp restarted, or the page slept)
	var DISP_RESYNC_MS = 2000;
	// the longest time in a picture the lamp can send: a snake or random step at the lamp's slowest pause (65535 ms) is
	// (108 * 65535 + 100) / 24 = 294912 ms, a growing step 2 * 65535, a fading level 65935; all fit in 300 s
	var DISP_MAX_MS = 300000;
	var DISP_SOURCES = ['none', 'effect', 'meas', 'error', 'event'];
	// the error words of /api/disp; index 0 means no error
	var DISP_ERRORS = ['none', 'wifi', 'ap', 'fs', 'sensors', 'ntp'];
	var DISP_LAYOUTS = ['standard', 'multi', 'night'];
	var DISP_KEYS = ['total', 'temp', 'humid', 'eco2', 'tvoc', 'pm25'];
	// the readings of the "total" picture, in the order the lamp checks them, with the range each one must stay in: out of range is
	// at or below the first number or at or above the second. The numbers are the lamp's fixed thresholds, the ends of its colour
	// scale (/api/status does not send them), so they must follow the firmware if it ever changes them
	var TOTAL_KEYS = ['temp', 'humid', 'eco2', 'tvoc', 'pm25'];
	var TOTAL_LIMITS = { temp: [20, 30], humid: [40, 80], eco2: [-Infinity, 5000], tvoc: [-Infinity, 1000], pm25: [-Infinity, 45] };
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
			lcTitle: 'Lamp', lcOff: 'Off', lcPower: 'Power', lcBright: 'Brightness', lcRead: 'Reading', lcRows: ['Top', 'Middle', 'Bottom'],
			lcTotal: 'Overall', lcFrom: 'Night light from', lcTo: 'to', lcColor: 'Color', lcNight: 'Night light', lcLamp: 'Lamp light',
			lcOffLegend: 'Lamp is off', lcNeeds: 'needs internet',
			lcHint: 'The night light fills up from the bottom during the night: the fill shows how much sleep is left.',
			lcLive: 'Changes apply instantly', lcLock: 'Controls are unavailable while the lamp isn’t responding',
			lcFail: 'The lamp did not accept the change.', lcNoAnswer: 'No answer from the lamp.',
			modes: { standard: ['Standard', 'One reading'], multi: ['Multi', 'Three readings'], night: ['Night', 'Reading + night light'], light: ['Lamp light', 'Solid color'] },
			colors: ['Warm white', 'Neutral', 'Cool white', 'Amber', 'Lavender', 'Teal'],
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
			lcTitle: 'Лампа', lcOff: 'Выключена', lcPower: 'Питание', lcBright: 'Яркость', lcRead: 'Показатель', lcRows: ['Верх', 'Середина', 'Низ'],
			lcTotal: 'Общий', lcFrom: 'Ночник с', lcTo: 'до', lcColor: 'Цвет', lcNight: 'Ночник', lcLamp: 'Светильник',
			lcOffLegend: 'Лампа выключена', lcNeeds: 'нужен интернет',
			lcHint: 'Ночник заполняется снизу вверх в течение ночи: по заполнению видно, сколько осталось спать.',
			lcLive: 'Изменения применяются сразу', lcLock: 'Управление недоступно, пока лампа не отвечает',
			lcFail: 'Лампа не приняла изменение.', lcNoAnswer: 'Лампа не ответила.',
			modes: { standard: ['Стандартный', 'Один показатель'], multi: ['Мульти', 'Три показателя'], night: ['Ночной', 'Показатель + ночник'], light: ['Светильник', 'Один цвет'] },
			colors: ['Тёплый белый', 'Нейтральный', 'Холодный', 'Янтарный', 'Лавандовый', 'Бирюзовый'],
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
	var disp = { pic: null, seq: -1, t0: 0, frozenAt: null, fails: 0, busy: false, timer: 0, raf: 0, sig: '', rnd: null, bright: 100 };
	// the outcome of a network switch that failed and restarted the lamp: { ssid, reason }, or null.
	// lastAsked: /api/join has answered since the lamp last came back, so the outcome is known
	var lastJoin = null;
	var lastAsked = false;
	// the lamp card. conf: the lamp as the lamp last confirmed it (null: no card). want: conf plus the edits not yet confirmed, pend:
	// those edits as a partial lamp still to send, busy: a POST is out, live: the brightness while the slider is dragged, err: the lamp's reply to a refused POST
	// gen: counts the confirmed POSTs, so a status answer that was asked before the last confirmation can be told from a current one
	var lc = { conf: null, want: null, pend: null, busy: false, live: null, err: '', timer: 0, gen: 0 };

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
		for (var r = 0; r < GRID_ROWS; r++) {
			for (var c = 0; c < GRID_COLS; c++) {
				var i = el('i');
				box.appendChild(i);
				cells.push(i);
			}
		}
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
			if (!intIn(d.seed, 0, 4294967295)) {
				return null;
			}
			pic.kind = 'random';
			pic.seed = d.seed;
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
			pic.lit = 0;
			pic.whiteColor = '';
			if (d.layout === 'night') {
				if (!intIn(d.lit, 0, NIGHT_CELLS) || !validColor(d.whiteColor)) {
					return null;
				}
				pic.lit = d.lit;
				pic.whiteColor = d.whiteColor;
			}
		} else {
			return null;
		}
		if (d.bright !== undefined && !intIn(d.bright, 0, 100)) {
			return null;
		}
		return { seq: d.seq, age: d.age, bright: d.bright === undefined ? 100 : d.bright, pic: pic };
	}

	function hex2(v) {
		var n = Math.round(v);
		return (n < 16 ? '0' : '') + n.toString(16);
	}

	// the colour the lamp shows for "total", as the firmware picks it: the first reading (temp, humid, eco2, tvoc, pm25) out of its
	// range shows its own colour, otherwise the colours of the readings that have a value are averaged channel by channel
	// (integer division). '' when no reading has one. A failed sensor does not blank it
	function totalColor(meas) {
		for (var i = 0; i < TOTAL_KEYS.length; i++) {
			var m = meas[TOTAL_KEYS[i]];
			var lim = TOTAL_LIMITS[TOTAL_KEYS[i]];
			if (validMeas(m) && (m.v <= lim[0] || m.v >= lim[1])) {
				return validColor(m.c) ? m.c : '';
			}
		}
		var sum = [0, 0, 0];
		var n = 0;
		TOTAL_KEYS.forEach(function (k) {
			var m = meas[k];
			if (validMeas(m) && validColor(m.c)) {
				var rgb = parseInt(m.c.slice(1), 16);
				sum[0] += (rgb >> 16) & 255;
				sum[1] += (rgb >> 8) & 255;
				sum[2] += rgb & 255;
				n++;
			}
		});
		return n ? '#' + hex2(Math.floor(sum[0] / n)) + hex2(Math.floor(sum[1] / n)) + hex2(Math.floor(sum[2] / n)) : '';
	}

	// the colour of one reading, or the one the lamp shows for "total"; '' when there is none
	function measColor(meas, key) {
		if (key === 'total') {
			return totalColor(meas);
		}
		var m = meas[key];
		return validMeas(m) && validColor(m.c) ? m.c : '';
	}

	function fillGrid(color) {
		var a = [];
		for (var i = 0; i < GRID_COLS * GRID_ROWS; i++) {
			a.push(color);
		}
		return a;
	}

	// Row 0 is the top. The lamp's strip starts at its bottom end: multi draws keys[0] nearest to it, so keys[0] takes the two
	// bottom rows. Night: the left column is the reading, the other 18 cells the night light, a stripe (row) of three at a time from the bottom
	function measFrame(pic) {
		var colors = fillGrid('');
		for (var i = 0; i < colors.length; i++) {
			var col = i % GRID_COLS;
			var fromBottom = GRID_ROWS - 1 - Math.floor(i / GRID_COLS);
			if (pic.layout === 'standard') {
				colors[i] = pic.colors[0];
			} else if (pic.layout === 'multi') {
				colors[i] = pic.colors[Math.floor(fromBottom / 2)];
			} else if (col === 0) {
				colors[i] = pic.colors[0];
			} else if (fromBottom * 3 + col - 1 < pic.lit) {
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

	// the lamp's generator (HAQuDA_XorShift32): xorshift32 with the shifts 13, 17, 5 on an unsigned 32-bit state
	function xorshift32(s) {
		var x = s >>> 0;
		x = (x ^ (x << 13)) >>> 0;
		x = (x ^ (x >>> 17)) >>> 0;
		x = (x ^ (x << 5)) >>> 0;
		return x;
	}

	// the grid cell a pixel of the lamp falls into. The lamp's strip is a column serpentine from the bottom (odd columns run top to
	// bottom), and its 9 x 12 pixels are spread over the 4 x 6 cells: a cell takes 2 pixel rows and 2 or 3 pixel columns
	function lampPixelCell(p) {
		var x = Math.floor(p / LAMP_ROWS);
		var r = p % LAMP_ROWS;
		var y = x % 2 === 1 ? LAMP_ROWS - 1 - r : r;
		return (GRID_ROWS - 1 - Math.floor((y * GRID_ROWS) / LAMP_ROWS)) * GRID_COLS + Math.floor((x * GRID_COLS) / LAMP_COLUMNS);
	}

	// the pixel order and colours of pass n as the lamp makes them (HAQuDA_RandomPixelsAnim): the pass seed is the seed of pass 0
	// run through xorshift32 n times (0 counts as 1), the order is a Fisher-Yates shuffle from the top, then three channels per pixel
	// (0..254), painted as they are. The page cannot show 108 pixels in 24 cells exactly: a cell takes the colour of the first
	// pixel lit in it and keeps it until the pass ends, as a lit pixel does on the lamp. The pass is kept, so every repaint inside
	// it draws the same picture
	function randomPass(seed, n) {
		var pass = disp.rnd;
		var i;
		if (!pass || pass.n > n) {
			pass = { n: 0, seed: seed !== 0 ? seed >>> 0 : 1 };
		}
		var s = pass.seed;
		for (i = pass.n; i < n; i++) {
			s = xorshift32(s);
		}
		if (disp.rnd && disp.rnd.n === n && disp.rnd.seed === s) {
			return disp.rnd;
		}
		var state = s;
		var order = [];
		var colors = [];
		for (i = 0; i < LAMP_PIXELS; i++) {
			order.push(i);
		}
		for (i = LAMP_PIXELS - 1; i >= 1; i--) {
			state = xorshift32(state);
			var j = state % (i + 1);
			var swap = order[i];
			order[i] = order[j];
			order[j] = swap;
		}
		for (i = 0; i < LAMP_PIXELS; i++) {
			var ch = [0, 0, 0];
			for (var c = 0; c < 3; c++) {
				state = xorshift32(state);
				ch[c] = state % 255;
			}
			colors.push('#' + hex2(ch[0]) + hex2(ch[1]) + hex2(ch[2]));
		}
		disp.rnd = { n: n, seed: s, order: order, colors: colors };
		return disp.rnd;
	}

	// Random at page time t. A cycle: pic.dark of dark, then the lamp's pixels one after the other in the order of the pass, the
	// full picture stays for pic.end more. The page step is one cell of 24, so a pixel is 24 / 108 of it. With step 0 every pixel is
	// lit at once after the dark part. full: the finished picture of the first pass, for reduced motion
	function randomFrame(pic, t, full) {
		var colors = fillGrid('');
		var cycle = pic.dark + GRID_COLS * GRID_ROWS * pic.step + pic.end;
		// the page cycle uses the rounded 24 * step, so over many passes its pass number can drift by one from the lamp's until the
		// resync (DISP_RESYNC_MS) restarts the picture
		var n = full || cycle <= 0 ? 0 : Math.floor(t / cycle);
		var p = full ? cycle : (cycle > 0 ? t % cycle : 0);
		if (p >= pic.dark) {
			var lit = pic.step > 0 ? Math.min(LAMP_PIXELS, 1 + Math.floor(((p - pic.dark) * LAMP_PIXELS) / (GRID_COLS * GRID_ROWS * pic.step))) : LAMP_PIXELS;
			var pass = randomPass(pic.seed, n);
			for (var k = 0; k < lit; k++) {
				var cell = lampPixelCell(pass.order[k]);
				if (colors[cell] === '') {
					colors[cell] = pass.colors[k];
				}
			}
		}
		return colors;
	}

	// the theme changed: the colour of an unlit cell must be read again
	function themeChanged() {
		offRgb = null;
		disp.sig = '';
		paintGrid(state.down);
	}

	// draws the grid from the lamp's picture only (/api/disp); no picture or the lamp down shows it dark. While a fading, snake,
	// growing or random picture plays, requestAnimationFrame calls it again, the DOM is touched only when the frame changed. With
	// reduced motion the picture stays still, without flashing: fading at its first level, snake at its first step (the tail at the
	// start of the path), growing and random as the full grid
	function paintGrid(down) {
		var pic = down ? null : disp.pic;
		var colors;
		var animated = !!pic && (pic.kind === 'fading' || pic.kind === 'snake' || pic.kind === 'growing' || pic.kind === 'random');
		var still = !!reduceMotion && reduceMotion.matches;
		var t = disp.frozenAt === null ? now() : disp.frozenAt;
		if (!pic) {
			colors = fillGrid('');
		} else if (pic.kind === 'static') {
			colors = fillGrid(pic.color);
		} else if (pic.kind === 'fading') {
			colors = fillGrid(fadeColor(pic.color, still ? pic.from / 100 : fadeAlpha(pic, t - disp.t0)));
		} else if (pic.kind === 'snake') {
			colors = snakeFrame(pic, still ? pic.dark : t - disp.t0);
		} else if (pic.kind === 'growing') {
			colors = still ? fillGrid(pic.color) : growingFrame(pic, t - disp.t0);
		} else if (pic.kind === 'random') {
			colors = randomFrame(pic, t - disp.t0, still);
		} else {
			colors = measFrame(pic);
		}
		// brightness changes alone do not raise seq; the faded colours are in the signature, so a change repaints. Black stays black
		// (the lamp is off), an unlit cell stays unlit. A fading picture is skipped: the lamp's fade sets its own brightness levels
		// and replaces the user's brightness while it plays
		if (pic && pic.kind !== 'fading' && disp.bright < 100) {
			var fade = 0.35 + (0.65 * disp.bright) / 100;
			colors = colors.map(function (c) {
				return c && c.toLowerCase() !== '#000000' ? fadeColor(c, fade) : c;
			});
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
				paintGrid(state.down);
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

	// the value of one reading for the legend; "total" is the air index word of the colour the lamp shows for it
	function readingText(meas, k) {
		var m = meas[k];
		if (k === 'total') {
			var c = totalColor(meas);
			return c ? [T.good, T.fair, T.poor][rank(c)] : T.noData;
		}
		return validMeas(m) ? fmt(k, m.v) + ' ' + UNITS[k] : (state.data ? T.noData : '--');
	}

	function readingName(k) {
		return k === 'total' ? T.lcTotal : T.labels[k];
	}

	// what the legend lists: the three readings of the default picture, or those of the lamp's mode while the card draws the matrix
	function legendRows(meas, model) {
		var v = model && view();
		if (!v) {
			return LEGEND_KEYS.map(function (k) {
				return [k === 'pm25' ? T.lDust : readingName(k), readingText(meas, k)];
			});
		}
		if (!v.on) {
			return [[T.lcOffLegend, '—']];
		}
		if (v.mode === 'multi') {
			return v.multi.map(function (k) {
				return [readingName(k), readingText(meas, k)];
			});
		}
		if (v.mode === 'standard') {
			return [[readingName(v.std), readingText(meas, v.std)]];
		}
		if (v.mode === 'night') {
			return [[readingName(v.night.param), readingText(meas, v.night.param)], [T.lcNight, v.night.from + '–' + v.night.to]];
		}
		return [[T.lcLamp, T.colors[v.color]]];
	}

	function renderMatrix(meas, down) {
		paintGrid(down);
		var model = !down && lampModel();
		var note = model ? '' : fxNoteKey();
		var rows = legendRows(meas, model);
		byId('legend').classList.toggle('fx', !!note);
		// the three-row legend lines up with the pairs of grid rows, a shorter one hugs its text
		byId('legend').classList.toggle('auto', rows.length < 3);
		byId('legendNote').textContent = note ? T[note] : '';
		Array.prototype.forEach.call(byId('legend').getElementsByTagName('div'), function (row, i) {
			row.hidden = i >= rows.length;
			if (rows[i]) {
				row.children[0].textContent = rows[i][0];
				row.children[1].textContent = rows[i][1];
			}
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

	// ---- lamp card: power, brightness, modes. The page edits a copy of the lamp's settings; the grid waits for the lamp's picture

	function hhmm(s) {
		return typeof s === 'string' && /^([01]\d|2[0-3]):[0-5]\d$/.test(s);
	}

	// the "lamp" block of /api/status or of a POST reply, or null when it is not as expected
	function parseLamp(l) {
		var n = l && l.night;
		var ok =
			!!n &&
			typeof l.on === 'boolean' &&
			intIn(l.bright, 5, 100) &&
			LAMP_MODES.indexOf(l.mode) >= 0 &&
			STD_KEYS.indexOf(l.std) >= 0 &&
			intIn(l.color, 0, LAMP_COLORS.length - 1) &&
			SEL_KEYS.indexOf(n.param) >= 0 &&
			hhmm(n.from) &&
			hhmm(n.to) &&
			Array.isArray(l.multi) &&
			l.multi.length === 3 &&
			l.multi.every(function (k) {
				return SEL_KEYS.indexOf(k) >= 0;
			});
		return ok
			? {
					on: l.on,
					bright: l.bright,
					mode: l.mode,
					std: l.std,
					multi: l.multi.slice(),
					night: { param: n.param, from: n.from, to: n.to },
					color: l.color,
					time: l.time === true,
					now: l.time === true && hhmm(l.now) ? l.now : ''
				}
			: null;
	}

	// b over a; the night fields merge one by one
	function merge(a, b) {
		var r = {};
		[a, b].forEach(function (o) {
			Object.keys(o).forEach(function (k) {
				r[k] = o[k];
			});
		});
		if (a.night && b.night) {
			r.night = { param: b.night.param || a.night.param, from: b.night.from || a.night.from, to: b.night.to || a.night.to };
		}
		return r;
	}

	// the lamp the card shows: the edited copy, with the brightness the slider is dragged to
	function view() {
		var v = lc.want || lc.conf;
		return v && lc.live !== null ? merge(v, { bright: lc.live }) : v;
	}

	// The legend lists the readings of the edited settings while an edit is not confirmed yet and while the lamp shows the user's
	// picture (measurements, the lamp light colour or dark). The grid never follows this: it shows what /api/disp sends.
	function lampModel() {
		var v = view();
		var p = disp.pic;
		if (!v) {
			return false;
		}
		if (lc.want || lc.live !== null || !p || p.src === 'meas') {
			return true;
		}
		if (p.kind !== 'static' || p.src !== 'effect') {
			return false;
		}
		var c = p.color.toLowerCase();
		return v.on ? v.mode === 'light' && c === LAMP_COLORS[v.color] : c === '#000000';
	}

	function lampErr(text) {
		clearTimeout(lc.timer);
		lc.err = text;
		if (text) {
			lc.timer = setTimeout(function () {
				lc.err = '';
				render();
			}, 10000);
		}
	}

	function parseJson(text) {
		try {
			return JSON.parse(text);
		} catch (e) {
			return null;
		}
	}

	// sends what is pending, one request at a time. The lamp's reply is the new lamp block; an error reply (its text is shown as it is)
	// or no reply puts the card back to the last confirmed settings
	function lampSend() {
		if (lc.busy || !lc.pend) {
			return;
		}
		var p = lc.pend;
		var n = p.night || {};
		var f = [];
		function put(k, v) {
			if (v !== undefined) {
				f.push(k + '=' + encodeURIComponent(v));
			}
		}
		put('on', p.on === undefined ? undefined : p.on ? 1 : 0);
		put('bright', p.bright);
		put('mode', p.mode);
		put('std', p.std);
		put('multi', p.multi && p.multi.join(','));
		put('night_param', n.param);
		put('night_from', n.from);
		put('night_to', n.to);
		put('color', p.color);
		lc.pend = null;
		lc.busy = true;
		fetchTimed('/api/lamp', { method: 'POST', headers: { 'Content-Type': 'application/x-www-form-urlencoded' }, body: f.join('&') }, function (r) {
			return r.text().then(function (t) {
				return { ok: r.ok, text: t };
			});
		})
			.then(
				function (a) {
					var l = a.ok ? parseLamp(parseJson(a.text)) : null;
					if (l) {
						lc.conf = l;
						lc.gen++;
					} else {
						lc.pend = lc.want = null;
						lampErr(!a.ok && a.text ? a.text : T.lcFail);
					}
				},
				function () {
					lc.pend = lc.want = null;
					lampErr(T.lcNoAnswer);
				}
			)
			.then(function () {
				lc.busy = false;
				if (lc.pend) {
					lc.want = merge(lc.conf, lc.pend);
					lampSend();
				} else {
					lc.want = null;
				}
				render();
			});
	}

	// the user changed something: p is a partial lamp
	function lampSet(p) {
		if (!lc.conf || state.down) {
			return;
		}
		lampErr('');
		lc.want = merge(lc.want || lc.conf, p);
		lc.pend = merge(lc.pend || {}, p);
		render();
		lampSend();
	}

	// an answer of /api/status: it must not overwrite edits the lamp has not confirmed
	function syncLamp(raw) {
		var l = parseLamp(raw);
		if (!l) {
			lc.conf = lc.want = lc.pend = null;
		} else if (!lc.conf || (!lc.busy && !lc.pend && !lc.want && lc.live === null)) {
			lc.conf = l;
		}
	}

	var lcEl = { modes: [], chips: [], dots: [], rows: [], cols: [] };

	function lcButton(cls, parent, kids, onClick) {
		var b = el('button', cls);
		b.type = 'button';
		kids.forEach(function (k) {
			b.appendChild(k);
		});
		b.addEventListener('click', onClick);
		parent.appendChild(b);
		return b;
	}

	function lcSelect(parent, onChange) {
		var s = el('select', 'sel');
		SEL_KEYS.forEach(function (k) {
			var o = el('option');
			o.value = k;
			s.appendChild(o);
		});
		s.addEventListener('change', onChange);
		parent.appendChild(s);
		return s;
	}

	function buildLamp() {
		var i;
		byId('lcPower').addEventListener('click', function () {
			lampSet({ on: !view().on });
		});
		var range = byId('lcRange');
		lcEl.range = range;
		// preview while dragging, send on release; the slider keeps its value until it is released or left
		range.addEventListener('input', function () {
			lc.live = +range.value;
			render();
		});
		range.addEventListener('change', function () {
			lc.live = null;
			lampSet({ bright: +range.value });
		});
		range.addEventListener('blur', function () {
			lc.live = null;
			render();
		});
		LAMP_MODES.forEach(function (m) {
			lcEl.modes.push(lcButton('md', byId('lcModes'), [el('b'), el('span')], function () {
				lampSet({ mode: m });
			}));
		});
		STD_KEYS.forEach(function (k) {
			var dot = el('i');
			lcEl.dots.push(dot);
			lcEl.chips.push(lcButton('chip', byId('lcChips'), [dot, el('span')], function () {
				lampSet({ std: k });
			}));
		});
		for (i = 0; i < 3; i++) {
			var row = el('div', 'mr3');
			var name = el('div', 'mr-l');
			var dot3 = el('i');
			name.appendChild(dot3);
			name.appendChild(el('span'));
			row.appendChild(name);
			var sel = lcSelect(row, function () {
				lampSet({
					multi: lcEl.rows.map(function (r) {
						return r.sel.value;
					})
				});
			});
			lcEl.rows.push({ dot: dot3, name: name.lastChild, sel: sel });
			byId('lcMulti').appendChild(row);
		}
		var nf = byId('lcNf');
		var nt = byId('lcNt');
		lcEl.np = byId('lcNp');
		SEL_KEYS.forEach(function (k) {
			var o = el('option');
			o.value = k;
			lcEl.np.appendChild(o);
		});
		lcEl.np.addEventListener('change', function () {
			lampSet({ night: { param: lcEl.np.value } });
		});
		nf.addEventListener('change', function () {
			if (nf.value) {
				lampSet({ night: { from: nf.value } });
			}
		});
		nt.addEventListener('change', function () {
			if (nt.value) {
				lampSet({ night: { to: nt.value } });
			}
		});
		LAMP_COLORS.forEach(function (c, n) {
			var sw = lcButton('cs', byId('lcCols'), [], function () {
				lampSet({ color: n });
			});
			sw.style.background = c;
			lcEl.cols.push(sw);
		});
	}

	function renderLamp(meas) {
		var v = view();
		var box = byId('lc');
		box.hidden = !v;
		if (!v) {
			return;
		}
		var down = state.down;
		var lock = down || !v.on;
		var modeName = T.modes[v.mode][0];
		box.classList.toggle('down', down);
		byId('lcDep').classList.toggle('off', !v.on);
		byId('lcSum').textContent = v.on ? modeName + ' · ' + v.bright + '%' : T.lcOff;
		var power = byId('lcPower');
		power.setAttribute('aria-checked', v.on ? 'true' : 'false');
		power.setAttribute('aria-label', T.lcPower);
		power.disabled = down;
		byId('lcBv').textContent = v.bright + '%';
		lcEl.range.setAttribute('aria-label', T.lcBright);
		if (lc.live === null) {
			lcEl.range.value = v.bright;
		}
		Array.prototype.forEach.call(byId('lcDep').querySelectorAll('button,input,select'), function (c) {
			c.disabled = lock;
		});
		LAMP_MODES.forEach(function (m, i) {
			var b = lcEl.modes[i];
			var noTime = m === 'night' && !v.time && v.mode !== 'night';
			b.setAttribute('aria-pressed', v.mode === m ? 'true' : 'false');
			b.disabled = lock || noTime;
			b.children[0].textContent = T.modes[m][0];
			b.children[1].textContent = noTime ? T.lcNeeds : T.modes[m][1];
		});
		byId('lcStd').hidden = v.mode !== 'standard';
		byId('lcMulti').hidden = v.mode !== 'multi';
		byId('lcNight').hidden = v.mode !== 'night';
		byId('lcLight').hidden = v.mode !== 'light';
		function setVal(c, value) {
			if (document.activeElement !== c) {
				c.value = value;
			}
		}
		function fillOptions(sel) {
			Array.prototype.forEach.call(sel.options, function (o) {
				o.textContent = readingName(o.value);
			});
		}
		STD_KEYS.forEach(function (k, i) {
			lcEl.chips[i].setAttribute('aria-pressed', v.std === k ? 'true' : 'false');
			lcEl.chips[i].children[1].textContent = readingName(k);
			lcEl.dots[i].style.background = measColor(meas, k);
		});
		lcEl.rows.forEach(function (r, i) {
			r.name.textContent = T.lcRows[i];
			r.dot.style.background = measColor(meas, v.multi[i]);
			fillOptions(r.sel);
			setVal(r.sel, v.multi[i]);
		});
		fillOptions(lcEl.np);
		setVal(lcEl.np, v.night.param);
		setVal(byId('lcNf'), v.night.from);
		setVal(byId('lcNt'), v.night.to);
		byId('lcCn').textContent = T.colors[v.color];
		lcEl.cols.forEach(function (b, i) {
			b.setAttribute('aria-label', T.colors[i]);
			b.setAttribute('aria-pressed', v.color === i ? 'true' : 'false');
		});
		byId('lcNote').textContent = down ? T.lcLock : T.lcLive;
		byId('lcReply').hidden = !lc.err;
		byId('lcReplyText').textContent = lc.err;
	}

	function render() {
		var d = state.data;
		var meas = (d && d.meas) || {};
		renderLamp(meas);
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
		disp.bright = p.bright;
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
		// a new picture (or another kind or seed) restarts the seed chain: it must not continue the passes of an earlier picture
		if (p.seq !== disp.seq || !disp.pic || disp.pic.kind !== p.pic.kind || disp.pic.seed !== p.pic.seed) {
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
		paintGrid(state.down);
	}

	// one request at a time: the next poll is scheduled before the page is redrawn, so a drawing error cannot stop polling
	function poll() {
		var gen = lc.gen;
		fetchTimed('/api/status', { cache: 'no-store' }, function (r) {
			if (r.ok) {
				return r.json().then(function (d) {
					if (state.down) {
						lastAsked = false;
					}
					state.data = d;
					// an answer asked before the last confirmed change still carries the old settings
					if (gen === lc.gen) {
						syncLamp(d && d.lamp);
					}
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
	buildLamp();
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
