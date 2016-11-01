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
	IPACM_Lan.h

	@brief
	This file implements the LAN iface definitions

	@Author
	Skylar Chang

*/
#ifndef IPACM_LAN_H
#define IPACM_LAN_H

#include <stdio.h>
#include <linux/msm_ipa.h>

#include "IPACM_CmdQueue.h"
#include "IPACM_Iface.h"
#include "IPACM_Routing.h"
#include "IPACM_Filtering.h"
#include "IPACM_Config.h"
#include "IPACM_Conntrack_NATApp.h"

#define IPA_WAN_DEFAULT_FILTER_RULE_HANDLES  1
#define IPA_PRIV_SUBNET_FILTER_RULE_HANDLES  3
#define IPA_NUM_ODU_ROUTE_RULES 2
#define MAX_WAN_UL_FILTER_RULES 20
#define NUM_IPV6_PREFIX_FLT_RULE 1
#define NUM_IPV6_ICMP_FLT_RULE 1

/* echo ipatetherstats <ifaceIn> <ifaceOut> */
/* <in->out_bytes> <in->out_pkts> <out->in_bytes> <out->in_pkts */

#define PIPE_STATS "%s %s %lu %lu %lu %lu"
#define IPA_PIPE_STATS_FILE_NAME "/data/misc/ipa/tether_stats"

/* store each lan-iface unicast routing rule and its handler*/
struct ipa_lan_rt_rule
{
	ipa_ip_type ip;
	uint32_t v4_addr;
	uint32_t v4_addr_mask;
	uint32_t v6_addr[4];
	uint32_t rt_rule_hdl[0];
};

/* Support multiple eth client */
typedef struct _eth_client_rt_hdl
{
	uint32_t eth_rt_rule_hdl_v4;
	uint32_t eth_rt_rule_hdl_v6[IPV6_NUM_ADDR];
	uint32_t eth_rt_rule_hdl_v6_wan[IPV6_NUM_ADDR];
}eth_client_rt_hdl;

typedef struct _ipa_eth_client
{
	uint8_t mac[IPA_MAC_ADDR_SIZE];
	uint32_t v4_addr;
	uint32_t v6_addr[IPV6_NUM_ADDR][4];
	uint32_t hdr_hdl_v4;
	uint32_t hdr_hdl_v6;
	bool route_rule_set_v4;
	int route_rule_set_v6;
	bool ipv4_set;
	int ipv6_set;
	bool ipv4_header_set;
	bool ipv6_header_set;
	eth_client_rt_hdl eth_rt_hdl[0]; /* depends on number of tx properties */
}ipa_eth_client;

struct lan2lan_flt_rule_hdl
{
	uint32_t rule_hdl;
	bool valid;
};

struct lan2lan_hdr_hdl
{
	uint32_t hdr_hdl;
	bool valid;
};

struct eth_bridge_client_flt_info
{
	uint8_t mac[IPA_MAC_ADDR_SIZE];
	uint32_t flt_rule_hdl_v4;
	bool flt_rule_set_v4;
	uint32_t flt_rule_hdl_v6;
	bool flt_rule_set_v6;
};

struct eth_bridge_client_rt_info
{
	uint8_t mac[IPA_MAC_ADDR_SIZE];
	uint32_t rt_rule_hdl[0];
};

struct hdr_proc_ctx_info
{
	uint32_t proc_ctx_hdl;
	bool valid;
};

struct eth_bridge_subnet_client_info
{
	uint8_t mac[IPA_MAC_ADDR_SIZE];
	int ipa_if_num;
};

/* lan iface */
class IPACM_Lan : public IPACM_Iface
{
public:

	IPACM_Lan(int iface_index);
	~IPACM_Lan();

	/* store lan's wan-up filter rule handlers */
	uint32_t lan_wan_fl_rule_hdl[IPA_WAN_DEFAULT_FILTER_RULE_HANDLES];

	/* store private-subnet filter rule handlers */
	uint32_t private_fl_rule_hdl[IPA_MAX_PRIVATE_SUBNET_ENTRIES];

	/* LAN-iface's callback function */
	void event_callback(ipa_cm_event_id event,
											void *data);

	virtual int handle_wan_up(ipa_ip_type ip_type);

	/* configure filter rule for wan_up event*/
	virtual int handle_wan_up_ex(ipacm_ext_prop* ext_prop, ipa_ip_type iptype);

	/* delete filter rule for wan_down event*/
	virtual int handle_wan_down(bool is_sta_mode);

	/* delete filter rule for wan_down event*/
	virtual int handle_wan_down_v6(bool is_sta_mode);

	/* configure private subnet filter rules*/
	virtual int handle_private_subnet(ipa_ip_type iptype);

	/* handle new_address event*/
	int handle_addr_evt(ipacm_event_data_addr *data);

	int handle_addr_evt_odu_bridge(ipacm_event_data_addr* data);

	static bool odu_up;

	/* install UL filter rule from Q6 */
	virtual int handle_uplink_filter_rule(ipacm_ext_prop* prop, ipa_ip_type iptype);

	int add_lan2lan_flt_rule(ipa_ip_type iptype, uint32_t src_v4_addr, uint32_t dst_v4_addr, uint32_t* src_v6_addr, uint32_t* dst_v6_addr, uint32_t* rule_hdl);

	int del_lan2lan_flt_rule(ipa_ip_type iptype, uint32_t rule_hdl);

	virtual int add_lan2lan_hdr(ipa_ip_type iptype, uint8_t* src_mac, uint8_t* dst_mac, uint32_t* hdr_hdl);

	int add_lan2lan_rt_rule(ipa_ip_type iptype, uint32_t src_v4_addr, uint32_t dst_v4_addr,
								uint32_t* src_v6_addr, uint32_t* dst_v6_addr, uint32_t hdr_hdl, lan_to_lan_rt_rule_hdl* rule_hdl);

	int del_lan2lan_rt_rule(ipa_ip_type iptype, lan_to_lan_rt_rule_hdl);

	int del_lan2lan_hdr(ipa_ip_type iptype, uint32_t hdr_hdl);



	static ipa_hdr_l2_type usb_hdr_type;
	static ipa_hdr_l2_type wlan_hdr_type;

	static uint32_t usb_hdr_template_hdl;
	static uint32_t wlan_hdr_template_hdl;

	static hdr_proc_ctx_info usb_to_wlan_hdr_proc_ctx, wlan_to_usb_hdr_proc_ctx;
	static hdr_proc_ctx_info wlan_to_wlan_hdr_proc_ctx;

	static eth_bridge_subnet_client_info eth_bridge_wlan_client[IPA_LAN_TO_LAN_MAX_WLAN_CLIENT];
	static eth_bridge_subnet_client_info eth_bridge_usb_client[IPA_LAN_TO_LAN_MAX_USB_CLIENT];

	static int num_wlan_client;
	static int num_usb_client;

protected:

	lan2lan_flt_rule_hdl wlan_client_flt_rule_hdl_v4[IPA_LAN_TO_LAN_MAX_WLAN_CLIENT];
	lan2lan_flt_rule_hdl wlan_client_flt_rule_hdl_v6[IPA_LAN_TO_LAN_MAX_WLAN_CLIENT];

	eth_bridge_client_flt_info eth_bridge_wlan_client_flt_info[IPA_LAN_TO_LAN_MAX_WLAN_CLIENT];
	int wlan_client_flt_info_count;

	eth_bridge_client_rt_info* eth_bridge_usb_client_rt_info_v4;
	eth_bridge_client_rt_info* eth_bridge_usb_client_rt_info_v6;
	int usb_client_rt_info_count_v4;
	int usb_client_rt_info_count_v6;
	int client_rt_info_size_v4;
	int client_rt_info_size_v6;

	int each_client_rt_rule_count_v4;
	int each_client_rt_rule_count_v6;

	virtual int eth_bridge_handle_dummy_wlan_client_flt_rule(ipa_ip_type iptype);

	virtual int eth_bridge_add_wlan_guest_ap_flt_rule(ipa_ip_type iptype);

	virtual int eth_bridge_handle_dummy_usb_client_flt_rule(ipa_ip_type iptype);

	int eth_bridge_add_wlan_client_flt_rule(uint8_t* mac, ipa_ip_type iptype);

	int eth_bridge_del_wlan_client_flt_rule(uint8_t* mac);

	int eth_bridge_post_lan_client_event(uint8_t* mac_addr, ipa_cm_event_id evt);

	int add_hdr_proc_ctx();

	int del_hdr_proc_ctx();

	ipa_hdr_proc_type get_hdr_proc_type(ipa_hdr_l2_type t1, ipa_hdr_l2_type t2);

	virtual int eth_bridge_install_cache_wlan_client_flt_rule(ipa_ip_type iptype);

	virtual int eth_bridge_install_cache_usb_client_flt_rule(ipa_ip_type iptype);

	int eth_bridge_add_usb_client_rt_rule(uint8_t* mac, ipa_ip_type iptype);

	int eth_bridge_del_usb_client_rt_rule(uint8_t* mac);

	eth_bridge_client_rt_info* eth_bridge_get_client_rt_info_ptr(uint8_t index, ipa_ip_type iptype);

	void eth_bridge_add_usb_client(uint8_t* mac);

	void eth_bridge_del_usb_client(uint8_t* mac);

	int eth_bridge_get_hdr_template_hdl(uint32_t* hdr_hdl);



	virtual int add_dummy_lan2lan_flt_rule(ipa_ip_type iptype);

	virtual int add_dummy_private_subnet_flt_rule(ipa_ip_type iptype);

	int handle_private_subnet_android(ipa_ip_type iptype);

	int reset_to_dummy_flt_rule(ipa_ip_type iptype, uint32_t rule_hdl);

	/*handle lan2lan client active*/
	int handle_lan2lan_client_active(ipacm_event_data_all *data, ipa_cm_event_id event);

	int install_ipv6_prefix_flt_rule(uint32_t* prefix);

	int install_ipv6_icmp_flt_rule();

	void post_del_self_evt();

	/* handle tethering stats */
	int handle_tethering_stats_event(ipa_get_data_stats_resp_msg_v01 *data);

	/* handle tethering client */
	int handle_tethering_client(bool reset, ipacm_client_enum ipa_client);

	lan2lan_flt_rule_hdl lan2lan_flt_rule_hdl_v4[MAX_OFFLOAD_PAIR];
	lan2lan_flt_rule_hdl lan2lan_flt_rule_hdl_v6[MAX_OFFLOAD_PAIR];

	uint8_t num_lan2lan_flt_rule_v4;
	uint8_t num_lan2lan_flt_rule_v6;

	lan2lan_hdr_hdl lan2lan_hdr_hdl_v4[MAX_OFFLOAD_PAIR];
	lan2lan_hdr_hdl lan2lan_hdr_hdl_v6[MAX_OFFLOAD_PAIR];

	/* store ipv4 UL filter rule handlers from Q6*/
	uint32_t wan_ul_fl_rule_hdl_v4[MAX_WAN_UL_FILTER_RULES];

	/* store ipv6 UL filter rule handlers from Q6*/
	uint32_t wan_ul_fl_rule_hdl_v6[MAX_WAN_UL_FILTER_RULES];

	virtual void install_tcp_ctl_flt_rule(ipa_ip_type iptype);

	uint32_t tcp_ctl_flt_rule_hdl_v4[NUM_TCP_CTL_FLT_RULE];
	uint32_t tcp_ctl_flt_rule_hdl_v6[NUM_TCP_CTL_FLT_RULE];

	uint32_t ipv6_prefix_flt_rule_hdl[NUM_IPV6_PREFIX_FLT_RULE];
	uint32_t ipv6_icmp_flt_rule_hdl[NUM_IPV6_ICMP_FLT_RULE];

	int num_wan_ul_fl_rule_v4;
	int num_wan_ul_fl_rule_v6;

	bool is_active;
	bool modem_ul_v4_set;
	bool modem_ul_v6_set;

	uint32_t if_ipv4_subnet;

private:

	/* dynamically allocate lan iface's unicast routing rule structure */

	bool is_mode_switch; /* indicate mode switch, need post internal up event */

	int eth_client_len;

	ipa_eth_client *eth_client;

	int header_name_count;

	int num_eth_client;

	NatApp *Nat_App;

    int ipv6_set;

	uint32_t ODU_hdr_hdl_v4, ODU_hdr_hdl_v6;

	uint32_t *odu_route_rule_v4_hdl;

	uint32_t *odu_route_rule_v6_hdl;

	bool ipv4_header_set;

	bool ipv6_header_set;

	inline ipa_eth_client* get_client_memptr(ipa_eth_client *param, int cnt)
	{
	    char *ret = ((char *)param) + (eth_client_len * cnt);
		return (ipa_eth_client *)ret;
	}

	inline int get_eth_client_index(uint8_t *mac_addr)
	{
		int cnt;
		int num_eth_client_tmp = num_eth_client;

		IPACMDBG_H("Passed MAC %02x:%02x:%02x:%02x:%02x:%02x\n",
						 mac_addr[0], mac_addr[1], mac_addr[2],
						 mac_addr[3], mac_addr[4], mac_addr[5]);

		for(cnt = 0; cnt < num_eth_client_tmp; cnt++)
		{
			IPACMDBG_H("stored MAC %02x:%02x:%02x:%02x:%02x:%02x\n",
							 get_client_memptr(eth_client, cnt)->mac[0],
							 get_client_memptr(eth_client, cnt)->mac[1],
							 get_client_memptr(eth_client, cnt)->mac[2],
							 get_client_memptr(eth_client, cnt)->mac[3],
							 get_client_memptr(eth_client, cnt)->mac[4],
							 get_client_memptr(eth_client, cnt)->mac[5]);

			if(memcmp(get_client_memptr(eth_client, cnt)->mac,
								mac_addr,
								sizeof(get_client_memptr(eth_client, cnt)->mac)) == 0)
			{
				IPACMDBG_H("Matched client index: %d\n", cnt);
				return cnt;
			}
		}

		return IPACM_INVALID_INDEX;
	}

	inline int delete_eth_rtrules(int clt_indx, ipa_ip_type iptype)
	{
		uint32_t tx_index;
		uint32_t rt_hdl;
		int num_v6;

		if(iptype == IPA_IP_v4)
		{
		    for(tx_index = 0; tx_index < iface_query->num_tx_props; tx_index++)
		    {
		        if((tx_prop->tx[tx_index].ip == IPA_IP_v4) && (get_client_memptr(eth_client, clt_indx)->route_rule_set_v4==true)) /* for ipv4 */
				{
					IPACMDBG_H("Delete client index %d ipv4 RT-rules for tx:%d\n",clt_indx,tx_index);
					rt_hdl = get_client_memptr(eth_client, clt_indx)->eth_rt_hdl[tx_index].eth_rt_rule_hdl_v4;

					if(m_routing.DeleteRoutingHdl(rt_hdl, IPA_IP_v4) == false)
					{
						return IPACM_FAILURE;
					}
				}
		    } /* end of for loop */

		     /* clean the ipv4 RT rules for eth-client:clt_indx */
		     if(get_client_memptr(eth_client, clt_indx)->route_rule_set_v4==true) /* for ipv4 */
		     {
				get_client_memptr(eth_client, clt_indx)->route_rule_set_v4 = false;
		     }
		}

		if(iptype == IPA_IP_v6)
		{
			for(tx_index = 0; tx_index < iface_query->num_tx_props; tx_index++)
			{
				if((tx_prop->tx[tx_index].ip == IPA_IP_v6) && (get_client_memptr(eth_client, clt_indx)->route_rule_set_v6 != 0)) /* for ipv6 */
				{
					for(num_v6 =0;num_v6 < get_client_memptr(eth_client, clt_indx)->route_rule_set_v6;num_v6++)
					{
						IPACMDBG_H("Delete client index %d ipv6 RT-rules for %d-st ipv6 for tx:%d\n", clt_indx,num_v6,tx_index);
						rt_hdl = get_client_memptr(eth_client, clt_indx)->eth_rt_hdl[tx_index].eth_rt_rule_hdl_v6[num_v6];
						if(m_routing.DeleteRoutingHdl(rt_hdl, IPA_IP_v6) == false)
							{
								return IPACM_FAILURE;
							}

							rt_hdl = get_client_memptr(eth_client, clt_indx)->eth_rt_hdl[tx_index].eth_rt_rule_hdl_v6_wan[num_v6];
							if(m_routing.DeleteRoutingHdl(rt_hdl, IPA_IP_v6) == false)
							{
								return IPACM_FAILURE;
							}
						}
                    }
		    } /* end of for loop */

		    /* clean the ipv6 RT rules for eth-client:clt_indx */
		    if(get_client_memptr(eth_client, clt_indx)->route_rule_set_v6 != 0) /* for ipv6 */
		    {
		        get_client_memptr(eth_client, clt_indx)->route_rule_set_v6 = 0;
            }
		}

		return IPACM_SUCCESS;
	}

	/* handle eth client initial, construct full headers (tx property) */
	int handle_eth_hdr_init(uint8_t *mac_addr);

	/* handle eth client ip-address */
	int handle_eth_client_ipaddr(ipacm_event_data_all *data);

	/* handle eth client routing rule*/
	int handle_eth_client_route_rule(uint8_t *mac_addr, ipa_ip_type iptype);

	/*handle eth client del mode*/
	int handle_eth_client_down_evt(uint8_t *mac_addr);

	/* handle odu client initial, construct full headers (tx property) */
	int handle_odu_hdr_init(uint8_t *mac_addr);

	/* handle odu default route rule configuration */
	int handle_odu_route_add();

	/* handle odu default route rule deletion */
	int handle_odu_route_del();

	/*handle lan iface down event*/
	int handle_down_evt();

	/*handle lan2lan internal mesg posting*/
	int post_lan2lan_client_disconnect_msg(ipa_ip_type iptype);

	/*handle reset usb-client rt-rules */
	int handle_lan_client_reset_rt(ipa_ip_type iptype);
};

#endif /* IPACM_LAN_H */
