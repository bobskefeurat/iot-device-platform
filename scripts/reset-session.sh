#!/usr/bin/env bash

set -euo pipefail

SCRIPT_DIR="$(cd "$(dirname "${BASH_SOURCE[0]}")" && pwd)"
REPO_ROOT="$(cd "$SCRIPT_DIR/.." && pwd)"
SESSION_NAME="iot"
TEMP_SESSION="${SESSION_NAME}-reset"

tmux kill-session -t "$TEMP_SESSION" 2>/dev/null || true

if [ -n "${TMUX:-}" ]; then
    env -u TMUX "$REPO_ROOT/scripts/dev-session.sh" --session-name "$TEMP_SESSION" --detach-only
    tmux switch-client -t "$TEMP_SESSION"
    tmux run-shell -b "sleep 0.2; tmux kill-session -t \"$SESSION_NAME\" 2>/dev/null || true; tmux rename-session -t \"$TEMP_SESSION\" \"$SESSION_NAME\""
else
    tmux kill-session -t "$SESSION_NAME" 2>/dev/null || true
    exec "$REPO_ROOT/scripts/dev-session.sh"
fi
