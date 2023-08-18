/*
 * Copyright (c) 2022 Nordic Semiconductor ASA
 *
 * SPDX-License-Identifier: LicenseRef-Nordic-5-Clause
 */

#ifndef ZB_SWIFT_DEVICE_H
#define ZB_SWIFT_DEVICE_H 1

/**
 *  @defgroup ZB_DEFINE_DEVICE_SWIFT_DEVICE
 *  @{
 *  @details
 *      - @ref ZB_ZCL_IDENTIFY \n
 *      - @ref ZB_ZCL_BASIC
 */

/** Swift Device ID*/
#define ZB_SWIFT_DEVICE_DEVICE_ID 0x0008

/** Swift Device version */
#define ZB_DEVICE_VER_SWIFT_DEVICE 0

/** @cond internals_doc */

/** Swift Device IN (server) clusters number */
#define ZB_SWIFT_DEVICE_IN_CLUSTER_NUM 4

/** Swift Device OUT (client) clusters number */
#define ZB_SWIFT_DEVICE_OUT_CLUSTER_NUM 0

#define ZB_SWIFT_DEVICE_CLUSTER_NUM \
	(ZB_SWIFT_DEVICE_IN_CLUSTER_NUM + ZB_SWIFT_DEVICE_OUT_CLUSTER_NUM)

/** Number of attribute for reporting on Swift Device */
#define ZB_SWIFT_DEVICE_REPORT_ATTR_COUNT (ZB_ZCL_ON_OFF_REPORT_ATTR_COUNT + ZB_ZCL_REL_HUMIDITY_MEASUREMENT_REPORT_ATTR_COUNT)

/** Missing attributes structure declaration **/
typedef struct {
    zb_uint16_t value;
    zb_uint16_t min_value;
    zb_uint16_t max_value;
} zb_zcl_rel_humidity_attrs_t;

/** @endcond */ /* internals_doc */

/**
 * @brief Declare cluster list for Swift Device device
 * @param cluster_list_name - cluster list variable name
 * @param basic_attr_list - attribute list for Basic cluster
 * @param identify_attr_list - attribute list for Identify cluster
 */
#define ZB_DECLARE_SWIFT_DEVICE_CLUSTER_LIST(			      \
		cluster_list_name,				      \
		basic_attr_list,				      \
		identify_attr_list,				      \
		on_off_attr_list,				      \
		rh_humidity_attr_list)				      \
zb_zcl_cluster_desc_t cluster_list_name[] =			      \
{								      \
	ZB_ZCL_CLUSTER_DESC(					      \
		ZB_ZCL_CLUSTER_ID_IDENTIFY,			      \
		ZB_ZCL_ARRAY_SIZE(identify_attr_list, zb_zcl_attr_t), \
		(identify_attr_list),				      \
		ZB_ZCL_CLUSTER_SERVER_ROLE,			      \
		ZB_ZCL_MANUF_CODE_INVALID			      \
	),							      \
	ZB_ZCL_CLUSTER_DESC(					      \
		ZB_ZCL_CLUSTER_ID_BASIC,			      \
		ZB_ZCL_ARRAY_SIZE(basic_attr_list, zb_zcl_attr_t),    \
		(basic_attr_list),				      \
		ZB_ZCL_CLUSTER_SERVER_ROLE,			      \
		ZB_ZCL_MANUF_CODE_INVALID			      \
	),							      \
	ZB_ZCL_CLUSTER_DESC(					      \
		ZB_ZCL_CLUSTER_ID_ON_OFF,			      \
		ZB_ZCL_ARRAY_SIZE(on_off_attr_list, zb_zcl_attr_t),   \
		(on_off_attr_list),				      \
		ZB_ZCL_CLUSTER_SERVER_ROLE,			      \
		ZB_ZCL_MANUF_CODE_INVALID			      \
	),							      \
	ZB_ZCL_CLUSTER_DESC(					      \
		ZB_ZCL_CLUSTER_ID_REL_HUMIDITY_MEASUREMENT,			      \
		ZB_ZCL_ARRAY_SIZE(rh_humidity_attr_list, zb_zcl_attr_t),   \
		(rh_humidity_attr_list),				      \
		ZB_ZCL_CLUSTER_SERVER_ROLE,			      \
		ZB_ZCL_MANUF_CODE_INVALID			      \
	)							      \
}

/** @cond internals_doc */

/**
 * @brief Declare simple descriptor for Swift Device
 * @param ep_name - endpoint variable name
 * @param ep_id - endpoint ID
 * @param in_clust_num - number of supported input clusters
 * @param out_clust_num - number of supported output clusters
 */
#define ZB_ZCL_DECLARE_SWIFT_DEVICE_SIMPLE_DESC(ep_name, ep_id, in_clust_num, out_clust_num) \
	ZB_DECLARE_SIMPLE_DESC(in_clust_num, out_clust_num);				       \
	ZB_AF_SIMPLE_DESC_TYPE(in_clust_num, out_clust_num) simple_desc_##ep_name =	       \
	{										       \
		ep_id,									       \
		ZB_AF_HA_PROFILE_ID,							       \
		ZB_SWIFT_DEVICE_DEVICE_ID,						       \
		ZB_DEVICE_VER_SWIFT_DEVICE,						       \
		0,									       \
		in_clust_num,								       \
		out_clust_num,								       \
		{									       \
			ZB_ZCL_CLUSTER_ID_BASIC,					       \
			ZB_ZCL_CLUSTER_ID_IDENTIFY,					       \
			ZB_ZCL_CLUSTER_ID_ON_OFF,					       \
			ZB_ZCL_CLUSTER_ID_REL_HUMIDITY_MEASUREMENT			       \
		}									       \
	}

/** @endcond */ /* internals_doc */

/**
 * @brief Declare endpoint for Swift Device
 * @param ep_name - endpoint variable name
 * @param ep_id - endpoint ID
 * @param cluster_list - endpoint cluster list
 */
#define ZB_DECLARE_SWIFT_DEVICE_EP(ep_name, ep_id, cluster_list)		      \
	ZB_ZCL_DECLARE_SWIFT_DEVICE_SIMPLE_DESC(ep_name, ep_id,		      \
		ZB_SWIFT_DEVICE_IN_CLUSTER_NUM, ZB_SWIFT_DEVICE_OUT_CLUSTER_NUM); \
	ZBOSS_DEVICE_DECLARE_REPORTING_CTX(reporting_info## ep_name,		      \
		ZB_SWIFT_DEVICE_REPORT_ATTR_COUNT);				      \
	ZB_AF_DECLARE_ENDPOINT_DESC(ep_name, ep_id, ZB_AF_HA_PROFILE_ID, 0, NULL,     \
		ZB_ZCL_ARRAY_SIZE(cluster_list, zb_zcl_cluster_desc_t), cluster_list, \
			(zb_af_simple_desc_1_1_t *)&simple_desc_##ep_name,	      \
			ZB_SWIFT_DEVICE_REPORT_ATTR_COUNT, reporting_info## ep_name,  \
			0, NULL) /* No CVC ctx */

/*! @} */

#endif /* ZB_SWIFT_DEVICE_H */
