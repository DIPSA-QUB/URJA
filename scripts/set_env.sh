#!/bin/bash

# ==============================================================================
# This script sets environment variables for the NaiveLogger application.
# It should be sourced, not executed directly.
# Example: source set_env.sh
# ==============================================================================
export URJA_SCRIPT_DIR="$HOME/urja/scripts"
export PATH="$URJA_SCRIPT_DIR:$PATH"

export URJA_PAPI_EVENTS=PAPI_TOT_CYC,PAPI_TOT_INS,PAPI_L3_TCM
export URJA_INTERVAL_MS=500
export URJA_LOGGER=naive

export URJA_NAIVE_MAX_FREQ="2.80"
export URJA_NAIVE_MIN_FREQ="0.80"
export URJA_NAIVE_THRESHOLD="0.0001"

echo "Environment variables for NaiveLogger have been set."
