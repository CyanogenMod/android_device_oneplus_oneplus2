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
	IPACM_Neighbor.cpp

	@brief
	This file implements the functionality of handling IPACM Neighbor events.

	@Author
	Skylar Chang

*/

#include <sys/ioctl.h>
#include <IPACM_Neighbor.h>
#include <IPACM_EvtDispatcher.h>
#include "IPACM_Defs.h"
#include "IPACM_Log.h"


IPACM_Neighbor::IPACM_Neighbor()
{
	num_neighbor_client = 0;
	IPACM_EvtDispatcher::registr(IPA_WLAN_CLIENT_ADD_EVENT_EX, this);
	IPACM_EvtDispatcher::registr(IPA_NEW_NEIGH_EVENT, this);
	IPACM_EvtDispatcher::registr(IPA_DEL_NEIGH_EVENT, this);
	return;
}

void IPACM_Neighbor::event_callback(ipa_cm_event_id event, void *param)
{
	ipacm_event_data_all *data_all = NULL;
	int i, ipa_interface_index;
	ipacm_cmd_q_data evt_data;
	int num_neighbor_client_temp = num_neighbor_client;

	IPACMDBG("Recieved event %d\n", event);

	switch (event)
	{
		case IPA_WLAN_CLIENT_ADD_EVENT_EX:
		{
			ipacm_event_data_wlan_ex *data = (ipacm_event_data_wlan_ex *)param;
			ipa_interface_index = IPACM_Iface::iface_ipa_index_query(data->if_index);
			uint8_t client_mac_addr[6];

			IPACMDBG_H("Received IPA_WLAN_CLIENT_ADD_EVENT\n");
			for(i = 0; i < data->num_of_attribs; i++)
			{
				if(data->attribs[i].attrib_type == WLAN_HDR_ATTRIB_MAC_ADDR)
				{
					memcpy(client_mac_addr,
							data->attribs[i].u.mac_addr,
							sizeof(client_mac_addr));
					IPACMDBG_H("AP Mac Address %02x:%02x:%02x:%02x:%02x:%02x\n",
							 client_mac_addr[0], client_mac_addr[1], client_mac_addr[2],
							 client_mac_addr[3], client_mac_addr[4], client_mac_addr[5]);
				}
				else
				{
					IPACMDBG_H("The attribute type is not expected!\n");
				}
			}

			for (i = 0; i < num_neighbor_client_temp; i++)
			{
				/* find the client */
				if (memcmp(neighbor_client[i].mac_addr, client_mac_addr, sizeof(neighbor_client[i].mac_addr)) == 0)
				{
					/* check if iface is not bridge interface*/
					if (strcmp(IPACM_Iface::ipacmcfg->ipa_virtual_iface_name, IPACM_Iface::ipacmcfg->iface_table[ipa_interface_index].iface_name) != 0)
					{
						/* use previous ipv4 first */
						if(data->if_index != neighbor_client[i].iface_index)
						{
							IPACMERR("update new kernel iface index \n");
							neighbor_client[i].iface_index = data->if_index;
						}

						/* check if client associated with previous network interface */
						if(ipa_interface_index != neighbor_client[i].ipa_if_num)
						{
							IPACMERR("client associate to different AP \n");
							return;
						}

						if (neighbor_client[i].v4_addr != 0) /* not 0.0.0.0 */
						{
							evt_data.event = IPA_NEIGH_CLIENT_IP_ADDR_ADD_EVENT;
							data_all = (ipacm_event_data_all *)malloc(sizeof(ipacm_event_data_all));
							if (data_all == NULL)
							{
								IPACMERR("Unable to allocate memory\n");
								return;
							}
							data_all->iptype = IPA_IP_v4;
							data_all->if_index = neighbor_client[i].iface_index;
							data_all->ipv4_addr = neighbor_client[i].v4_addr; //use previous ipv4 address
							memcpy(data_all->mac_addr,
									neighbor_client[i].mac_addr,
												sizeof(data_all->mac_addr));
							evt_data.evt_data = (void *)data_all;
							IPACM_EvtDispatcher::PostEvt(&evt_data);
							/* ask for replaced iface name*/
							ipa_interface_index = IPACM_Iface::iface_ipa_index_query(data_all->if_index);
							IPACMDBG_H("Posted event %d, with %s for ipv4 client re-connect\n",
											evt_data.event,
											IPACM_Iface::ipacmcfg->iface_table[ipa_interface_index].iface_name);
						}
					}
					break;
				}
			}
		}
		break;

		default:
		{
			ipacm_event_data_all *data = (ipacm_event_data_all *)param;
			ipa_interface_index = IPACM_Iface::iface_ipa_index_query(data->if_index);

			if (data->iptype == IPA_IP_v4)
			{
				if (data->ipv4_addr != 0) /* not 0.0.0.0 */
				{
					IPACMDBG(" Got New_Neighbor event with ipv4 address \n");
					/* check if iface is bridge interface*/
					if (strcmp(IPACM_Iface::ipacmcfg->ipa_virtual_iface_name, IPACM_Iface::ipacmcfg->iface_table[ipa_interface_index].iface_name) == 0)
					{
						/* searh if seen this client or not*/
						for (i = 0; i < num_neighbor_client_temp; i++)
						{
							if (memcmp(neighbor_client[i].mac_addr, data->mac_addr, sizeof(neighbor_client[i].mac_addr)) == 0)
							{
								data->if_index = neighbor_client[i].iface_index;
								neighbor_client[i].v4_addr = data->ipv4_addr; // cache client's previous ipv4 address
								/* construct IPA_NEIGH_CLIENT_IP_ADDR_ADD_EVENT command and insert to command-queue */
								if (event == IPA_NEW_NEIGH_EVENT)
									evt_data.event = IPA_NEIGH_CLIENT_IP_ADDR_ADD_EVENT;
								else
									evt_data.event = IPA_NEIGH_CLIENT_IP_ADDR_DEL_EVENT;

								data_all = (ipacm_event_data_all *)malloc(sizeof(ipacm_event_data_all));
								if (data_all == NULL)
								{
									IPACMERR("Unable to allocate memory\n");
									return;
								}
								memcpy(data_all, data, sizeof(ipacm_event_data_all));
								evt_data.evt_data = (void *)data_all;
								IPACM_EvtDispatcher::PostEvt(&evt_data);

								/* ask for replaced iface name*/
								ipa_interface_index = IPACM_Iface::iface_ipa_index_query(data_all->if_index);
								IPACMDBG_H("Posted event %d, with %s for ipv4\n",
												evt_data.event,
												IPACM_Iface::ipacmcfg->iface_table[ipa_interface_index].iface_name);
								break;
							};
						}
					}
					else
					{
						/* construct IPA_NEIGH_CLIENT_IP_ADDR_ADD_EVENT command and insert to command-queue */
						if (event == IPA_NEW_NEIGH_EVENT)
							evt_data.event = IPA_NEIGH_CLIENT_IP_ADDR_ADD_EVENT;
						else
							evt_data.event = IPA_NEIGH_CLIENT_IP_ADDR_DEL_EVENT;

						data_all = (ipacm_event_data_all *)malloc(sizeof(ipacm_event_data_all));
						if (data_all == NULL)
						{
							IPACMERR("Unable to allocate memory\n");
							return;
						}
						memcpy(data_all, data, sizeof(ipacm_event_data_all));
						evt_data.evt_data = (void *)data_all;
						IPACM_EvtDispatcher::PostEvt(&evt_data);
						IPACMDBG_H("Posted event %d with %s for ipv4\n",
										evt_data.event,
										IPACM_Iface::ipacmcfg->iface_table[ipa_interface_index].iface_name);
						/* Also save to cache for ipv4 */
						/*searh if seen this client or not*/
						for (i = 0; i < num_neighbor_client_temp; i++)
						{
							/* find the client */
							if (memcmp(neighbor_client[i].mac_addr, data->mac_addr, sizeof(neighbor_client[i].mac_addr)) == 0)
							{
								/* update the network interface client associated */
								neighbor_client[i].iface_index = data->if_index;
								neighbor_client[i].ipa_if_num = ipa_interface_index;
								neighbor_client[i].v4_addr = data->ipv4_addr; // cache client's previous ipv4 address
								IPACMDBG_H("update cache %d-entry, with %s iface, ipv4 address: 0x%x\n",
												i,
												IPACM_Iface::ipacmcfg->iface_table[ipa_interface_index].iface_name,
												data->ipv4_addr);
								break;
							}
						}
						/* not find client */
						if (i == num_neighbor_client_temp)
						{
							if (num_neighbor_client_temp < IPA_MAX_NUM_NEIGHBOR_CLIENTS)
							{
								memcpy(neighbor_client[num_neighbor_client_temp].mac_addr,
											data->mac_addr,
											sizeof(data->mac_addr));
								neighbor_client[num_neighbor_client_temp].iface_index = data->if_index;
								/* cache the network interface client associated */
								neighbor_client[num_neighbor_client_temp].ipa_if_num = ipa_interface_index;
								neighbor_client[num_neighbor_client_temp].v4_addr = data->ipv4_addr;
								IPACMDBG_H("Cache wlan-iface client MAC %02x:%02x:%02x:%02x:%02x:%02x\n, total client: %d\n",
												neighbor_client[num_neighbor_client_temp].mac_addr[0],
												neighbor_client[num_neighbor_client_temp].mac_addr[1],
												neighbor_client[num_neighbor_client_temp].mac_addr[2],
												neighbor_client[num_neighbor_client_temp].mac_addr[3],
												neighbor_client[num_neighbor_client_temp].mac_addr[4],
												neighbor_client[num_neighbor_client_temp].mac_addr[5],
												num_neighbor_client);
								num_neighbor_client++;
								return;
							}
							else
							{
								IPACMERR("error:  neighbor client oversize!");
								return;
							}
						}
					}
				}
			}
			else
			{   //ipv6 starts

				if ((data->ipv6_addr[0]) || (data->ipv6_addr[1]) || (data->ipv6_addr[2]) || (data->ipv6_addr[3]))
				{
					IPACMDBG(" Got New_Neighbor event with ipv6 address \n");
					/* check if iface is bridge interface*/
					if (strcmp(IPACM_Iface::ipacmcfg->ipa_virtual_iface_name, IPACM_Iface::ipacmcfg->iface_table[ipa_interface_index].iface_name) == 0)
					{
						/* searh if seen this client or not*/
						for (i = 0; i < num_neighbor_client_temp; i++)
						{
							if (memcmp(neighbor_client[i].mac_addr, data->mac_addr, sizeof(neighbor_client[i].mac_addr)) == 0)
							{
								data->if_index = neighbor_client[i].iface_index;
								/* construct IPA_NEIGH_CLIENT_IP_ADDR_ADD_EVENT command and insert to command-queue */
								if (event == IPA_NEW_NEIGH_EVENT) evt_data.event = IPA_NEIGH_CLIENT_IP_ADDR_ADD_EVENT;
								else evt_data.event = IPA_NEIGH_CLIENT_IP_ADDR_DEL_EVENT;
								data_all = (ipacm_event_data_all *)malloc(sizeof(ipacm_event_data_all));
								if (data_all == NULL)
								{
									IPACMERR("Unable to allocate memory\n");
									return;
								}
								memcpy(data_all, data, sizeof(ipacm_event_data_all));
								evt_data.evt_data = (void *)data_all;
								IPACM_EvtDispatcher::PostEvt(&evt_data);
								/* ask for replaced iface name*/
								ipa_interface_index = IPACM_Iface::iface_ipa_index_query(data_all->if_index);
								IPACMDBG_H("Posted event %d, with %s for ipv6\n",
												evt_data.event,
												IPACM_Iface::ipacmcfg->iface_table[ipa_interface_index].iface_name);
								break;
							};
						}
					}
					else
					{
						/* construct IPA_NEIGH_CLIENT_IP_ADDR_ADD_EVENT command and insert to command-queue */
						if (event == IPA_NEW_NEIGH_EVENT)
							evt_data.event = IPA_NEIGH_CLIENT_IP_ADDR_ADD_EVENT;
						else
							evt_data.event = IPA_NEIGH_CLIENT_IP_ADDR_DEL_EVENT;
						data_all = (ipacm_event_data_all *)malloc(sizeof(ipacm_event_data_all));
						if (data_all == NULL)
						{
							IPACMERR("Unable to allocate memory\n");
							return;
						}
						memcpy(data_all, data, sizeof(ipacm_event_data_all));
						evt_data.evt_data = (void *)data_all;
						IPACM_EvtDispatcher::PostEvt(&evt_data);
						IPACMDBG_H("Posted event %d with %s for ipv6\n",
										evt_data.event,
										IPACM_Iface::ipacmcfg->iface_table[ipa_interface_index].iface_name);
					}
				}
				else
				{
					IPACMDBG(" Got New_Neighbor event with no ipv6/ipv4 address \n");
					/*no ipv6 in data searh if seen this client or not*/
					for (i = 0; i < num_neighbor_client_temp; i++)
					{
						/* find the client */
						if (memcmp(neighbor_client[i].mac_addr, data->mac_addr, sizeof(neighbor_client[i].mac_addr)) == 0)
						{
							/* check if iface is not bridge interface*/
							if (strcmp(IPACM_Iface::ipacmcfg->ipa_virtual_iface_name, IPACM_Iface::ipacmcfg->iface_table[ipa_interface_index].iface_name) != 0)
							{
								/* use previous ipv4 first */
								if(data->if_index != neighbor_client[i].iface_index)
								{
									IPACMERR("update new kernel iface index \n");
									neighbor_client[i].iface_index = data->if_index;
								}

								/* check if client associated with previous network interface */
								if(ipa_interface_index != neighbor_client[i].ipa_if_num)
								{
									IPACMERR("client associate to different AP \n");
									return;
								}

								if (neighbor_client[i].v4_addr != 0) /* not 0.0.0.0 */
								{
									evt_data.event = IPA_NEIGH_CLIENT_IP_ADDR_ADD_EVENT;
									data_all = (ipacm_event_data_all *)malloc(sizeof(ipacm_event_data_all));
									if (data_all == NULL)
									{
										IPACMERR("Unable to allocate memory\n");
										return;
									}
									data_all->iptype = IPA_IP_v4;
									data_all->if_index = neighbor_client[i].iface_index;
									data_all->ipv4_addr = neighbor_client[i].v4_addr; //use previous ipv4 address
									memcpy(data_all->mac_addr,
											neighbor_client[i].mac_addr,
														sizeof(data_all->mac_addr));
									evt_data.evt_data = (void *)data_all;
									IPACM_EvtDispatcher::PostEvt(&evt_data);
									/* ask for replaced iface name*/
									ipa_interface_index = IPACM_Iface::iface_ipa_index_query(data_all->if_index);
									IPACMDBG_H("Posted event %d, with %s for ipv4 client re-connect\n",
													evt_data.event,
													IPACM_Iface::ipacmcfg->iface_table[ipa_interface_index].iface_name);
								}
							}
							break;
						}
					}
					/* not find client */
					if (i == num_neighbor_client_temp)
					{
						/* check if iface is not bridge interface*/
						if (strcmp(IPACM_Iface::ipacmcfg->ipa_virtual_iface_name, IPACM_Iface::ipacmcfg->iface_table[ipa_interface_index].iface_name) != 0)
						{
							if (num_neighbor_client_temp < IPA_MAX_NUM_NEIGHBOR_CLIENTS)
							{
								memcpy(neighbor_client[num_neighbor_client_temp].mac_addr,
											data->mac_addr,
											sizeof(data->mac_addr));
								neighbor_client[num_neighbor_client_temp].iface_index = data->if_index;
								/* cache the network interface client associated */
								neighbor_client[num_neighbor_client_temp].ipa_if_num = ipa_interface_index;
								neighbor_client[num_neighbor_client_temp].v4_addr = 0;
								IPACMDBG_H("Copy wlan-iface client MAC %02x:%02x:%02x:%02x:%02x:%02x\n, total client: %d\n",
												neighbor_client[num_neighbor_client_temp].mac_addr[0],
												neighbor_client[num_neighbor_client_temp].mac_addr[1],
												neighbor_client[num_neighbor_client_temp].mac_addr[2],
												neighbor_client[num_neighbor_client_temp].mac_addr[3],
												neighbor_client[num_neighbor_client_temp].mac_addr[4],
												neighbor_client[num_neighbor_client_temp].mac_addr[5],
												num_neighbor_client);
								num_neighbor_client++;
								return;
							}
							else
							{
								IPACMERR("error:  neighbor client oversize!");
								return;
							}
						}
					}
				}
			} //ipv6 ends
		}
		break;
	}
	return;
}
