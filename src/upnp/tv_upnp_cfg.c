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

#include "tv_upnp_cfg.h"
#include "tv_upnp_browse.h"
#include "tvstring.h"
#include <unistd.h>

void
tv_upnp_configure(void)
{
    tvs_new( &tv_upnp_server_ip_g, NULL );
    tv_upnp_server_port_g = 0;

    tvs_new( &tv_upnp_web_root_g, "/home/skuhnke/dvlp/test/itplatform.39/tvheadend/src/web" ); // TODO: configure
    tvs_new( &tv_upnp_virt_dir_g, "." );
    tvs_new( &tv_upnp_dev_name_g, "TvHeadEnd@" );

    char bf[256];
    bf[255] = 0;
    gethostname(bf, 255);
    tvs_cat( tv_upnp_dev_name_g, bf );
    tvs_assign( tv_upnp_dev_name_g, "mrf-upnp" ); // TODO: remove

    tvs_new( &tv_upnp_dev_udn_g, "uuid:Upnp-TVHeadend-0_1-1234567896661" );

    tv_upnp_browse_tv_tree_init();

}
void
tv_upnp_describe(void)
{
    tvs_new( &tv_upnp_dev_desc_g,
            "<?xml version = \"1.0\" encoding = \"utf-8\"?> \
            <root xmlns=\"urn:schemas-upnp-org:device-1-0\" \
                  xmlns:dlna=\"urn:schemas-dlna-org:device-1-0\"> \
                <specVersion> \
                    <major>1</major> \
                    <minor>0</minor> \
                </specVersion> \
                <URLBase>http://" );
    tvs_append( tv_upnp_dev_desc_g, tv_upnp_server_ip_g );
    tvs_cat( tv_upnp_dev_desc_g, ":" );
    tvs_cat_int( tv_upnp_dev_desc_g, tv_upnp_server_port_g );
    tvs_cat( tv_upnp_dev_desc_g,
            "/</URLBase> \
                <device> \
                    <deviceType>urn:schemas-upnp-org:device:MediaServer:1</deviceType> \
                    <friendlyName>" );
    tvs_append( tv_upnp_dev_desc_g, tv_upnp_dev_name_g );
    tvs_cat( tv_upnp_dev_desc_g,
            "</friendlyName> \
                    <manufacturer>tvheadend</manufacturer> \
                    <manufacturerURL>http://www.lonelycoder.com/hts</manufacturerURL> \
                    <modelDescription>tvheadend-dlna</modelDescription> \
                    <modelName>hts-tvheadend</modelName> \
                    <modelNumber>0.1</modelNumber> \
                    <modelURL>http://www.lonelycoder.com/hts</modelURL> \
                    <serialNumber>08/15-4711-42</serialNumber> \
                    <UDN>" );
    tvs_append( tv_upnp_dev_desc_g, tv_upnp_dev_udn_g );
    tvs_cat( tv_upnp_dev_desc_g,
                    "</UDN> \
                    <presentationURL>http://" );
    tvs_append( tv_upnp_dev_desc_g, tv_upnp_server_ip_g );
    tvs_cat( tv_upnp_dev_desc_g, ":" );
    tvs_cat_int( tv_upnp_dev_desc_g, tv_upnp_server_port_g );
    tvs_cat( tv_upnp_dev_desc_g,
            "/</presentationURL>\
                    <dlna:X_DLNADOC>DMS-1.50</dlna:X_DLNADOC> \
                    <serviceList> \
                        <service> \
                            <serviceType>urn:schemas-upnp-org:service:ConnectionManager:1</serviceType> \
                            <serviceId>urn:upnp-org:serviceId:ConnectionManager</serviceId> \
                            <SCPDURL>cm.xml</SCPDURL> \
                            <controlURL>/upnp/control/cm</controlURL> \
                            <eventSubURL>/upnp/event/cm</eventSubURL> \
                        </service> \
                        <service> \
                            <serviceType>urn:schemas-upnp-org:service:ContentDirectory:1</serviceType> \
                            <serviceId>urn:upnp-org:serviceId:ContentDirectory</serviceId> \
                            <SCPDURL>cds.xml</SCPDURL> \
                            <controlURL>/upnp/control/cds</controlURL> \
                            <eventSubURL>/upnp/event/cds</eventSubURL> \
                        </service> \
                    </serviceList> \
                </device> \
            </root>"
    );
}

