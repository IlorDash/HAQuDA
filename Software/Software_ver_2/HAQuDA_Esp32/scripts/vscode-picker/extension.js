"use strict";

// Feeds the "inputs" of type "command" in .vscode/tasks.json. The lists come from the bench script
// (--action Sources / Ports, JSON on stdout), so the menu logic exists only once, in Python.
const vscode = require("vscode");
const fs = require("fs");
const path = require("path");
const { execFile } = require("child_process");

const SCRIPT_REL = path.join("Software", "Software_ver_2", "HAQuDA_Esp32", "scripts", "haquda-bench.py");
const SCRIPT_TIMEOUT_MS = 60 * 1000; // the registry lookup takes well under a second

// With several workspace folders only the one that holds the script is meaningful.
function findScript() {
	for (const folder of vscode.workspace.workspaceFolders || []) {
		const script = path.join(folder.uri.fsPath, SCRIPT_REL);
		if (fs.existsSync(script)) {
			return script;
		}
	}
	return undefined;
}

function runScript(script, action) {
	return new Promise((resolve, reject) => {
		const options = {
			windowsHide: true,
			timeout: SCRIPT_TIMEOUT_MS,
			// Python writes stderr in the console code page otherwise; the user reads it in a popup.
			env: { ...process.env, PYTHONIOENCODING: "utf-8" },
		};
		execFile("py", ["-3", script, "--action", action], options, (error, stdout, stderr) => {
			if (error) {
				reject(new Error(stderr.trim() || error.message));
				return;
			}
			let list;
			try {
				list = JSON.parse(stdout);
			} catch (e) {
				list = undefined;
			}
			if (Array.isArray(list)) {
				resolve(list);
			} else {
				reject(new Error("The script printed no JSON list: " + stdout.trim().slice(0, 200)));
			}
		});
	});
}

// Resolves to the chosen item's value, undefined on Escape (VS Code then cancels the task).
function showPicker(title, placeholder, items) {
	return new Promise((resolve) => {
		const picker = vscode.window.createQuickPick();
		picker.title = title;
		picker.placeholder = placeholder;
		picker.items = items;
		picker.matchOnDescription = true; // typing a path fragment or "Bluetooth" finds the entry
		picker.matchOnDetail = true;
		const last = items.find((item) => item.isLast);
		if (last) {
			picker.activeItems = [last];
		}
		picker.ignoreFocusOut = true; // a click elsewhere must not cancel the build
		let chosen;
		picker.onDidAccept(() => {
			chosen = picker.selectedItems[0] || picker.activeItems[0];
			picker.hide();
		});
		picker.onDidHide(() => {
			picker.dispose();
			resolve(chosen ? chosen.value : undefined);
		});
		picker.show();
	});
}

async function pick(action, title, placeholder, toItem) {
	const script = findScript();
	if (!script) {
		vscode.window.showErrorMessage("HAQuDA picker: " + SCRIPT_REL + " is not in any workspace folder.");
		return undefined;
	}
	let items;
	try {
		// Mapping stays inside the try: an entry of an unexpected shape ends in the error popup too.
		items = (await runScript(script, action)).map(toItem);
	} catch (e) {
		vscode.window.showErrorMessage("HAQuDA picker (" + action + "): " + e.message);
		return undefined;
	}
	if (items.length === 0) {
		vscode.window.showErrorMessage("HAQuDA picker: nothing to choose from (" + action + ").");
		return undefined;
	}
	return showPicker(title, placeholder, items);
}

function activate(context) {
	context.subscriptions.push(
		vscode.commands.registerCommand("haquda.pickSource", () =>
			pick("Sources", "HAQuDA: firmware source", "Folder whose sketches are built", (s) => ({
				label: s.label,
				description: s.path,
				detail: s.last ? "Last used" : undefined,
				value: s.path,
				isLast: s.last,
			}))
		),
		vscode.commands.registerCommand("haquda.pickPort", () =>
			pick("Ports", "HAQuDA: COM port", "Port of the lamp", (p) => ({
				label: p.name,
				description: p.present ? p.label : "not present now",
				detail: p.last ? "Stored" : undefined,
				value: p.name,
				isLast: p.last,
			}))
		)
	);
}

function deactivate() {}

module.exports = { activate, deactivate };
