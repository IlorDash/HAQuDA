"""MkDocs hook: the firmware version lives in one place and is read at build time.

The version is the FW_VERSION constant in the firmware's HAQuDA_Version.h. This hook
- appends it to the site title, so every page states which release it documents;
- replaces {{ app_version }} in Markdown pages;
- sets the copyright line to "HAQuDA v<version>".

It is a hook rather than a CI variable so `mkdocs serve` shows the same version
locally as the workflow publishes.
"""

import re
from pathlib import Path

from mkdocs.exceptions import PluginError

VERSION_HEADER = Path(__file__).resolve().parent.parent / "Software" / "Software_ver_2" / "HAQuDA_Esp32" / "sketches" / "HAQuDA_Version.h"
VERSION_PATTERN = re.compile(r'FW_VERSION\s*\[\s*\]\s*=\s*"([^"]+)"')
PLACEHOLDER = "{{ app_version }}"

_version = ""


def _read_version():
    try:
        text = VERSION_HEADER.read_text(encoding="utf-8")
    except OSError as error:
        raise PluginError(f"Cannot read the firmware version from {VERSION_HEADER}: {error}")
    match = VERSION_PATTERN.search(text)
    if match is None:
        raise PluginError(f"FW_VERSION not found in {VERSION_HEADER}")
    return match.group(1)


def on_config(config):
    global _version
    _version = _read_version()
    # No translatable text, so the line reads the same in both language trees.
    config["copyright"] = f"HAQuDA v{_version}"
    return config


def on_page_markdown(markdown, page, config, files):
    return markdown.replace(PLACEHOLDER, _version)


def on_page_context(context, page, config, nav):
    # mkdocs-static-i18n swaps site_name for the per-language one after on_config, so the
    # suffix has to be appended per page. The same config object is reused for every page
    # of a language, hence the guard against appending twice.
    page_config = context["config"]
    suffix = f" v{_version}"
    if not page_config["site_name"].endswith(suffix):
        page_config["site_name"] = page_config["site_name"] + suffix
    return context
