/* 
Copyright (c) 2013, The Linux Foundation. All rights reserved.

Redistribution and use in source and binary forms, with or without
modification, are permitted provided that the following conditions are
met:
    * Redistributions of source code must retain the above copyright
      notice, this list of conditions and the following disclaimer.
    * Redistributions in binary form must reproduce the above
      copyright notice, this list of conditions and the following
      disclaimer in the documentation and/or other materials provided
      with the distribution.
    * Neither the name of The Linux Foundation nor the names of its
      contributors may be used to endorse or promote products derived
      from this software without specific prior written permission.

THIS SOFTWARE IS PROVIDED "AS IS" AND ANY EXPRESS OR IMPLIED
WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES OF
MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND NON-INFRINGEMENT
ARE DISCLAIMED.  IN NO EVENT SHALL THE COPYRIGHT OWNER OR CONTRIBUTORS
BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR
CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF
SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR
BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY,
WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE
OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN
IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
*/
/*!
	@file
	IPACM_Defs.h

	@brief
	This file implements the common definitions amon all ifaces.

	@Author
	Skylar Chang

*/
#ifndef IPA_CM_DEFS_H
#define IPA_CM_DEFS_H

#include <unistd.h>
#include <fcntl.h>
#include <linux/msm_ipa.h>
#include "IPACM_Log.h"

extern "C"
{
#include <libnetfilter_conntrack/libnetfilter_conntrack.h>
#include <libnetfilter_conntrack/libnetfilter_conntrack_tcp.h>
}

#define IF_NAME_LEN 16
#define IPA_MAX_FILE_LEN  64
#define IPA_IFACE_NAME_LEN 16
#define IPA_ALG_PROTOCOL_NAME_LEN  10

#define IPA_WLAN_PARTIAL_HDR_OFFSET  0 // dst mac first then src mac
//#define IPA_ETH_PARTIAL_HDR_OFFSET  8 // dst mac first then src mac
#define IPA_ODU_PARTIAL_HDR_OFFSET  8 // dst mac first then src mac
#define IPA_WLAN_PARTIAL_HDR_NAME_v4  "IEEE802_3_v4"
#define IPA_WLAN_PARTIAL_HDR_NAME_v6  "IEEE802_3_v6"
#define IPA_WAN_PARTIAL_HDR_NAME_v4  "IEEE802_3_STA_v4"
#define IPA_WAN_PARTIAL_HDR_NAME_v6  "IEEE802_3_STA_v6"
#define IPA_ETH_HDR_NAME_v4  "IPACM_ETH_v4"
#define IPA_ETH_HDR_NAME_v6  "IPACM_ETH_v6"
#define IPA_ODU_HDR_NAME_v4  "IPACM_ODU_v4"
#define IPA_ODU_HDR_NAME_v6  "IPACM_ODU_v6"


#define IPA_MAX_IFACE_ENTRIES 15
#define IPA_MAX_PRIVATE_SUBNET_ENTRIES 3
#define IPA_MAX_ALG_ENTRIES 20
#define IPA_MAX_RM_ENTRY 6

#define V4_DEFAULT_ROUTE_TABLE_NAME  "ipa_dflt_rt"
#define V4_LAN_ROUTE_TABLE_NAME  "COMRTBLLANv4"
#define V4_WAN_ROUTE_TABLE_NAME  "WANRTBLv4"
#define WAN_DL_ROUTE_TABLE_NAME "ipa_dflt_wan_rt"
#define V6_COMMON_ROUTE_TABLE_NAME  "COMRTBLv6"
#define V6_WAN_ROUTE_TABLE_NAME  "WANRTBLv6"
#define V4_LAN_TO_LAN_ROUTE_TABLE_NAME "LANTOLANRTBLv4"
#define V6_LAN_TO_LAN_ROUTE_TABLE_NAME "LANTOLANRTBLv6"
#define V4_ODU_ROUTE_TABLE_NAME  "ODURTBLv4"
#define V6_ODU_ROUTE_TABLE_NAME  "ODURTBLv6"

#define ETH_BRIDGE_USB_WLAN_ROUTE_TABLE_NAME_V4 "ETH_BRIDGE_USB_WLAN_RTBLv4"
#define ETH_BRIDGE_WLAN_WLAN_ROUTE_TABLE_NAME_V4 "ETH_BRIDGE_WLAN_WLAN_RTBLv4"
#define ETH_BRIDGE_USB_WLAN_ROUTE_TABLE_NAME_V6 "ETH_BRIDGE_USB_WLAN_RTBLv6"
#define ETH_BRIDGE_WLAN_WLAN_ROUTE_TABLE_NAME_V6 "ETH_BRIDGE_WLAN_WLAN_RTBLv6"

#define WWAN_QMI_IOCTL_DEVICE_NAME "/dev/wwan_ioctl"
#define IPA_DEVICE_NAME "/dev/ipa"
#define IPA_MAX_FLT_RULE 50

#define MAX_OFFLOAD_PAIR 3
#define MAX_NUM_PROP 8
#define IPA_LAN_TO_LAN_USB_HDR_NAME_V4 "Lan2Lan_USB_v4"
#define IPA_LAN_TO_LAN_USB_HDR_NAME_V6 "Lan2Lan_USB_v6"
#define IPA_LAN_TO_LAN_WLAN_HDR_NAME_V4 "Lan2Lan_Wlan_v4"
#define IPA_LAN_TO_LAN_WLAN_HDR_NAME_V6 "Lan2Lan_Wlan_v6"
#define IPA_LAN_TO_LAN_MAX_WLAN_CLIENT 32
#define IPA_LAN_TO_LAN_MAX_USB_CLIENT 1
#define TCP_FIN_SHIFT 16
#define TCP_SYN_SHIFT 17
#define TCP_RST_SHIFT 18
#define NUM_TCP_CTL_FLT_RULE 3

/*---------------------------------------------------------------------------
										Return values indicating error status
---------------------------------------------------------------------------*/

#define IPACM_SUCCESS                0         /* Successful operation   */
#define IPACM_FAILURE               -1         /* Unsuccessful operation */

#define IPACM_IP_NULL (ipa_ip_type)0xFF
#define IPACM_INVALID_INDEX (ipa_ip_type)0xFF

#define IPA_MAX_NUM_WIFI_CLIENTS  32
#define IPA_MAX_NUM_WAN_CLIENTS  10
#define IPA_MAX_NUM_ETH_CLIENTS  15
#define IPA_MAX_NUM_AMPDU_RULE  15
#define IPA_MAC_ADDR_SIZE  6

/*===========================================================================
										 GLOBAL DEFINITIONS AND DECLARATIONS
===========================================================================*/
typedef enum
{
	IPA_CFG_CHANGE_EVENT = 1,                 /* 1 NULL */
	IPA_LINK_UP_EVENT,                        /* 2 ipacm_event_data_fid */
	IPA_LINK_DOWN_EVENT,                      /* 3 ipacm_event_data_fid */
	IPA_ADDR_ADD_EVENT,                       /* 4 ipacm_event_data_addr */
	IPA_ADDR_DEL_EVENT,                       /* 5 no use */
	IPA_ROUTE_ADD_EVENT,                      /* 6 ipacm_event_data_addr */
	IPA_ROUTE_DEL_EVENT,                      /* 7 ipacm_event_data_addr */
	IPA_FIREWALL_CHANGE_EVENT,                /* 8 NULL */
	IPA_WLAN_AP_LINK_UP_EVENT,                /* 9 ipacm_event_data_mac */
	IPA_WLAN_STA_LINK_UP_EVENT,               /* 10 ipacm_event_data_mac */
	IPA_WLAN_CLIENT_ADD_EVENT,                /* 11 ipacm_event_data_mac */
	IPA_WLAN_CLIENT_DEL_EVENT,                /* 12 ipacm_event_data_mac */
	IPA_WLAN_CLIENT_POWER_SAVE_EVENT,         /* 13 ipacm_event_data_mac */
	IPA_WLAN_CLIENT_RECOVER_EVENT,            /* 14 ipacm_event_data_mac */
	IPA_NEW_NEIGH_EVENT,                      /* 15 ipacm_event_data_all */
	IPA_DEL_NEIGH_EVENT,                      /* 16 ipacm_event_data_all */
	IPA_NEIGH_CLIENT_IP_ADDR_ADD_EVENT,       /* 17 ipacm_event_data_all */
	IPA_NEIGH_CLIENT_IP_ADDR_DEL_EVENT,       /* 18 ipacm_event_data_all */
	IPA_SW_ROUTING_ENABLE,                    /* 19 NULL */
	IPA_SW_ROUTING_DISABLE,                   /* 20 NULL */
	IPA_PROCESS_CT_MESSAGE,                   /* 21 ipacm_ct_evt_data */
	IPA_HANDLE_WAN_UP,                        /* 22 ipacm_event_iface_up  */
	IPA_HANDLE_WAN_DOWN,                      /* 23 ipacm_event_iface_up  */
	IPA_HANDLE_WLAN_UP,                       /* 24 ipacm_event_iface_up */
	IPA_HANDLE_LAN_UP,                        /* 25 ipacm_event_iface_up */
	IPA_WLAN_CLIENT_ADD_EVENT_EX,             /* 26 ipacm_event_data_wlan_ex */
	IPA_HANDLE_WAN_UP_V6,					  /* 27 NULL */
	IPA_HANDLE_WAN_DOWN_V6,					  /* 28 NULL */
	IPA_LAN_CLIENT_ACTIVE,					  /* 29 ipacm_event_lan_client*/
	IPA_LAN_CLIENT_INACTIVE,				  /* 30 ipacm_event_lan_client*/
	IPA_LAN_CLIENT_DISCONNECT,				  /* 31 ipacm_event_lan_client*/
	IPA_LAN_CLIENT_POWER_SAVE,				  /* 32 ipacm_event_lan_client*/
	IPA_LAN_CLIENT_POWER_RECOVER,			  /* 33 ipacm_event_lan_client*/
	IPA_LAN_TO_LAN_NEW_CONNECTION,			  /* 34 ipacm_event_connection */
	IPA_LAN_TO_LAN_DEL_CONNECTION,			  /* 35 ipacm_event_connection */
	IPA_LAN_DELETE_SELF,					  /* 36 ipacm_event_data_fid */
	IPA_WLAN_LINK_DOWN_EVENT,                 /* 37 ipacm_event_data_mac */
	IPA_USB_LINK_UP_EVENT,                    /* 38 ipacm_event_data_fid */
	IPA_PROCESS_CT_MESSAGE_V6,                /* 39 ipacm_ct_evt_data */
	IPA_PRIVATE_SUBNET_CHANGE_EVENT,          /* 40 ipacm_event_data_fid */
	IPA_WAN_UPSTREAM_ROUTE_ADD_EVENT,         /* 41 ipacm_event_data_fid */
	IPA_WAN_UPSTREAM_ROUTE_DEL_EVENT,         /* 42 ipacm_event_data_fid */
	IPA_WAN_EMBMS_LINK_UP_EVENT,              /* 43 ipacm_event_data_mac */
	IPA_ETH_BRIDGE_USB_CLIENT_ADD_EVENT,      /* 44 ipacm_event_data_mac */
	IPA_ETH_BRIDGE_WLAN_CLIENT_ADD_EVENT,     /* 45 ipacm_event_data_mac */
	IPA_ETH_BRIDGE_USB_CLIENT_DEL_EVENT,      /* 46 ipacm_event_data_mac */
	IPA_ETH_BRIDGE_WLAN_CLIENT_DEL_EVENT,     /* 47 ipacm_event_data_mac */
	IPA_ETH_BRIDGE_HDR_PROC_CTX_SET_EVENT,    /* 48 ipacm_event_data_if_cat */
	IPA_ETH_BRIDGE_HDR_PROC_CTX_UNSET_EVENT,  /* 49 ipacm_event_data_if_cat */
	IPA_WLAN_SWITCH_TO_SCC,                   /* 50 No Data */
	IPA_WLAN_SWITCH_TO_MCC,                   /* 51 No Data */
	IPA_CRADLE_WAN_MODE_SWITCH,               /* 52 ipacm_event_cradle_wan_mode */
	IPA_WAN_XLAT_CONNECT_EVENT,               /* 53 ipacm_event_data_fid */
	IPA_TETHERING_STATS_UPDATE_EVENT,         /* 54 ipacm_event_data_fid */
	IPA_NETWORK_STATS_UPDATE_EVENT,           /* 55 ipacm_event_data_fid */
	IPA_HANDLE_WAN_UP_TETHER,                 /* 56 ipacm_event_iface_up_tehter */
	IPA_HANDLE_WAN_DOWN_TETHER,               /* 57 ipacm_event_iface_up_tehter */
	IPA_HANDLE_WAN_UP_V6_TETHER,		  /* 58 ipacm_event_iface_up_tehter */
	IPA_HANDLE_WAN_DOWN_V6_TETHER,		  /* 59 ipacm_event_iface_up_tehter */
	IPACM_EVENT_MAX
} ipa_cm_event_id;

typedef struct
{
	uint8_t num_rule;
	uint32_t rule_hdl[MAX_NUM_PROP];
} lan_to_lan_rt_rule_hdl;

typedef enum
{
	LAN_IF = 0,
	WLAN_IF,
	WAN_IF,
	VIRTUAL_IF,
	ETH_IF,
	EMBMS_IF,
	ODU_IF,
	UNKNOWN_IF
} ipacm_iface_type;

typedef struct
{
	struct nf_conntrack *ct;
	enum nf_conntrack_msg_type type;
}ipacm_ct_evt_data;

typedef struct
{
	char iface_name[IPA_IFACE_NAME_LEN];
	ipacm_iface_type if_cat;
	int netlink_interface_index;
} ipa_ifi_dev_name_t;

typedef struct
{
	uint32_t subnet_addr;
	uint32_t subnet_mask;
} ipa_private_subnet;


typedef struct _ipacm_event_data_all
{
	enum ipa_ip_type iptype;
	int if_index;
	uint32_t  ipv4_addr;
	uint32_t  ipv6_addr[4];
	uint8_t mac_addr[IPA_MAC_ADDR_SIZE];
} ipacm_event_data_all;

class IPACM_Lan;

typedef struct
{
	enum ipa_ip_type iptype;
	uint32_t ipv4_addr;
	uint32_t ipv6_addr[4];
	uint8_t mac_addr[6];
	IPACM_Lan* p_iface;
} ipacm_event_lan_client;

typedef struct
{
	enum ipa_ip_type iptype;
	uint32_t src_ipv4_addr;
	uint32_t dst_ipv4_addr;
	uint32_t src_ipv6_addr[4];
	uint32_t dst_ipv6_addr[4];
} ipacm_event_connection;

typedef struct _ipacm_event_data_fid
{
	int if_index;
} ipacm_event_data_fid;

typedef struct _ipacm_event_data_iptype
{
	int if_index;
	int if_index_tether;
	enum ipa_ip_type iptype;
} ipacm_event_data_iptype;


typedef struct _ipacm_event_data_addr
{
	enum ipa_ip_type iptype;
	int if_index;
	uint32_t  ipv4_addr;
	uint32_t  ipv4_addr_mask;
	uint32_t  ipv6_addr[4];
	uint32_t  ipv6_addr_mask[4];
} ipacm_event_data_addr;

typedef struct _ipacm_event_data_mac
{
	int if_index;
	uint8_t mac_addr[IPA_MAC_ADDR_SIZE];
} ipacm_event_data_mac;

typedef struct
{
	int if_index;
	uint8_t num_of_attribs;
	struct ipa_wlan_hdr_attrib_val attribs[0];
} ipacm_event_data_wlan_ex;

typedef struct _ipacm_event_iface_up
{
	char ifname[IPA_IFACE_NAME_LEN];
	uint32_t ipv4_addr;
	uint32_t addr_mask;
	uint32_t ipv6_prefix[2];
	bool is_sta;
}ipacm_event_iface_up;

typedef struct _ipacm_event_iface_up_tether
{
	uint32_t if_index_tether;
	uint32_t ipv6_prefix[2];
	bool is_sta;
}ipacm_event_iface_up_tehter;

typedef enum
{
	Q6_WAN = 0,
	WLAN_WAN,
	ECM_WAN
} ipacm_wan_iface_type;

typedef struct _ipacm_ifacemgr_data
{
	int if_index;
	ipacm_wan_iface_type if_type;
	uint8_t mac_addr[IPA_MAC_ADDR_SIZE];
}ipacm_ifacemgr_data;

#endif /* IPA_CM_DEFS_H */
