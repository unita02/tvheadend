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

#ifndef TV_UPNP_H_
#define TV_UPNP_H_

void tv_upnp_init(void);
void tv_upnp_deinit(void);
int  tv_upnp_is_inited(void);

#endif /* TV_UPNP_H_ */
