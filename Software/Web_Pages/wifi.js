(function () {
	'use strict';

	var POLL_MS = 5000;
	var FETCH_TIMEOUT_MS = 4000;
	// while the lamp checks a network: ask for the result this often; when it stops answering (the phone can lose the
	// lamp's WiFi for a moment while the lamp joins the other network) keep asking quietly this long before giving up
	var JOIN_POLL_MS = 2000;
	var JOIN_QUIET_MS = 60000;
	// After a check passes the lamp holds its own WiFi for the time /api/join reports (restart_ms; 20 s in the firmware),
	// restarts, and joins the new network about 8-10 s later (2 s boot effect, file system, connect). This much is added
	// on top, with some margin, so the addresses turn active when the lamp is most likely on the new network and the
	// phone is back on it. When the lamp does not report restart_ms the hold is assumed to be this long.
	var JOIN_BOOT_MS = 15000;
	var JOIN_HOLD_FALLBACK_MS = 20000;
	// Opening the new address by itself: the page asks both addresses every PROBE_ROUND_MS and goes to the first that
	// answers. Probing starts only PROBE_MARGIN_MS after the lamp's restart: during the hold the lamp owns the new
	// station address too and answers it on its own access point, so an earlier probe from a phone still on the lamp's
	// WiFi would succeed. A probe also counts only when the answer shows the lamp has restarted (see lampRestarted), which
	// covers a restart that comes late. Probing stops PROBE_TOTAL_MS later.
	var PROBE_MARGIN_MS = 2000;
	var PROBE_ROUND_MS = 3000;
	var PROBE_TOTAL_MS = 120000;
	// the lamp keeps the name and the password in 31 bytes plus a terminator, and at most 10 networks
	var MAX_BYTES = 31;
	var MAX_NETS = 10;

	var TEXT = {
		en: {
			connecting: 'Connecting to the lamp…', sta: 'Connected', dbm: 'dBm',
			staTo: function (n) {
				return 'Connected to ' + n;
			},
			wait: function (s) {
				return 'available in ' + s + ' s';
			},
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
			main: 'Main page', log: 'Live log', fw: 'Firmware',
			checking: function (n) {
				return 'Checking “' + n + '”…';
			},
			mayLose: 'The phone may lose the lamp’s WiFi for a moment…',
			fail: function (reason, n) {
				var q = '“' + n + '”';
				if (reason === 'wrong_password') {
					return 'The password for ' + q + ' was not accepted. Check it and try again.';
				}
				if (reason === 'no_network') {
					return 'The lamp cannot find ' + q + '. Check the name and that the router is on and in range, then try again.';
				}
				if (reason === 'save_failed') {
					return 'The lamp joined ' + q + ' but could not save it. Try again. If it fails again, delete all networks and add this one again.';
				}
				if (reason === 'lost') {
					return 'The lamp has probably joined ' + q + ' and restarted. Connect this phone to ' + q + ' and open http://haquda.local/. If the page does not open, reconnect to the lamp’s WiFi and try again.';
				}
				return q + ' did not answer in time. Check the password and the signal, then try again.';
			},
			okTitle: function (n) {
				return 'The lamp joined “' + n + '”';
			},
			okText: function (n) {
				return 'Saved. The lamp restarts in a few seconds and joins “' + n + '”. Connect this phone to “' + n + '”: this page then opens the new address by itself once the lamp is reachable. The addresses below work by hand and turn active shortly.';
			},
			addrIp: 'By IP address', addrHost: 'By name', copy: 'Copy', copied: 'Copied', copyFail: 'Select and copy'
		},
		ru: {
			connecting: 'Подключаемся к лампе…', sta: 'Подключена', dbm: 'дБм',
			staTo: function (n) {
				return 'Подключена к ' + n;
			},
			wait: function (s) {
				return 'доступен через ' + s + ' с';
			},
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
			main: 'Главная', log: 'Живой лог', fw: 'Прошивка',
			checking: function (n) {
				return 'Проверяем «' + n + '»…';
			},
			mayLose: 'Телефон может ненадолго потерять WiFi лампы…',
			fail: function (reason, n) {
				var q = '«' + n + '»';
				if (reason === 'wrong_password') {
					return 'Пароль сети ' + q + ' не принят. Проверьте его и попробуйте снова.';
				}
				if (reason === 'no_network') {
					return 'Лампа не находит сеть ' + q + '. Проверьте имя и что роутер включён и в зоне досягаемости, и попробуйте снова.';
				}
				if (reason === 'save_failed') {
					return 'Лампа подключилась к ' + q + ', но не смогла сохранить сеть. Попробуйте снова. Если не получится, удалите все сети и добавьте эту заново.';
				}
				if (reason === 'lost') {
					return 'Лампа, скорее всего, подключилась к ' + q + ' и перезагрузилась. Подключите этот телефон к ' + q + ' и откройте http://haquda.local/. Если страница не открывается, подключитесь к WiFi лампы и попробуйте снова.';
				}
				return 'Сеть ' + q + ' не ответила вовремя. Проверьте пароль и уровень сигнала и попробуйте снова.';
			},
			okTitle: function (n) {
				return 'Лампа подключилась к «' + n + '»';
			},
			okText: function (n) {
				return 'Сохранено. Лампа перезагрузится через несколько секунд и подключится к «' + n + '». Подключите этот телефон к «' + n + '»: эта страница сама откроет новый адрес, когда лампа станет доступна. Адреса ниже работают вручную и скоро станут активными.';
			},
			addrIp: 'По IP-адресу', addrHost: 'По имени', copy: 'Копировать', copied: 'Скопировано', copyFail: 'Выделите и скопируйте'
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
	// the lamp checks a network before it saves it (access point mode):
	// join: a check is running, { ssid, lostSinceMs: when the lamp stopped answering, 0 while it answers }
	// failure: the last check failed, { reason, ssid }; joined: the check passed, { ssid, ip, host }
	var join = null;
	var failure = null;
	var joined = null;

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
		byId('banner').hidden = !(d && d.mode === 'ap') || !!reboot || !!joined;
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
		save.disabled = ssidLong || passLong || full || busy || !!join;
		ssid.disabled = !!join;
		pass.disabled = !!join;
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

	// the check status goes into the form's reply box: the form stays visible with its values
	function renderJoin() {
		var box = byId('reply');
		if (join) {
			showReply(box, byId('replyText'), T.checking(join.ssid) + (join.lostSinceMs ? ' ' + T.mayLose : ''), false);
			box.classList.add('wait');
		} else if (failure) {
			showReply(box, byId('replyText'), T.fail(failure.reason, failure.ssid), true);
		}
	}

	// the address comes from the lamp: only a value that looks like an address is shown, and the link href is built from
	// that value alone. Until the countdown ends the link has no href, so it cannot be followed
	function renderAddr(boxId, linkId, waitId, value, pattern, secondsLeft) {
		var ok = typeof value === 'string' && pattern.test(value);
		var link = byId(linkId);
		var wait = byId(waitId);
		var url = ok ? 'http://' + value + '/' : '';
		byId(boxId).hidden = !ok;
		link.textContent = url;
		if (ok && secondsLeft <= 0) {
			link.setAttribute('href', url);
			link.removeAttribute('aria-disabled');
		} else {
			link.removeAttribute('href');
			link.setAttribute('aria-disabled', 'true');
		}
		wait.hidden = !ok || secondsLeft <= 0;
		wait.textContent = secondsLeft > 0 ? T.wait(secondsLeft) : '';
	}

	function joinedSecondsLeft() {
		return Math.max(0, Math.ceil((joined.endsAtMs - nowMs()) / 1000));
	}

	function renderJoined() {
		byId('jok').hidden = !joined;
		if (!joined) {
			return;
		}
		var left = joinedSecondsLeft();
		byId('jokTitle').textContent = T.okTitle(joined.ssid);
		renderAddr('jokIpBox', 'jokIp', 'jokIpWait', joined.ip, /^\d{1,3}(\.\d{1,3}){3}$/, left);
		renderAddr('jokHostBox', 'jokHost', 'jokHostWait', joined.host, /^[a-z0-9.-]{1,63}$/i, left);
		// after the probing gave up the user is told what to do by hand
		byId('jokText').textContent = joined.gaveUp ? T.fail('lost', joined.ssid) : T.okText(joined.ssid);
	}

	// the validated address URLs, the IP first; the same values the links use
	function joinedUrls() {
		var urls = [];
		if (typeof joined.ip === 'string' && /^\d{1,3}(\.\d{1,3}){3}$/.test(joined.ip)) {
			urls.push('http://' + joined.ip + '/');
		}
		if (typeof joined.host === 'string' && /^[a-z0-9.-]{1,63}$/i.test(joined.host)) {
			urls.push('http://' + joined.host + '/');
		}
		return urls;
	}

	// the answer is the lamp's /api/boot (the only route that allows a cross-origin read; it holds no network data), and its
	// uptime is shorter than the time since the page saw the check pass: the lamp booted after the check, so it has
	// restarted. A lamp that is late with its restart, or another device that has the address on another network, fails
	// this. Date.now(), not nowMs(): the wall clock keeps counting while a sleeping phone stops performance.now(), and
	// the lamp's uptime counts that time too
	function lampRestarted(d) {
		return !!d && typeof d === 'object' && typeof d.fw === 'string' && typeof d.host === 'string' &&
			typeof d.uptime === 'number' && isFinite(d.uptime) && d.uptime >= 0 && d.uptime * 1000 < Date.now() - joined.okAtMs;
	}

	function probe(url) {
		return fetchTimed(url + 'api/boot', { cache: 'no-store' }, function (r) {
			if (!r.ok) {
				throw new Error(r.status);
			}
			return r.json();
		}).then(function (d) {
			return !!joined && lampRestarted(d);
		}, function () {
			return false;
		});
	}

	// one chain: a round asks every address, goes to the first that answered as the restarted lamp, else schedules the next round
	function probeRound() {
		if (!joined || joined.navigating) {
			return;
		}
		var urls = joinedUrls();
		if (nowMs() > joined.probeUntilMs || !urls.length) {
			joined.gaveUp = true;
			renderJoined();
			return;
		}
		Promise.all(urls.map(probe)).then(function (answers) {
			if (!joined) {
				return;
			}
			var at = answers.indexOf(true);
			if (at >= 0) {
				joined.navigating = true;
				location.href = urls[at];
				return;
			}
			setTimeout(probeRound, PROBE_ROUND_MS);
		});
	}

	// the countdown runs on the page's own clock: it goes on when the lamp's WiFi is gone and the phone loses the lamp
	function joinedTick() {
		if (!joined) {
			return;
		}
		renderJoined();
		if (joinedSecondsLeft() > 0) {
			setTimeout(joinedTick, 1000);
		}
	}

	function renderReboot() {
		byId('rb').hidden = !reboot;
		byId('edit').hidden = !!reboot || !!joined;
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
		renderJoin();
		renderJoined();
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

	function showOk(name, d) {
		join = null;
		failure = null;
		var holdMs = typeof d.restart_ms === 'number' && isFinite(d.restart_ms) && d.restart_ms >= 0 ? d.restart_ms : JOIN_HOLD_FALLBACK_MS;
		var probeFromMs = nowMs() + holdMs + PROBE_MARGIN_MS;
		joined = {
			ssid: name, ip: d.ip, host: d.host, okAtMs: Date.now(), endsAtMs: nowMs() + holdMs + JOIN_BOOT_MS,
			probeUntilMs: probeFromMs + PROBE_TOTAL_MS, gaveUp: false, navigating: false
		};
		pass.value = '';
		renderAll();
		setTimeout(joinedTick, 1000);
		setTimeout(probeRound, holdMs + PROBE_MARGIN_MS);
	}

	function showFail(name, reason) {
		join = null;
		failure = { reason: typeof reason === 'string' ? reason : 'timeout', ssid: name };
		renderAll();
	}

	function readJoin(r) {
		if (!r.ok) {
			throw new Error(r.status);
		}
		return r.json();
	}

	// d: the /api/join reply, null when the lamp did not answer
	function applyJoin(d) {
		if (!join) {
			return;
		}
		if (!d || typeof d !== 'object') {
			var now = nowMs();
			if (!join.lostSinceMs) {
				join.lostSinceMs = now;
			}
			if (now - join.lostSinceMs > JOIN_QUIET_MS) {
				showFail(join.ssid, 'lost');
				return;
			}
			setTimeout(joinPoll, JOIN_POLL_MS);
			renderJoin();
			return;
		}
		join.lostSinceMs = 0;
		if (d.state === 'checking') {
			setTimeout(joinPoll, JOIN_POLL_MS);
			renderJoin();
		} else if (d.state === 'ok') {
			showOk(typeof d.ssid === 'string' ? d.ssid : join.ssid, d);
		} else if (d.state === 'fail') {
			showFail(join.ssid, d.reason);
		} else {
			// idle: the lamp restarted or forgot the check
			showFail(join.ssid, 'lost');
		}
	}

	// polls only while a check runs; the chain ends when join is cleared
	function joinPoll() {
		if (!join) {
			return;
		}
		fetchTimed('/api/join', { cache: 'no-store' }, readJoin)
			.catch(function () {
				return null;
			})
			.then(applyJoin);
	}

	function startJoin(name) {
		var fresh = !join;
		join = { ssid: name, lostSinceMs: 0 };
		failure = null;
		renderForm();
		renderJoin();
		if (fresh) {
			setTimeout(joinPoll, JOIN_POLL_MS);
		}
	}

	// asks once for a check the page did not start itself (another phone, this page opened again, or a 409 reply).
	// A running check is picked up always; a finished one only when showFinal is set
	function probeJoin(showFinal) {
		fetchTimed('/api/join', { cache: 'no-store' }, readJoin)
			.then(function (d) {
				if (!d || join || joined || typeof d.ssid !== 'string') {
					return;
				}
				if (d.state === 'checking') {
					startJoin(d.ssid);
				} else if (showFinal && d.state === 'ok') {
					showOk(d.ssid, d);
				} else if (showFinal && d.state === 'fail') {
					if (!ssid.value) {
						ssid.value = d.ssid;
					}
					showFail(d.ssid, d.reason);
				}
			})
			.catch(function () {});
	}

	// plain http pages have no navigator.clipboard, so the textarea fallback is the usual path on the lamp
	function copyFallback(text) {
		var area = null;
		try {
			area = document.createElement('textarea');
			area.value = text;
			area.setAttribute('readonly', '');
			area.style.position = 'fixed';
			area.style.top = '0';
			area.style.opacity = '0';
			document.body.appendChild(area);
			area.select();
			area.setSelectionRange(0, text.length);
			var ok = document.execCommand('copy');
			document.body.removeChild(area);
			return !!ok;
		} catch (e) {
			try {
				if (area && area.parentNode) {
					area.parentNode.removeChild(area);
				}
			} catch (e2) {}
			return false;
		}
	}

	function copyText(text, button) {
		function done(ok) {
			button.textContent = ok ? T.copied : T.copyFail;
			setTimeout(function () {
				button.textContent = T.copy;
			}, 2000);
		}
		try {
			if (navigator.clipboard && navigator.clipboard.writeText && window.isSecureContext) {
				navigator.clipboard.writeText(text).then(function () {
					done(true);
				}, function () {
					done(copyFallback(text));
				});
				return;
			}
		} catch (e) {}
		done(copyFallback(text));
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
		failure = null;
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
					// 202: access point mode, the lamp checks the network first; the text is for pages without scripts
					if (r.status === 202) {
						startJoin(name);
						return;
					}
					if (r.ok) {
						byId('reply').hidden = true;
						startReboot(name, t);
						return;
					}
					showReply(byId('reply'), byId('replyText'), t, true);
					renderForm();
					loadNets();
					if (r.status === 409) {
						// a check is running, or one passed and the lamp restarts: show it
						probeJoin(true);
					}
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
	byId('jokIpCopy').addEventListener('click', function () {
		copyText(byId('jokIp').textContent, this);
	});
	byId('jokHostCopy').addEventListener('click', function () {
		copyText(byId('jokHost').textContent, this);
	});

	setLang(saved);
	poll();
	loadNets();
	probeJoin(true);
})();
