#include "cap.h"

#include <zephyr/device.h>
#include <zephyr/devicetree.h>
#include <zephyr/drivers/adc.h>
#include <zephyr/logging/log.h>

LOG_MODULE_REGISTER(cap, LOG_LEVEL_INF);

#if !DT_NODE_EXISTS(DT_PATH(zephyr_user)) || \
    !DT_NODE_HAS_PROP(DT_PATH(zephyr_user), io_channels)
#error "No suitable devicetree overlay specified"
#endif

#define ADC_DT_SPEC(node_id, prop, idx)                            \
    COND_CODE_1(DT_PHA_HAS_CELL_AT_IDX(node_id, prop, idx, input), \
                (ADC_DT_SPEC_GET_BY_IDX(node_id, idx), ), ())

static const struct adc_dt_spec chans[] = {
    DT_FOREACH_PROP_ELEM(DT_PATH(zephyr_user), io_channels, ADC_DT_SPEC)};

static const int chan_cnt = ARRAY_SIZE(chans);

int cap_init(void) {
    for (size_t i = 0; i < chan_cnt; i++) {
        if (!adc_is_ready_dt(&chans[i])) {
            LOG_ERR("Channel not ready: %d", i);
            return -ENODEV;
        }

        int err = adc_channel_setup_dt(&chans[i]);
        if (err != 0) {
            LOG_ERR("Could not setup channel: %d", i);
            return err;
        }
    }

    return 0;
}

int cap_read(void) {
    int err;
    uint16_t buf = 0;
    struct adc_sequence seq = {
        .buffer = &buf,
        .buffer_size = sizeof(buf),
    };

#ifndef CONFIG_COVERAGE
    while (1) {
#else
    for (size_t i = 0; i < 10; i++) {
#endif
        for (size_t i = 0U; i < chan_cnt; i++) {
            int32_t val;

            /* Clear buffer */
            buf = 0;

            (void)adc_sequence_init_dt(&chans[i], &seq);

            err = adc_read_dt(&chans[i], &seq);
            if (err != 0) {
                LOG_ERR("Could not read channel: %d", i);
                continue;
            }

            if (chans[i].channel_cfg.differential) {
                val = (int32_t)(int16_t)buf;
            } else {
                val = (int32_t)buf;
            }

            err = adc_raw_to_millivolts_dt(&chans[i], &val);
            if (err != 0) {
                LOG_ERR("Value not available in mV");
            }

            LOG_INF("Channel: %d    mV: %" PRId32 "", i, val);
        }

        k_msleep(500);
    }
}