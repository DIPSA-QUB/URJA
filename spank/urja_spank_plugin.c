/**
 * urja_spank_plugin.c
 * * Config file: /etc/slurm/urja.conf
 */

#define _GNU_SOURCE
#include <slurm/spank.h>
#include <string.h>
#include <stdio.h>
#include <stdlib.h>
#include <ctype.h>
#include <unistd.h>

SPANK_PLUGIN(spank_preload_urja, 1);

static const char *CONFIG_FILE_PATH = "/etc/slurm/urja.conf";

static const char *ENV_URJA_ENABLE = "URJA_ENABLE";
static const char *ENV_SLURM_SPANK_URJA_ENABLED = "SLURM_SPANK_URJA_ENABLED";
static const char *ENV_LD_PRELOAD = "LD_PRELOAD";
static const char *ENV_URJA_LIB = "URJA_LIB";
static const char *ENV_URJA_LOG_FILE = "URJA_LOG_FILE";
static const char *ENV_URJA_LOG_DIR = "URJA_LOG_DIR";

#define SMALL_BUF 32
#define PATH_BUF 512
#define LARGE_BUF 4096
#define PRELOAD_BUF 8192
#define LINE_BUF 1024

static struct spank_option urja_options[] = {
    { "enable-urja", NULL, "Enable Urja monitoring for the job", 0, 0, NULL },
    SPANK_OPTIONS_TABLE_END
};

static char *trim(char *s) {
    char *ptr;
    if (!s) return NULL;
    while (isspace(*s)) s++;
    if (*s == 0) return s;
    ptr = s + strlen(s) - 1;
    while (ptr > s && isspace(*ptr)) ptr--;
    *(ptr + 1) = 0;
    return s;
}

static int is_truthy(const char *s) {
    if (!s || !s[0]) return 0;
    if (!strcasecmp(s, "1")) return 1;
    if (!strcasecmp(s, "on")) return 1;
    if (!strcasecmp(s, "true")) return 1;
    return 0;
}

static void set_default_env(spank_t sp, const char *key, const char *value) {
    char probe[LARGE_BUF] = {0};
    if (spank_getenv(sp, key, probe, sizeof(probe)) != ESPANK_SUCCESS || probe[0] == '\0') {
        spank_setenv(sp, key, value, 1);
    }
}

static void load_urja_config(spank_t sp) {
    FILE *fp = fopen(CONFIG_FILE_PATH, "r");
    if (!fp) {
        slurm_error("[URJA-SPANK] Warning: Could not open config file %s", CONFIG_FILE_PATH);
        return; 
    }

    char line[LINE_BUF];
    while (fgets(line, sizeof(line), fp)) {
        char *p = trim(line);

        if (p[0] == '#' || p[0] == '\0') continue;

        char *eq = strchr(p, '=');
        if (!eq) continue;

        *eq = '\0';
        char *key = trim(p);
        char *val = trim(eq + 1);

        if (key && key[0] && val && val[0]) {
            set_default_env(sp, key, val);
        }
    }
    fclose(fp);
}

static void build_preload(const char *urja_lib, char *out, size_t outsz, const char *existing) {
    if (existing && existing[0] != '\0' && strstr(existing, urja_lib) != NULL) {
        snprintf(out, outsz, "%s", existing);
        return;
    }
    if (existing && existing[0] != '\0') {
        snprintf(out, outsz, "%s:%s", urja_lib, existing);
    } else {
        snprintf(out, outsz, "%s", urja_lib);
    }
}

int slurm_spank_init(spank_t sp, int ac, char **av) {
    for (int i = 0; urja_options[i].name != NULL; ++i) {
        if (spank_option_register(sp, &urja_options[i]) != ESPANK_SUCCESS) {
            return ESPANK_ERROR;
        }
    }
    return ESPANK_SUCCESS;
}

int slurm_spank_init_post_opt(spank_t sp, int ac, char **av) {
    char *opt_arg = NULL;
    if (spank_option_getopt(sp, &urja_options[0], &opt_arg) == ESPANK_SUCCESS) {
        spank_setenv(sp, ENV_SLURM_SPANK_URJA_ENABLED, "1", 1);
        spank_setenv(sp, ENV_URJA_ENABLE, "1", 1);
    }
    return ESPANK_SUCCESS;
}

int slurm_spank_task_init(spank_t sp, int ac, char **av) {
    char val[SMALL_BUF] = {0};
    int enabled = 0;

    if (spank_getenv(sp, ENV_SLURM_SPANK_URJA_ENABLED, val, sizeof(val)) == ESPANK_SUCCESS && val[0] != '\0') {
        enabled = 1;
    }
    if (!enabled && spank_getenv(sp, ENV_URJA_ENABLE, val, sizeof(val)) == ESPANK_SUCCESS && val[0] != '\0') {
        if (is_truthy(val)) enabled = 1;
    }

    if (!enabled) return ESPANK_SUCCESS;

    load_urja_config(sp);

    char log_dir[PATH_BUF] = {0};
    if (spank_getenv(sp, ENV_URJA_LOG_DIR, log_dir, sizeof(log_dir)) != ESPANK_SUCCESS || log_dir[0] == '\0') {
        strcpy(log_dir, ".");
    }

    int jobid = 0;
    char logfile[PATH_BUF] = {0};
    if (spank_get_item(sp, S_JOB_ID, &jobid) == ESPANK_SUCCESS && jobid > 0) {
        snprintf(logfile, sizeof(logfile), "%s/urja-job-%d.log", log_dir, jobid);
    } else {
        snprintf(logfile, sizeof(logfile), "%s/urja.log", log_dir);
    }
    set_default_env(sp, ENV_URJA_LOG_FILE, logfile);

    char urja_lib[LARGE_BUF] = {0};
    if (spank_getenv(sp, ENV_URJA_LIB, urja_lib, sizeof(urja_lib)) != ESPANK_SUCCESS || urja_lib[0] == '\0') {
        slurm_error("[URJA-SPANK] Error: URJA_LIB not set in config or env. Aborting injection.");
        return ESPANK_SUCCESS;
    }

    char current_preload[LARGE_BUF] = {0};
    char new_preload[PRELOAD_BUF] = {0};
    int have_preload = (spank_getenv(sp, ENV_LD_PRELOAD, current_preload, sizeof(current_preload)) == ESPANK_SUCCESS
                        && current_preload[0] != '\0');
    
    build_preload(urja_lib, new_preload, sizeof(new_preload), have_preload ? current_preload : NULL);

    if (spank_setenv(sp, ENV_LD_PRELOAD, new_preload, 1) != ESPANK_SUCCESS) {
        slurm_error("[URJA-SPANK] failed to set LD_PRELOAD");
        return ESPANK_ERROR;
    }

    return ESPANK_SUCCESS;
}