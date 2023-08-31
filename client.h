/* vim: set et ts=4 sts=4 sw=4 : */
/********************************************************************\
 * This program is free software; you can redistribute it and/or    *
 * modify it under the terms of the GNU General Public License as   *
 * published by the Free Software Foundation; either version 2 of   *
 * the License, or (at your option) any later version.              *
 *                                                                  *
 * This program is distributed in the hope that it will be useful,  *
 * but WITHOUT ANY WARRANTY; without even the implied warranty of   *
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the    *
 * GNU General Public License for more details.                     *
 *                                                                  *
 * You should have received a copy of the GNU General Public License*
 * along with this program; if not, contact:                        *
 *                                                                  *
 * Free Software Foundation           Voice:  +1-617-542-5942       *
 * 59 Temple Place - Suite 330        Fax:    +1-617-542-2652       *
 * Boston, MA  02111-1307,  USA       gnu@gnu.org                   *
 *                                                                  *
\********************************************************************/

/** @file client.h
    @brief xfrpc proxy client related
    @author Copyright (C) 2016 Dengfeng Liu <liu_df@qq.com>
*/

#ifndef _CLIENT_H_
#define _CLIENT_H_

#include <stdint.h>

#include "uthash.h"
#include "common.h"
#include "tcpmux.h"

struct event_base;
struct base_conf;
struct bufferevent;
struct event;
struct proxy_service;

#define SOCKS5_ADDRES_LEN 20
struct socks5_addr {
	uint8_t 	addr[SOCKS5_ADDRES_LEN];
	uint16_t	port;
	uint8_t 	type;
	uint8_t		reserve;
};

enum socks5_state {
	SOCKS5_INIT,
	SOCKS5_HANDSHAKE,
	SOCKS5_CONNECT,
	SOCKS5_ESTABLISHED,
};

struct proxy_client {
	struct event_base 	*base;
	struct bufferevent	*ctl_bev; // xfrpc proxy <---> frps
	struct bufferevent 	*local_proxy_bev; // xfrpc proxy <---> local service
	struct base_conf	*bconf;
	struct tmux_stream 	stream;
	
	uint32_t				stream_id;
	int						connected;
	int 					work_started;
	struct 	proxy_service 	*ps;
	unsigned char			*data_tail; // storage untreated data
	size_t					data_tail_size;
	
	// socks5 only
	struct 	socks5_addr remote_addr;
	enum 	socks5_state state;

	// private arguments
	UT_hash_handle hh;
};

struct proxy_service {
	char 	*proxy_name;
	char 	*proxy_type;
	char 	*ftp_cfg_proxy_name;
	int 	use_encryption;
	int		use_compression;

	// tcp or udp
	char	*local_ip;
	int		remote_port;
	int 	remote_data_port;
	int 	local_port;

	// http and https only
	char 	*custom_domains;
	char 	*subdomain;
	char	*locations;
	char	*host_header_rewrite;
	char	*http_user;
	char	*http_pwd;

	// load balance
	char	*group;
	char	*group_key;

	// plugin
	char	*plugin;
	char	*plugin_user;
	char	*plugin_pwd;
	
	// private arguments
	UT_hash_handle hh;
};

// after frp server accept client connection request
// frp server send xfrp client NoticeUserConn request
// when xfrp client receive that request, it will start
// frp tunnel
// if client has data-tail(not NULL), client value will be changed 
void start_xfrp_tunnel(struct proxy_client *client);

void del_proxy_client_by_stream_id(uint32_t sid);

struct proxy_client	*get_proxy_client(uint32_t sid);

int send_client_data_tail(struct proxy_client *client);

int is_ftp_proxy(const struct proxy_service *ps);

int is_socks5_proxy(const struct proxy_service *ps);

int is_udp_proxy(const struct proxy_service *ps);

struct proxy_client *new_proxy_client();

void clear_all_proxy_client();

void xfrp_proxy_event_cb(struct bufferevent *bev, short what, void *ctx);

#endif //_CLIENT_H_
