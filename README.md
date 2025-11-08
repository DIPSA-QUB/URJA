# URJA: Unified Runtime Job Analyzer

**URJA** is a lightweight dynamic monitoring library designed to log per-thread hardware performance counters (via PAPI) and energy consumption (via RAPL) for multithreaded applications with no code changes to the target program.

It also provides simple, built-in dynamic frequency scaling policies based on runtime metrics (e.g., L3 cache miss ratio).

## Requirements

- Linux
- papi (6.0+)
- C++17

### Assumptions

- You have installed PAPI and exported `PAPI_DIR` pointing to its installation location,
- The target program uses `pthread_create` for thread spawning,
- For shell energy backend - file `/path/to/rapl_read.sh` is available and accessible (see below).

## Installation

Install PAPI and point to it

```bash
export PAPI_DIR=/path/to/papi/
```

Build the shared library

```bash
make
```

### Running

There are two primary ways to run your application with **URJA**.

#### Method 1: Using the urja Wrapper (Recommended)

This method uses a wrapper script to automatically set the `LD_PRELOAD` path for you.

Source the environment script once per session to add the urja wrapper to your PATH:

For **Naive Controller** (default):

```bash
source set_env.sh
# or
source set_env.sh naive
```

For **Trident Controller**:

```bash
source set_env.sh trident
```

> ⚠️ Ensure that the `URJA_SCRIPT_DIR` variable inside `scripts/set_env.sh` points to the correct path

> ⚠️ Set your desired configuration options in the script (see the next section).

Run your application via the wrapper:

```bash
urja /path/to/your_app
```

#### Method 2: Manual LD_PRELOAD

You can also manually preload the library.

Set your desired configuration options (see below).

Run your application, specifying `LD_PRELOAD` directly:

```bash
LD_PRELOAD=build/bin/liburja.so /path/to/your_app
```

## Configuration (Environment Variables)

**URJA** is configured entirely through environment variables.

### General Settings

#### URJA_PAPI_EVENTS
- Description: Comma-separated list of PAPI events to monitor.
- Example: `export URJA_PAPI_EVENTS=PAPI_TOT_CYC,PAPI_TOT_INS,PAPI_L3_TCM`

#### URJA_INTERVAL_MS
- Description: The monitoring and logging interval in milliseconds.
- Example: `export URJA_INTERVAL_MS=200`

#### URJA_ENERGY_BACKEND
- Description: Selects the backend for reading energy consumption.
- Options:
    - `sysfs`: (Recommended) Reads from `/sys/class/powercap/intel-rapl`.
    - `shell`: Uses the deprecated `rapl_read.sh` script (requires root).
- Example: `export URJA_ENERGY_BACKEND=sysfs`

### Logger & Policy Selection

#### URJA_LOGGER
- Description: Selects the output logger or dynamic scaling policy.
- Options:
    - `stdio`: (Default) Prints monitoring data to stdout.
    - `file`: Writes monitoring data to a specified log file.
    - `naive`: Enables a simple dynamic frequency scaling policy.
    - `trident`: Enables a 3-level dynamic frequency scaling policy.
- Example: `export URJA_LOGGER=naive`

### Logger-Specific Settings

#### LOGGER: **file**
##### `URJA_LOG_FILE`
- Description: The full path for the output log file when using `URJA_LOGGER=file`.
- Example: `export URJA_LOG_FILE=/tmp/urja.log`

#### LOGGER: **naive** (Naive Policy)
This policy adjusts CPU frequency between two levels (min/max) based on a single threshold.

##### `URJA_NAIVE_THRESHOLD`
- Description: The performance counter ratio (e.g., L3 miss ratio) threshold to trigger frequency changes.
- Example: `export URJA_NAIVE_THRESHOLD=0.005`

##### `URJA_NAIVE_MIN_FREQ`
- Description: The minimum frequency (in kHz) for the policy.
- Example: `export URJA_NAIVE_MIN_FREQ=800000`

##### `URJA_NAIVE_MAX_FREQ`
- Description: The maximum frequency (in kHz) for the policy.
- Example: `export URJA_NAIVE_MAX_FREQ=2800000`

#### LOGGER: **trident** (Trident Policy)
This policy adjusts CPU frequency between three levels (min/mid/max) based on two thresholds.

##### `URJA_TRIDENT_LOWER_THRESHOLD`
- Description: The lower ratio threshold. If below this, frequency is set to `MIN_FREQ`.
- Example: `export URJA_TRIDENT_LOWER_THRESHOLD=0.01`

##### `URJA_TRIDENT_UPPER_THRESHOLD`
- Description: The upper ratio threshold. If above this, frequency is set to `MAX_FREQ`. (If between lower and upper, MID_FREQ is used).
- Example: `export URJA_TRIDENT_UPPER_THRESHOLD=0.02`

##### `URJA_TRIDENT_MIN_FREQ`
- Description: The minimum frequency (in kHz).
- Example: `export URJA_TRIDENT_MIN_FREQ=800000`

##### `URJA_TRIDENT_MID_FREQ`
- Description: The middle frequency (in kHz).
- Example: `export URJA_TRIDENT_MID_FREQ=1400000`

##### `URJA_TRIDENT_MAX_FREQ`
- Description: The maximum frequency (in kHz).
- Example: `export URJA_TRIDENT_MAX_FREQ=2000000`

## Set perf_event_paranoid

To allow URJA (via PAPI) to access hardware counters, make sure:

```bash
cat /proc/sys/kernel/perf_event_paranoid
```

Returns 2 or less (e.g., 2, 1, 0, or -1).

Use the following command to lower it temporarily:

```bash
sudo sh -c 'echo 2 > /proc/sys/kernel/perf_event_paranoid'
```

### Requirements for `URJA_ENERGY_BACKEND=shell`

> ⚠️ This is not recommended and will be depricated in the future.

You only need to run URJA with root access if you choose the shell energy backend (e.g., `/path/to/rapl_read.sh`).

In that case, the script must be executable and present in the sudoers list without password prompt to allow non-interactive execution.

For example, add this line to your `/etc/sudoers` file using `visudo`:

```bash
your_username ALL=(ALL) NOPASSWD: /path/to/rapl_read.sh
```

Make sure the script is readable and executable:

```bash
sudo chmod +x /path/to/rapl_read.sh
```

The source code for /path/to/rapl_read.sh is:

```bash
#!/bin/sh

MODE="$1"

if [ -z "$MODE" ]; then
    printf '%-20s; %-20s; %-15s; %20s; %20s\n' "name" "socket:domain_id"  "domain" "energy_uj"  "max_energy_uj"
    for f in `find /sys/class/powercap/intel-rapl\:* | grep -P "\d+"`; do
        name=`echo $f | rev | cut -d/ -f1 | rev`
        id=`echo $name | cut -d: -f2,3`;
        domain=`cat $f/name`
        energy=`cat $f/energy_uj`
        max_energy=`cat $f/max_energy_range_uj`
        printf '%-20s; %-20s; %-15s; %20s; %20s\n' ${name} ${id} ${domain} ${energy} ${max_energy}
    done
    exit 0
fi

for f in $(find /sys/class/powercap/intel-rapl\:* | grep -P "\d+"); do
    domain_name=$(basename "$f")
    case "$MODE" in
        -h|--help)
            echo "Usage: $0 [-n|--name | -m|--max | -i|--instant]"
            echo "  -n, --name      Print <name>-<domain>"
            echo "  -m, --max       Print max_energy_uj values"
            echo "  -i, --instant   Print current energy_uj values"
            exit 0
            ;;
        -n|--name)
            domain=$(cat "$f/name" | tr ' ' '-')
            echo "${domain_name}-${domain}"
            ;;
        -m|--max)
            cat "$f/max_energy_range_uj"
            ;;
        -i|--instant)
            cat "$f/energy_uj"
            ;;
        *)
            echo "Usage: $0 [-n|--name | -m|--max | -i|--instant]"
            exit 1
            ;;
    esac
done
```