(function () {
	'use strict';

	var POLL_MS = 5000;
	var FETCH_TIMEOUT_MS = 4000;
	var PADS = [2, 6, 8, 9];
	var ACTIONS = ['none', 'power', 'up', 'down', 'next', 'mode', 'show'];
	// the keys the lamp takes for the mode and show actions, as lamp_mode and lamp_param name them
	var MODES = ['standard', 'multi', 'night', 'light'];
	var PARAMS = ['total', 'temp', 'humid', 'eco2', 'tvoc', 'pm25'];
	var ARGS = { mode: MODES, show: PARAMS };
	var ARG_DEFAULT = { mode: 'standard', show: 'total' };
	// the lamp's own defaults (reset=1 restores them); the page uses them only to draw the result before the lamp answers
	var DEFAULTS = { 2: { a: 'power' }, 6: { a: 'up' }, 8: { a: 'next' }, 9: { a: 'down' } };
	// the pad's rotation on the board, and its label box: left, top, rotation
	var ROT = { 2: -45, 6: 45, 8: 135, 9: -135 };
	var LABEL = { 2: [48.5, 68.5, -45], 6: [170.5, 68.5, 45], 8: [170.5, 191.5, -45], 9: [48.5, 191.5, 45] };
	var PAD_PATH = 'M -15 -52 L 15 -52 Q 23 -52 26.01 -59.41 L 42.99 -101.25 Q 46 -108.66 38 -111.71 A 118 118 0 0 0 -38 -111.71 Q -46 -108.66 -42.99 -101.25 L -26.01 -59.41 Q -23 -52 -15 -52 Z';
	var SVG_NS = 'http://www.w3.org/2000/svg';

	var TEXT = {
		en: {
			connecting: 'Connecting to the lamp…', sta: 'Connected', dbm: 'dBm',
			staTo: function (n) {
				return 'Connected to ' + n;
			},
			ap: 'Access point · no internet', off: 'WiFi connection lost', down: 'Lamp not responding · retry in 5 s',
			title: 'Touch buttons', tapHint: 'Tap a button to choose what it does', onTap: 'action on tap',
			acts: {
				none: ['Nothing', ''], power: ['Turn lamp on / off', ''], up: ['Brighter', '5% step, hold — smooth'],
				down: ['Dimmer', '5% step, hold — smooth'], next: ['Next mode', 'Standard → Multi → Night → Lamp light'],
				mode: ['Switch to mode', ''], show: ['Show a reading', 'For 10 s, then the lamp returns to its mode']
			},
			short: { none: '—', power: 'On/off', up: 'Brighter', down: 'Dimmer', next: 'Mode ›' },
			modes: { standard: 'Standard', multi: 'Multi', night: 'Night', light: 'Lamp light' },
			params: { total: 'Overall', temp: 'Temperature', humid: 'Humidity', eco2: 'eCO₂', tvoc: 'TVOC', pm25: 'PM2.5' },
			pshort: { total: 'Overall', temp: 'Temp.', humid: 'Humid.' },
			warn: 'No button turns the lamp off — you’ll only be able to switch it off from this page.',
			lockTitle: 'Lock the buttons', lockOn: 'Locked: taps do nothing until you unlock. Hold TOUCH 6 and 8 on the lamp for 3 s to unlock there.',
			lockOff: 'Taps change the lamp.',
			live: 'Changes apply instantly', footLocked: 'You can still edit while locked', footDown: 'Settings are unavailable while the lamp isn’t responding',
			reset: 'Defaults', fail: 'The lamp did not accept the change.', noAnswer: 'No answer from the lamp.',
			sysTitle: 'System gestures', sysTag: 'always on',
			g1k: 'Hold', g1d: 'On a Brighter or Dimmer button the brightness changes smoothly while your finger stays on it.',
			g2k: '6 + 8 · 3 s', g2d: 'Hold TOUCH 6 and 8 together for 3 s to lock or unlock the buttons. The lamp blinks.',
			g3k: '2 + 8 · 10 s', g3d: 'Hold TOUCH 2 and 8 together for 10 s: the lamp restarts and starts its own WiFi network. Works while the buttons are locked.',
			main: 'Main page', wifi: 'WiFi networks', fw: 'Firmware',
			saveTitle: 'Couldn’t save', saveText: 'The lamp couldn’t write the settings to its memory. It went back to the last saved settings.', ok: 'OK'
		},
		ru: {
			connecting: 'Подключаемся к лампе…', sta: 'Подключена', dbm: 'дБм',
			staTo: function (n) {
				return 'Подключена к ' + n;
			},
			ap: 'Точка доступа · без интернета', off: 'Нет связи с WiFi', down: 'Лампа не отвечает · повтор через 5 с',
			title: 'Сенсорные кнопки', tapHint: 'Нажмите на кнопку, чтобы выбрать её действие', onTap: 'действие на касание',
			acts: {
				none: ['Ничего', ''], power: ['Вкл / выкл лампу', ''], up: ['Ярче', 'Шаг 5%, удержание — плавно'],
				down: ['Тусклее', 'Шаг 5%, удержание — плавно'], next: ['Следующий режим', 'Стандартный → Мульти → Ночной → Светильник'],
				mode: ['Включить режим', ''], show: ['Показать измерение', 'На 10 с, затем лампа вернётся в свой режим']
			},
			short: { none: '—', power: 'Вкл/выкл', up: 'Ярче', down: 'Тусклее', next: 'Режим ›' },
			modes: { standard: 'Стандартный', multi: 'Мульти', night: 'Ночной', light: 'Светильник' },
			params: { total: 'Общий', temp: 'Температура', humid: 'Влажность', eco2: 'eCO₂', tvoc: 'TVOC', pm25: 'PM2.5' },
			pshort: { total: 'Общий', temp: 'Темп.', humid: 'Влажн.' },
			warn: 'Ни одна кнопка не выключает лампу — выключить её можно будет только с этой страницы.',
			lockTitle: 'Заблокировать кнопки', lockOn: 'Заблокированы: касания не работают, пока вы не разблокируете. На лампе удержите TOUCH 6 и 8 в течение 3 с.',
			lockOff: 'Касания меняют лампу.',
			live: 'Изменения применяются сразу', footLocked: 'Можно настраивать и при блокировке', footDown: 'Настройка недоступна, пока лампа не отвечает',
			reset: 'По умолчанию', fail: 'Лампа не приняла изменение.', noAnswer: 'Лампа не ответила.',
			sysTitle: 'Системные жесты', sysTag: 'всегда работают',
			g1k: 'Удержание', g1d: 'На кнопке «Ярче» или «Тусклее» яркость меняется плавно, пока палец на кнопке.',
			g2k: '6 + 8 · 3 с', g2d: 'Удержите TOUCH 6 и 8 вместе 3 с — кнопки блокируются или разблокируются. Лампа мигнёт.',
			g3k: '2 + 8 · 10 с', g3d: 'Удержите TOUCH 2 и 8 вместе 10 с — лампа перезагрузится и включит свою WiFi-сеть. Работает и при блокировке кнопок.',
			main: 'Главная', wifi: 'WiFi-сети', fw: 'Прошивка',
			saveTitle: 'Не удалось сохранить', saveText: 'Лампа не смогла записать настройки в память и вернула последние сохранённые.', ok: 'OK'
		}
	};

	var root = document.documentElement;
	var T = TEXT.en;
	// data: the last good /api/status reply (the header); down: the last poll failed
	var state = { data: null, down: false };
	// conf: the buttons as the lamp last confirmed them (null: not known yet). want: conf plus the edits not yet confirmed, pend: those
	// edits still to send ({ reset, locked, buttons }), busy: a POST is out, err: the lamp's reply to a refused POST, sel: the pad shown
	// in the action card, saveErr: the lamp's count of failed writes at the last poll (null before the first answer)
	// gen: counts the confirmed POSTs, so an answer asked before the last confirmation can be told from a current one
	var bt = { conf: null, want: null, pend: null, busy: false, err: '', errTimer: 0, sel: 6, saveErr: null, gen: 0 };
	var padEls = {};
	var rowEls = {};
	var modalFrom = null;

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

	function intIn(v, lo, hi) {
		return typeof v === 'number' && isFinite(v) && Math.floor(v) === v && v >= lo && v <= hi;
	}

	function parseJson(text) {
		try {
			return JSON.parse(text);
		} catch (e) {
			return null;
		}
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

	function getJson(url) {
		return fetchTimed(url, { cache: 'no-store' }, function (r) {
			if (!r.ok) {
				throw new Error(r.status);
			}
			return r.json();
		});
	}

	// the /api/buttons reply (GET or POST), or null when it is not as expected: only whitelisted words get through
	function parseButtons(d) {
		var b = d && d.buttons;
		if (!b || typeof d.locked !== 'boolean') {
			return null;
		}
		var r = { locked: d.locked, buttons: {}, saveErr: intIn(d.saveErr, 0, 4294967295) ? d.saveErr : 0 };
		for (var i = 0; i < PADS.length; i++) {
			var x = b[PADS[i]];
			if (!x || ACTIONS.indexOf(x.a) < 0) {
				return null;
			}
			if (ARGS[x.a]) {
				if (ARGS[x.a].indexOf(x.arg) < 0) {
					return null;
				}
				r.buttons[PADS[i]] = { a: x.a, arg: x.arg };
			} else {
				r.buttons[PADS[i]] = { a: x.a };
			}
		}
		return r;
	}

	// base with the edit p on top; a reset comes first, the bindings after it are later edits
	function apply(base, p) {
		var r = { locked: p.locked === undefined ? base.locked : p.locked, buttons: {} };
		PADS.forEach(function (n) {
			r.buttons[n] = p.buttons && p.buttons[n] ? p.buttons[n] : p.reset ? DEFAULTS[n] : base.buttons[n];
		});
		return r;
	}

	function view() {
		return bt.want || bt.conf;
	}

	function showErr(text) {
		clearTimeout(bt.errTimer);
		bt.err = text;
		if (text) {
			bt.errTimer = setTimeout(function () {
				bt.err = '';
				render();
			}, 10000);
		}
	}

	// sends what is pending, one request at a time. reset=1 cannot share a request with other fields, so what came after it
	// stays pending. The lamp's reply is the new state; an error reply (its text is shown as it is) or no reply puts the page back
	// to the last confirmed buttons
	function btSend() {
		if (bt.busy || !bt.pend) {
			return;
		}
		var p = bt.pend;
		var f = [];
		var rest = null;
		if (p.reset) {
			f.push('reset=1');
			if (p.locked !== undefined || p.buttons) {
				rest = { locked: p.locked, buttons: p.buttons };
			}
		} else {
			if (p.locked !== undefined) {
				f.push('locked=' + (p.locked ? 1 : 0));
			}
			PADS.forEach(function (n) {
				var b = p.buttons && p.buttons[n];
				if (b) {
					f.push('a' + n + '=' + b.a);
					if (b.arg) {
						f.push('arg' + n + '=' + b.arg);
					}
				}
			});
		}
		bt.pend = rest;
		bt.busy = true;
		fetchTimed('/api/buttons', { method: 'POST', headers: { 'Content-Type': 'application/x-www-form-urlencoded' }, body: f.join('&') }, function (r) {
			return r.text().then(function (t) {
				return { ok: r.ok, text: t };
			});
		})
			.then(
				function (a) {
					var s = a.ok ? parseButtons(parseJson(a.text)) : null;
					if (s) {
						bt.conf = s;
						bt.gen++;
					} else {
						bt.pend = bt.want = null;
						showErr(!a.ok && a.text ? a.text : T.fail);
					}
				},
				function () {
					bt.pend = bt.want = null;
					showErr(T.noAnswer);
				}
			)
			.then(function () {
				bt.busy = false;
				if (bt.pend) {
					bt.want = apply(bt.conf, bt.pend);
					btSend();
				} else {
					bt.want = null;
				}
				render();
			});
	}

	// the user changed something: p is { reset } or { locked } or { buttons: { pad: binding } }
	function btSet(p) {
		if (!bt.conf || state.down) {
			return;
		}
		showErr('');
		var q = bt.pend || {};
		if (p.reset) {
			// the reset restores the bindings and keeps the lock: earlier binding edits are gone, a lock edit stays
			q = { reset: true, locked: q.locked };
		}
		if (p.locked !== undefined) {
			q.locked = p.locked;
		}
		if (p.buttons) {
			q.buttons = q.buttons || {};
			PADS.forEach(function (n) {
				if (p.buttons[n]) {
					q.buttons[n] = p.buttons[n];
				}
			});
		}
		bt.pend = q;
		bt.want = apply(bt.want || bt.conf, p);
		render();
		btSend();
	}

	function setBinding(a, arg) {
		var b = {};
		b[bt.sel] = ARGS[a] ? { a: a, arg: arg } : { a: a };
		btSet({ buttons: b });
	}

	function openModal() {
		if (!byId('modal').hidden) {
			return;
		}
		modalFrom = document.activeElement;
		byId('modal').hidden = false;
		byId('mdOk').focus();
	}

	function closeModal() {
		byId('modal').hidden = true;
		if (modalFrom && modalFrom.focus) {
			modalFrom.focus();
		}
		modalFrom = null;
	}

	// an answer of GET /api/buttons: it must not overwrite edits the lamp has not confirmed. A grown count of failed writes means the
	// lamp went back to its last saved buttons: this answer carries them, the page takes them and tells the user
	function syncButtons(s) {
		var grew = bt.saveErr !== null && s.saveErr > bt.saveErr;
		bt.saveErr = s.saveErr;
		if (grew) {
			bt.conf = s;
			bt.want = bt.pend ? apply(s, bt.pend) : null;
			openModal();
		} else if (!bt.conf || (!bt.busy && !bt.pend && !bt.want)) {
			bt.conf = s;
		}
	}

	function shortName(b) {
		if (b.a === 'mode') {
			return T.modes[b.arg];
		}
		return b.a === 'show' ? T.pshort[b.arg] || T.params[b.arg] : T.short[b.a];
	}

	function renderHeader() {
		var d = state.data;
		var dot = '';
		var text = T.connecting;
		var level = 0;
		if (state.down) {
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
		byId('mfw').textContent = d ? T.fw + ' ' + d.fw : '';
		byId('mip').textContent = d ? 'IP ' + d.ip : '';
	}

	function render() {
		renderHeader();
		var v = view();
		var off = state.down || !v;
		byId('bwrap').classList.toggle('dim', off);
		byId('lockCard').classList.toggle('dim', off);
		byId('actT').textContent = 'TOUCH ' + bt.sel;
		if (!v) {
			byId('foot').textContent = state.down ? T.footDown : '';
			return;
		}
		var b = v.buttons[bt.sel];
		var noPower = true;
		PADS.forEach(function (n) {
			var x = v.buttons[n];
			padEls[n].g.setAttribute('aria-pressed', n === bt.sel ? 'true' : 'false');
			padEls[n].lab.className = 'pl' + (n === bt.sel ? ' on' : '');
			padEls[n].name.textContent = shortName(x);
			noPower = noPower && x.a !== 'power';
		});
		ACTIONS.forEach(function (a) {
			var r = rowEls[a];
			var on = b.a === a;
			r.box.className = 'act' + (on ? ' on' : '');
			r.radio.checked = on;
			r.radio.disabled = off;
			r.title.textContent = T.acts[a][0];
			r.sub.textContent = T.acts[a][1];
			r.sub.hidden = !T.acts[a][1];
			if (r.sel) {
				r.sel.hidden = !on;
				r.sel.disabled = off;
				r.sel.setAttribute('aria-label', T.acts[a][0]);
				Array.prototype.forEach.call(r.sel.options, function (o) {
					o.textContent = (a === 'mode' ? T.modes : T.params)[o.value];
				});
				if (on && document.activeElement !== r.sel) {
					r.sel.value = b.arg;
				}
			}
		});
		byId('warn').hidden = !noPower;
		byId('reply').hidden = !bt.err;
		byId('replyText').textContent = bt.err;
		byId('foot').textContent = state.down ? T.footDown : (v.locked ? T.footLocked : T.live);
		byId('reset').disabled = off;
		var sw = byId('lockSw');
		sw.setAttribute('aria-checked', v.locked ? 'true' : 'false');
		sw.disabled = off;
		byId('lockCard').classList.toggle('on', v.locked);
		byId('lockS').textContent = v.locked ? T.lockOn : T.lockOff;
	}

	function buildBoard() {
		var board = byId('board');
		var svg = board.querySelector('svg');
		PADS.forEach(function (n) {
			var g = document.createElementNS(SVG_NS, 'g');
			var path = document.createElementNS(SVG_NS, 'path');
			g.setAttribute('class', 'pad');
			g.setAttribute('transform', 'translate(150 150) rotate(' + ROT[n] + ')');
			g.setAttribute('role', 'button');
			g.setAttribute('tabindex', '0');
			g.setAttribute('aria-label', 'TOUCH ' + n);
			path.setAttribute('d', PAD_PATH);
			g.appendChild(path);
			svg.appendChild(g);
			function pick() {
				bt.sel = n;
				render();
			}
			g.addEventListener('click', pick);
			g.addEventListener('keydown', function (e) {
				if (e.key === 'Enter' || e.key === ' ') {
					e.preventDefault();
					pick();
				}
			});
			var lab = el('div', 'pl');
			var name = el('span');
			lab.style.left = LABEL[n][0] + 'px';
			lab.style.top = LABEL[n][1] + 'px';
			lab.style.transform = 'rotate(' + LABEL[n][2] + 'deg)';
			lab.appendChild(name);
			lab.appendChild(el('i', '', 'TOUCH ' + n));
			board.appendChild(lab);
			padEls[n] = { g: g, lab: lab, name: name };
		});
	}

	function buildActs() {
		ACTIONS.forEach(function (a) {
			var box = el('div', 'act');
			var label = el('label', 'ar');
			var radio = el('input');
			var text = el('span', 'at');
			var title = el('b');
			var sub = el('small');
			var sel = null;
			radio.type = 'radio';
			radio.name = 'act';
			radio.addEventListener('change', function () {
				if (radio.checked) {
					setBinding(a, ARG_DEFAULT[a]);
				}
			});
			text.appendChild(title);
			text.appendChild(sub);
			label.appendChild(radio);
			label.appendChild(text);
			box.appendChild(label);
			if (ARGS[a]) {
				sel = el('select', 'sel');
				ARGS[a].forEach(function (k) {
					var o = el('option');
					o.value = k;
					sel.appendChild(o);
				});
				sel.addEventListener('change', function () {
					setBinding(a, sel.value);
				});
				box.appendChild(sel);
			}
			byId('acts').appendChild(box);
			rowEls[a] = { box: box, radio: radio, title: title, sub: sub, sel: sel };
		});
	}

	function setLang(code) {
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

	// one request chain: the next poll is scheduled before the page is redrawn, so a drawing error cannot stop polling
	function poll() {
		var gen = bt.gen;
		getJson('/api/status')
			.then(function (d) {
				state.data = d;
				return getJson('/api/buttons');
			})
			.then(function (d) {
				var s = parseButtons(d);
				if (!s) {
					throw new Error('shape');
				}
				state.down = false;
				// an answer asked before the last confirmed change still carries the old buttons
				if (gen === bt.gen) {
					syncButtons(s);
				}
			})
			.catch(function () {
				state.down = true;
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

	buildBoard();
	buildActs();
	byId('theme').addEventListener('click', toggleTheme);
	byId('lockSw').addEventListener('click', function () {
		btSet({ locked: !view().locked });
	});
	byId('reset').addEventListener('click', function () {
		btSet({ reset: true });
	});
	byId('mdOk').addEventListener('click', closeModal);
	document.addEventListener('keydown', function (e) {
		if (byId('modal').hidden) {
			return;
		}
		if (e.key === 'Escape') {
			closeModal();
		} else if (e.key === 'Tab') {
			e.preventDefault();
			byId('mdOk').focus();
		}
	});
	Array.prototype.forEach.call(document.querySelectorAll('[data-lang]'), function (b) {
		b.addEventListener('click', function () {
			setLang(b.getAttribute('data-lang'));
		});
	});
	setLang(saved);
	poll();
})();
