#!/usr/bin/env bash
# Refuse a release while a release-notes page is still marked unreleased.
#
# Usage: ci/check-release-markers.sh [docs-dir]        (default: docs)
#
# Run it yourself before opening a release merge request:
#
#   bash ci/check-release-markers.sh
#
# Exit status is 1 when a marker is found, and also when the search itself
# fails: a check that cannot read the files must not be mistaken for a clean
# result, because everything downstream trusts it.
set -euo pipefail

docs_dir="${1:-docs}"

# grep's exit code carries three distinct meanings here, so it is captured and
# branched on rather than folded into a plain `|| true`, which would report a
# failed search as "nothing found" and let a marked page through.
set +e
offending=$(grep -rliE --include='*.md' -- '<!--[[:space:]]*unreleased' "$docs_dir")
grep_exit_code=$?
set -e

case "$grep_exit_code" in
  0)
    echo "error: an unreleased marker is still present in:"
    echo "$offending"
    echo "       set the release date and remove the marker and the banner above it"
    exit 1
    ;;
  1)
    echo "release notes OK: no unreleased markers under $docs_dir"
    ;;
  *)
    echo "error: could not search $docs_dir for release markers (grep exit $grep_exit_code)"
    exit 1
    ;;
esac
