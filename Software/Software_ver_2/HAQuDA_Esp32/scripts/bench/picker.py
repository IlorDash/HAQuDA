"""Packs the VS Code picker extension (scripts/vscode-picker) into a .vsix and installs it."""
import json
import os
import shutil
import subprocess
import zipfile

from . import output

EXTENSION_DIR = os.path.join(os.path.dirname(os.path.dirname(os.path.abspath(__file__))), "vscode-picker")
FILES = ("package.json", "extension.js")

CONTENT_TYPES = (
    '<?xml version="1.0" encoding="utf-8"?>\n'
    '<Types xmlns="http://schemas.openxmlformats.org/package/2006/content-types">'
    '<Default Extension=".json" ContentType="application/json"/>'
    '<Default Extension=".js" ContentType="application/javascript"/>'
    '<Default Extension=".vsixmanifest" ContentType="text/xml"/>'
    "</Types>\n")

MANIFEST = (
    '<?xml version="1.0" encoding="utf-8"?>\n'
    '<PackageManifest Version="2.0.0" xmlns="http://schemas.microsoft.com/developer/vsx-schema/2011">\n'
    "  <Metadata>\n"
    '    <Identity Language="en-US" Id="%(name)s" Version="%(version)s" Publisher="%(publisher)s"/>\n'
    "    <DisplayName>%(displayName)s</DisplayName>\n"
    '    <Description xml:space="preserve">%(description)s</Description>\n'
    "    <Categories>Other</Categories>\n"
    "    <Properties>\n"
    '      <Property Id="Microsoft.VisualStudio.Code.Engine" Value="%(engine)s"/>\n'
    "    </Properties>\n"
    "  </Metadata>\n"
    '  <Installation><InstallationTarget Id="Microsoft.VisualStudio.Code"/></Installation>\n'
    "  <Dependencies/>\n"
    '  <Assets><Asset Type="Microsoft.VisualStudio.Code.Manifest" Path="extension/package.json" Addressable="true"/></Assets>\n'
    "</PackageManifest>\n")


def _xml_escape(text):
    return text.replace("&", "&amp;").replace("<", "&lt;").replace(">", "&gt;")


def pack_vsix(bench):
    """Writes <bench>\\<name>-<version>.vsix and returns its path."""
    with open(os.path.join(EXTENSION_DIR, "package.json"), encoding="utf-8") as f:
        meta = json.load(f)
    fields = {key: _xml_escape(str(meta[key])) for key in ("name", "version", "publisher", "displayName", "description")}
    fields["engine"] = _xml_escape(meta["engines"]["vscode"])
    vsix = os.path.join(bench, "%s-%s.vsix" % (meta["name"], meta["version"]))
    os.makedirs(bench, exist_ok=True)
    with zipfile.ZipFile(vsix, "w", zipfile.ZIP_DEFLATED) as z:
        z.writestr("[Content_Types].xml", CONTENT_TYPES)
        z.writestr("extension.vsixmanifest", MANIFEST % fields)
        for name in FILES:
            z.write(os.path.join(EXTENSION_DIR, name), "extension/" + name)
    return vsix


def install(bench):
    vsix = pack_vsix(bench)
    output.line("Packed " + vsix)
    code = shutil.which("code")
    if not code:
        output.warn("'code' is not on PATH. Install the extension by hand, in a terminal where VS Code's 'code' works:")
        output.line('code --install-extension "%s" --force' % vsix)
        return 1
    # check=False: the exit code and the message of the code CLI are passed on unchanged.
    done = subprocess.run([code, "--install-extension", vsix, "--force"], check=False)
    if done.returncode != 0:
        return done.returncode
    output.line("Installed. Run 'Developer: Reload Window' in VS Code once, then start a HAQuDA task.")
    return 0
