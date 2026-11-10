(function () {
	'use strict';

	var POLL_MS = 5000;
	var FETCH_TIMEOUT_MS = 4000;
	// the lamp keeps the name and the password in 31 bytes plus a terminator, and at most 10 networks
	var MAX_BYTES = 31;
	var MAX_NETS = 10;

	var TEXT = {
		en: {
			connecting: 'Connecting to the lamp…', sta: 'Connected', dbm: 'dBm',
			ap: 'Access point · no internet', off: 'WiFi connection lost', down: 'Lamp not responding · retry in 5 s',
			title: 'WiFi networks', addTitle: 'Add network', ssid: 'Network name (SSID)', pass: 'Password',
			ssidPh: 'e.g. HomeNet', passPh: 'Network password', show: 'Show', hide: 'Hide', bytes: 'bytes',
			add: 'Save and restart', update: 'Update password', saving: 'Saving…', noAnswer: 'The lamp does not answer.',
			badChars: 'The name or the password contains a character the lamp cannot receive.',
			tooLong: 'Over 31 bytes: emoji and non-Latin letters take 2–4 bytes each.',
			exists: 'This network is already saved — its password will be updated.',
			full: '10 networks saved — that’s the limit. Delete all to add a new one.',
			saved: 'Saved networks', empty: 'No saved networks yet.', upd: 'will update', listFail: 'Cannot read the saved networks.',
			delAll: 'Delete all networks', cancel: 'Cancel', delYes: 'Delete all',
			confirm: function (n) {
				return 'Delete all ' + n + ' networks? After its next restart the lamp starts its own access point.';
			},
			bAp: 'You’re on the lamp’s own network. Add your home WiFi — the lamp will restart and join it.',
			rbTitle: 'Saved. The lamp is restarting', rbWait: 'Waiting for the lamp · checking every 5 s', toMain: 'Back to main page',
			s1: 'Restart, about 3 seconds.',
			s2: function (n) {
				return 'The lamp joins “' + n + '”.';
			},
			s3sta: 'If it’s the same network, this page comes back on its own at this address.',
			s3ap: 'The lamp’s network will disappear. Reconnect your phone to home WiFi and find the lamp’s new address in your router.',
			main: 'Main page', log: 'Live log', fw: 'Firmware'
		},
		ru: {
			connecting: 'Подключаемся к лампе…', sta: 'Подключена', dbm: 'дБм',
			ap: 'Точка доступа · без интернета', off: 'Нет связи с WiFi', down: 'Лампа не отвечает · повтор через 5 с',
			title: 'WiFi-сети', addTitle: 'Добавить сеть', ssid: 'Имя сети (SSID)', pass: 'Пароль',
			ssidPh: 'Например, HomeNet', passPh: 'Пароль сети', show: 'Показать', hide: 'Скрыть', bytes: 'байт',
			add: 'Сохранить и перезагрузить', update: 'Обновить пароль', saving: 'Сохраняем…', noAnswer: 'Лампа не отвечает.',
			badChars: 'Имя или пароль содержит символ, который лампа не сможет принять.',
			tooLong: 'Длиннее 31 байта: эмодзи и кириллица занимают 2–4 байта на символ.',
			exists: 'Эта сеть уже сохранена — пароль будет обновлён.',
			full: 'Сохранено 10 сетей — это максимум. Удалите все, чтобы добавить новую.',
			saved: 'Сохранённые сети', empty: 'Сохранённых сетей пока нет.', upd: 'обновится', listFail: 'Не удалось прочитать сохранённые сети.',
			delAll: 'Удалить все сети', cancel: 'Отмена', delYes: 'Удалить все',
			confirm: function (n) {
				return 'Удалить все сети (' + n + ')? После следующей перезагрузки лампа включит свою точку доступа.';
			},
			bAp: 'Вы подключены к сети лампы. Добавьте домашний WiFi — лампа перезагрузится и подключится к нему.',
			rbTitle: 'Сохранено. Лампа перезагружается', rbWait: 'Ждём лампу · проверка каждые 5 с', toMain: 'На главную',
			s1: 'Перезагрузка, около 3 секунд.',
			s2: function (n) {
				return 'Лампа подключается к «' + n + '».';
			},
			s3sta: 'Если сеть та же, страница вернётся сама по этому адресу.',
			s3ap: 'Сеть лампы исчезнет. Подключите телефон к домашнему WiFi и найдите новый адрес лампы в роутере.',
			main: 'Главная', log: 'Живой лог', fw: 'Прошивка'
		}
	};

	var root = document.documentElement;
	var T = TEXT.en;
	var state = { data: null, down: false };
	var nets = [];
	var tagEls = [];
	// reboot: set after the lamp accepted a network; upBefore and sawDown tell the restart apart from a lamp that never went away
	var reboot = null;
	var busy = false;
	var confirming = false;

	var form = byId('form');
	var ssid = byId('ssid');
	var pass = byId('pass');
	var save = byId('save');

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

	// the lamp limit counts bytes, not characters
	function byteLen(s) {
		try {
			if (window.TextEncoder) {
				return new TextEncoder().encode(s).length;
			}
			return unescape(encodeURIComponent(s)).length;
		} catch (e) {
			// a lone surrogate cannot be encoded: count it as over the limit so the button stays off
			return MAX_BYTES + 1;
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

	// monotonic where available: a changed system clock must not fake or hide a restart
	function nowMs() {
		return window.performance && performance.now ? performance.now() : Date.now();
	}

	// text is the lamp's reply or a page message, always set as text
	function showReply(box, textEl, text, bad) {
		box.hidden = !text;
		box.className = 'reply' + (bad ? ' bad' : '');
		textEl.textContent = text;
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
				text = T.sta + ' · ' + d.rssi + ' ' + T.dbm;
				level = d.rssi > -55 ? 4 : (d.rssi > -65 ? 3 : (d.rssi > -75 ? 2 : 1));
			}
		}
		byId('sdot').className = 'sdot ' + dot;
		byId('stext').textContent = text;
		Array.prototype.forEach.call(byId('bars').children, function (bar, i) {
			bar.className = i < level ? 'on' : '';
		});
		byId('banner').hidden = !(d && d.mode === 'ap') || !!reboot;
		byId('mfw').textContent = d ? T.fw + ' ' + d.fw : '';
		byId('mip').textContent = d ? 'IP ' + d.ip : '';
	}

	function renderForm() {
		var sb = byteLen(ssid.value);
		var pb = byteLen(pass.value);
		var ssidLong = sb > MAX_BYTES;
		var passLong = pb > MAX_BYTES;
		var exists = ssid.value !== '' && nets.indexOf(ssid.value) >= 0;
		// a saved name can still be updated at 10 networks, a new one cannot
		var full = nets.length >= MAX_NETS && !exists;
		var hint = ssidLong ? T.tooLong : (exists ? T.exists : (full ? T.full : ''));
		var hintEl = byId('ssidHint');

		byId('ssidCnt').textContent = sb + ' / ' + MAX_BYTES + ' ' + T.bytes;
		byId('ssidCnt').className = 'cnt' + (ssidLong ? ' bad' : '');
		byId('ssidBox').className = 'inp' + (ssidLong ? ' bad' : (exists ? ' same' : ''));
		hintEl.hidden = !hint;
		hintEl.className = 'hint' + (ssidLong ? ' bad' : '');
		hintEl.textContent = hint;

		byId('passCnt').textContent = pb + ' / ' + MAX_BYTES + ' ' + T.bytes;
		byId('passCnt').className = 'cnt' + (passLong ? ' bad' : '');
		byId('passBox').className = 'inp' + (passLong ? ' bad' : '');
		byId('passHint').hidden = !passLong;
		byId('passHint').textContent = passLong ? T.tooLong : '';

		byId('showPass').textContent = pass.type === 'password' ? T.show : T.hide;
		save.textContent = exists ? T.update : T.add;
		save.disabled = ssidLong || passLong || full || busy;
		form.classList.toggle('dim-form', full);
		tagEls.forEach(function (tag, i) {
			tag.textContent = exists && nets[i] === ssid.value ? T.upd : '';
		});
	}

	function renderNets() {
		var box = byId('nets');
		box.textContent = '';
		tagEls = [];
		nets.forEach(function (name) {
			var row = el('div', 'row');
			var mini = el('span', 'mini');
			var tag = el('span', 'tag');
			mini.appendChild(el('i'));
			mini.appendChild(el('i'));
			mini.appendChild(el('i'));
			row.appendChild(mini);
			// network names come from the lamp: text only
			row.appendChild(el('span', 'nm', name));
			row.appendChild(tag);
			box.appendChild(row);
			tagEls.push(tag);
		});
		byId('netCnt').textContent = nets.length + ' / ' + MAX_NETS;
		byId('netCnt').className = 'cnt' + (nets.length >= MAX_NETS ? ' bad' : '');
		byId('empty').hidden = nets.length > 0;
		byId('delBox').hidden = nets.length === 0;
		renderConfirm();
		renderForm();
	}

	function renderConfirm() {
		byId('confirm').hidden = !confirming;
		byId('delAsk').hidden = confirming;
		byId('confirmText').textContent = T.confirm(nets.length);
	}

	function renderReboot() {
		byId('rb').hidden = !reboot;
		byId('edit').hidden = !!reboot;
		if (!reboot) {
			return;
		}
		byId('rbResp').textContent = reboot.text;
		byId('rbS1').textContent = T.s1;
		byId('rbS2').textContent = T.s2(reboot.ssid);
		byId('rbS3').textContent = reboot.ap ? T.s3ap : T.s3sta;
	}

	function renderAll() {
		renderHeader();
		renderForm();
		renderConfirm();
		renderReboot();
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
		ssid.placeholder = T.ssidPh;
		pass.placeholder = T.passPh;
		renderAll();
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

	function loadNets() {
		fetchTimed('/api/wifi', { cache: 'no-store' }, function (r) {
			if (!r.ok) {
				throw new Error(r.status);
			}
			return r.json();
		})
			.then(function (list) {
				nets = Array.isArray(list) ? list.filter(function (n) {
					return typeof n === 'string';
				}) : [];
				byId('listErr').hidden = true;
				renderNets();
			})
			.catch(function () {
				byId('listErr').hidden = false;
			});
	}

	function startReboot(name, text) {
		reboot = {
			ssid: name,
			text: text,
			ap: !!state.data && state.data.mode === 'ap',
			savedAtMs: nowMs()
		};
		pass.value = '';
		renderAll();
	}

	// the restart is over when the lamp reports an uptime shorter than the time since the save: it booted after the save
	function checkRestarted() {
		if (!reboot || state.down || typeof state.data.uptime !== 'number') {
			return;
		}
		if (state.data.uptime < (nowMs() - reboot.savedAtMs) / 1000) {
			location.reload();
		}
	}

	// one request at a time: the next poll is scheduled before the page is redrawn, so a drawing error cannot stop polling
	function poll() {
		fetchTimed('/api/status', { cache: 'no-store' }, function (r) {
			if (r.ok) {
				return r.json().then(function (d) {
					state.data = d;
					state.down = false;
				});
			}
			state.down = true;
		})
			.catch(function () {
				state.down = true;
			})
			.then(function () {
				setTimeout(poll, POLL_MS);
				renderHeader();
				checkRestarted();
			});
	}

	function submit(e) {
		e.preventDefault();
		if (save.disabled) {
			return;
		}
		var name = ssid.value;
		var body = '';
		try {
			body = 'ssid=' + encodeURIComponent(ssid.value) + '&pass=' + encodeURIComponent(pass.value);
		} catch (err) {
			// a lone surrogate in the name or the password cannot be encoded
			showReply(byId('reply'), byId('replyText'), T.badChars, true);
			return;
		}
		busy = true;
		renderForm();
		showReply(byId('reply'), byId('replyText'), T.saving, false);
		fetch('/add_wifi_creds', {
			method: 'POST',
			headers: { 'Content-Type': 'application/x-www-form-urlencoded' },
			body: body
		})
			.then(function (r) {
				return r.text().then(function (t) {
					busy = false;
					if (r.ok) {
						byId('reply').hidden = true;
						startReboot(name, t);
						return;
					}
					showReply(byId('reply'), byId('replyText'), t, true);
					renderForm();
					loadNets();
				});
			})
			.catch(function () {
				busy = false;
				showReply(byId('reply'), byId('replyText'), T.noAnswer, true);
				renderForm();
			});
	}

	function deleteAll() {
		byId('delYes').disabled = true;
		byId('delNo').disabled = true;
		fetch('/delete_wifi_creds', { method: 'POST' })
			.then(function (r) {
				return r.text().then(function (t) {
					showReply(byId('delReply'), byId('delReplyText'), t, !r.ok);
				});
			})
			.catch(function () {
				showReply(byId('delReply'), byId('delReplyText'), T.noAnswer, true);
			})
			.then(function () {
				byId('delYes').disabled = false;
				byId('delNo').disabled = false;
				confirming = false;
				renderConfirm();
				loadNets();
			});
	}

	var saved = load('lang');
	if (saved !== 'en' && saved !== 'ru') {
		saved = /^ru/i.test(navigator.language || '') ? 'ru' : 'en';
	}

	byId('theme').addEventListener('click', toggleTheme);
	Array.prototype.forEach.call(document.querySelectorAll('[data-lang]'), function (b) {
		b.addEventListener('click', function () {
			setLang(b.getAttribute('data-lang'));
		});
	});
	byId('showPass').addEventListener('click', function () {
		pass.type = pass.type === 'password' ? 'text' : 'password';
		renderForm();
	});
	ssid.addEventListener('input', renderForm);
	pass.addEventListener('input', renderForm);
	form.addEventListener('submit', submit);
	byId('delAsk').addEventListener('click', function () {
		confirming = true;
		showReply(byId('delReply'), byId('delReplyText'), '', false);
		renderConfirm();
	});
	byId('delNo').addEventListener('click', function () {
		confirming = false;
		renderConfirm();
	});
	byId('delYes').addEventListener('click', deleteAll);

	setLang(saved);
	poll();
	loadNets();
})();
