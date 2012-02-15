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

#ifndef TV_UPNP_BROWSW_H_
#define TV_UPNP_BROWSW_H_

#include "tvstring.h"

void tv_upnp_browse_tv_tree_init(void);
void tv_upnp_browse_tv_tree_release(void);

void tv_upnp_browse_tv_tree_node( tvstring_t* didl, int node_id, int children, int nd_start, int* nr_ret, int* nr_ttl );

#endif /* TV_UPNP_BROWSW_H_ */
