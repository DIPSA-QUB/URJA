# URJA: Unified Runtime Job Analyzer

URJA is a lightweight dynamic monitoring library designed to log per-thread hardware performance counters (via PAPI) and energy consumption (via RAPL) for multithreaded applications with no code changes to the target program.

## Requirements

1. Linux
2. papi (6.0+)
3. C++17

## Assumptions

1. You have installed `PAPI` and exported `PAPI_DIR` pointing to its installation location,
2. The target program uses `pthread_create` for thread spawning,
3. For `shell` backend - file `/path/to/rapl_read.sh` is available and accessible (see below).

## Installation

### Set appropriate environment variables

```bash
export PAPI_DIR=/path/to/papi/src/install
export URJA_PAPI_EVENTS=PAPI_TOT_CYC,PAPI_TOT_INS,PAPI_L2_DCM,PAPI_TLB_DM
export URJA_ENERGY_BACKEND=sysfs     # Options: sysfs, shell
export URJA_INTERVAL_MS=1000         # Monitoring interval in milliseconds
export URJA_LOGGER=stdio             # Options: stdio, file
export URJA_LOG_FILE=/tmp/urja.log   # If using URJA_LOGGER=file
```

### Build the shared library

```bash
make
```

## Usage

```bash
LD_LIBRARY_PATH=/path/to/papi/src/install/lib LD_PRELOAD=build/bin/liburja.so ../kll-urja/build/bin/cpu_stress
```

### Requirements for `URJA_ENERGY_BACKEND=shell`

You only need to run **URJA** with root access if you choose the `shell` energy backend (e.g., `/path/to/rapl_read.sh`).

In that case, the script must be executable and present in the `sudoers` list without password prompt to allow non-interactive execution.

For example, add this line to your `/etc/sudoers` file using `visudo`:

```bash
your_username ALL=(ALL) NOPASSWD: /path/to/rapl_read.sh
```

Make sure the script is readable and executable:

```bash
sudo chmod +x /path/to/rapl_read.sh
```

The source code for `/path/to/rapl_read.sh` is:

```bash
#!/bin/sh

printf '%-20s; %-20s; %-15s; %20s; %20s\n' "name" "socket:domain_id"  "domain" "energy_uj"  "max_energy_uj"
for f in `find /sys/class/powercap/intel-rapl\:* | grep -P "\d+"`; do
    # echo $f;
    name=`echo $f | rev | cut -d/ -f1 | rev`
    id=`echo $name | cut -d: -f2,3`;
    domain=`cat $f/name`
    energy=`cat $f/energy_uj`
    max_energy=`cat $f/max_energy_range_uj`
    printf '%-20s; %-20s; %-15s; %20s; %20s\n' ${name} ${id} ${domain} ${energy} ${max_energy}
done
```

### Set perf_event_paranoid

To allow **URJA** (via **PAPI**) to access hardware counters, make sure:

```bash
cat /proc/sys/kernel/perf_event_paranoid
```

Returns `2` or less (e.g., `2`, `1`, `0`, or `-1`).

Use the following command to lower it temporarily:

```bash
sudo sh -c 'echo 2 > /proc/sys/kernel/perf_event_paranoid'
```

## Warning

This tool hooks into low-level system behavior like threads, performance counters, and power monitoring (e.g., via `LD_PRELOAD`, `sudo`, or hardware monitoring interfaces like **RAPL** and **PAPI**). It can be risky if used improperly. Use it with care — preferably in safe, controlled environments. Avoid running it on critical systems unless you know exactly what you're doing. This is in active development and is not rigorously tested.