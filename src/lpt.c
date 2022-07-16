#include "ibm.h"
#include "io.h"

#include "lpt.h"
#include "lpt_dac.h"
#include "lpt_dss.h"
#include "lpt_epsonprinter.h"

int lpt1_current;
int lpt2_current;

static struct
{
        char name[64];
        char internal_name[32];
        lpt_device_t *lpt;
} lpt_devices[] =
{
        {"None",                         "none",             NULL},
        {"Disney Sound Source",          "dss",              &lpt_dss_device},
        {"LPT DAC / Covox Speech Thing", "lpt_dac",          &lpt_dac_device},
        {"Stereo LPT DAC",               "lpt_dac_stereo",   &lpt_dac_stereo_device},
        {"Epson LX-810 Printer",         "lpt_epsonprinter", &lpt_epsonprinter_device},
        {"", "", NULL}
};

char *lpt_device_get_name(int id)
{
        if (strlen(lpt_devices[id].name) == 0)
                return NULL;
        return lpt_devices[id].name;
}
char *lpt_device_get_internal_name(int id)
{
        if (strlen(lpt_devices[id].internal_name) == 0)
                return NULL;
        return lpt_devices[id].internal_name;
}
device_t *lpt1_device_getdevice(int id)
{
        if (lpt_devices[id].lpt)
                if (lpt_devices[id].lpt->device_lpt1)
                        return lpt_devices[id].lpt->device_lpt1;
        return NULL;
}
device_t *lpt2_device_getdevice(int id)
{
        if (lpt_devices[id].lpt)
                if (lpt_devices[id].lpt->device_lpt2)
                        return lpt_devices[id].lpt->device_lpt2;
        return NULL;
}
int lpt_device_get_from_internal_name(char *s)
{
	int c = 0;
	
	while (strlen(lpt_devices[c].internal_name))
	{
		if (!strcmp(lpt_devices[c].internal_name, s))
			return c;
		c++;
	}
	
	return 0;
}
int lpt1_device_has_config(int id)
{
        if (!lpt_devices[id].lpt)
                return 0;
        if (!lpt_devices[id].lpt->device_lpt1)
                return 0;
        return lpt_devices[id].lpt->device_lpt1->config ? 1 : 0;
}
int lpt2_device_has_config(int id)
{
        if (!lpt_devices[id].lpt)
                return 0;
        if (!lpt_devices[id].lpt->device_lpt2)
                return 0;
        return lpt_devices[id].lpt->device_lpt2->config ? 1 : 0;
}

void lpt1_device_init()
{
        if (lpt_devices[lpt1_current].lpt)
                if (lpt_devices[lpt1_current].lpt->device_lpt1)
                        device_add(lpt_devices[lpt1_current].lpt->device_lpt1);
}
void lpt2_device_init()
{
        if (lpt_devices[lpt2_current].lpt)
                if (lpt_devices[lpt2_current].lpt->device_lpt2)
                        device_add(lpt_devices[lpt2_current].lpt->device_lpt2);
}

static lpt_device_t *lpt1_device = NULL;
static void *lpt1_device_p = NULL;
static lpt_device_t *lpt2_device = NULL;
static void *lpt2_device_p = NULL;

// the devices will call these map lpt I/O to themselves
void lpt1_device_attach(lpt_device_t *device, void *p)
{
        lpt1_device = device;
        lpt1_device_p = p;
}
void lpt2_device_attach(lpt_device_t *device, void *p)
{
        lpt2_device = device;
        lpt2_device_p = p;
}

void lpt1_device_detach()
{
        lpt1_device = NULL;
        lpt1_device_p = NULL;
}
void lpt2_device_detach()
{
        lpt2_device = NULL;
        lpt2_device_p = NULL;
}

static uint8_t lpt1_dat, lpt2_dat;
static uint8_t lpt1_ctrl, lpt2_ctrl;

void lpt1_write(uint16_t port, uint8_t val, void *priv)
{
        switch (port & 3)
        {
                case 0:
                if (lpt1_device)
                        lpt1_device->write_data(val, lpt1_device_p);
                lpt1_dat = val;
                break;
                case 2:
                if (lpt1_device)
                        lpt1_device->write_ctrl(val, lpt1_device_p);
                lpt1_ctrl = val;
                break;
        }
}
uint8_t lpt1_read(uint16_t port, void *priv)
{
        switch (port & 3)
        {
                case 0:
                return lpt1_dat;
                case 1:
                if (lpt1_device)
                        return lpt1_device->read_status(lpt1_device_p);
                return 0;
                case 2:
                if (lpt1_device)
                        if (lpt1_device->read_ctrl)
                                return lpt1_device->read_ctrl(lpt1_device_p);
                return lpt1_ctrl;
        }
        return 0xff;
}

void lpt2_write(uint16_t port, uint8_t val, void *priv)
{
        switch (port & 3)
        {
                case 0:
                if (lpt2_device)
                        lpt2_device->write_data(val, lpt2_device_p);
                lpt2_dat = val;
                break;
                case 2:
                if (lpt2_device)
                        lpt2_device->write_ctrl(val, lpt2_device_p);
                lpt2_ctrl = val;
                break;
        }
}
uint8_t lpt2_read(uint16_t port, void *priv)
{
        switch (port & 3)
        {
                case 0:
                return lpt2_dat;
                case 1:
                if (lpt2_device)
                        return lpt2_device->read_status(lpt2_device_p);
                return 0;
                case 2:
                if (lpt2_device)
                        if (lpt2_device->read_ctrl)
                                return lpt2_device->read_ctrl(lpt2_device_p);
                return lpt2_ctrl;
        }
        return 0xff;
}

void lpt_init()
{
        io_sethandler(0x0378, 0x0003, lpt1_read, NULL, NULL, lpt1_write, NULL, NULL,  NULL);
        io_sethandler(0x0278, 0x0003, lpt2_read, NULL, NULL, lpt2_write, NULL, NULL,  NULL);
}

void lpt1_init(uint16_t port)
{
        if (port)
                io_sethandler(port, 0x0003, lpt1_read, NULL, NULL, lpt1_write, NULL, NULL,  NULL);
}
void lpt1_remove()
{
        io_removehandler(0x0278, 0x0003, lpt1_read, NULL, NULL, lpt1_write, NULL, NULL,  NULL);
        io_removehandler(0x0378, 0x0003, lpt1_read, NULL, NULL, lpt1_write, NULL, NULL,  NULL);
        io_removehandler(0x03bc, 0x0003, lpt1_read, NULL, NULL, lpt1_write, NULL, NULL,  NULL);
}
void lpt2_init(uint16_t port)
{
        if (port)
                io_sethandler(port, 0x0003, lpt2_read, NULL, NULL, lpt2_write, NULL, NULL,  NULL);
}
void lpt2_remove()
{
        io_removehandler(0x0278, 0x0003, lpt2_read, NULL, NULL, lpt2_write, NULL, NULL,  NULL);
        io_removehandler(0x0378, 0x0003, lpt2_read, NULL, NULL, lpt2_write, NULL, NULL,  NULL);
        io_removehandler(0x03bc, 0x0003, lpt2_read, NULL, NULL, lpt2_write, NULL, NULL,  NULL);
}

void lpt2_remove_ams()
{
        io_removehandler(0x0379, 0x0002, lpt2_read, NULL, NULL, lpt2_write, NULL, NULL,  NULL);
}
