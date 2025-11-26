#!/bin/bash

# Paths
SCRIPT_DIR="$(cd "$(dirname "${BASH_SOURCE[0]}")" && pwd)"
ADDON_ROOT="$(cd "$SCRIPT_DIR/../../.." && pwd)"
CONFIG_FILE="$ADDON_ROOT/EmotiBitOscilloscope/bin/data/emotibitCommSettings.json"
APP="$ADDON_ROOT/EmotiBitOscilloscope/bin/EmotiBitOscilloscope_debug.exe"
TIMEOUT=5

# ===== EDIT CHECK PHRASES HERE =====
PHRASES_TRUE=("auxillary control enabled")
PHRASES_FALSE=("auxillary control disabled")
PHRASES_REMOVED=("Auxillary control settings not specified in the settings file")
# ====================================

FAILED=0

# Backup original config
cp "$CONFIG_FILE" "${CONFIG_FILE}.backup"
trap "cp ${CONFIG_FILE}.backup $CONFIG_FILE; rm ${CONFIG_FILE}.backup" EXIT

# Remove comments from JSON and save clean version
sed 's|//.*||g' "$CONFIG_FILE" | sed '/^[[:space:]]*$/d' > "${CONFIG_FILE}.tmp"
mv "${CONFIG_FILE}.tmp" "$CONFIG_FILE"

# Test function
run_test() {
    local name=$1
    local -n phrases=$2

    echo "=== $name ==="

    # Run app and capture output
    local output=$(timeout ${TIMEOUT}s "$APP" 2>&1 || true)

    # Print app output
    echo "--- App Output ---"
    echo "$output"
    echo "--- End Output ---"
    echo ""

    # Check phrases
    local passed=1
    for phrase in "${phrases[@]}"; do
        if echo "$output" | grep -qi "$phrase"; then
            echo "✓ Found: '$phrase'"
        else
            echo "✗ Missing: '$phrase'"
            passed=0
        fi
    done

    if [ $passed -eq 0 ]; then
        echo "FAILED"
        ((FAILED++))
    else
        echo "PASSED"
    fi
    echo ""
}

# Test 1: auxillaryControl = true
jq '.auxillaryControl = true' "$CONFIG_FILE" > "${CONFIG_FILE}.tmp" && mv "${CONFIG_FILE}.tmp" "$CONFIG_FILE"
run_test "Test 1: auxillaryControl = true" PHRASES_TRUE

# Test 2: auxillaryControl = false
jq '.auxillaryControl = false' "$CONFIG_FILE" > "${CONFIG_FILE}.tmp" && mv "${CONFIG_FILE}.tmp" "$CONFIG_FILE"
run_test "Test 2: auxillaryControl = false" PHRASES_FALSE

# Test 3: auxillaryControl removed
jq 'del(.auxillaryControl)' "$CONFIG_FILE" > "${CONFIG_FILE}.tmp" && mv "${CONFIG_FILE}.tmp" "$CONFIG_FILE"
run_test "Test 3: auxillaryControl removed" PHRASES_REMOVED

# Exit with appropriate code
echo "Tests failed: $FAILED"
exit $FAILED
