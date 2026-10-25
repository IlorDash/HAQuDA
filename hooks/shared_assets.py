"""MkDocs hook: publish the lamp pictures from Data/Article/Photos without copying them.

MkDocs only collects files inside docs_dir, so the pictures are registered as generated
files under assets/. Pages write ![...](assets/<name>.png) and MkDocs resolves the link
per page, so one copy in the built site serves both language trees.

The list is narrow on purpose: the folder also holds PDF drawings and photos of an
earlier prototype that have no place in the guide.
"""

from pathlib import Path

from mkdocs.exceptions import PluginError
from mkdocs.structure.files import File

PHOTOS_DIR = Path(__file__).resolve().parent.parent / "Data" / "Article" / "Photos"
PHOTO_PATTERNS = ("Assembly.png", "Multi-mode.png", "Night_*.png")


def on_files(files, config):
    for pattern in PHOTO_PATTERNS:
        sources = sorted(PHOTOS_DIR.glob(pattern))
        if not sources:
            raise PluginError(f"No file matches {pattern} in {PHOTOS_DIR}")
        for source in sources:
            files.append(File.generated(config, f"assets/{source.name}", abs_src_path=str(source)))
    return files
