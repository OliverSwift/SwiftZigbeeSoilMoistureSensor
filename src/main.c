/*
 * Copyright (c) 2021 Nordic Semiconductor ASA
 *
 * SPDX-License-Identifier: LicenseRef-Nordic-5-Clause
 */

/** @file
 *
 * @brief Zigbee application template.
 */

#include <zephyr/kernel.h>
#include <zephyr/logging/log.h>
#include <zephyr/drivers/gpio.h>
#include <dk_buttons_and_leds.h>

#include <zboss_api.h>
#include <zboss_api_addons.h>
#include <zigbee/zigbee_error_handler.h>
#include <zigbee/zigbee_app_utils.h>
#include <zb_nrf_platform.h>
#include "zb_swift_device.h"

static const struct gpio_dt_spec probe_vdd = GPIO_DT_SPEC_GET(DT_NODELABEL(probe_vdd), gpios);

/* Device endpoint, used to receive ZCL commands. */
#define APP_TEMPLATE_ENDPOINT               10

/* Type of power sources available for the device.
 * For possible values see section 3.2.2.2.8 of ZCL specification.
 */
#define TEMPLATE_INIT_BASIC_POWER_SOURCE    ZB_ZCL_BASIC_POWER_SOURCE_DC_SOURCE

/* LED indicating that device successfully joined Zigbee network. */
#define ZIGBEE_NETWORK_STATE_LED            DK_LED1

/* LED used for device identification. */
#define IDENTIFY_LED                        DK_LED2

/* LED used for device on_off. */
#define ON_OFF_LED			    DK_LED3

/* Button used to enter the Identify mode. */
#define IDENTIFY_MODE_BUTTON                DK_BTN1_MSK

/* Button to start Factory Reset */
#define FACTORY_RESET_BUTTON                IDENTIFY_MODE_BUTTON

LOG_MODULE_REGISTER(app, LOG_LEVEL_INF);

/* Main application customizable context.
 * Stores all settings and static values.
 */
struct zb_device_ctx {
	zb_zcl_basic_attrs_ext_t basic_attr;
	zb_zcl_on_off_attrs_t on_off_attr;
	zb_zcl_identify_attrs_t identify_attr;
	zb_zcl_rel_humidity_attrs_t rel_humidity_attr;
};

/* Zigbee device application context storage. */
static struct zb_device_ctx dev_ctx;

ZB_ZCL_DECLARE_IDENTIFY_ATTRIB_LIST(
	identify_attr_list,
	&dev_ctx.identify_attr.identify_time);

ZB_ZCL_DECLARE_BASIC_ATTRIB_LIST_EXT(
	basic_attr_list,
	&dev_ctx.basic_attr.zcl_version,
	&dev_ctx.basic_attr.app_version,
	&dev_ctx.basic_attr.stack_version,
	&dev_ctx.basic_attr.hw_version,
	dev_ctx.basic_attr.mf_name,
	dev_ctx.basic_attr.model_id,
	dev_ctx.basic_attr.date_code,
	&dev_ctx.basic_attr.power_source,
	dev_ctx.basic_attr.location_id,
	&dev_ctx.basic_attr.ph_env,
	dev_ctx.basic_attr.sw_ver);

ZB_ZCL_DECLARE_ON_OFF_ATTRIB_LIST(
	on_off_attr_list,
	&dev_ctx.on_off_attr.on_off);

ZB_ZCL_DECLARE_REL_HUMIDITY_MEASUREMENT_ATTRIB_LIST(
	rel_humidity_attr_list,
	&dev_ctx.rel_humidity_attr.value,
	&dev_ctx.rel_humidity_attr.min_value,
	&dev_ctx.rel_humidity_attr.max_value
);

ZB_DECLARE_SWIFT_DEVICE_CLUSTER_LIST(app_template_clusters, basic_attr_list, identify_attr_list, on_off_attr_list, rel_humidity_attr_list);

ZB_DECLARE_SWIFT_DEVICE_EP(
	app_template_ep,
	APP_TEMPLATE_ENDPOINT,
	app_template_clusters);

ZBOSS_DECLARE_DEVICE_CTX_1_EP(
	app_template_ctx,
	app_template_ep);

/* Manufacturer name (32 bytes). */
#define TEMPLATE_INIT_BASIC_MANUF_NAME      "Nordic"

/* Model number assigned by manufacturer (32-bytes long string). */
#define TEMPLATE_INIT_BASIC_MODEL_ID        "Swift switch"

/**@brief Function for initializing all clusters attributes. */
static void app_clusters_attr_init(void)
{
	/* Basic cluster attributes data */
	dev_ctx.basic_attr.zcl_version = ZB_ZCL_VERSION;
	dev_ctx.basic_attr.power_source = TEMPLATE_INIT_BASIC_POWER_SOURCE;

	ZB_ZCL_SET_STRING_VAL(
		dev_ctx.basic_attr.mf_name,
		TEMPLATE_INIT_BASIC_MANUF_NAME,
		ZB_ZCL_STRING_CONST_SIZE(TEMPLATE_INIT_BASIC_MANUF_NAME));

	ZB_ZCL_SET_STRING_VAL(
		dev_ctx.basic_attr.model_id,
		TEMPLATE_INIT_BASIC_MODEL_ID,
		ZB_ZCL_STRING_CONST_SIZE(TEMPLATE_INIT_BASIC_MODEL_ID));

	/* On/Off cluster attributes data. */
	dev_ctx.on_off_attr.on_off = (zb_bool_t)ZB_ZCL_ON_OFF_IS_OFF;

	ZB_ZCL_SET_ATTRIBUTE(
		APP_TEMPLATE_ENDPOINT,
		ZB_ZCL_CLUSTER_ID_ON_OFF,
		ZB_ZCL_CLUSTER_SERVER_ROLE,
		ZB_ZCL_ATTR_ON_OFF_ON_OFF_ID,
		(zb_uint8_t *)&dev_ctx.on_off_attr.on_off,
		ZB_FALSE);

	/* Identify cluster attributes data. */
	dev_ctx.identify_attr.identify_time = ZB_ZCL_IDENTIFY_IDENTIFY_TIME_DEFAULT_VALUE;

	/* Relative Humidity cluster attributes data. */
	dev_ctx.rel_humidity_attr.value = ZB_ZCL_ATTR_REL_HUMIDITY_MEASUREMENT_VALUE_UNKNOWN;
	dev_ctx.rel_humidity_attr.min_value = ZB_ZCL_ATTR_REL_HUMIDITY_MEASUREMENT_MIN_VALUE_MIN_VALUE;
	dev_ctx.rel_humidity_attr.max_value = ZB_ZCL_ATTR_REL_HUMIDITY_MEASUREMENT_MIN_VALUE_MAX_VALUE;

	ZB_ZCL_SET_ATTRIBUTE(
		APP_TEMPLATE_ENDPOINT,
		ZB_ZCL_CLUSTER_ID_REL_HUMIDITY_MEASUREMENT,
		ZB_ZCL_CLUSTER_SERVER_ROLE,
		ZB_ZCL_ATTR_REL_HUMIDITY_MEASUREMENT_VALUE_ID,
		(zb_uint8_t *)&dev_ctx.rel_humidity_attr.value,
		ZB_FALSE);

	ZB_ZCL_SET_ATTRIBUTE(
		APP_TEMPLATE_ENDPOINT,
		ZB_ZCL_CLUSTER_ID_REL_HUMIDITY_MEASUREMENT,
		ZB_ZCL_CLUSTER_SERVER_ROLE,
		ZB_ZCL_ATTR_REL_HUMIDITY_MEASUREMENT_MIN_VALUE_ID,
		(zb_uint8_t *)&dev_ctx.rel_humidity_attr.min_value,
		ZB_FALSE);

	ZB_ZCL_SET_ATTRIBUTE(
		APP_TEMPLATE_ENDPOINT,
		ZB_ZCL_CLUSTER_ID_REL_HUMIDITY_MEASUREMENT,
		ZB_ZCL_CLUSTER_SERVER_ROLE,
		ZB_ZCL_ATTR_REL_HUMIDITY_MEASUREMENT_MAX_VALUE_ID,
		(zb_uint8_t *)&dev_ctx.rel_humidity_attr.max_value,
		ZB_FALSE);

	// Modify min reporting interval period
	zb_zcl_reporting_info_t *rep_info;

	rep_info = zb_zcl_find_reporting_info(APP_TEMPLATE_ENDPOINT, ZB_ZCL_CLUSTER_ID_REL_HUMIDITY_MEASUREMENT, ZB_ZCL_CLUSTER_SERVER_ROLE, ZB_ZCL_ATTR_REL_HUMIDITY_MEASUREMENT_VALUE_ID);

	if (rep_info) {
	    rep_info->u.send_info.def_min_interval = 60; // 1 minute
	} else {
	    LOG_ERR("Can't find HUMIDITY attribute");
	}
}

/**@brief Function to toggle the identify LED
 *
 * @param  bufid  Unused parameter, required by ZBOSS scheduler API.
 */
static void toggle_identify_led(zb_bufid_t bufid)
{
	static int blink_status;

	dk_set_led(IDENTIFY_LED, (++blink_status) % 2);
	ZB_SCHEDULE_APP_ALARM(toggle_identify_led, bufid, ZB_MILLISECONDS_TO_BEACON_INTERVAL(100));
}

/**@brief Function to handle identify notification events on the first endpoint.
 *
 * @param  bufid  Unused parameter, required by ZBOSS scheduler API.
 */
static void identify_cb(zb_bufid_t bufid)
{
	zb_ret_t zb_err_code;

	if (bufid) {
		/* Schedule a self-scheduling function that will toggle the LED */
		ZB_SCHEDULE_APP_CALLBACK(toggle_identify_led, bufid);
	} else {
		/* Cancel the toggling function alarm and turn off LED */
		zb_err_code = ZB_SCHEDULE_APP_ALARM_CANCEL(toggle_identify_led, ZB_ALARM_ANY_PARAM);
		ZVUNUSED(zb_err_code);

		dk_set_led(IDENTIFY_LED, 0);
	}
}

/**@brief Starts identifying the device.
 *
 * @param  bufid  Unused parameter, required by ZBOSS scheduler API.
 */
static void start_identifying(zb_bufid_t bufid)
{
	ZVUNUSED(bufid);

	if (ZB_JOINED()) {
		/* Check if endpoint is in identifying mode,
		 * if not put desired endpoint in identifying mode.
		 */
		if (dev_ctx.identify_attr.identify_time ==
		ZB_ZCL_IDENTIFY_IDENTIFY_TIME_DEFAULT_VALUE) {

			zb_ret_t zb_err_code = zb_bdb_finding_binding_target(
				APP_TEMPLATE_ENDPOINT);

			if (zb_err_code == RET_OK) {
				LOG_INF("Enter identify mode");
			} else if (zb_err_code == RET_INVALID_STATE) {
				LOG_WRN("RET_INVALID_STATE - Cannot enter identify mode");
			} else {
				ZB_ERROR_CHECK(zb_err_code);
			}
		} else {
			LOG_INF("Cancel identify mode");
			zb_bdb_finding_binding_target_cancel();
		}
	} else {
		LOG_WRN("Device not in a network - cannot enter identify mode");
	}
}

/**@brief Callback for button events.
 *
 * @param[in]   button_state  Bitmask containing buttons state.
 * @param[in]   has_changed   Bitmask containing buttons
 *                            that have changed their state.
 */
static void button_changed(uint32_t button_state, uint32_t has_changed)
{
	if (IDENTIFY_MODE_BUTTON & has_changed) {
		if (IDENTIFY_MODE_BUTTON & button_state) {
			/* Button changed its state to pressed */
		} else {
			/* Button changed its state to released */
			if (was_factory_reset_done()) {
				/* The long press was for Factory Reset */
				LOG_DBG("After Factory Reset - ignore button release");
			} else   {
				/* Button released before Factory Reset */

				/* Start identification mode */
				ZB_SCHEDULE_APP_CALLBACK(start_identifying, 0);
			}
		}
	}

	check_factory_reset_button(button_state, has_changed);
}

/**@brief Function for initializing LEDs and Buttons. */
static void configure_gpio(void)
{
	int err;

	err = dk_buttons_init(button_changed);
	if (err) {
		LOG_ERR("Cannot init buttons (err: %d)", err);
	}

	err = dk_leds_init();
	if (err) {
		LOG_ERR("Cannot init LEDs (err: %d)", err);
	}

	if (!gpio_is_ready_dt(&probe_vdd)) {
	    LOG_ERR("Can't get GPIO1.15 ready");
	} else
	if (gpio_pin_configure_dt(&probe_vdd, GPIO_OUTPUT_INACTIVE) < 0) {
	    LOG_ERR("Can't configure GPIO1.15");
	}
}

/**@brief Function for turning ON/OFF the light bulb.
 *
 * @param[in]   on   Boolean light bulb state.
 */
static void on_off_set_value(zb_bool_t on)
{
	LOG_INF("Set ON/OFF value: %i", on);

	ZB_ZCL_SET_ATTRIBUTE(
		APP_TEMPLATE_ENDPOINT,
		ZB_ZCL_CLUSTER_ID_ON_OFF,
		ZB_ZCL_CLUSTER_SERVER_ROLE,
		ZB_ZCL_ATTR_ON_OFF_ON_OFF_ID,
		(zb_uint8_t *)&on,
		ZB_FALSE);

	dk_set_led(ON_OFF_LED, on);
}

/**@brief Callback function for handling ZCL commands.
 *
 * @param[in]   bufid   Reference to Zigbee stack buffer
 *                      used to pass received data.
 */
static void zcl_device_cb(zb_bufid_t bufid)
{
	zb_uint8_t cluster_id;
	zb_uint8_t attr_id;
	zb_zcl_device_callback_param_t  *device_cb_param =
		ZB_BUF_GET_PARAM(bufid, zb_zcl_device_callback_param_t);

	LOG_INF("%s id %hd", __func__, device_cb_param->device_cb_id);

	/* Set default response value. */
	device_cb_param->status = RET_OK;

	switch (device_cb_param->device_cb_id) {
	case ZB_ZCL_SET_ATTR_VALUE_CB_ID:
		cluster_id = device_cb_param->cb_param.
			     set_attr_value_param.cluster_id;
		attr_id = device_cb_param->cb_param.
			  set_attr_value_param.attr_id;

		if (cluster_id == ZB_ZCL_CLUSTER_ID_ON_OFF) {
			uint8_t value =
				device_cb_param->cb_param.set_attr_value_param
				.values.data8;

			LOG_INF("on/off attribute setting to %hd", value);
			if (attr_id == ZB_ZCL_ATTR_ON_OFF_ON_OFF_ID) {
				on_off_set_value((zb_bool_t)value);
			}
		} else {
			/* Other clusters can be processed here */
			LOG_INF("Unhandled cluster attribute id: %d",
				cluster_id);
			device_cb_param->status = RET_NOT_IMPLEMENTED;
		}
		break;

	default:
		device_cb_param->status = RET_NOT_IMPLEMENTED;
		break;
	}

	LOG_INF("%s status: %hd", __func__, device_cb_param->status);
}

/**@brief Zigbee stack event handler.
 *
 * @param[in]   bufid   Reference to the Zigbee stack buffer
 *                      used to pass signal.
 */
void zboss_signal_handler(zb_bufid_t bufid)
{
	/* Update network status LED. */
	zigbee_led_status_update(bufid, ZIGBEE_NETWORK_STATE_LED);

	/* No application-specific behavior is required.
	 * Call default signal handler.
	 */
	ZB_ERROR_CHECK(zigbee_default_signal_handler(bufid));

	/* All callbacks should either reuse or free passed buffers.
	 * If bufid == 0, the buffer is invalid (not passed).
	 */
	if (bufid) {
		zb_buf_free(bufid);
	}
}

int adc_setup(void); // In adc.c
int32_t adc_run(void); // In adc.c

#define PROBE_INTERVAL_MS 10000

void do_humidity_measurement(zb_uint8_t param) {
    // These comes from Capacitive Soil Moisture Sensor v1.2 powered by 3.3V
#define MIN_MV 910
#define MAX_MV 2160

	int32_t val_mv;
	uint16_t humidity;
	static uint16_t humidity_last = 0xffff;

	// Power on the probe
	//gpio_pin_set_dt(&probe_vdd,1);
	//k_msleep(1000); // Wait for output to stabilize

	// Measurement
	val_mv = adc_run();

	// Power off the probe
	//gpio_pin_set_dt(&probe_vdd,0);

	if (val_mv < MIN_MV) {
	    humidity = 100; // Max humidity
	} else if (val_mv > MAX_MV) {
	    humidity = 0; // Min humidity
	} else {
	    // MIN_MV => 100% (water)
	    // MAX_MV => 0% (air)
	    humidity = 100-((val_mv - MIN_MV)*100/(MAX_MV-MIN_MV));
	}

	// Low filter
	if (humidity_last != 0xffff) {
	    humidity = (humidity_last + humidity)/2;
	}

	if (humidity != humidity_last) {
	    dev_ctx.rel_humidity_attr.value = 100*humidity;

	    ZB_ZCL_SET_ATTRIBUTE(
		    APP_TEMPLATE_ENDPOINT,
		    ZB_ZCL_CLUSTER_ID_REL_HUMIDITY_MEASUREMENT,
		    ZB_ZCL_CLUSTER_SERVER_ROLE,
		    ZB_ZCL_ATTR_REL_HUMIDITY_MEASUREMENT_VALUE_ID,
		    (zb_uint8_t *)&dev_ctx.rel_humidity_attr.value,
		    ZB_FALSE);

	    humidity_last = humidity;

	    LOG_INF("Updating humidity value: %d%%", humidity);
	}

	ZB_SCHEDULE_APP_ALARM(do_humidity_measurement, 0, ZB_MILLISECONDS_TO_BEACON_INTERVAL(PROBE_INTERVAL_MS));
}

int main(void)
{
	LOG_INF("Starting ADC reading on AIN0");
	adc_setup();

	LOG_INF("Starting Zigbee application template example");

	/* Initialize */
	configure_gpio();
	register_factory_reset_button(FACTORY_RESET_BUTTON);

	/* Configure trace */
	ZB_SET_TRACE_LEVEL(4);
	ZB_SET_TRACE_MASK(0x0800);

	/* Register callback for handling ZCL commands. */
	ZB_ZCL_REGISTER_DEVICE_CB(zcl_device_cb);

	/* Register device context (endpoints). */
	ZB_AF_REGISTER_DEVICE_CTX(&app_template_ctx);

	app_clusters_attr_init();

	/* Register handlers to identify notifications */
	ZB_AF_SET_IDENTIFY_NOTIFICATION_HANDLER(APP_TEMPLATE_ENDPOINT, identify_cb);

	/* Start Zigbee default thread */
	zigbee_enable();

	/* Start reporting */
	if (RET_OK != zb_zcl_start_attr_reporting(APP_TEMPLATE_ENDPOINT,
						  ZB_ZCL_CLUSTER_ID_REL_HUMIDITY_MEASUREMENT,
						  ZB_ZCL_CLUSTER_SERVER_ROLE,
						  ZB_ZCL_ATTR_REL_HUMIDITY_MEASUREMENT_VALUE_ID)) {
	    LOG_INF("Failed to start Attribute reporting");
	}

	LOG_INF("Zigbee application template started");

	ZB_SCHEDULE_APP_ALARM(do_humidity_measurement, 0, ZB_MILLISECONDS_TO_BEACON_INTERVAL(2000));

	return 0;
}
