#include "device.h"

extern void lpt_init();
extern void lpt1_init(uint16_t port);
extern void lpt1_remove();
extern void lpt2_init(uint16_t port);
extern void lpt2_remove();
extern void lpt2_remove_ams();

void lpt1_write(uint16_t port, uint8_t val, void *priv);
uint8_t lpt1_read(uint16_t port, void *priv);

void lpt1_device_init();
void lpt2_device_init();

void lpt1_device_attach();
void lpt1_device_detach();
void lpt2_device_attach();
void lpt2_device_detach();

char *lpt_device_get_name(int id);
char *lpt_device_get_internal_name(int id);
device_t *lpt1_device_getdevice(int id);
device_t *lpt2_device_getdevice(int id);
int lpt1_device_has_config(int id);
int lpt2_device_has_config(int id);
int lpt_device_get_from_internal_name(char *s);


extern int lpt1_current;
extern int lpt2_current;

typedef struct
{
        void (*write_data)(uint8_t val, void *p);
        void (*write_ctrl)(uint8_t val, void *p);
        uint8_t (*read_status)(void *p);
        uint8_t (*read_ctrl)(void *p);
        device_t *device_lpt1;
        device_t *device_lpt2;
} lpt_device_t;
