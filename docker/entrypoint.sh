#!/usr/bin/env bash
# =============================================================================
# Development Container Entrypoint
# Provides setup command and default shell
# =============================================================================
set -euo pipefail

# Handle 'setup' command specially to use the setup script
if [ "${1:-}" = "setup" ]; then
    shift || true
    exec /usr/local/bin/setup "$@"
fi

# Default: execute the provided command
exec "$@"
