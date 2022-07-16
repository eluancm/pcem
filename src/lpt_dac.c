#include <stdlib.h>
#include "ibm.h"
#include "filters.h"
#include "lpt.h"
#include "lpt_dac.h"
#include "sound.h"

typedef struct lpt_dac_t
{
        uint8_t dac_val_l, dac_val_r;
        
        int is_stereo;
        int channel;
        
        int16_t buffer[2][MAXSOUNDBUFLEN];
        int pos;
} lpt_dac_t;

static void dac_update(lpt_dac_t *lpt_dac)
{
        for (; lpt_dac->pos < sound_pos_global; lpt_dac->pos++)
        {
                lpt_dac->buffer[0][lpt_dac->pos] = (int8_t)(lpt_dac->dac_val_l ^ 0x80) * 0x40;
                lpt_dac->buffer[1][lpt_dac->pos] = (int8_t)(lpt_dac->dac_val_r ^ 0x80) * 0x40;
        }
}


static void dac_write_data(uint8_t val, void *p)
{
        lpt_dac_t *lpt_dac = (lpt_dac_t *)p;
        
        if (lpt_dac->is_stereo)
        {
                if (lpt_dac->channel)
                        lpt_dac->dac_val_r = val;
                else
                        lpt_dac->dac_val_l = val;
        }
        else        
                lpt_dac->dac_val_l = lpt_dac->dac_val_r = val;
        dac_update(lpt_dac);
}

static void dac_write_ctrl(uint8_t val, void *p)
{
        lpt_dac_t *lpt_dac = (lpt_dac_t *)p;

        if (lpt_dac->is_stereo)
                lpt_dac->channel = val & 0x01;
}

static uint8_t dac_read_status(void *p)
{
        return 0;
}


static void dac_get_buffer(int32_t *buffer, int len, void *p)
{
        lpt_dac_t *lpt_dac = (lpt_dac_t *)p;
        int c;
        
        dac_update(lpt_dac);
        
        for (c = 0; c < len; c++)
        {
                buffer[c*2]     += dac_iir(0, lpt_dac->buffer[0][c]);
                buffer[c*2 + 1] += dac_iir(1, lpt_dac->buffer[1][c]);
        }
        lpt_dac->pos = 0;
}

static void *dac_init()
{
        lpt_dac_t *lpt_dac = malloc(sizeof(lpt_dac_t));
        memset(lpt_dac, 0, sizeof(lpt_dac_t));

        sound_add_handler(dac_get_buffer, lpt_dac);
                
        return lpt_dac;
}
static void *dac_stereo_init()
{
        lpt_dac_t *lpt_dac = dac_init();
        
        lpt_dac->is_stereo = 1;
                
        return lpt_dac;
}
static void dac_close(void *p)
{
        lpt_dac_t *lpt_dac = (lpt_dac_t *)p;
        
        free(lpt_dac);
}

void *dac_init_lpt1()
{
        void *p = dac_init();
        lpt1_device_attach(&lpt_dac_device, p);

        return p;
}
void dac_close_lpt1(void *p)
{
        dac_close(p);
        lpt1_device_detach();
}

void *dac_init_lpt2()
{
        void *p = dac_init();
        lpt2_device_attach(&lpt_dac_device, p);

        return p;
}
void dac_close_lpt2(void *p)
{
        dac_close(p);
        lpt2_device_detach();
}

device_t dac_device_lpt1 =
{
        "LPT DAC / Covox Speech Thing",
        0,
        dac_init_lpt1,
        dac_close_lpt1,
        NULL,
        NULL,
        NULL,
        NULL,
        NULL
};
device_t dac_device_lpt2 =
{
        "LPT DAC / Covox Speech Thing",
        0,
        dac_init_lpt2,
        dac_close_lpt2,
        NULL,
        NULL,
        NULL,
        NULL,
        NULL
};

void *dac_stereo_init_lpt1()
{
        void *p = dac_stereo_init();
        lpt1_device_attach(&lpt_dac_stereo_device, p);

        return p;
}

void *dac_stereo_init_lpt2()
{
        void *p = dac_stereo_init();
        lpt2_device_attach(&lpt_dac_stereo_device, p);

        return p;
}

device_t dac_stereo_device_lpt1 =
{
        "Stereo LPT DAC",
        0,
        dac_stereo_init_lpt1,
        dac_close_lpt1,
        NULL,
        NULL,
        NULL,
        NULL,
        NULL
};
device_t dac_stereo_device_lpt2 =
{
        "Stereo LPT DAC",
        0,
        dac_stereo_init_lpt2,
        dac_close_lpt2,
        NULL,
        NULL,
        NULL,
        NULL,
        NULL
};

lpt_device_t lpt_dac_device =
{
        dac_write_data,
        dac_write_ctrl,
        dac_read_status,
        NULL,
        &dac_device_lpt1,
        &dac_device_lpt2
};
lpt_device_t lpt_dac_stereo_device =
{
        dac_write_data,
        dac_write_ctrl,
        dac_read_status,
        NULL,
        &dac_stereo_device_lpt1,
        &dac_stereo_device_lpt2
};
