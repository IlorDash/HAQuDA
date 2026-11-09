(function () {
	'use strict';

	var form = document.getElementById('form');
	var save = document.getElementById('save');
	var msg = document.getElementById('msg');
	var pass = document.getElementById('pass');

	document.getElementById('showPass').addEventListener('change', function (e) {
		pass.type = e.target.checked ? 'text' : 'password';
	});

	form.addEventListener('submit', function (e) {
		e.preventDefault();
		// maxlength counts characters, the lamp limit is bytes: a long multi-byte name is refused with a message
		save.disabled = true;
		msg.className = 'muted';
		msg.textContent = 'Saving...';
		fetch('/add_wifi_creds', {
			method: 'POST',
			headers: { 'Content-Type': 'application/x-www-form-urlencoded' },
			body: new URLSearchParams(new FormData(form)).toString()
		})
			.then(function (r) {
				return r.text().then(function (t) {
					msg.className = r.ok ? 'muted' : 'err';
					msg.textContent = t;
					save.disabled = r.ok;
				});
			})
			.catch(function () {
				msg.className = 'err';
				msg.textContent = 'The lamp does not answer.';
				save.disabled = false;
			});
	});
})();
