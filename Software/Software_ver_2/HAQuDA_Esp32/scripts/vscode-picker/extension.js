"use strict";

// Feeds the "inputs" of type "command" in .vscode/tasks.json. The lists come from the bench script
// (--action Sources / Ports, JSON on stdout), so the menu logic exists only once, in Python.
const vscode = require("vscode");
const fs = require("fs");
const path = require("path");
const { execFile } = require("child_process");

const SCRIPT_REL = path.join("Software", "Software_ver_2", "HAQuDA_Esp32", "scripts", "haquda-bench.py");
const SCRIPT_TIMEOUT_MS = 60 * 1000; // the registry lookup takes well under a second

// Bump "version" in package.json on every change of this folder: the block below compares only
// the version numbers, so an unbumped change never tells installed copies to update.
const INSTALL_TASK = "HAQuDA: Install picker";
const MONITOR_TASK = "HAQuDA: Monitor";
const DEFAULT_TASK = "HAQuDA: Build + Flash + Monitor";
const START_WAIT_MS = 10 * 1000;
const INSTALL_WAIT_MS = 10 * 60 * 1000; // the install is short; a task that never ends stops being watched
const STOP_WAIT_MS = 5 * 1000; // a monitor that does not end by then no longer holds up the next start
let installedVersion;

// The repo's picker version when it differs from the installed one; undefined when they match or when
// the repo file cannot be read or parsed (no block then).
function outdatedAgainst(script) {
	if (!installedVersion) {
		return undefined;
	}
	let repoVersion;
	try {
		const file = path.join(path.dirname(script), "vscode-picker", "package.json");
		repoVersion = JSON.parse(fs.readFileSync(file, "utf8")).version;
	} catch (e) {
		return undefined;
	}
	return typeof repoVersion === "string" && repoVersion !== installedVersion ? repoVersion : undefined;
}

// The one live watch for the end of the Install picker task: its listener and the timeout that drops it.
let installWatch;

function disposeInstallWatch() {
	if (installWatch) {
		clearTimeout(installWatch.timer);
		installWatch.listener.dispose();
		installWatch = undefined;
	}
}

// Runs the Install picker task and offers the window reload when it succeeds. Not awaited by the caller.
async function installPicker() {
	const folder = findWorkspaceFolder();
	disposeInstallWatch(); // a listener of an earlier install must not report a second time
	// Registered before the task starts, so its end cannot be missed.
	const listener = vscode.tasks.onDidEndTaskProcess((event) => {
		if (folder === undefined || !isOurTask(event.execution.task, INSTALL_TASK, folder)) {
			return;
		}
		disposeInstallWatch();
		if (event.exitCode === 0) {
			vscode.window.showInformationMessage("HAQuDA picker installed.", { modal: true }, "Reload Window").then((choice) => {
				if (choice === "Reload Window") {
					vscode.commands.executeCommand("workbench.action.reloadWindow");
				}
			});
		} else if (event.exitCode !== undefined) {
			vscode.window.showErrorMessage("HAQuDA picker: the " + INSTALL_TASK + " task failed (exit code " + event.exitCode + ").");
		}
	});
	installWatch = { listener, timer: setTimeout(disposeInstallWatch, INSTALL_WAIT_MS) };
	try {
		await vscode.commands.executeCommand("workbench.action.tasks.runTask", INSTALL_TASK);
	} catch (e) {
		disposeInstallWatch();
		vscode.window.showErrorMessage("HAQuDA picker: cannot run the " + INSTALL_TASK + " task: " + e.message);
	}
}

// True when the pick must not go on: the installed picker is outdated. A modal message says so; the
// caller returns undefined, which makes VS Code cancel the task.
async function blockedAsOutdated(script) {
	const repoVersion = outdatedAgainst(script);
	if (repoVersion === undefined) {
		return false;
	}
	const choice = await vscode.window.showWarningMessage(
		"HAQuDA picker " + installedVersion + " is outdated, the repo has " + repoVersion + ". The task was not started.",
		{ modal: true },
		"Install picker"
	);
	if (choice === "Install picker") {
		installPicker();
	}
	return true;
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
	if (await blockedAsOutdated(script)) {
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

// Resolves when the stop of the monitor ends (its task end event) or after STOP_WAIT_MS; undefined
// while no stop is under way.
let pendingStop;
let liveBuilds = 0; // build processes of the default task that started and have not ended yet

// Terminates the running monitor task(s) of the folder. Called when the build is terminated (a restart
// closes only the build's terminal): a monitor left alone in the group would make the next build open
// its terminal to the right of the monitor.
function stopMonitors(folder) {
	const running = vscode.tasks.taskExecutions.filter((execution) => isOurTask(execution.task, MONITOR_TASK, folder));
	if (running.length === 0) {
		return;
	}
	const waits = running.map(
		(execution) =>
			new Promise((resolve) => {
				// Registered before terminate(), so the end event cannot be missed.
				const listener = vscode.tasks.onDidEndTask((event) => {
					if (isOurTask(event.execution.task, MONITOR_TASK, folder)) {
						done();
					}
				});
				const timer = setTimeout(done, STOP_WAIT_MS);
				function done() {
					clearTimeout(timer);
					listener.dispose();
					resolve();
				}
				try {
					execution.terminate();
				} catch (e) {
					done(); // nothing to wait for: a failed stop must not leave a rejected pendingStop
				}
			})
	);
	const finish = () => {
		if (pendingStop === stop) {
			pendingStop = undefined;
		}
	};
	const stop = Promise.all([pendingStop, ...waits]).then(finish, finish);
	pendingStop = stop;
}

// The monitor starts from here, not as a dependsOn of the default task: that task would open the
// monitor's terminal at once, before the build's, and the shared panel would show them swapped.
async function startMonitor(folder) {
	// A restart starts the new build while the old monitor is still ending: wait for it, otherwise it
	// would still be listed below and no new monitor would start.
	if (pendingStop) {
		await pendingStop;
	}
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
		// exitCode is undefined only when the task was terminated (stop or restart of the build); any
		// number, also a failure or Ctrl+C in the terminal, is a normal end and keeps the monitor running.
		// Counts the live build processes of the default task. A late end event of an old build, after the
		// new one has started, leaves the counter above 0: the monitor in the group belongs to the new build.
		vscode.tasks.onDidStartTaskProcess((event) => {
			const folder = findWorkspaceFolder();
			if (folder !== undefined && isOurTask(event.execution.task, DEFAULT_TASK, folder)) {
				liveBuilds++;
			}
		}),
		vscode.tasks.onDidEndTaskProcess((event) => {
			const folder = findWorkspaceFolder();
			if (folder === undefined || !isOurTask(event.execution.task, DEFAULT_TASK, folder)) {
				return;
			}
			liveBuilds = Math.max(0, liveBuilds - 1);
			if (event.exitCode === undefined && liveBuilds === 0) {
				stopMonitors(folder);
			}
		}),
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

function deactivate() {
	disposeInstallWatch();
}

module.exports = { activate, deactivate };
