CC          = gcc
CFLAGS      = -Wall -Wextra -O2 -g -fPIC
LDFLAGS     = -shared

# Slurm Paths (Adjust if Slurm is installed in /opt/slurm or elsewhere)
# If 'spank.h' is not found, change SLURM_PREFIX to install path (e.g., /opt/slurm)
SLURM_PREFIX ?= /usr
CPPFLAGS    = -I$(SLURM_PREFIX)/include

# Installation Paths
DESTDIR     ?= 
PLUGIN_DIR  = /usr/lib64/slurm
CONFIG_DIR  = /etc/slurm

# Target Filenames
TARGET_LIB  = urja_spank_plugin.so
SRC         = urja_spank_plugin.c
CONFIG_FILE = urja.conf

.PHONY: all clean install install-lib install-conf

all: $(TARGET_LIB)

# Build the shared object
$(TARGET_LIB): $(SRC)
	@echo "[BUILD] Compiling $(TARGET_LIB)..."
	$(CC) $(CFLAGS) $(CPPFLAGS) $(LDFLAGS) -o $@ $<
	@echo "[BUILD] Success."

# Clean build artifacts
clean:
	@echo "[CLEAN] Removing build artifacts..."
	rm -f $(TARGET_LIB)

# Install everything
install: all install-lib install-conf
	@echo "--------------------------------------------------------"
	@echo "Installation complete."
	@echo "1. Ensure $(PLUGIN_DIR)/$(TARGET_LIB) is in /etc/slurm/plugstack.conf"
	@echo "2. Edit $(CONFIG_DIR)/$(CONFIG_FILE) to set your preferences."
	@echo "3. Restart slurmd on compute nodes: 'systemctl restart slurmd'"
	@echo "--------------------------------------------------------"

# Install the library .so
install-lib:
	@echo "[INSTALL] Installing plugin to $(DESTDIR)$(PLUGIN_DIR)..."
	@install -d -m 755 $(DESTDIR)$(PLUGIN_DIR)
	@install -m 755 $(TARGET_LIB) $(DESTDIR)$(PLUGIN_DIR)

# Install the configuration file (non-destructive)
install-conf:
	@echo "[INSTALL] Installing config to $(DESTDIR)$(CONFIG_DIR)..."
	@install -d -m 755 $(DESTDIR)$(CONFIG_DIR)
	@if [ -f "$(DESTDIR)$(CONFIG_DIR)/$(CONFIG_FILE)" ]; then \
		echo "   [NOTE] Config file exists at $(DESTDIR)$(CONFIG_DIR)/$(CONFIG_FILE). Skipping overwrite."; \
		echo "   [NOTE] A sample config 'urja.conf.sample' will be created instead."; \
		install -m 644 $(CONFIG_FILE) $(DESTDIR)$(CONFIG_DIR)/$(CONFIG_FILE).sample; \
	else \
		install -m 644 $(CONFIG_FILE) $(DESTDIR)$(CONFIG_DIR); \
	fi