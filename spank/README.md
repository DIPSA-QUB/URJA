# URJA SPANK Plugin

A Slurm plugin to inject URJA (`liburja.so`) into user jobs via `LD_PRELOAD`.

## 1. Build & Install

```bash
make
sudo make install
````

*Installs plugin to `/usr/lib64/slurm/` and config to `/etc/slurm/urja.conf`.*

## 2. Register with Slurm

Add the following line to `/etc/slurm/plugstack.conf`:

```text
optional /usr/lib64/slurm/urja_spank_plugin.so
```

*Restart `slurmd` on all compute nodes to apply changes.*

## 3. Configuration

Edit `/etc/slurm/urja.conf` to set:

  * **URJA\_LIB**: Path to `liburja.so`.
  * **URJA\_LOGGER**: `naive` or `trident`.
  * **Thresholds/Frequencies**: Specific logic settings.

## 4. Usage

### Interactive (srun)

```bash
srun --enable-urja ./my_application
```

### Batch Scripts (sbatch)

Add the flag to your submission script directives:

```bash
#!/bin/bash
#SBATCH --job-name=my_job
#SBATCH --nodes=1
#SBATCH --enable-urja

./my_application
```

*Alternatively, pass the flag on the command line: `sbatch --enable-urja script.sh`*
