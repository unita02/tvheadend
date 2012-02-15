/*
 *  tvheadend, Upnp interface
 *  Copyright (C) 2012 Steffen Kuhnke
 *
 *  This program is free software: you can redistribute it and/or modify
 *  it under the terms of the GNU General Public License as published by
 *  the Free Software Foundation, either version 3 of the License, or
 *  (at your option) any later version.
 *
 *  This program is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *  GNU General Public License for more details.
 *
 *  You should have received a copy of the GNU General Public License
 *  along with this program.  If not, see <http://www.gnu.org/licenses/>.
 */

#ifndef TV_UPNP_CFG_H_
#define TV_UPNP_CFG_H_

#include "tvstring.h"

extern int tv_upnp_ini_try_g;
extern int tv_upnp_is_inited_g;

extern tvstring_t* tv_upnp_server_ip_g;
extern unsigned short tv_upnp_server_port_g;

extern tvstring_t* tv_upnp_web_root_g;
extern tvstring_t* tv_upnp_virt_dir_g;

extern tvstring_t* tv_upnp_dev_name_g;
extern tvstring_t* tv_upnp_dev_desc_g;
extern tvstring_t* tv_upnp_dev_udn_g;

extern int upnp_event_subscr_count_g;
extern int upnp_event_subscr_max_g;

/*----------------------------------------------------------------------------*/

void tv_upnp_configure(void);

void tv_upnp_describe(void);


#endif /* TV_UPNP_CFG_H_ */
