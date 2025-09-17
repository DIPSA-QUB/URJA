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

### Install PAPI and point to it

```bash
export PAPI_DIR=/path/to/papi/
```

### Build the shared library

```bash
make
```

## Running

If you want to run `URJA` with `NaiveLogger`, run -

```bash
source scripts/set_env.sh
```

once and execute any app with -

```bash
urja  /path/to/your_app
```

Ensure that the `URJA_SCRIPT_DIR` variable in `scripts/set_env.sh` points to the correct path.

### Set appropriate environment variables

```bash
export URJA_PAPI_EVENTS=PAPI_TOT_CYC,PAPI_TOT_INS,PAPI_L2_DCM,PAPI_TLB_DM
export URJA_INTERVAL_MS=1000         # Monitoring interval in milliseconds
export URJA_LOGGER=stdio             # Options: stdio, file
export URJA_LOG_FILE=/tmp/urja.log   # If using URJA_LOGGER=file
```

(**Optionally**) To profile energy consumption -

```bash
export URJA_ENERGY_BACKEND=sysfs     # Options: sysfs, shell
```

Then run the application using -

```bash
LD_PRELOAD=build/bin/liburja.so /path/to/your_app
```

### Requirements for `URJA_ENERGY_BACKEND=shell`

##### Warning: This is not recommended and will be depricated in the future.

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

