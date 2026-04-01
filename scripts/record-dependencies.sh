#!/usr/bin/env bash
# record-dependencies.sh
#
# Usage: bash scripts/record-dependencies.sh <ADDONS_DIR> <PLATFORM>
#
# Reads dependencies.json from the repo root, inspects each listed addon
# directory that is already present on the runner, and records its actual
# HEAD commit SHA and ref into a Markdown report file:
#   dependencies-report-<PLATFORM>.txt
#
# Does NOT clone or modify any addon directory (Phase 1 — logging only).
# Run this immediately after checking out ofxEmotiBit in CI so the record
# is written before any build starts.

set -euo pipefail

if [ $# -lt 2 ]; then
    echo "Usage: $0 <ADDONS_DIR> <PLATFORM>"
    exit 1
fi

ADDONS_DIR="$1"
PLATFORM="$2"

SCRIPT_DIR="$(cd "$(dirname "${BASH_SOURCE[0]}")" && pwd)"
REPO_ROOT="$(dirname "$SCRIPT_DIR")"
DEPS_JSON="$REPO_ROOT/dependencies.json"
REPORT="$REPO_ROOT/dependencies-report-${PLATFORM}.txt"

if [ ! -f "$DEPS_JSON" ]; then
    echo "ERROR: dependencies.json not found at $DEPS_JSON"
    exit 1
fi

OFXEMOTIBIT_COMMIT=$(git -C "$REPO_ROOT" rev-parse HEAD)
OFXEMOTIBIT_REF=$(git -C "$REPO_ROOT" symbolic-ref --short HEAD 2>/dev/null || echo "(detached)")

cat > "$REPORT" <<EOF
# EmotiBit Dependency Report — ${PLATFORM}
# Generated: $(date -u '+%Y-%m-%dT%H:%M:%SZ')
# ofxEmotiBit ref:    ${OFXEMOTIBIT_REF}
# ofxEmotiBit commit: ${OFXEMOTIBIT_COMMIT}

| dependency | requested_ref | resolved_commit | resolved_ref |
|---|---|---|---|
EOF

python3 -c "
import json, sys
with open('$DEPS_JSON') as f:
    for d in json.load(f)['dependencies']:
        print(d['name'] + ' ' + d['ref'])
" | while IFS=' ' read -r name ref; do
    target="$ADDONS_DIR/$name"
    if [ -d "$target/.git" ]; then
        resolved_commit=$(git -C "$target" rev-parse HEAD)
        resolved_ref=$(git -C "$target" symbolic-ref --short HEAD 2>/dev/null \
                       || git -C "$target" describe --tags --exact-match HEAD 2>/dev/null \
                       || echo "(detached)")
    else
        resolved_commit="NOT FOUND"
        resolved_ref="NOT FOUND"
    fi
    echo "| $name | \`$ref\` | \`$resolved_commit\` | \`$resolved_ref\` |" >> "$REPORT"
done

echo ""
echo "Dependency report written to: $REPORT"
echo ""
cat "$REPORT"
