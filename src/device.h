#ifndef _DEVICE_H_
#define _DEVICE_H_

#define CONFIG_STRING 0
#define CONFIG_INT 1
#define CONFIG_BINARY 2
#define CONFIG_SELECTION 3
#define CONFIG_MIDI 4

typedef struct device_config_selection_t
{
        char description[256];
        int value;
} device_config_selection_t;

typedef struct device_config_t
{
        char name[256];
        char description[256];
        int type;
        char default_string[256];
        int default_int;
        device_config_selection_t selection[20];
} device_config_t;

typedef struct device_t
{
        char name[50];
        uint32_t flags;
        void *(*init)();
        void (*close)(void *p);
        int  (*available)();
        void (*speed_changed)(void *p);
        void (*force_redraw)(void *p);
        void (*add_status_info)(char *s, int max_len, void *p);
        device_config_t *config;
} device_t;

void device_init();
void device_add(device_t *d);
void device_close_all();
int device_available(device_t *d);
void device_speed_changed();
void device_force_redraw();
void device_add_status_info(char *s, int max_len);

extern char *current_device_name;

int device_get_config_int(char *name);
char *device_get_config_string(char *name);

enum
{
        DEVICE_NOT_WORKING = 1, /*Device does not currently work correctly and will be disabled in a release build*/
        DEVICE_AT = 2,          /*Device requires an AT-compatible system*/
        DEVICE_MCA = 0x20,      /*Device requires an MCA system*/
        DEVICE_PCI = 0x40,      /*Device requires a PCI system*/
        DEVICE_PS1 = 0x80       /*Device is only for IBM PS/1 Model 2011*/
};

int model_get_config_int(char *s);
char *model_get_config_string(char *s);

#endif
