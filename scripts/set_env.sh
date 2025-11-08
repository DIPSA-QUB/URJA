#!/bin/bash

# ==============================================================================
# This script sets environment variables for URJA-based experiments.
# It configures both NaiveLogger and TridentController.
#
# Usage:
#   source set_env.sh naive
#   source set_env.sh trident
#
# Default: naive
# ==============================================================================

# --- Default Setup ---
export URJA_SCRIPT_DIR="$HOME/urja/scripts"
export PATH="$URJA_SCRIPT_DIR:$PATH"

export URJA_PAPI_EVENTS="PAPI_TOT_CYC,PAPI_TOT_INS,PAPI_L3_TCM"
export URJA_INTERVAL_MS=500
export URJA_ENERGY_BACKEND=sysfs

# --- Logger Selection ---
LOGGER=${1:-naive}   # Default to 'naive' if no argument is provided

# --- NaiveLogger Configuration ---
if [[ "$LOGGER" == "naive" ]]; then
  export URJA_LOGGER=naive

  export URJA_NAIVE_MAX_FREQ="2.80"
  export URJA_NAIVE_MIN_FREQ="0.80"
  export URJA_NAIVE_THRESHOLD="0.01"

  echo "[URJA][ENV] NaiveLogger configuration loaded."
  echo "  MAX_FREQ: $URJA_NAIVE_MAX_FREQ"
  echo "  MIN_FREQ: $URJA_NAIVE_MIN_FREQ"
  echo "  THRESHOLD: $URJA_NAIVE_THRESHOLD"

# --- TridentController Configuration ---
elif [[ "$LOGGER" == "trident" ]]; then
  export URJA_LOGGER=trident

  export URJA_TRIDENT_MAX_FREQ="2.00"
  export URJA_TRIDENT_MID_FREQ="1.40"
  export URJA_TRIDENT_MIN_FREQ="0.80"

  export URJA_TRIDENT_LOWER_THRESHOLD="0.01"
  export URJA_TRIDENT_UPPER_THRESHOLD="0.02"

  echo "[URJA][ENV] TridentController configuration loaded."
  echo "  MAX_FREQ: $URJA_TRIDENT_MAX_FREQ"
  echo "  MID_FREQ: $URJA_TRIDENT_MID_FREQ"
  echo "  MIN_FREQ: $URJA_TRIDENT_MIN_FREQ"
  echo "  LOWER_THRESHOLD: $URJA_TRIDENT_LOWER_THRESHOLD"
  echo "  UPPER_THRESHOLD: $URJA_TRIDENT_UPPER_THRESHOLD"

# --- Invalid Option ---
else
  echo "[URJA][ERROR] Unknown logger type: $LOGGER"
  echo "Usage: source set_env.sh [naive|trident]"
  return 1
fi

echo "[URJA][ENV] URJA_LOGGER set to '$URJA_LOGGER'."