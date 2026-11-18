"use strict";

// Feeds the "inputs" of type "command" in .vscode/tasks.json. The lists come from the bench script
// (--action Sources / Ports, JSON on stdout), so the menu logic exists only once, in Python.
const vscode = require("vscode");
const fs = require("fs");
const path = require("path");
const { execFile } = require("child_process");

const SCRIPT_REL = path.join("Software", "Software_ver_2", "HAQuDA_Esp32", "scripts", "haquda-bench.py");
const SCRIPT_TIMEOUT_MS = 60 * 1000; // the registry lookup takes well under a second

// Bump "version" in package.json on every change of this folder: the warning below compares only
// the version numbers, so an unbumped change never tells installed copies to update.
const INSTALL_TASK = "HAQuDA: Install picker";
const MONITOR_TASK = "HAQuDA: Monitor";
const DEFAULT_TASK = "HAQuDA: Build + Flash + Monitor";
const START_WAIT_MS = 10 * 1000;
let installedVersion;
let versionWarned = false; // at most one warning per window session

// Warns, without blocking the pick, when the installed extension differs from the repo's copy.
// A repo file that cannot be read or parsed gives no warning.
function warnIfOutdated(script) {
	if (versionWarned || !installedVersion) {
		return;
	}
	let repoVersion;
	try {
		const file = path.join(path.dirname(script), "vscode-picker", "package.json");
		repoVersion = JSON.parse(fs.readFileSync(file, "utf8")).version;
	} catch (e) {
		return;
	}
	if (typeof repoVersion !== "string" || repoVersion === installedVersion) {
		return;
	}
	versionWarned = true;
	// Not awaited: the picker must open at once.
	vscode.window
		.showWarningMessage(
			"HAQuDA picker " + installedVersion + " differs from the repo's " + repoVersion +
				". Run the Install picker task, then Reload Window.",
			"Install picker"
		)
		.then((choice) => {
			if (choice === "Install picker") {
				return vscode.commands.executeCommand("workbench.action.tasks.runTask", INSTALL_TASK);
			}
			return undefined;
		})
		.then(undefined, (e) => {
			vscode.window.showErrorMessage("HAQuDA picker: cannot run the " + INSTALL_TASK + " task: " + e.message);
		});
}

// With several workspace folders only the one that holds the script is meaningful.
function findWorkspaceFolder() {
	for (const folder of vscode.workspace.workspaceFolders || []) {
		if (fs.existsSync(path.join(folder.uri.fsPath, SCRIPT_REL))) {
			return folder.uri.fsPath;
		}
	}
	return undefined;
}

function findScript() {
	const folder = findWorkspaceFolder();
	return folder === undefined ? undefined : path.join(folder, SCRIPT_REL);
}

// A tasks.json task of the workspace folder that holds the script; a task of the same label from
// another folder or provider is not ours.
function isOurTask(task, label, folder) {
	const scope = task.scope;
	return (
		task.name === label &&
		task.source === "Workspace" &&
		!!scope && !!scope.uri &&
		scope.uri.fsPath.toLowerCase() === folder.toLowerCase()
	);
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
	warnIfOutdated(script);
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

// The monitor starts from here, not as a dependsOn of the default task: that task would open the
// monitor's terminal at once, before the build's, and the shared panel would show them swapped.
async function startMonitor(folder) {
	if (vscode.tasks.taskExecutions.some((execution) => isOurTask(execution.task, MONITOR_TASK, folder))) {
		return; // a running monitor is kept, it steps aside for the flash by itself
	}
	const task = (await vscode.tasks.fetchTasks()).find((t) => isOurTask(t, MONITOR_TASK, folder));
	if (!task) {
		throw new Error('task "' + MONITOR_TASK + '" not found');
	}
	await vscode.tasks.executeTask(task);
}

// Starts the monitor once the build task's process (and so its terminal) exists, which keeps the
// build on the left of the shared panel. The listener ends on the first match or after
// START_WAIT_MS; a build that never starts (cancelled input, failed launch) leaves no monitor.
function startMonitorAfterBuild(folder) {
	let timer;
	const listener = vscode.tasks.onDidStartTaskProcess((event) => {
		if (!isOurTask(event.execution.task, DEFAULT_TASK, folder)) {
			return;
		}
		clearTimeout(timer);
		listener.dispose();
		startMonitor(folder).catch((e) => {
			// The build runs anyway.
			vscode.window.showErrorMessage("HAQuDA picker: cannot start the " + MONITOR_TASK + " task: " + e.message);
		});
	});
	timer = setTimeout(() => listener.dispose(), START_WAIT_MS);
}

async function pickSource(args) {
	const source = await pick("Sources", "HAQuDA: firmware source", "Folder whose sketches are built", (s) => ({
		label: s.label,
		description: s.path,
		detail: s.last ? "Last used" : undefined,
		value: s.path,
		isLast: s.last,
	}));
	const folder = findWorkspaceFolder();
	// The listener is registered before the command returns, so the build start cannot be missed.
	if (source !== undefined && folder !== undefined && args && args.startMonitor === true) {
		startMonitorAfterBuild(folder);
	}
	return source;
}

function activate(context) {
	installedVersion = context.extension.packageJSON.version;
	context.subscriptions.push(
		vscode.commands.registerCommand("haquda.pickSource", pickSource),
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
