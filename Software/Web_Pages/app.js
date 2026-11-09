(function () {
	'use strict';

	var POLL_MS = 5000;
	var info = document.getElementById('info');
	var nets = document.getElementById('nets');
	var msg = document.getElementById('msg');
	var rows = {};
	Array.prototype.forEach.call(document.querySelectorAll('#meas li'), function (li) {
		rows[li.getAttribute('data-k')] = li;
	});

	function uptimeText(s) {
		var d = Math.floor(s / 86400);
		var h = Math.floor((s % 86400) / 3600);
		var m = Math.floor((s % 3600) / 60);
		return (d ? d + ' d ' : '') + (d || h ? h + ' h ' : '') + m + ' min';
	}

	function show(d) {
		var link = d.mode === 'ap' ? 'access point' : (d.rssi === null ? 'WiFi' : 'WiFi ' + d.rssi + ' dBm');
		info.textContent = d.ip + ' | ' + link + ' | up ' + uptimeText(d.uptime) + ' | firmware ' + d.fw;
		Object.keys(rows).forEach(function (k) {
			var m = d.meas[k];
			rows[k].querySelector('.val').textContent = m ? m.v : '--';
			rows[k].querySelector('.dot').style.background = m ? m.c : '';
		});
	}

	function poll() {
		fetch('/api/status', { cache: 'no-store' })
			.then(function (r) {
				if (!r.ok) {
					throw new Error(r.status);
				}
				return r.json();
			})
			.then(show)
			.catch(function () {
				info.textContent = 'The lamp does not answer. Retrying...';
			})
			.then(function () {
				setTimeout(poll, POLL_MS);
			});
	}

	function listNets() {
		fetch('/api/wifi', { cache: 'no-store' })
			.then(function (r) {
				if (!r.ok) {
					throw new Error(r.status);
				}
				return r.json();
			})
			.then(function (list) {
				nets.textContent = '';
				if (!list.length) {
					list = null;
				}
				(list || ['No saved networks']).forEach(function (name) {
					var li = document.createElement('li');
					li.textContent = name;
					if (!list) {
						li.className = 'muted';
					}
					nets.appendChild(li);
				});
			})
			.catch(function () {
				nets.textContent = 'Cannot read the saved networks.';
			});
	}

	document.getElementById('wipe').addEventListener('click', function () {
		if (!confirm('Delete all saved WiFi networks? The lamp then opens its own access point after the next restart.')) {
			return;
		}
		msg.className = 'muted';
		fetch('/delete_wifi_creds', { method: 'POST' })
			.then(function (r) {
				return r.text().then(function (t) {
					msg.className = r.ok ? 'muted' : 'err';
					msg.textContent = t;
				});
			})
			.catch(function () {
				msg.className = 'err';
				msg.textContent = 'The lamp does not answer.';
			})
			.then(listNets);
	});

	poll();
	listNets();
})();
