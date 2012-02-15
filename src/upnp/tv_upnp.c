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

#include "tvheadend.h"
#include "tv_upnp.h"
#include "tvstring.h"
#include "tv_upnp_cfg.h"
#include "tv_upnp_cb.h"
#include "tv_upnp_browse.h"


#define true 1
#define false 0

void report_upnp_err( int err );
void tv_upnp_configure(void);
void tv_upnp_describe(void);


int tv_upnp_ini_try_g = 0;
int tv_upnp_is_inited_g = 0;

tvstring_t* tv_upnp_server_ip_g = NULL;
unsigned short tv_upnp_server_port_g = 0;

tvstring_t* tv_upnp_web_root_g = NULL;
tvstring_t* tv_upnp_virt_dir_g = NULL;

tvstring_t* tv_upnp_dev_name_g = NULL;
tvstring_t* tv_upnp_dev_desc_g = NULL;
tvstring_t* tv_upnp_dev_udn_g = NULL;

UpnpDevice_Handle tv_upnp_hdev_g = 0;

int upnp_event_subscr_count_g = 0;
int upnp_event_subscr_max_g = 2; // TODO cfg

/******************************************************************************/

int
tv_upnp_is_inited(void)
{
    return tv_upnp_is_inited_g;
}

void
report_upnp_err( int err )
{
    const char* err_msg = "";
    switch(err) {
    case UPNP_E_OUTOF_MEMORY:
        err_msg = "Insufficient resources exist to initialize the SDK.";
        break;
    case UPNP_E_INIT:
        err_msg = "The SDK is already initialized.";
        break;
    case UPNP_E_INIT_FAILED:
        err_msg = "The SDK initialization failed for an unknown reason.";
        break;
    case UPNP_E_SOCKET_BIND:
        err_msg = "An error occurred binding a socket.";
        break;
    case UPNP_E_LISTEN:
        err_msg = "An error occurred listening to a socket.";
        break;
    case UPNP_E_OUTOF_SOCKET:
        err_msg = "An error ocurred creating a socket.";
        break;
    case UPNP_E_INTERNAL_ERROR:
        err_msg = "An internal error ocurred.";
        break;

    }
    tvhlog(LOG_NOTICE, "UPNP", "ERROR starting UPnP: '%s'", err_msg );
}
void
tv_upnp_init(void)
{
    int ret = UPNP_E_INTERNAL_ERROR;

    if(++tv_upnp_ini_try_g > 1)
        return;

    tv_upnp_configure();

    do {
        if( tvs_size( tv_upnp_server_ip_g ) > 0 )
            ret = UpnpInit( tvs_cstr(tv_upnp_server_ip_g), tv_upnp_server_port_g );
        else
            ret = UpnpInit( NULL, tv_upnp_server_port_g );
        if( UPNP_E_SUCCESS != ret )
            ret = UpnpInit( NULL, 0 );
        if( UPNP_E_SUCCESS != ret ) break;

        tv_upnp_server_port_g = UpnpGetServerPort();
        tvs_assign( tv_upnp_server_ip_g, UpnpGetServerIpAddress() );

        ret = UpnpSetWebServerRootDir( tvs_cstr(tv_upnp_web_root_g) );
        if( UPNP_E_SUCCESS != ret ) break;

        ret = UpnpAddVirtualDir( tvs_cstr(tv_upnp_virt_dir_g) );
        if( UPNP_E_SUCCESS != ret ) break;

        tv_upnp_describe();
        // register root device with the library
        ret = UpnpRegisterRootDevice2( UPNPREG_BUF_DESC
                                     , tvs_cstr(tv_upnp_dev_desc_g)
                                     , tvs_size(tv_upnp_dev_desc_g) + 1
                                     , true
                                     , tv_upnp_callback
                                     , &tv_upnp_hdev_g
                                     , &tv_upnp_hdev_g);
        if( UPNP_E_SUCCESS != ret ) break;

        // "inspired" by mediatomb: reg, unreg, reg to cleanup old regs
        ret = UpnpUnRegisterRootDevice(tv_upnp_hdev_g);
        if( UPNP_E_SUCCESS != ret ) break;

        ret = UpnpRegisterRootDevice2( UPNPREG_BUF_DESC
                                     , tvs_cstr(tv_upnp_dev_desc_g)
                                     , tvs_size(tv_upnp_dev_desc_g) + 1
                                     , true
                                     , tv_upnp_callback
                                     , &tv_upnp_hdev_g
                                     , &tv_upnp_hdev_g);
        if( UPNP_E_SUCCESS != ret ) break;


        tv_upnp_is_inited_g = 1;
    }
    while(0);

    if(tv_upnp_is_inited_g) {
        tvhlog(LOG_NOTICE, "UPNP", "OK - UPnP started on %s:%d as '%s'", tv_upnp_server_ip_g->c_str, tv_upnp_server_port_g, tv_upnp_dev_name_g->c_str );
    }
    else {
        report_upnp_err( ret );
    }
}


void
tv_upnp_deinit(void)
{
    if(tv_upnp_is_inited_g) {
        tvs_del(&tv_upnp_server_ip_g);
        tvs_del(&tv_upnp_web_root_g);
        tvs_del(&tv_upnp_virt_dir_g);
        tvs_del(&tv_upnp_dev_name_g);
        tvs_del(&tv_upnp_dev_desc_g);
        tvs_del(&tv_upnp_dev_udn_g);

        UpnpUnRegisterRootDevice(tv_upnp_hdev_g);
        UpnpFinish();
        tv_upnp_is_inited_g = 0;

        tv_upnp_browse_tv_tree_release();
    }
}

