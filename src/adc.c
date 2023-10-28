/*
 * Copyright (c) 2020 Libre Solar Technologies GmbH
 *
 * SPDX-License-Identifier: Apache-2.0
 */

#include <inttypes.h>
#include <stddef.h>
#include <stdint.h>

#include <zephyr/device.h>
#include <zephyr/devicetree.h>
#include <zephyr/drivers/adc.h>
#include <zephyr/kernel.h>
#include <zephyr/logging/log.h>

LOG_MODULE_REGISTER(adc, LOG_LEVEL_INF);

#if !DT_NODE_EXISTS(DT_PATH(zephyr_user)) || \
	!DT_NODE_HAS_PROP(DT_PATH(zephyr_user), io_channels)
#error "No suitable devicetree overlay specified"
#endif

#define DT_SPEC_AND_COMMA(node_id, prop, idx) \
	ADC_DT_SPEC_GET_BY_IDX(node_id, idx),

/* Data of ADC io-channels specified in devicetree. */
static const struct adc_dt_spec adc_channels[] = {
	DT_FOREACH_PROP_ELEM(DT_PATH(zephyr_user), io_channels,
			     DT_SPEC_AND_COMMA)
};

int adc_setup(void)
{
	int err;

	/* Configure channel 0 prior to sampling. */
	if (!device_is_ready(adc_channels[0].dev)) {
		LOG_ERR("ADC controller device %s not ready\n", adc_channels[0].dev->name);
		return 0;
	}

	err = adc_channel_setup_dt(&adc_channels[0]);
	if (err < 0) {
		LOG_ERR("Could not setup channel 0 (%d)\n", err);
		return 0;
	}

	/* Configure channel 1 prior to sampling. */
	if (!device_is_ready(adc_channels[1].dev)) {
		LOG_ERR("ADC controller device %s not ready\n", adc_channels[1].dev->name);
		return 0;
	}

	err = adc_channel_setup_dt(&adc_channels[1]);
	if (err < 0) {
		LOG_ERR("Could not setup channel 1 (%d)\n", err);
		return 0;
	}

	return 1; // Ok
}

int32_t adc_probe()
{
	int err;
	uint16_t buf;
	struct adc_sequence sequence = {
		.buffer = &buf,
		/* buffer size in bytes, not number of samples */
		.buffer_size = sizeof(buf),
	};
	int32_t val_mv;

	(void)adc_sequence_init_dt(&adc_channels[0], &sequence);

	err = adc_read(adc_channels[0].dev, &sequence);
	if (err < 0) {
		LOG_ERR("Could not read (%d)\n", err);
		return -1;
	}

	val_mv = (int32_t)((int16_t)buf);

	err = adc_raw_to_millivolts_dt(&adc_channels[0], &val_mv);

	LOG_INF("- %s, channel %d: %"PRId32" mV", adc_channels[0].dev->name, adc_channels[0].channel_id, val_mv);

	return val_mv;
}

uint8_t adc_battery()
{
	int err;
	uint16_t buf;
	struct adc_sequence sequence = {
		.buffer = &buf,
		/* buffer size in bytes, not number of samples */
		.buffer_size = sizeof(buf),
	};
	int32_t val_mv;

	(void)adc_sequence_init_dt(&adc_channels[1], &sequence);

	err = adc_read(adc_channels[1].dev, &sequence);
	if (err < 0) {
		LOG_ERR("Could not read (%d)\n", err);
		return -1;
	}

	val_mv = (int32_t)((int16_t)buf);

	err = adc_raw_to_millivolts_dt(&adc_channels[0], &val_mv);

	LOG_INF("- %s, channel %d: %"PRId32" mV", adc_channels[1].dev->name, adc_channels[1].channel_id, val_mv);

	return (uint8_t)(val_mv/100);
}
