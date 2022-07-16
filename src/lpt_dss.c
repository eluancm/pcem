#include <stdlib.h>
#include "ibm.h"
#include "filters.h"
#include "lpt.h"
#include "lpt_dss.h"
#include "sound.h"

typedef struct dss_t
{
        uint8_t fifo[16];
        int read_idx, write_idx;
        
        uint8_t dac_val;
        
        pc_timer_t timer;
        
        int16_t buffer[MAXSOUNDBUFLEN];
        int pos;
} dss_t;

static void dss_update(dss_t *dss)
{
        for (; dss->pos < sound_pos_global; dss->pos++)
                dss->buffer[dss->pos] = (int8_t)(dss->dac_val ^ 0x80) * 0x40;
}


static void dss_write_data(uint8_t val, void *p)
{
        dss_t *dss = (dss_t *)p;

        if ((dss->write_idx - dss->read_idx) < 16)
        {
                dss->fifo[dss->write_idx & 15] = val;
                dss->write_idx++;
        }
}

static void dss_write_ctrl(uint8_t val, void *p)
{
}

static uint8_t dss_read_status(void *p)
{
        dss_t *dss = (dss_t *)p;

        if ((dss->write_idx - dss->read_idx) >= 16)
                return 0x40;
        return 0;
}


static void dss_get_buffer(int32_t *buffer, int len, void *p)
{
        dss_t *dss = (dss_t *)p;
        int c;
        
        dss_update(dss);
        
        for (c = 0; c < len*2; c += 2)
        {
                int16_t val = (int16_t)dss_iir((float)dss->buffer[c >> 1]);
                
                buffer[c] += val;
                buffer[c+1] += val;
        }

        dss->pos = 0;
}

static void dss_callback(void *p)
{
        dss_t *dss = (dss_t *)p;

        dss_update(dss);

        if ((dss->write_idx - dss->read_idx) > 0)
        {
                dss->dac_val = dss->fifo[dss->read_idx & 15];
                dss->read_idx++;
        }
        
        timer_advance_u64(&dss->timer, (TIMER_USEC * (1000000.0 / 7000.0)));
}

static void *dss_init()
{
        dss_t *dss = malloc(sizeof(dss_t));
        memset(dss, 0, sizeof(dss_t));

        sound_add_handler(dss_get_buffer, dss);
        timer_add(&dss->timer, dss_callback, dss, 1);
                
        return dss;
}
static void dss_close(void *p)
{
        dss_t *dss = (dss_t *)p;
        
        free(dss);
}

void *dss_init_lpt1()
{
        void *p = dss_init();
        lpt1_device_attach(&lpt_dss_device, p);

        return p;
}
void dss_close_lpt1(void *p)
{
        dss_close(p);
        lpt1_device_detach();
}

void *dss_init_lpt2()
{
        void *p = dss_init();
        lpt2_device_attach(&lpt_dss_device, p);

        return p;
}
void dss_close_lpt2(void *p)
{
        dss_close(p);
        lpt2_device_detach();
}

device_t dss_device_lpt1 =
{
        "Disney Sound Source",
        0,
        dss_init_lpt1,
        dss_close_lpt1,
        NULL,
        NULL,
        NULL,
        NULL,
        NULL
};
device_t dss_device_lpt2 =
{
        "Disney Sound Source",
        0,
        dss_init_lpt2,
        dss_close_lpt2,
        NULL,
        NULL,
        NULL,
        NULL,
        NULL
};

lpt_device_t lpt_dss_device =
{
        dss_write_data,
        dss_write_ctrl,
        dss_read_status,
        NULL,
        &dss_device_lpt1,
        &dss_device_lpt2
};
