(function () {
	'use strict';

	var POLL_MS = 5000;
	var FETCH_TIMEOUT_MS = 4000;
	// while the lamp checks a network: ask for the result this often; when it stops answering (the phone can lose the
	// lamp's WiFi for a moment while the lamp joins the other network) keep asking quietly this long before giving up
	var JOIN_POLL_MS = 2000;
	var JOIN_QUIET_MS = 60000;
	// station mode: the lamp leaves the network this page is served over, joins the new one, and restarts. The page asks its own
	// origin every STA_PROBE_MS while the lamp is away and waits this long for it to come back before it says the lamp has
	// probably joined the new network. A failed switch restarts the lamp, which then tries its saved networks in file order,
	// up to about 32 s per network, so a lamp whose old network is not the first record is back only after several of those
	// plus the boot; 180 s covers about four unreachable records ahead of it.
	var STA_PROBE_MS = 3000;
	var STA_QUIET_MS = 180000;
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
			full: function (oldest) {
				return '10 networks saved: adding this one removes “' + oldest + '”.';
			},
			saved: 'Saved networks', empty: 'No saved networks yet.', upd: 'will update', listFail: 'Cannot read the saved networks.',
			delAll: 'Delete all networks', cancel: 'Cancel', delYes: 'Delete all',
			confirm: function (n) {
				return 'Delete all ' + n + ' networks? After its next restart the lamp starts its own access point.';
			},
			bAp: 'You’re on the lamp’s own network. Add your home WiFi — the lamp will restart and join it.',
			main: 'Main page', log: 'Live log', fw: 'Firmware',
			checking: function (n) {
				return 'Checking “' + n + '”…';
			},
			switching: function (from, n) {
				return 'The lamp leaves ' + (from ? '“' + from + '”' : 'its network') + ' to try “' + n + '”; this page loses it for about a minute.';
			},
			staOk: function (n) {
				return 'The lamp joined “' + n + '”. If this page stops answering, connect this phone to “' + n + '” and open http://haquda.local/.';
			},
			close: 'Close',
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
			mayLose: 'The phone may lose the lamp’s WiFi for a moment…',
			fail: function (reason, n) {
				var q = '“' + n + '”';
				if (reason === 'wrong_password') {
					return 'The password for ' + q + ' was not accepted. Check it and try again.';
				}
				if (reason === 'no_network') {
					return 'The lamp cannot see ' + q + '. The lamp works only with 2.4 GHz WiFi: if this is a 5 GHz network, choose the router’s 2.4 GHz network or turn 2.4 GHz on in the router. Otherwise check the name and that the router is on and in range, then try again.';
				}
				if (reason === 'save_failed') {
					return 'The lamp joined ' + q + ' but could not save it. Try again. If it fails again, delete all networks and add this one again.';
				}
				if (reason === 'lost') {
					return 'The lamp has probably joined ' + q + ' and restarted. Connect this phone to ' + q + ' and open http://haquda.local/. If the page does not open, reconnect to the lamp’s WiFi and try again.';
				}
				if (reason === 'scan_failed') {
					return 'The lamp could not search for networks, so it did not try ' + q + '. Try again.';
				}
				if (reason === 'unknown') {
					return 'The lamp is back, but it did not report how the switch to ' + q + ' ended. It is probably on its previous network. Try again.';
				}
				if (reason === 'lostSta') {
					return 'The lamp has probably joined ' + q + '. Connect this phone to ' + q + ' and open http://haquda.local/.';
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
			full: function (oldest) {
				return 'Сохранено 10 сетей: при добавлении этой будет удалена «' + oldest + '».';
			},
			saved: 'Сохранённые сети', empty: 'Сохранённых сетей пока нет.', upd: 'обновится', listFail: 'Не удалось прочитать сохранённые сети.',
			delAll: 'Удалить все сети', cancel: 'Отмена', delYes: 'Удалить все',
			confirm: function (n) {
				return 'Удалить все сети (' + n + ')? После следующей перезагрузки лампа включит свою точку доступа.';
			},
			bAp: 'Вы подключены к сети лампы. Добавьте домашний WiFi — лампа перезагрузится и подключится к нему.',
			main: 'Главная', log: 'Живой лог', fw: 'Прошивка',
			checking: function (n) {
				return 'Проверяем «' + n + '»…';
			},
			switching: function (from, n) {
				return 'Лампа покидает ' + (from ? '«' + from + '»' : 'свою сеть') + ', чтобы попробовать «' + n + '»; эта страница потеряет лампу примерно на минуту.';
			},
			staOk: function (n) {
				return 'Лампа подключилась к «' + n + '». Если страница перестанет отвечать, подключите этот телефон к «' + n + '» и откройте http://haquda.local/.';
			},
			close: 'Закрыть',
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
			mayLose: 'Телефон может ненадолго потерять WiFi лампы…',
			fail: function (reason, n) {
				var q = '«' + n + '»';
				if (reason === 'wrong_password') {
					return 'Пароль сети ' + q + ' не принят. Проверьте его и попробуйте снова.';
				}
				if (reason === 'no_network') {
					return 'Лампа не видит сеть ' + q + '. Лампа работает только с WiFi 2,4 ГГц: если это сеть 5 ГГц, выберите сеть роутера на 2,4 ГГц или включите 2,4 ГГц в настройках роутера. Иначе проверьте имя и что роутер включён и в зоне досягаемости, и попробуйте снова.';
				}
				if (reason === 'save_failed') {
					return 'Лампа подключилась к ' + q + ', но не смогла сохранить сеть. Попробуйте снова. Если не получится, удалите все сети и добавьте эту заново.';
				}
				if (reason === 'lost') {
					return 'Лампа, скорее всего, подключилась к ' + q + ' и перезагрузилась. Подключите этот телефон к ' + q + ' и откройте http://haquda.local/. Если страница не открывается, подключитесь к WiFi лампы и попробуйте снова.';
				}
				if (reason === 'scan_failed') {
					return 'Лампа не смогла искать сети и не стала пробовать ' + q + '. Попробуйте снова.';
				}
				if (reason === 'unknown') {
					return 'Лампа снова на связи, но не сообщила, чем закончился переход на ' + q + '. Скорее всего, она в прежней сети. Попробуйте снова.';
				}
				if (reason === 'lostSta') {
					return 'Лампа, скорее всего, подключилась к ' + q + '. Подключите этот телефон к ' + q + ' и откройте http://haquda.local/.';
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
	var busy = false;
	var confirming = false;
	// the lamp checks a network before it saves it (access point mode):
	// join: a check is running, { ssid, lostSinceMs: when the lamp stopped answering, 0 while it answers, sta: station mode (the
	// lamp leaves this page's network for the check), fromSsid: the network it leaves, switching: the lamp has announced the leave }
	// failure: the last check failed, { reason, ssid }; joined: the check passed, { ssid, ip, host }
	var join = null;
	var failure = null;
	var joined = null;
	// the outcome of an earlier station mode switch that failed and restarted the lamp: { ssid, reason }, or null
	var lastJoin = null;

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
		byId('banner').hidden = !(d && d.mode === 'ap') || !!joined;
		byId('mfw').textContent = d ? T.fw + ' ' + d.fw : '';
		byId('mip').textContent = d ? 'IP ' + d.ip : '';
		renderLast();
	}

	// the names come from the lamp: text only
	function renderLast() {
		var d = state.data;
		var cur = d && d.mode === 'sta' && typeof d.ssid === 'string' ? d.ssid : '';
		byId('lastBanner').hidden = !lastJoin;
		byId('lastBannerText').textContent = lastJoin ? T.lastFail(lastJoin.reason, lastJoin.ssid, cur) : '';
	}

	// the lamp forgets the outcome once the user has read it
	function dismissLast() {
		lastJoin = null;
		renderLast();
		fetch('/api/join/dismiss', { method: 'POST' }).catch(function () {});
	}

	function renderForm() {
		var sb = byteLen(ssid.value);
		var pb = byteLen(pass.value);
		var ssidLong = sb > MAX_BYTES;
		var passLong = pb > MAX_BYTES;
		var exists = ssid.value !== '' && nets.indexOf(ssid.value) >= 0;
		// at 10 networks a new name is still saved: the lamp keeps the new one first and drops the last of the list (the list
		// has the file's order, and the lamp cuts from the end of the file). A saved name is only updated, nothing is removed
		var oldest = nets.length >= MAX_NETS && !exists ? nets[nets.length - 1] : '';
		var hint = ssidLong ? T.tooLong : (exists ? T.exists : (oldest ? T.full(oldest) : ''));
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
		save.disabled = ssidLong || passLong || busy || !!join;
		ssid.disabled = !!join;
		pass.disabled = !!join;
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
			var text = T.checking(join.ssid) + (join.lostSinceMs ? ' ' + T.mayLose : '');
			// station mode: the lamp is leaving or has left this page's network, whether or not the announce was seen
			if (join.sta && (join.switching || join.lostSinceMs)) {
				text = T.switching(join.fromSsid, join.ssid);
			}
			showReply(box, byId('replyText'), text, false);
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

	// after a passed access point check the form gives way to the addresses card
	function renderEdit() {
		byId('edit').hidden = !!joined;
	}

	function renderAll() {
		renderHeader();
		renderForm();
		renderConfirm();
		renderEdit();
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

	// the station mode check passed: the lamp is in the new network and this page still reaches it (or the lamp says so
	// before it restarts). The outcome is shown here, so the lamp forgets it
	function showStaOk(name) {
		join = null;
		failure = null;
		pass.value = '';
		renderAll();
		showReply(byId('reply'), byId('replyText'), T.staOk(name), false);
		loadNets();
		dismissLast();
	}

	// the lamp is back after a station mode switch and tells how it ended; last is the "last" field of /api/join
	function applyStaLast(last) {
		var name = last && typeof last.ssid === 'string' && last.ssid ? last.ssid : join.ssid;
		if (last && last.result === 'ok') {
			showStaOk(name);
		} else if (last && last.result === 'fail') {
			showFail(name, last.reason);
			dismissLast();
		} else {
			// back with nothing to report (a power cut, or another page dismissed it): the outcome is unknown, and a lamp
			// that answers at this origin again is on the network this page was served over, not on the new one
			showFail(join.ssid, 'unknown');
		}
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
			if (now - join.lostSinceMs > (join.sta ? STA_QUIET_MS : JOIN_QUIET_MS)) {
				showFail(join.ssid, join.sta ? 'lostSta' : 'lost');
				return;
			}
			setTimeout(joinPoll, join.sta ? STA_PROBE_MS : JOIN_POLL_MS);
			renderJoin();
			return;
		}
		join.lostSinceMs = 0;
		if (d.state === 'checking') {
			setTimeout(joinPoll, JOIN_POLL_MS);
			renderJoin();
		} else if (d.state === 'switching') {
			join.sta = true;
			join.switching = true;
			setTimeout(joinPoll, JOIN_POLL_MS);
			renderJoin();
		} else if (d.state === 'ok') {
			if (join.sta) {
				showStaOk(typeof d.ssid === 'string' ? d.ssid : join.ssid);
			} else {
				showOk(typeof d.ssid === 'string' ? d.ssid : join.ssid, d);
			}
		} else if (d.state === 'fail') {
			showFail(join.ssid, d.reason);
			if (join.sta) {
				dismissLast();
			}
		} else if (join.sta) {
			// idle after the lamp restarted: it reports how the switch ended
			applyStaLast(d.last);
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

	// the lamp serves this page over a saved network (not its own access point)
	function inStationMode() {
		return !!state.data && state.data.mode === 'sta';
	}

	// runs next once the lamp's mode is known: the first status poll may not have arrived yet, and a lamp in station mode
	// disappears during a check, so the mode cannot be read afterwards
	function withStatus(next) {
		if (state.data) {
			next();
			return;
		}
		fetchTimed('/api/status', { cache: 'no-store' }, function (r) {
			if (!r.ok) {
				throw new Error(r.status);
			}
			return r.json();
		})
			.then(function (d) {
				state.data = d;
			})
			.catch(function () {})
			.then(next);
	}

	// sta: station mode, the lamp leaves its network for the check. fromSsid is read now: once the lamp is away the status is stale
	function startJoin(name, sta) {
		var fresh = !join;
		join = {
			ssid: name, lostSinceMs: 0, sta: !!sta, switching: false,
			fromSsid: sta && state.data && typeof state.data.ssid === 'string' ? state.data.ssid : ''
		};
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
				if (!d || typeof d !== 'object') {
					return;
				}
				// a failed switch that restarted the lamp: told on both pages until the user closes it
				var l = d.last;
				if (l && l.result === 'fail' && typeof l.ssid === 'string' && l.ssid && !join) {
					lastJoin = { ssid: l.ssid, reason: typeof l.reason === 'string' ? l.reason : '' };
					renderLast();
				}
				if (join || joined || typeof d.ssid !== 'string') {
					return;
				}
				if (d.state === 'checking') {
					withStatus(function () {
						if (!join) {
							startJoin(d.ssid, inStationMode());
						}
					});
				} else if (d.state === 'switching') {
					startJoin(d.ssid, true);
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
		// the mode is read before the request: after it the lamp may already be away
		withStatus(function () {
			fetch('/add_wifi_creds', {
				method: 'POST',
				headers: { 'Content-Type': 'application/x-www-form-urlencoded' },
				body: body
			})
				.then(function (r) {
					return r.text().then(function (t) {
						busy = false;
						// 202: the lamp checks the network before it saves it; the text is for pages without scripts. In
						// station mode the lamp leaves the network this page is served over for the check
						if (r.status === 202) {
							startJoin(name, inStationMode());
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
	byId('lastClose').addEventListener('click', dismissLast);
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
