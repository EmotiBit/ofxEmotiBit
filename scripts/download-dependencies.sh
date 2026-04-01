#!/usr/bin/env bash
# download-dependencies.sh
#
# Usage:
#   bash scripts/download-dependencies.sh <ADDONS_DIR>           # clone all deps
#   bash scripts/download-dependencies.sh --cleanup <ADDONS_DIR> # remove all dep clones
#
# Reads dependencies.json from the repo root. Each dependency is cloned into
# ADDONS_DIR at the specified ref (branch, tag, or commit SHA), mirroring how
# ofxEmotiBit itself is checked out and placed in ADDONS_DIR by the CI workflow.
#
# Phase 2: enforces the manifest. Run before record-dependencies.sh so the
# report reflects exactly what was downloaded.

set -euo pipefail

# --- argument parsing ---
CLEANUP=false
if [ "$1" = "--cleanup" ]; then
    CLEANUP=true
    ADDONS_DIR="$2"
else
    ADDONS_DIR="$1"
fi

SCRIPT_DIR="$(cd "$(dirname "${BASH_SOURCE[0]}")" && pwd)"
REPO_ROOT="$(dirname "$SCRIPT_DIR")"
# On Windows, Git Bash uses MSYS2 paths (/c/...) but native Windows Python
# requires Windows-style paths (C:/...). pwd -W returns the Windows-style
# path in Git Bash; on macOS/Linux it is unrecognized and we fall back to pwd.
REPO_ROOT_PY="$(cd "$REPO_ROOT" && pwd -W 2>/dev/null || pwd)"
DEPS_JSON="$REPO_ROOT/dependencies.json"
DEPS_JSON_PY="$REPO_ROOT_PY/dependencies.json"

if [ ! -f "$DEPS_JSON" ]; then
    echo "ERROR: dependencies.json not found at $DEPS_JSON"
    exit 1
fi

# --- cleanup mode: remove all dep directories ---
if [ "$CLEANUP" = true ]; then
    echo "Cleaning up cloned dependencies from $ADDONS_DIR"
    python3 -c "
import json
with open('$DEPS_JSON_PY') as f:
    for d in json.load(f)['dependencies']:
        print(d['name'])
" | tr -d '\r' | while read -r name; do
        target="$ADDONS_DIR/$name"
        if [ -d "$target" ]; then
            echo "  removing $name"
            rm -rf "$target"
        fi
    done
    echo "Cleanup complete."
    exit 0
fi

# --- download mode: clone each dep at specified ref ---
echo "Downloading dependencies into $ADDONS_DIR"

python3 -c "
import json
with open('$DEPS_JSON_PY') as f:
    for d in json.load(f)['dependencies']:
        print(d['name'] + ' ' + d['repo'] + ' ' + d['ref'])
" | tr -d '\r' | while IFS=' ' read -r name repo ref; do
    target="$ADDONS_DIR/$name"

    if [ -d "$target" ]; then
        echo "==> $name: removing existing clone"
        rm -rf "$target"
    fi

    echo "==> $name: cloning $repo at ref '$ref'"
    git clone "$repo" "$target"
    git -C "$target" checkout "$ref"
    echo "    resolved: $(git -C "$target" rev-parse HEAD)"
done

echo ""
echo "All dependencies downloaded."
