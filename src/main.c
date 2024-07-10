/*
 * Copyright (c) 2021 Nordic Semiconductor ASA
 *
 * SPDX-License-Identifier: LicenseRef-Nordic-5-Clause
 */

/** @file
 *
* @brief Zigbee application swift.
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
#include "adc.h"

static const struct gpio_dt_spec probe_vdd = GPIO_DT_SPEC_GET(DT_NODELABEL(probe_vdd), gpios);

/* Device endpoint, used to receive ZCL commands. */
#define APP_SWIFT_ENDPOINT               10

/* Type of power sources available for the device.
 * For possible values see section 3.2.2.2.8 of ZCL specification.
 */
#define SWIFT_INIT_BASIC_POWER_SOURCE    ZB_ZCL_BASIC_POWER_SOURCE_BATTERY

/* LED indicating that device successfully joined Zigbee network. */
#define ZIGBEE_NETWORK_STATE_LED            DK_LED1

/* Probe measurement interval */
#define PROBE_INTERVAL_MS 60000

LOG_MODULE_REGISTER(app, LOG_LEVEL_INF);

/* Main application customizable context.
 * Stores all settings and static values.
 */
struct zb_device_ctx {
	zb_zcl_basic_attrs_ext_t basic_attr;
	zb_zcl_power_config_attrs_t power_config_attr;
	zb_zcl_rel_humidity_attrs_t rel_humidity_attr;
};

/* Zigbee device application context storage. */
static struct zb_device_ctx dev_ctx;

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

ZB_ZCL_DECLARE_REL_HUMIDITY_MEASUREMENT_ATTRIB_LIST(
	rel_humidity_attr_list,
	&dev_ctx.rel_humidity_attr.value,
	&dev_ctx.rel_humidity_attr.min_value,
	&dev_ctx.rel_humidity_attr.max_value
);

ZB_ZCL_DECLARE_POWER_CONFIG_ATTRIB_LIST2(
	power_config_attr_list,
	&dev_ctx.power_config_attr.voltage,
	&dev_ctx.power_config_attr.percentage_remaining,
	&dev_ctx.power_config_attr.alarm_state
);

ZB_DECLARE_SWIFT_DEVICE_CLUSTER_LIST(app_swift_clusters, basic_attr_list, power_config_attr_list, rel_humidity_attr_list);

ZB_DECLARE_SWIFT_DEVICE_EP(
	app_swift_ep,
	APP_SWIFT_ENDPOINT,
	app_swift_clusters);

ZBOSS_DECLARE_DEVICE_CTX_1_EP(
	app_swift_ctx,
	app_swift_ep);

/* Manufacturer name (32 bytes). */
#define SWIFT_INIT_BASIC_MANUF_NAME      "Swift"

/* Model number assigned by manufacturer (32-bytes long string). */
#define SWIFT_INIT_BASIC_MODEL_ID        "Soil Moisture Sensor"

/* Functions */
void do_battery_measurement();
void do_humidity_measurement(zb_uint8_t param);
void check_join_status(zb_uint8_t param);

/**@brief Function for initializing all clusters attributes. */
static void app_clusters_attr_init(void)
{
	/* Basic cluster attributes data */
	dev_ctx.basic_attr.zcl_version = ZB_ZCL_VERSION;
	dev_ctx.basic_attr.power_source = SWIFT_INIT_BASIC_POWER_SOURCE;

	ZB_ZCL_SET_STRING_VAL(
		dev_ctx.basic_attr.mf_name,
		SWIFT_INIT_BASIC_MANUF_NAME,
		ZB_ZCL_STRING_CONST_SIZE(SWIFT_INIT_BASIC_MANUF_NAME));

	ZB_ZCL_SET_STRING_VAL(
		dev_ctx.basic_attr.model_id,
		SWIFT_INIT_BASIC_MODEL_ID,
		ZB_ZCL_STRING_CONST_SIZE(SWIFT_INIT_BASIC_MODEL_ID));

	/* Power Config attributes data. */
	dev_ctx.power_config_attr.voltage = ZB_ZCL_POWER_CONFIG_BATTERY_VOLTAGE_INVALID;

	do_battery_measurement();

	/* Relative Humidity cluster attributes data. */
	dev_ctx.rel_humidity_attr.value = ZB_ZCL_ATTR_REL_HUMIDITY_MEASUREMENT_VALUE_UNKNOWN;
	dev_ctx.rel_humidity_attr.min_value = ZB_ZCL_ATTR_REL_HUMIDITY_MEASUREMENT_MIN_VALUE_MIN_VALUE;
	dev_ctx.rel_humidity_attr.max_value = ZB_ZCL_ATTR_REL_HUMIDITY_MEASUREMENT_MIN_VALUE_MAX_VALUE;

	ZB_ZCL_SET_ATTRIBUTE(
		APP_SWIFT_ENDPOINT,
		ZB_ZCL_CLUSTER_ID_REL_HUMIDITY_MEASUREMENT,
		ZB_ZCL_CLUSTER_SERVER_ROLE,
		ZB_ZCL_ATTR_REL_HUMIDITY_MEASUREMENT_VALUE_ID,
		(zb_uint8_t *)&dev_ctx.rel_humidity_attr.value,
		ZB_FALSE);

	ZB_ZCL_SET_ATTRIBUTE(
		APP_SWIFT_ENDPOINT,
		ZB_ZCL_CLUSTER_ID_REL_HUMIDITY_MEASUREMENT,
		ZB_ZCL_CLUSTER_SERVER_ROLE,
		ZB_ZCL_ATTR_REL_HUMIDITY_MEASUREMENT_MIN_VALUE_ID,
		(zb_uint8_t *)&dev_ctx.rel_humidity_attr.min_value,
		ZB_FALSE);

	ZB_ZCL_SET_ATTRIBUTE(
		APP_SWIFT_ENDPOINT,
		ZB_ZCL_CLUSTER_ID_REL_HUMIDITY_MEASUREMENT,
		ZB_ZCL_CLUSTER_SERVER_ROLE,
		ZB_ZCL_ATTR_REL_HUMIDITY_MEASUREMENT_MAX_VALUE_ID,
		(zb_uint8_t *)&dev_ctx.rel_humidity_attr.max_value,
		ZB_FALSE);

	// Modify min reporting interval period
	zb_zcl_reporting_info_t *rep_info;

	rep_info = zb_zcl_find_reporting_info(APP_SWIFT_ENDPOINT, ZB_ZCL_CLUSTER_ID_REL_HUMIDITY_MEASUREMENT, ZB_ZCL_CLUSTER_SERVER_ROLE, ZB_ZCL_ATTR_REL_HUMIDITY_MEASUREMENT_VALUE_ID);

	if (rep_info) {
	    rep_info->u.send_info.def_min_interval = PROBE_INTERVAL_MS/1000;
	    rep_info->u.send_info.def_max_interval = 7200; // 2 hours
	} else {
	    LOG_ERR("Can't find HUMIDITY attribute");
	}

	rep_info = zb_zcl_find_reporting_info(APP_SWIFT_ENDPOINT, ZB_ZCL_CLUSTER_ID_POWER_CONFIG, ZB_ZCL_CLUSTER_SERVER_ROLE, ZB_ZCL_ATTR_POWER_CONFIG_BATTERY_PERCENTAGE_REMAINING_ID);

	if (rep_info) {
	    rep_info->u.send_info.def_min_interval = PROBE_INTERVAL_MS/1000;
	    rep_info->u.send_info.def_max_interval = 7200; // 2 hours
	} else {
	    LOG_ERR("Can't find POWER CONFIG attribute");
	}
}

/**@brief Function for initializing LEDs and Buttons. */
static void configure_gpio(void)
{
	int err;

	err = dk_buttons_init(NULL);
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

/**@brief Callback function for handling ZCL commands.
 *
 * @param[in]   bufid   Reference to Zigbee stack buffer
 *                      used to pass received data.
 */
static void zcl_device_cb(zb_bufid_t bufid)
{
	zb_zcl_device_callback_param_t  *device_cb_param =
		ZB_BUF_GET_PARAM(bufid, zb_zcl_device_callback_param_t);

	LOG_INF("%s id %hd", __func__, device_cb_param->device_cb_id);

	/* Set default response value. */
	device_cb_param->status = RET_OK;

	switch (device_cb_param->device_cb_id) {
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
	static uint8_t setup_poll_interval = 1;

	/* Change long poll interval once device has joined */
	if (setup_poll_interval && ZB_JOINED()) {
	    zb_zdo_pim_set_long_poll_interval(PROBE_INTERVAL_MS);
	    setup_poll_interval = 0;
	}

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

#define BATTERY_HIGH_100MV 30
#define BATTERY_LOW_100MV 16

void do_battery_measurement() {
	uint8_t battery_voltage;
	static uint8_t last_battery_voltage = 0xff;

	battery_voltage = adc_battery(); // 100mv per unit

	dev_ctx.power_config_attr.voltage = battery_voltage;

	if (battery_voltage > BATTERY_HIGH_100MV) {
	    dev_ctx.power_config_attr.percentage_remaining = 200; // 200 half percent
	} else if (battery_voltage < BATTERY_LOW_100MV) {
	    dev_ctx.power_config_attr.percentage_remaining = 0;
	} else {
	    dev_ctx.power_config_attr.percentage_remaining = (battery_voltage-BATTERY_LOW_100MV)*200/(BATTERY_HIGH_100MV-BATTERY_LOW_100MV);
	}

	LOG_INF("Battery voltage (capacity): %d mv (%d%%)", battery_voltage*100, dev_ctx.power_config_attr.percentage_remaining/2);

	if (last_battery_voltage != battery_voltage) {
	    ZB_ZCL_SET_ATTRIBUTE(
		    APP_SWIFT_ENDPOINT,
		    ZB_ZCL_CLUSTER_ID_POWER_CONFIG,
		    ZB_ZCL_CLUSTER_SERVER_ROLE,
		    ZB_ZCL_ATTR_POWER_CONFIG_BATTERY_VOLTAGE_ID,
		    (zb_uint8_t *)&dev_ctx.power_config_attr.voltage,
		    ZB_FALSE);

	    ZB_ZCL_SET_ATTRIBUTE(
		    APP_SWIFT_ENDPOINT,
		    ZB_ZCL_CLUSTER_ID_POWER_CONFIG,
		    ZB_ZCL_CLUSTER_SERVER_ROLE,
		    ZB_ZCL_ATTR_POWER_CONFIG_BATTERY_PERCENTAGE_REMAINING_ID,
		    (zb_uint8_t *)&dev_ctx.power_config_attr.percentage_remaining,
		    ZB_FALSE);

	    last_battery_voltage = battery_voltage;

	    LOG_INF("Reporting battery remaining percentage");
	}
}

void do_humidity_measurement(zb_uint8_t param) {
#ifdef VDD_3V
    // These comes from Capacitive Soil Moisture Sensor v1.2 powered by 3.0V
#pragma message("Probe supply 3V")
#define MIN_MV 450
#define MAX_MV 1825
#else
    // These comes from Capacitive Soil Moisture Sensor v1.2 powered by 3.3V
#define MIN_MV 910
#define MAX_MV 2160
#endif

#define NB_SAMPLES 10
#define PROBE_POWERUP_TIME_MS 500
#define COUNTDOWN_INIT (4*3600*1000/PROBE_INTERVAL_MS)

	int32_t val_mv;
	static int32_t val_mv_samples[NB_SAMPLES];
	static int val_mv_cur = 0;
	static int32_t val_mv_sum = -1;
	uint16_t humidity; // 100 x H%
	static uint16_t humidity_last = 0xffff;
	static uint32_t force_report_countdown = COUNTDOWN_INIT; // When falling to 0, 4 hours, force reporting

	// Power on the probe
	gpio_pin_set_dt(&probe_vdd,1);
	k_msleep(PROBE_POWERUP_TIME_MS); // Wait for output to stabilize

	// Measurement
	val_mv = adc_probe();

	// Power off the probe
	gpio_pin_set_dt(&probe_vdd,0);

	// Check minimum valid measurement
	if (val_mv < 100) {
	    LOG_WRN("Probe certainly unplugged. Retesting in 2 seconds.");
	    ZB_SCHEDULE_APP_ALARM(do_humidity_measurement, 0, ZB_MILLISECONDS_TO_BEACON_INTERVAL(2000)); // Come back to check in 2 secs
	    return;
	}

	// Low filtering
	if (val_mv_sum == -1) {
	    for(int i=0; i < NB_SAMPLES; i++) {
		val_mv_samples[i] = val_mv;
	    }
	    val_mv_sum = NB_SAMPLES*val_mv;
	} else {
	    val_mv_sum -= val_mv_samples[val_mv_cur];
	    val_mv_samples[val_mv_cur] = val_mv;
	    val_mv_sum += val_mv;
	    val_mv_cur = (val_mv_cur + 1) % NB_SAMPLES;
	}

	val_mv = val_mv_sum / NB_SAMPLES;

	if (val_mv < MIN_MV) {
	    humidity = 100; // Max humidity
	} else if (val_mv > MAX_MV) {
	    humidity = 0; // Min humidity
	} else {
	    // MIN_MV => 100% (water)
	    // MAX_MV => 0% (air)
	    humidity = 100-((val_mv - MIN_MV)*100/(MAX_MV-MIN_MV));
	}

	humidity *= 100;

	// Low filter
	if (humidity_last != 0xffff) {
	    humidity = (humidity_last*3 + humidity)/4;
	}

	LOG_INF("Mean %dmv -> Humidity %d [%d]", val_mv, humidity, humidity_last);

	if (humidity/100 != humidity_last/100 || (force_report_countdown-- == 0)) {
	    force_report_countdown = COUNTDOWN_INIT;

	    do_battery_measurement(); // Take opportunity to update battery health

	    dev_ctx.rel_humidity_attr.value = (humidity/10)*10; // Rounding at 10th

	    ZB_ZCL_SET_ATTRIBUTE(
		    APP_SWIFT_ENDPOINT,
		    ZB_ZCL_CLUSTER_ID_REL_HUMIDITY_MEASUREMENT,
		    ZB_ZCL_CLUSTER_SERVER_ROLE,
		    ZB_ZCL_ATTR_REL_HUMIDITY_MEASUREMENT_VALUE_ID,
		    (zb_uint8_t *)&dev_ctx.rel_humidity_attr.value,
		    ZB_FALSE);

	    LOG_INF("Updating humidity value: %d%%", humidity/100);
	}

	humidity_last = humidity;

	ZB_SCHEDULE_APP_ALARM(do_humidity_measurement, 0, ZB_MILLISECONDS_TO_BEACON_INTERVAL(PROBE_INTERVAL_MS));
}

#define NETWORK_LED_PERIOD_MS 200

void check_join_status(zb_uint8_t param) {
    static uint8_t led_state = 1;

    if (ZB_JOINED()) {
	// Light off LED and start measurements
	dk_set_led(ZIGBEE_NETWORK_STATE_LED, 0);
	do_humidity_measurement(0);
	return;
    }

    led_state ^= 1;

    dk_set_led(ZIGBEE_NETWORK_STATE_LED, led_state);

    ZB_SCHEDULE_APP_ALARM(check_join_status, 0, ZB_MILLISECONDS_TO_BEACON_INTERVAL(NETWORK_LED_PERIOD_MS));
}

int main(void)
{
	LOG_INF("Starting ADC reading on AIN0 and AIN1");
	adc_setup();

	LOG_INF("Starting Zigbee application swift example");

	/* Initialize */
	configure_gpio();

	/* Set Led on at startup */
	dk_set_led(ZIGBEE_NETWORK_STATE_LED, 1);

	/* Register callback for handling ZCL commands. */
	ZB_ZCL_REGISTER_DEVICE_CB(zcl_device_cb);

	/* Register device context (endpoints). */
	ZB_AF_REGISTER_DEVICE_CTX(&app_swift_ctx);

	app_clusters_attr_init();

	/* Set Sleepy End Device mode */
	zigbee_configure_sleepy_behavior(true);

	/* Start Zigbee default thread */
	zigbee_enable();

	if (dk_get_buttons() & DK_BTN3_MSK) {
	    LOG_INF("BUTTON 2 pressed at start up - Scheduling Factory Reset");
	    ZB_SCHEDULE_APP_CALLBACK(zb_bdb_reset_via_local_action, 0);
	}

	/* Start reporting */
	if (RET_OK != zb_zcl_start_attr_reporting(APP_SWIFT_ENDPOINT,
						  ZB_ZCL_CLUSTER_ID_REL_HUMIDITY_MEASUREMENT,
						  ZB_ZCL_CLUSTER_SERVER_ROLE,
						  ZB_ZCL_ATTR_REL_HUMIDITY_MEASUREMENT_VALUE_ID)) {
	    LOG_INF("Failed to start Attribute reporting");
	}

	if (RET_OK != zb_zcl_start_attr_reporting(APP_SWIFT_ENDPOINT,
						  ZB_ZCL_CLUSTER_ID_POWER_CONFIG,
						  ZB_ZCL_CLUSTER_SERVER_ROLE,
						  ZB_ZCL_ATTR_POWER_CONFIG_BATTERY_PERCENTAGE_REMAINING_ID)) {
	    LOG_INF("Failed to start Attribute reporting");
	}

	LOG_INF("Zigbee application swift started");

	ZB_SCHEDULE_APP_ALARM(check_join_status, 0, ZB_MILLISECONDS_TO_BEACON_INTERVAL(NETWORK_LED_PERIOD_MS));

	return 0;
}
