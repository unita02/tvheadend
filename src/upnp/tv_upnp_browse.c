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

/*
 * upnp-Tree:
 *
 * root(0)  - liveTV(1)  - Channel_1(1001)
 *                           ...
 *                       - Channel_N(1000+N)
 *          - IPTV(2)    - Channel_1(2001)
 *                           ...
 *                       - Channel_N(2000+N)
 *          - Recs(3)    - File_1(10001)
 *                           ...
 *                       - File_N(10000+N)
 *
 *  files should be the biggest amount
 *  treat dirs in rec(Recordings) as files
 *
 */

#include "tvheadend.h"
#include "channels.h"
#include "tv_upnp_browse.h"
#include "tv_upnp_cfg.h"
#include "dvr/dvr.h"

#include <stdlib.h>
#include <string.h>

typedef struct upnp_br_leaf   upnp_br_leaf_t;
typedef struct upnp_br_node   upnp_br_node_t;


struct upnp_br_leaf {
    char path[1025]; // TODO
};
struct upnp_br_node {
    upnp_br_node_t* parent;
    upnp_br_node_t* prev_s;
    upnp_br_node_t* next_s;
    size_t child_count;
    upnp_br_node_t* child1;

    int id;
    int restr;
    char name[257]; // TODO
    upnp_br_leaf_t* leaf;
};

static upnp_br_node_t* upnp_br_root = NULL;

/*----------------------------------------------------------------------------*/

void tv_upnp_browse_node2xml( tvstring_t* xml, upnp_br_node_t* node );

upnp_br_node_t* new_node( const char* name, int id, int rest );
upnp_br_node_t* new_leaf( const char* name, int id, int rest );

void release_node( upnp_br_node_t* node );
void release_leaf( upnp_br_leaf_t* leaf );

void append_as_child( upnp_br_node_t* parent, upnp_br_node_t* node );

int is_leaf( upnp_br_node_t* node );
upnp_br_node_t* find_node( int id, upnp_br_node_t* base );

/*----------------------------------------------------------------------------*/

upnp_br_node_t* new_node( const char* name, int id, int rest  ) {
    upnp_br_node_t* nd = (upnp_br_node_t *)malloc(sizeof(upnp_br_node_t));
    if( nd != NULL ) {
        nd->parent = NULL;
        nd->prev_s = NULL;
        nd->next_s = NULL;
        nd->child_count = 0;
        nd->child1 = NULL;

        strncpy( nd->name, name, sizeof(nd->name) );
        nd->name[ sizeof(nd->name) - 1 ] = 0;
        nd->id = id;
        nd->restr = rest;
        nd->leaf = NULL;
    }
    return nd;
}

upnp_br_node_t* new_leaf( const char* name, int id, int rest  ) {
    upnp_br_node_t* nd = new_node( name, id, rest  );
    if( nd != NULL )
        nd->leaf = (upnp_br_leaf_t*) calloc(sizeof(upnp_br_leaf_t), 1);

    return nd;
}

void append_as_child( upnp_br_node_t* parent, upnp_br_node_t* node )
{
    upnp_br_node_t* prev = NULL;

    if( node == NULL )
        return;

    if( parent == NULL ) {
        release_node( node );
        return;
    }
    node->parent = parent;

    ++(parent->child_count);
    prev = parent->child1;
    while( prev != NULL ) {
        if( prev->next_s == NULL )
            break;
        prev = prev->next_s;
    }

    if( prev == NULL ) {
        parent->child1 = node;
    } else {
        prev->next_s = node;
        node->prev_s = prev;
    }
}

void release_node( upnp_br_node_t* node )
{
    //TODO
}

void release_leaf( upnp_br_leaf_t* leaf )
{
    //TODO
}

int is_leaf( upnp_br_node_t* node )
{
    if( node == NULL ) return -1;
    if( node->leaf == NULL ) return 0;

    return 1;
}

upnp_br_node_t* find_node( int id, upnp_br_node_t* base )
{
    upnp_br_node_t* curr;
    upnp_br_node_t* ret;
    if( base == NULL ) return NULL;
    if( base->id == id ) return base;
    for(curr = base->child1; curr != NULL; curr = curr->next_s ) {
        if( NULL != (ret = find_node( id, curr )) )
            return ret;
    }
    return NULL;
}

/*----------------------------------------------------------------------------*/

void tv_upnp_browse_tv_tree_init(void)
{
    channel_t* ch = NULL;
    int ch_curr = 1000;
    dvr_entry_t* de = NULL;
    int rec_curr = 10000;

    upnp_br_root = new_node( "Root", 0, 1 );
    upnp_br_node_t* live = new_node( "liveTV", 1, 1 );
    upnp_br_node_t* iptv = new_node( "IPTV", 2, 1 );
    upnp_br_node_t* recs = new_node( "Recs", 3, 1 );

    append_as_child( upnp_br_root, live );
    append_as_child( upnp_br_root, iptv );
    append_as_child( upnp_br_root, recs );

    /* Send all channels */
    RB_FOREACH( ch, &channel_name_tree, ch_name_link ) {
        upnp_br_node_t* ch_leaf = new_leaf( ch->ch_name, ++ch_curr, 1 );
        append_as_child( live, ch_leaf );
    }


    /* Send all DVR entries */
    LIST_FOREACH(de, &dvrentries, de_global_link) {
        upnp_br_node_t* rec_leaf = new_leaf( de->de_filename, ++rec_curr, 1 );
        append_as_child( recs, rec_leaf );
    }
}

void tv_upnp_browse_tv_tree_release(void)
{
    release_node( upnp_br_root );
}

void tv_upnp_browse_tv_tree_node( tvstring_t* didl, int node_id, int children, int nd_start, int* nr_ret, int* nr_ttl )
{
    upnp_br_node_t* nd = find_node( node_id, upnp_br_root );

    if( nd == NULL ) {
        (*nr_ttl) = 0;
        (*nr_ret) = 0;
        return;
    }

    if( children ) {
        upnp_br_node_t* curr = NULL;
        int i = 0;
        if( is_leaf(nd) ) return;
        if( (*nr_ret) < 1 )
            (*nr_ret) = nd->child_count;

        for(curr = nd->child1; (curr != NULL)&&(i < nd_start + (*nr_ret) ); curr = curr->next_s, ++i ) {
            tv_upnp_browse_node2xml( didl, curr );
        }
        if(!children) {
            (*nr_ret) = i - nd_start;
            (*nr_ttl) = nd->child_count;
        }

    } else {
        tv_upnp_browse_node2xml( didl, nd );
        (*nr_ttl) = 1;
        (* nr_ret) = 1;
    }
}

void tv_upnp_browse_node2xml( tvstring_t* xml, upnp_br_node_t* node )
{
    int par_id = -1;
    if( node == NULL )
        return;

    if( !is_leaf(node) ) {
        if( node->parent != NULL )
            par_id = node->parent->id;

        tvs_cat( xml, "<container id=\"" );
        tvs_cat_int( xml, node->id );
        tvs_cat( xml, "\" parentID=\"");
        tvs_cat_int( xml, par_id );
        tvs_cat( xml, "\" restricted=\"" );
        tvs_cat_int( xml, node->restr );
        tvs_cat( xml, "\" childCount=\"" );
        tvs_cat_int( xml, node->child_count );
        tvs_cat( xml, "\">"
                      "<dc:title>" );
        tvs_cat( xml, node->name );
        tvs_cat( xml, "</dc:title>"
                      "<upnp:class>object.container</upnp:class>"
                      "</container>" );
    } else {
        if( node->parent != NULL )
            par_id = node->parent->id;

        tvs_cat( xml, "<item id=\"" );
        tvs_cat_int( xml, node->id );
        tvs_cat( xml, "\" parentID=\"");
        tvs_cat_int( xml, par_id );
        tvs_cat( xml, "\" restricted=\"" );
        tvs_cat_int( xml, node->restr );
        tvs_cat( xml, "\">" );
        tvs_cat( xml, "<dc:title>" );
        tvs_cat( xml, node->name );
        tvs_cat( xml, "</dc:title>" );
        tvs_cat( xml, "<upnp:class>object.item.videoItem</upnp:class>" );
        tvs_cat( xml, "<res protocolInfo=\"http-get:*:video/x-matroska:*\" size=\"1172428172\" duration=\"00:14:48.0\" bitrate=\"640\" resolution=\"1920x818\" sampleFrequency=\"48000\" nrAudioChannels=\"1\">" );
        tvs_cat( xml, "http://" );
        tvs_append( xml, tv_upnp_server_ip_g );
        tvs_cat( xml, ":" );
        tvs_cat_int( xml, tv_upnp_server_port_g );
        tvs_cat( xml, "/Sintel.2010.1080p.mkv" );
        tvs_cat( xml, "</res>" );
        tvs_cat( xml, "</item>" );
    }
}


/* RTP ex001
 <DIDL-Lite xmlns="urn:schemas-upnp-org:metadata-1-0/DIDL-Lite/" xmlns:dc="http://purl.org/dc/elements/1.1/" xmlns:upnp="urn:schemas-upnp-org:metadata-1-0/upnp/">
<item id="11" parentID="1" restricted="1">
<dc:title>Network Camera Stream 1</dc:title>
<upnp:class>object.item.videoItem</upnp:class>
<res protocolInfo="rtsp-rtp-udp:*:video/mpeg4-generic:*" resolution="640x480">rtsp://10.42.0.103:554/live.sdp</res>
</item>
<item id="12" parentID="1" restricted="1">
<dc:title>Network Camera Stream 2</dc:title>
<upnp:class>object.item.videoItem</upnp:class>
<res protocolInfo="rtsp-rtp-udp:*:video/mpeg4-generic:*" resolution="176x144">rtsp://10.42.0.103:554/live2.sdp</res>
</item>
</DIDL-Lite>
 */

/* http://www.cybergarage.org/twiki/bin/view/Main/Windows7DLNA

DLNA Profiles for Windows7

UPnP/AV Media Server
urn:upnp-org:serviceId:ConnectionManager:1
GetProtocolInfo

http-get:*:video/x-ms-asf:DLNA.ORG_PN=MPEG4_P2_ASF_ASP_L4_SO_G726
http-get:*:video/x-ms-wmv:DLNA.ORG_PN=WMVMED_PRO
http-get:*:image/jpeg:DLNA.ORG_PN=JPEG_TN
http-get:*:audio/x-ms-wma:DLNA.ORG_PN=WMAFULL
http-get:*:video/x-ms-asf:DLNA.ORG_PN=VC1_ASF_AP_L1_WMA
http-get:*:image/jpeg:DLNA.ORG_PN=JPEG_SM
http-get:*:audio/L16;rate=44100;channels=2:DLNA.ORG_PN=LPCM
http-get:*:video/x-ms-wmv:DLNA.ORG_PN=WMVMED_BASE
http-get:*:video/x-ms-wmv:DLNA.ORG_PN=WMVSPML_MP3
http-get:*:audio/x-ms-wma:DLNA.ORG_PN=WMABASE
http-get:*:video/mpeg:DLNA.ORG_PN=MPEG_PS_PAL
http-get:*:video/x-ms-wmv:DLNA.ORG_PN=WMVHIGH_FULL
http-get:*:video/mpeg:DLNA.ORG_PN=MPEG_PS_PAL_XAC3
http-get:*:image/jpeg:DLNA.ORG_PN=JPEG_MED
http-get:*:video/x-ms-asf:DLNA.ORG_PN=MPEG4_P2_ASF_SP_G726
http-get:*:audio/L16;rate=48000;channels=2:DLNA.ORG_PN=LPCM
http-get:*:audio/mpeg:DLNA.ORG_PN=MP3
http-get:*:video/x-ms-wmv:DLNA.ORG_PN=WMVHIGH_PRO
http-get:*:audio/mpeg:DLNA.ORG_PN=MP3X
http-get:*:video/x-ms-asf:DLNA.ORG_PN=MPEG4_P2_ASF_ASP_L5_SO_G726
http-get:*:video/x-ms-wmv:DLNA.ORG_PN=WMVSPLL_BASE
http-get:*:video/mpeg:DLNA.ORG_PN=MPEG_PS_NTSC
http-get:*:video/mpeg:DLNA.ORG_PN=MPEG1
http-get:*:video/x-ms-wmv:DLNA.ORG_PN=WMVSPML_BASE
http-get:*:video/x-ms-wmv:DLNA.ORG_PN=WMVMED_FULL
http-get:*:audio/x-ms-wma:DLNA.ORG_PN=WMAPRO
http-get:*:image/jpeg:DLNA.ORG_PN=JPEG_LRG
http-get:*:video/mpeg:DLNA.ORG_PN=MPEG_PS_NTSC_XAC3
http-get:*:audio/L16;rate=44100;channels=1:DLNA.ORG_PN=LPCM
http-get:*:image/x-ycbcr-yuv420:*
http-get:*:video/x-ms-wmv:*
http-get:*:audio/x-ms-wma:*
http-get:*:video/wtv:*
rtsp-rtp-udp:*:video/x-ms-asf:DLNA.ORG_PN=MPEG4_P2_ASF_SP_G726
rtsp-rtp-udp:*:video/x-ms-wmv:DLNA.ORG_PN=WMVMED_PRO
rtsp-rtp-udp:*:video/x-ms-asf:DLNA.ORG_PN=MPEG4_P2_ASF_ASP_L5_SO_G726
rtsp-rtp-udp:*:video/x-ms-asf:DLNA.ORG_PN=MPEG4_P2_ASF_ASP_L4_SO_G726
rtsp-rtp-udp:*:video/x-ms-wmv:DLNA.ORG_PN=WMVSPLL_BASE
rtsp-rtp-udp:*:video/x-ms-wmv:DLNA.ORG_PN=WMVSPML_BASE
rtsp-rtp-udp:*:video/x-ms-wmv:DLNA.ORG_PN=WMVHIGH_PRO
rtsp-rtp-udp:*:video/x-ms-wmv:DLNA.ORG_PN=WMVMED_FULL
rtsp-rtp-udp:*:video/x-ms-asf:DLNA.ORG_PN=VC1_ASF_AP_L1_WMA
rtsp-rtp-udp:*:video/x-ms-wmv:DLNA.ORG_PN=WMVMED_BASE
rtsp-rtp-udp:*:video/x-ms-wmv:DLNA.ORG_PN=WMVHIGH_FULL
rtsp-rtp-udp:*:video/x-ms-wmv:DLNA.ORG_PN=WMVSPML_MP3
rtsp-rtp-udp:*:video/x-ms-wmv:*

urn:upnp-org:serviceId:ContentDirectory:1
Browse (Video)

<DIDL-Lite xmlns:dc="http://purl.org/dc/elements/1.1/"
xmlns:upnp="urn:schemas-upnp-org:metadata-1-0/upnp/"
xmlns="urn:schemas-upnp-org:metadata-1-0/DIDL-Lite/">
<item id="{1982D7A6-13AF-4CCB-8725-10B384BB9527}.0.8" restricted="0" parentID="8">
<dc:title>Landscapes</dc:title>
<dc:creator>None</dc:creator>
<res size="9699328" duration="0:00:10.000" protocolInfo="http-get:*:video/wtv:DLNA.ORG_OP=01;DLNA.ORG_FLAGS=01500000000000000000000000000000">http://127.0.0.1:10243/WMPNSSv4/2804982740/1_ezE5ODJEN0E2LTEzQUYtNENDQi04NzI1LTEwQjM4NEJCOTUyN30uMC44.wtv</res>
<res duration="0:00:10.000" bitrate="1024000" resolution="720x480" protocolInfo="http-get:*:video/mpeg:DLNA.ORG_PN=MPEG_PS_NTSC;DLNA.ORG_OP=10;DLNA.ORG_CI=1;DLNA.ORG_FLAGS=01500000000000000000000000000000" nrAudioChannels="2" microsoft:codec="{E06D8026-DB46-11CF-B4D1-00805F6CBBEA}" xmlns:microsoft="urn:schemas-microsoft-com:WMPNSS-1-0/">http://127.0.0.1:10243/WMPNSSv4/2804982740/ezE5ODJEN0E2LTEzQUYtNENDQi04NzI1LTEwQjM4NEJCOTUyN30uMC44.mpg?formatID=33</res>
<res duration="0:00:10.000" bitrate="1024000" resolution="720x576" protocolInfo="http-get:*:video/mpeg:DLNA.ORG_PN=MPEG_PS_PAL;DLNA.ORG_OP=10;DLNA.ORG_CI=1;DLNA.ORG_FLAGS=01500000000000000000000000000000" nrAudioChannels="2" microsoft:codec="{E06D8026-DB46-11CF-B4D1-00805F6CBBEA}" xmlns:microsoft="urn:schemas-microsoft-com:WMPNSS-1-0/">http://127.0.0.1:10243/WMPNSSv4/2804982740/ezE5ODJEN0E2LTEzQUYtNENDQi04NzI1LTEwQjM4NEJCOTUyN30uMC44.mpg?formatID=39</res>
<res duration="0:00:10.000" bitrate="250000" resolution="720x576" protocolInfo="http-get:*:video/x-ms-asf:DLNA.ORG_PN=VC1_ASF_AP_L1_WMA;DLNA.ORG_OP=10;DLNA.ORG_CI=1;DLNA.ORG_FLAGS=01500000000000000000000000000000" sampleFrequency="44100" nrAudioChannels="2" microsoft:codec="{31435657-0000-0010-8000-00AA00389B71}" xmlns:microsoft="urn:schemas-microsoft-com:WMPNSS-1-0/">http://127.0.0.1:10243/WMPNSSv4/2804982740/ezE5ODJEN0E2LTEzQUYtNENDQi04NzI1LTEwQjM4NEJCOTUyN30uMC44.asf?formatID=40</res>
<res duration="0:00:10.000" bitrate="250000" resolution="720x576" protocolInfo="rtsp-rtp-udp:*:video/x-ms-asf:DLNA.ORG_PN=VC1_ASF_AP_L1_WMA;DLNA.ORG_OP=10;DLNA.ORG_CI=1;DLNA.ORG_FLAGS=83100000000000000000000000000000;DLNA.ORG_MAXSP=5" sampleFrequency="44100" nrAudioChannels="2" microsoft:codec="{31435657-0000-0010-8000-00AA00389B71}" xmlns:microsoft="urn:schemas-microsoft-com:WMPNSS-1-0/">rtsp://127.0.0.1:554/WMPNSSv4/2804982740/ezE5ODJEN0E2LTEzQUYtNENDQi04NzI1LTEwQjM4NEJCOTUyN30uMC44.asf?formatID=41</res>
<res duration="0:00:10.000" bitrate="250000" resolution="720x576" protocolInfo="http-get:*:video/x-ms-wmv:DLNA.ORG_PN=WMVMED_BASE;DLNA.ORG_OP=10;DLNA.ORG_CI=1;DLNA.ORG_FLAGS=01500000000000000000000000000000" sampleFrequency="44100" nrAudioChannels="2" microsoft:codec="{33564D57-0000-0010-8000-00AA00389B71}" xmlns:microsoft="urn:schemas-microsoft-com:WMPNSS-1-0/">http://127.0.0.1:10243/WMPNSSv4/2804982740/ezE5ODJEN0E2LTEzQUYtNENDQi04NzI1LTEwQjM4NEJCOTUyN30uMC44.wmv?formatID=62</res>
<res duration="0:00:10.000" bitrate="250000" resolution="720x576" protocolInfo="rtsp-rtp-udp:*:video/x-ms-wmv:DLNA.ORG_PN=WMVMED_BASE;DLNA.ORG_OP=10;DLNA.ORG_CI=1;DLNA.ORG_FLAGS=83100000000000000000000000000000;DLNA.ORG_MAXSP=5" sampleFrequency="44100" nrAudioChannels="2" microsoft:codec="{33564D57-0000-0010-8000-00AA00389B71}" xmlns:microsoft="urn:schemas-microsoft-com:WMPNSS-1-0/">rtsp://127.0.0.1:554/WMPNSSv4/2804982740/ezE5ODJEN0E2LTEzQUYtNENDQi04NzI1LTEwQjM4NEJCOTUyN30uMC44.wmv?formatID=63</res>
<res duration="0:00:10.000" bitrate="37500" resolution="352x288" protocolInfo="http-get:*:video/x-ms-wmv:DLNA.ORG_PN=WMVSPML_BASE;DLNA.ORG_OP=10;DLNA.ORG_CI=1;DLNA.ORG_FLAGS=01500000000000000000000000000000" sampleFrequency="44100" nrAudioChannels="2" microsoft:codec="{33564D57-0000-0010-8000-00AA00389B71}" xmlns:microsoft="urn:schemas-microsoft-com:WMPNSS-1-0/">http://127.0.0.1:10243/WMPNSSv4/2804982740/ezE5ODJEN0E2LTEzQUYtNENDQi04NzI1LTEwQjM4NEJCOTUyN30uMC44.wmv?formatID=80</res>
<res duration="0:00:10.000" bitrate="37500" resolution="352x288" protocolInfo="rtsp-rtp-udp:*:video/x-ms-wmv:DLNA.ORG_PN=WMVSPML_BASE;DLNA.ORG_OP=10;DLNA.ORG_CI=1;DLNA.ORG_FLAGS=83100000000000000000000000000000;DLNA.ORG_MAXSP=5" sampleFrequency="44100" nrAudioChannels="2" microsoft:codec="{33564D57-0000-0010-8000-00AA00389B71}" xmlns:microsoft="urn:schemas-microsoft-com:WMPNSS-1-0/">
rtsp://127.0.0.1:554/WMPNSSv4/2804982740/ezE5ODJEN0E2LTEzQUYtNENDQi04NzI1LTEwQjM4NEJCOTUyN30uMC44.wmv?formatID=81</res>
<upnp:class>object.item.videoItem.videoBroadcast</upnp:class>
<upnp:genre>General</upnp:genre>
<upnp:genre>Movies</upnp:genre>
<upnp:artist role="Performer">None</upnp:artist>
<upnp:album>None</upnp:album>
<dc:date>2009-07-15</dc:date>
<upnp:actor>None</upnp:actor>
<upnp:channelName>Windows Media Center</upnp:channelName>
<upnp:scheduledStartTime>2009-07-15T00:50:56</upnp:scheduledStartTime>
<upnp:longDescription>A sample TV program that shows scenery, mountain-bike riding, and rafting. Produced by Small World Productions, Tourism New Zealand.</upnp:longDescription>
<upnp:albumArtURI dlna:profileID="JPEG_SM" xmlns:dlna="urn:schemas-dlna-org:metadata-1-0/">http://127.0.0.1:10243/WMPNSSv4/2804982740/0_ezE5ODJEN0E2LTEzQUYtNENDQi04NzI1LTEwQjM4NEJCOTUyN30uMC44.jpg?albumArt=true</upnp:albumArtURI>
<upnp:albumArtURI dlna:profileID="JPEG_TN" xmlns:dlna="urn:schemas-dlna-org:metadata-1-0/">http://127.0.0.1:10243/WMPNSSv4/2804982740/ezE5ODJEN0E2LTEzQUYtNENDQi04NzI1LTEwQjM4NEJCOTUyN30uMC44.jpg?albumArt=true,formatID=13</upnp:albumArtURI>
<desc id="artist" nameSpace="urn:schemas-microsoft-com:WMPNSS-1-0/" xmlns:microsoft="urn:schemas-microsoft-com:WMPNSS-1-0/">&lt;microsoft:artistPerformer&gt;[&#20316;&#25104;&#32773;&#24773;&#22577;&#12394;&#12375;]&lt;/microsoft:artistPerformer&gt;</desc>
<desc id="UserRating" nameSpace="urn:schemas-microsoft-com:WMPNSS-1-0/" xmlns:microsoft="urn:schemas-microsoft-com:WMPNSS-1-0/">&lt;microsoft:userEffectiveRatingInStars&gt;3&lt;/microsoft:userEffectiveRatingInStars&gt;&lt;microsoft:userEffectiveRating&gt;50&lt;/microsoft:userEffectiveRating&gt;</desc>
<desc id="folderPath" nameSpace="urn:schemas-microsoft-com:WMPNSS-1-0/" xmlns:microsoft="urn:schemas-microsoft-com:WMPNSS-1-0/">&lt;microsoft:folderPath&gt;Recorded TV\Sample Media&lt;/microsoft:folderPath&gt;</desc>
</item>
</DIDL-Lite>

Browse (Photo)

<DIDL-Lite xmlns:dc="http://purl.org/dc/elements/1.1/"
xmlns:upnp="urn:schemas-upnp-org:metadata-1-0/upnp/"
xmlns="urn:schemas-upnp-org:metadata-1-0/DIDL-Lite/">
<item id="{3319EBDD-685A-4EF9-AC05-76A89B4F62E0}.0.B" restricted="0" parentID="B">
<dc:title>Chrysanthemum</dc:title>
<res size="879394" resolution="1024x768" protocolInfo="http-get:*:image/jpeg:DLNA.ORG_PN=JPEG_MED;DLNA.ORG_OP=01;DLNA.ORG_FLAGS=00d00000000000000000000000000000" colorDepth="24">http://127.0.0.1:10243/WMPNSSv4/2804982740/1_ezMzMTlFQkRELTY4NUEtNEVGOS1BQzA1LTc2QTg5QjRGNjJFMH0uMC5C.jpg</res>
<res resolution="160x120" protocolInfo="http-get:*:image/jpeg:DLNA.ORG_PN=JPEG_TN;DLNA.ORG_OP=01;DLNA.ORG_CI=1;DLNA.ORG_FLAGS=00d00000000000000000000000000000" colorDepth="24">http://127.0.0.1:10243/WMPNSSv4/2804982740/ezMzMTlFQkRELTY4NUEtNEVGOS1BQzA1LTc2QTg5QjRGNjJFMH0uMC5C.jpg?thumbnail=true,formatID=13,width=160,height=120</res>
<res resolution="640x480" protocolInfo="http-get:*:image/jpeg:DLNA.ORG_PN=JPEG_SM;DLNA.ORG_OP=01;DLNA.ORG_CI=1;DLNA.ORG_FLAGS=00d00000000000000000000000000000" colorDepth="24">http://127.0.0.1:10243/WMPNSSv4/2804982740/ezMzMTlFQkRELTY4NUEtNEVGOS1BQzA1LTc2QTg5QjRGNjJFMH0uMC5C.jpg?formatID=16,width=640,height=480</res>
<res resolution="1024x768" protocolInfo="http-get:*:image/jpeg:DLNA.ORG_PN=JPEG_MED;DLNA.ORG_OP=01;DLNA.ORG_CI=1;DLNA.ORG_FLAGS=00d00000000000000000000000000000" colorDepth="24">http://127.0.0.1:10243/WMPNSSv4/2804982740/ezMzMTlFQkRELTY4NUEtNEVGOS1BQzA1LTc2QTg5QjRGNjJFMH0uMC5C.jpg?formatID=17,width=1024,height=768</res>
<res resolution="136x90" protocolInfo="http-get:*:image/x-ycbcr-yuv420:DLNA.ORG_OP=01;DLNA.ORG_CI=1;DLNA.ORG_FLAGS=00d00000000000000000000000000000" colorDepth="24">http://127.0.0.1:10243/WMPNSSv4/2804982740/ezMzMTlFQkRELTY4NUEtNEVGOS1BQzA1LTc2QTg5QjRGNjJFMH0uMC5C.jpg?formatID=82,width=136,height=90,thumbnail=false,aspectRatio=9:8,rFill=20,gFill=20,bFill=20</res>
<res resolution="684x456" protocolInfo="http-get:*:image/x-ycbcr-yuv420:DLNA.ORG_OP=01;DLNA.ORG_CI=1;DLNA.ORG_FLAGS=00d00000000000000000000000000000" colorDepth="24">http://127.0.0.1:10243/WMPNSSv4/2804982740/ezMzMTlFQkRELTY4NUEtNEVGOS1BQzA1LTc2QTg5QjRGNjJFMH0uMC5C.jpg?formatID=83,width=684,height=456,thumbnail=false,aspectRatio=9:8,rFill=20,gFill=20,bFill=20</res>
<res resolution="1024x768" protocolInfo="http-get:*:image/x-ycbcr-yuv420:DLNA.ORG_OP=01;DLNA.ORG_CI=1;DLNA.ORG_FLAGS=00d00000000000000000000000000000" colorDepth="24">http://127.0.0.1:10243/WMPNSSv4/2804982740/ezMzMTlFQkRELTY4NUEtNEVGOS1BQzA1LTc2QTg5QjRGNjJFMH0uMC5C.jpg?formatID=84,width=1024,height=768,thumbnail=false,aspectRatio=1:1,rFill=20,gFill=20,bFill=20</res>
<upnp:class>object.item.imageItem.photo</upnp:class>
<dc:rights>© Corbis.  All Rights Reserved.</dc:rights>
<upnp:album>None</upnp:album>
<dc:date>2008-03-14</dc:date>
<upnp:scheduledStartTime>2008-03-14T13:59:26</upnp:scheduledStartTime>
<upnp:albumArtURI dlna:profileID="JPEG_SM" xmlns:dlna="urn:schemas-dlna-org:metadata-1-0/">http://127.0.0.1:10243/WMPNSSv4/2804982740/0_ezMzMTlFQkRELTY4NUEtNEVGOS1BQzA1LTc2QTg5QjRGNjJFMH0uMC5C.jpg?albumArt=true</upnp:albumArtURI>
<upnp:albumArtURI dlna:profileID="JPEG_TN" xmlns:dlna="urn:schemas-dlna-org:metadata-1-0/">http://127.0.0.1:10243/WMPNSSv4/2804982740/ezMzMTlFQkRELTY4NUEtNEVGOS1BQzA1LTc2QTg5QjRGNjJFMH0uMC5C.jpg?albumArt=true,formatID=13</upnp:albumArtURI>
<desc id="UserRating" nameSpace="urn:schemas-microsoft-com:WMPNSS-1-0/" xmlns:microsoft="urn:schemas-microsoft-com:WMPNSS-1-0/">&lt;microsoft:userRatingInStars&gt;4&lt;/microsoft:userRatingInStars&gt;&lt;microsoft:userRating&gt;75&lt;/microsoft:userRating&gt;&lt;microsoft:userEffectiveRating&gt;75&lt;/microsoft:userEffectiveRating&gt;</desc>
<desc id="folderPath" nameSpace="urn:schemas-microsoft-com:WMPNSS-1-0/" xmlns:microsoft="urn:schemas-microsoft-com:WMPNSS-1-0/">&lt;microsoft:folderPath&gt;&#20849;&#26377;&#30011;&#20687;\Sample Pictures&lt;/microsoft:folderPath&gt;</desc>
</item>
</DIDL-Lite>

Browse (Music)

<DIDL-Lite xmlns:dc="http://purl.org/dc/elements/1.1/"
xmlns:upnp="urn:schemas-upnp-org:metadata-1-0/upnp/"
xmlns="urn:schemas-upnp-org:metadata-1-0/DIDL-Lite/">
<item id="{289EBB88-F8C7-4011-9C5F-79A656556E0C}.0.4" restricted="0" parentID="4">
<dc:title>Kalimba</dc:title>
<dc:creator>Mr. Scruff</dc:creator>
<res size="8414449" duration="0:05:48.000" bitrate="24000" protocolInfo="http-get:*:audio/mpeg:DLNA.ORG_PN=MP3;DLNA.ORG_OP=01;DLNA.ORG_FLAGS=01500000000000000000000000000000" sampleFrequency="44100" bitsPerSample="16" nrAudioChannels="2" microsoft:codec="{00000055-0000-0010-8000-00AA00389B71}" xmlns:microsoft="urn:schemas-microsoft-com:WMPNSS-1-0/">http://127.0.0.1:10243/WMPNSSv4/2804982740/1_ezI4OUVCQjg4LUY4QzctNDAxMS05QzVGLTc5QTY1NjU1NkUwQ30uMC40.mp3</res>
<res duration="0:05:48.000" bitrate="176400" protocolInfo="http-get:*:audio/L16;rate=44100;channels=2:DLNA.ORG_PN=LPCM;DLNA.ORG_OP=10;DLNA.ORG_CI=1;DLNA.ORG_FLAGS=01500000000000000000000000000000" sampleFrequency="44100" bitsPerSample="16" nrAudioChannels="2" microsoft:codec="{00000001-0000-0010-8000-00AA00389B71}" xmlns:microsoft="urn:schemas-microsoft-com:WMPNSS-1-0/">http://127.0.0.1:10243/WMPNSSv4/2804982740/ezI4OUVCQjg4LUY4QzctNDAxMS05QzVGLTc5QTY1NjU1NkUwQ30uMC40.mp3?formatID=20</res>
<res duration="0:05:48.000" bitrate="88200" protocolInfo="http-get:*:audio/L16;rate=44100;channels=1:DLNA.ORG_PN=LPCM;DLNA.ORG_OP=10;DLNA.ORG_CI=1;DLNA.ORG_FLAGS=01500000000000000000000000000000" sampleFrequency="44100" bitsPerSample="16" nrAudioChannels="1" microsoft:codec="{00000001-0000-0010-8000-00AA00389B71}" xmlns:microsoft="urn:schemas-microsoft-com:WMPNSS-1-0/">http://127.0.0.1:10243/WMPNSSv4/2804982740/ezI4OUVCQjg4LUY4QzctNDAxMS05QzVGLTc5QTY1NjU1NkUwQ30uMC40.mp3?formatID=18</res>
<res duration="0:05:48.000" bitrate="8000" protocolInfo="http-get:*:audio/x-ms-wma:DLNA.ORG_PN=WMABASE;DLNA.ORG_OP=10;DLNA.ORG_CI=1;DLNA.ORG_FLAGS=01500000000000000000000000000000" sampleFrequency="44100" nrAudioChannels="2" microsoft:codec="{00000161-0000-0010-8000-00AA00389B71}" xmlns:microsoft="urn:schemas-microsoft-com:WMPNSS-1-0/">http://127.0.0.1:10243/WMPNSSv4/2804982740/ezI4OUVCQjg4LUY4QzctNDAxMS05QzVGLTc5QTY1NjU1NkUwQ30uMC40.wma?formatID=54</res>
<upnp:class>object.item.audioItem.musicTrack</upnp:class>
<upnp:genre>Electronic</upnp:genre>
<dc:publisher>Ninja Tune</dc:publisher>
<upnp:artist role="AlbumArtist">Mr. Scruff</upnp:artist>
<upnp:artist role="Performer">Mr. Scruff</upnp:artist>
<upnp:author role="Composer">A. Carthy and A. Kingslow</upnp:author>
<upnp:album>Ninja Tuna</upnp:album>
<upnp:originalTrackNumber>1</upnp:originalTrackNumber>
<dc:date>2008-01-02</dc:date>
<upnp:actor>Mr. Scruff</upnp:actor>
<upnp:albumArtURI dlna:profileID="JPEG_SM" xmlns:dlna="urn:schemas-dlna-org:metadata-1-0/">http://127.0.0.1:10243/WMPNSSv4/2804982740/0_ezI4OUVCQjg4LUY4QzctNDAxMS05QzVGLTc5QTY1NjU1NkUwQ30uMC40.jpg?albumArt=true</upnp:albumArtURI>
<upnp:albumArtURI dlna:profileID="JPEG_TN" xmlns:dlna="urn:schemas-dlna-org:metadata-1-0/">http://127.0.0.1:10243/WMPNSSv4/2804982740/ezI4OUVCQjg4LUY4QzctNDAxMS05QzVGLTc5QTY1NjU1NkUwQ30uMC40.jpg?albumArt=true,formatID=13</upnp:albumArtURI>
<desc id="artist" nameSpace="urn:schemas-microsoft-com:WMPNSS-1-0/" xmlns:microsoft="urn:schemas-microsoft-com:WMPNSS-1-0/">&lt;microsoft:artistAlbumArtist&gt;Mr. Scruff&lt;/microsoft:artistAlbumArtist&gt;&lt;microsoft:artistPerformer&gt;Mr. Scruff&lt;/microsoft:artistPerformer&gt;</desc>
<desc id="author" nameSpace="urn:schemas-microsoft-com:WMPNSS-1-0/" xmlns:microsoft="urn:schemas-microsoft-com:WMPNSS-1-0/">&lt;microsoft:authorComposer&gt;A. Carthy and A. Kingslow&lt;/microsoft:authorComposer&gt;</desc>
<desc id="Year" nameSpace="urn:schemas-microsoft-com:WMPNSS-1-0/" xmlns:microsoft="urn:schemas-microsoft-com:WMPNSS-1-0/">&lt;microsoft:year&gt;2008&lt;/microsoft:year&gt;</desc>
<desc id="UserRating" nameSpace="urn:schemas-microsoft-com:WMPNSS-1-0/" xmlns:microsoft="urn:schemas-microsoft-com:WMPNSS-1-0/">&lt;microsoft:userEffectiveRatingInStars&gt;3&lt;/microsoft:userEffectiveRatingInStars&gt;&lt;microsoft:userEffectiveRating&gt;60&lt;/microsoft:userEffectiveRating&gt;</desc>
<desc id="folderPath" nameSpace="urn:schemas-microsoft-com:WMPNSS-1-0/" xmlns:microsoft="urn:schemas-microsoft-com:WMPNSS-1-0/">&lt;microsoft:folderPath&gt;Share Music\Sample Music&lt;/microsoft:folderPath&gt;</desc>
</item>
</DIDL-Lite>

UPnP/AV Media Renderer
urn:upnp-org:serviceId:ConnectionManager:1
GetProtocolInfo

http-get:*:audio/vnd.dlna.adts:DLNA.ORG_PN=AAC_ADTS
http-get:*:video/x-ms-wmv:DLNA.ORG_PN=WMVSPML_MP3
http-get:*:audio/vnd.dlna.adts:DLNA.ORG_PN=AAC_ADTS_192
http-get:*:audio/vnd.dlna.adts:DLNA.ORG_PN=AAC_ADTS_320
http-get:*:audio/mp4:DLNA.ORG_PN=AAC_ISO
http-get:*:video/x-ms-wmv:DLNA.ORG_PN=WMVSPML_BASE
http-get:*:audio/3gpp:DLNA.ORG_PN=AAC_ISO
http-get:*:video/x-ms-wmv:DLNA.ORG_PN=WMVSPLL_BASE
http-get:*:audio/mp4:DLNA.ORG_PN=AAC_ISO_192
http-get:*:audio/3gpp:DLNA.ORG_PN=AAC_ISO_192
http-get:*:audio/mp4:DLNA.ORG_PN=AAC_ISO_320
http-get:*:audio/3gpp:DLNA.ORG_PN=AAC_ISO_320
http-get:*:audio/vnd.dlna.adts:DLNA.ORG_PN=AAC_MULT5_ADTS
http-get:*:audio/mp4:DLNA.ORG_PN=AAC_MULT5_ISO
http-get:*:audio/3gpp:DLNA.ORG_PN=AAC_MULT5_ISO
http-get:*:video/3gpp:DLNA.ORG_PN=AVC_3GPP_BL_QCIF15_AAC
http-get:*:video/x-ms-wmv:DLNA.ORG_PN=WMVMED_PRO
http-get:*:video/3gpp:DLNA.ORG_PN=AVC_3GPP_BL_QCIF15_HEAAC
http-get:*:video/mp4:DLNA.ORG_PN=AVC_MP4_BL_CIF15_AAC
http-get:*:video/x-ms-wmv:DLNA.ORG_PN=WMVMED_FULL
http-get:*:video/mp4:DLNA.ORG_PN=AVC_MP4_BL_CIF15_AAC_350
http-get:*:video/mp4:DLNA.ORG_PN=AVC_MP4_BL_CIF15_AAC_520
http-get:*:video/mp4:DLNA.ORG_PN=AVC_MP4_BL_CIF15_HEAAC
http-get:*:video/mp4:DLNA.ORG_PN=AVC_MP4_BL_CIF15_HEAAC_350
http-get:*:video/mp4:DLNA.ORG_PN=AVC_MP4_BL_CIF30_AAC_940
http-get:*:video/mp4:DLNA.ORG_PN=AVC_MP4_BL_CIF30_AAC_MULT5
http-get:*:video/mp4:DLNA.ORG_PN=AVC_MP4_BL_CIF30_HEAAC_L2
http-get:*:video/mp4:DLNA.ORG_PN=AVC_MP4_BL_CIF30_MPEG1_L3
http-get:*:video/mp4:DLNA.ORG_PN=AVC_MP4_BL_L12_CIF15_HEAAC
http-get:*:video/mp4:DLNA.ORG_PN=AVC_MP4_BL_L12_CIF15_HEAACv2
http-get:*:video/mp4:DLNA.ORG_PN=AVC_MP4_BL_L12_CIF15_HEAACv2_350
http-get:*:video/mp4:DLNA.ORG_PN=AVC_MP4_BL_L1B_QCIF15_HEAAC
http-get:*:video/mp4:DLNA.ORG_PN=AVC_MP4_BL_L1B_QCIF15_HEAACv2
http-get:*:video/mp4:DLNA.ORG_PN=AVC_MP4_BL_L2_CIF30_AAC
http-get:*:video/mp4:DLNA.ORG_PN=AVC_MP4_BL_L31_HD_AAC
http-get:*:video/mp4:DLNA.ORG_PN=AVC_MP4_BL_L32_HD_AAC
http-get:*:video/mp4:DLNA.ORG_PN=AVC_MP4_BL_L3L_SD_AAC
http-get:*:video/mp4:DLNA.ORG_PN=AVC_MP4_BL_L3L_SD_HEAAC
http-get:*:video/mp4:DLNA.ORG_PN=AVC_MP4_BL_L3_SD_AAC
http-get:*:video/mp4:DLNA.ORG_PN=AVC_MP4_MP_SD_AAC_LC
http-get:*:video/mp4:DLNA.ORG_PN=AVC_MP4_MP_SD_AAC_MULT5
http-get:*:video/mp4:DLNA.ORG_PN=AVC_MP4_MP_SD_HEAAC_L2
http-get:*:video/mp4:DLNA.ORG_PN=AVC_MP4_MP_SD_HEAAC_L4
http-get:*:video/mp4:DLNA.ORG_PN=AVC_MP4_MP_SD_MPEG1_L3
http-get:*:video/vnd.dlna.mpeg-tts:DLNA.ORG_PN=AVC_TS_BL_CIF15_AAC
http-get:*:video/x-ms-wmv:DLNA.ORG_PN=WMVMED_BASE
http-get:*:video/vnd.dlna.mpeg-tts:DLNA.ORG_PN=AVC_TS_BL_CIF15_AAC_540
http-get:*:video/mpeg:DLNA.ORG_PN=AVC_TS_BL_CIF15_AAC_540_ISO
http-get:*:video/x-ms-wmv:DLNA.ORG_PN=WMVHM_BASE
http-get:*:video/vnd.dlna.mpeg-tts:DLNA.ORG_PN=AVC_TS_BL_CIF15_AAC_540_T
http-get:*:video/mpeg:DLNA.ORG_PN=AVC_TS_BL_CIF15_AAC_ISO
http-get:*:video/vnd.dlna.mpeg-tts:DLNA.ORG_PN=AVC_TS_BL_CIF15_AAC_T
http-get:*:video/vnd.dlna.mpeg-tts:DLNA.ORG_PN=AVC_TS_BL_CIF30_AAC_940
http-get:*:video/mpeg:DLNA.ORG_PN=AVC_TS_BL_CIF30_AAC_940_ISO
http-get:*:video/vnd.dlna.mpeg-tts:DLNA.ORG_PN=AVC_TS_BL_CIF30_AAC_940_T
http-get:*:video/vnd.dlna.mpeg-tts:DLNA.ORG_PN=AVC_TS_BL_CIF30_AAC_MULT5
http-get:*:video/mpeg:DLNA.ORG_PN=AVC_TS_BL_CIF30_AAC_MULT5_ISO
http-get:*:video/vnd.dlna.mpeg-tts:DLNA.ORG_PN=AVC_TS_BL_CIF30_AAC_MULT5_T
http-get:*:video/vnd.dlna.mpeg-tts:DLNA.ORG_PN=AVC_TS_BL_CIF30_AC3
http-get:*:video/mpeg:DLNA.ORG_PN=AVC_TS_BL_CIF30_AC3_ISO
http-get:*:video/vnd.dlna.mpeg-tts:DLNA.ORG_PN=AVC_TS_BL_CIF30_AC3_T
http-get:*:video/vnd.dlna.mpeg-tts:DLNA.ORG_PN=AVC_TS_BL_CIF30_MPEG1_L3
http-get:*:video/mpeg:DLNA.ORG_PN=AVC_TS_BL_CIF30_MPEG1_L3_ISO
http-get:*:video/vnd.dlna.mpeg-tts:DLNA.ORG_PN=AVC_TS_BL_CIF30_MPEG1_L3_T
http-get:*:video/vnd.dlna.mpeg-tts:DLNA.ORG_PN=AVC_TS_JP_AAC_T
http-get:*:video/vnd.dlna.mpeg-tts:DLNA.ORG_PN=AVC_TS_MP_HD_AAC_MULT5
http-get:*:video/mpeg:DLNA.ORG_PN=AVC_TS_MP_HD_AAC_MULT5_ISO
http-get:*:video/vnd.dlna.mpeg-tts:DLNA.ORG_PN=AVC_TS_MP_HD_AAC_MULT5_T
http-get:*:video/vnd.dlna.mpeg-tts:DLNA.ORG_PN=AVC_TS_MP_HD_AC3
http-get:*:video/mpeg:DLNA.ORG_PN=AVC_TS_MP_HD_AC3_ISO
http-get:*:video/vnd.dlna.mpeg-tts:DLNA.ORG_PN=AVC_TS_MP_HD_AC3_T
http-get:*:video/vnd.dlna.mpeg-tts:DLNA.ORG_PN=AVC_TS_MP_HD_MPEG1_L3
http-get:*:video/mpeg:DLNA.ORG_PN=AVC_TS_MP_HD_MPEG1_L3_ISO
http-get:*:video/vnd.dlna.mpeg-tts:DLNA.ORG_PN=AVC_TS_MP_HD_MPEG1_L3_T
http-get:*:video/vnd.dlna.mpeg-tts:DLNA.ORG_PN=AVC_TS_MP_SD_AAC_MULT5
http-get:*:video/mpeg:DLNA.ORG_PN=AVC_TS_MP_SD_AAC_MULT5_ISO
http-get:*:video/vnd.dlna.mpeg-tts:DLNA.ORG_PN=AVC_TS_MP_SD_AAC_MULT5_T
http-get:*:video/vnd.dlna.mpeg-tts:DLNA.ORG_PN=AVC_TS_MP_SD_AC3
http-get:*:video/mpeg:DLNA.ORG_PN=AVC_TS_MP_SD_AC3_ISO
http-get:*:video/vnd.dlna.mpeg-tts:DLNA.ORG_PN=AVC_TS_MP_SD_AC3_T
http-get:*:video/vnd.dlna.mpeg-tts:DLNA.ORG_PN=AVC_TS_MP_SD_MPEG1_L3
http-get:*:video/mpeg:DLNA.ORG_PN=AVC_TS_MP_SD_MPEG1_L3_ISO
http-get:*:video/vnd.dlna.mpeg-tts:DLNA.ORG_PN=AVC_TS_MP_SD_MPEG1_L3_T
http-get:*:audio/mp4:DLNA.ORG_PN=HEAACv2_L2
http-get:*:audio/3gpp:DLNA.ORG_PN=HEAACv2_L2
http-get:*:audio/mp4:DLNA.ORG_PN=HEAACv2_L2_128
http-get:*:audio/3gpp:DLNA.ORG_PN=HEAACv2_L2_128
http-get:*:audio/mp4:DLNA.ORG_PN=HEAACv2_L2_320
http-get:*:audio/3gpp:DLNA.ORG_PN=HEAACv2_L2_320
http-get:*:audio/mp4:DLNA.ORG_PN=HEAACv2_L3
http-get:*:audio/3gpp:DLNA.ORG_PN=HEAACv2_L3
http-get:*:audio/mp4:DLNA.ORG_PN=HEAACv2_L4
http-get:*:audio/3gpp:DLNA.ORG_PN=HEAACv2_L4
http-get:*:audio/mp4:DLNA.ORG_PN=HEAACv2_MULT5
http-get:*:audio/3gpp:DLNA.ORG_PN=HEAACv2_MULT5
http-get:*:audio/mp4:DLNA.ORG_PN=HEAAC_L2_ISO
http-get:*:audio/3gpp:DLNA.ORG_PN=HEAAC_L2_ISO
http-get:*:audio/mp4:DLNA.ORG_PN=HEAAC_L2_ISO_128
http-get:*:audio/3gpp:DLNA.ORG_PN=HEAAC_L2_ISO_128
http-get:*:audio/mp4:DLNA.ORG_PN=HEAAC_L2_ISO_320
http-get:*:audio/3gpp:DLNA.ORG_PN=HEAAC_L2_ISO_320
http-get:*:audio/mp4:DLNA.ORG_PN=HEAAC_L3_ISO
http-get:*:audio/3gpp:DLNA.ORG_PN=HEAAC_L3_ISO
http-get:*:audio/mp4:DLNA.ORG_PN=HEAAC_MULT5_ISO
http-get:*:audio/3gpp:DLNA.ORG_PN=HEAAC_MULT5_ISO
http-get:*:image/jpeg:DLNA.ORG_PN=JPEG_LRG
http-get:*:video/x-ms-wmv:DLNA.ORG_PN=WMVHIGH_PRO
http-get:*:image/jpeg:DLNA.ORG_PN=JPEG_LRG_ICO
http-get:*:image/jpeg:DLNA.ORG_PN=JPEG_MED
http-get:*:image/jpeg:DLNA.ORG_PN=JPEG_SM
http-get:*:image/jpeg:DLNA.ORG_PN=JPEG_SM_ICO
http-get:*:image/jpeg:DLNA.ORG_PN=JPEG_TN
http-get:*:audio/L16:DLNA.ORG_PN=LPCM
http-get:*:video/x-ms-wmv:DLNA.ORG_PN=WMVHIGH_FULL
http-get:*:audio/L16:DLNA.ORG_PN=LPCM_low
http-get:*:audio/mpeg:DLNA.ORG_PN=MP3
http-get:*:video/x-ms-wmv:DLNA.ORG_PN=WMDRM_WMVSPML_MP3
http-get:*:audio/mpeg:DLNA.ORG_PN=MP3X
http-get:*:video/mpeg:DLNA.ORG_PN=MPEG1
http-get:*:video/3gpp:DLNA.ORG_PN=MPEG4_P2_3GPP_SP_L0B_AAC
http-get:*:video/mp4:DLNA.ORG_PN=MPEG4_P2_MP4_ASP_AAC
http-get:*:video/mp4:DLNA.ORG_PN=MPEG4_P2_MP4_ASP_HEAAC
http-get:*:video/mp4:DLNA.ORG_PN=MPEG4_P2_MP4_ASP_HEAAC_MULT5
http-get:*:video/mp4:DLNA.ORG_PN=MPEG4_P2_MP4_SP_AAC
http-get:*:video/mp4:DLNA.ORG_PN=MPEG4_P2_MP4_SP_HEAAC
http-get:*:video/mp4:DLNA.ORG_PN=MPEG4_P2_MP4_SP_L2_AAC
http-get:*:video/mp4:DLNA.ORG_PN=MPEG4_P2_MP4_SP_VGA_AAC
http-get:*:video/mp4:DLNA.ORG_PN=MPEG4_P2_MP4_SP_VGA_HEAAC
http-get:*:video/vnd.dlna.mpeg-tts:DLNA.ORG_PN=MPEG4_P2_TS_SP_AC3
http-get:*:video/mpeg:DLNA.ORG_PN=MPEG4_P2_TS_SP_AC3_ISO
http-get:*:video/vnd.dlna.mpeg-tts:DLNA.ORG_PN=MPEG4_P2_TS_SP_AC3_T
http-get:*:video/vnd.dlna.mpeg-tts:DLNA.ORG_PN=MPEG4_P2_TS_SP_MPEG1_L3
http-get:*:video/mpeg:DLNA.ORG_PN=MPEG4_P2_TS_SP_MPEG1_L3_ISO
http-get:*:video/vnd.dlna.mpeg-tts:DLNA.ORG_PN=MPEG4_P2_TS_SP_MPEG1_L3_T
http-get:*:video/vnd.dlna.mpeg-tts:DLNA.ORG_PN=MPEG4_P2_TS_SP_MPEG2_L2
http-get:*:video/mpeg:DLNA.ORG_PN=MPEG4_P2_TS_SP_MPEG2_L2_ISO
http-get:*:video/vnd.dlna.mpeg-tts:DLNA.ORG_PN=MPEG4_P2_TS_SP_MPEG2_L2_T
http-get:*:video/mpeg:DLNA.ORG_PN=MPEG_PS_NTSC
http-get:*:video/mpeg:DLNA.ORG_PN=MPEG_PS_NTSC_XAC3
http-get:*:video/mpeg:DLNA.ORG_PN=MPEG_PS_PAL
http-get:*:video/mpeg:DLNA.ORG_PN=MPEG_PS_PAL_XAC3
http-get:*:video/vnd.dlna.mpeg-tts:DLNA.ORG_PN=MPEG_TS_HD_KO
http-get:*:video/mpeg:DLNA.ORG_PN=MPEG_TS_HD_KO_ISO
http-get:*:video/vnd.dlna.mpeg-tts:DLNA.ORG_PN=MPEG_TS_HD_KO_T
http-get:*:video/vnd.dlna.mpeg-tts:DLNA.ORG_PN=MPEG_TS_HD_KO_XAC3
http-get:*:video/mpeg:DLNA.ORG_PN=MPEG_TS_HD_KO_XAC3_ISO
http-get:*:video/vnd.dlna.mpeg-tts:DLNA.ORG_PN=MPEG_TS_HD_KO_XAC3_T
http-get:*:video/vnd.dlna.mpeg-tts:DLNA.ORG_PN=MPEG_TS_HD_NA
http-get:*:video/mpeg:DLNA.ORG_PN=MPEG_TS_HD_NA_ISO
http-get:*:video/vnd.dlna.mpeg-tts:DLNA.ORG_PN=MPEG_TS_HD_NA_T
http-get:*:video/vnd.dlna.mpeg-tts:DLNA.ORG_PN=MPEG_TS_HD_NA_XAC3
http-get:*:video/mpeg:DLNA.ORG_PN=MPEG_TS_HD_NA_XAC3_ISO
http-get:*:video/vnd.dlna.mpeg-tts:DLNA.ORG_PN=MPEG_TS_HD_NA_XAC3_T
http-get:*:video/vnd.dlna.mpeg-tts:DLNA.ORG_PN=MPEG_TS_JP_T
http-get:*:video/vnd.dlna.mpeg-tts:DLNA.ORG_PN=MPEG_TS_MP_LL_AAC
http-get:*:video/mpeg:DLNA.ORG_PN=MPEG_TS_MP_LL_AAC_ISO
http-get:*:video/vnd.dlna.mpeg-tts:DLNA.ORG_PN=MPEG_TS_MP_LL_AAC_T
http-get:*:video/vnd.dlna.mpeg-tts:DLNA.ORG_PN=MPEG_TS_SD_EU
http-get:*:video/mpeg:DLNA.ORG_PN=MPEG_TS_SD_EU_ISO
http-get:*:video/vnd.dlna.mpeg-tts:DLNA.ORG_PN=MPEG_TS_SD_EU_T
http-get:*:video/vnd.dlna.mpeg-tts:DLNA.ORG_PN=MPEG_TS_SD_KO
http-get:*:video/mpeg:DLNA.ORG_PN=MPEG_TS_SD_KO_ISO
http-get:*:video/vnd.dlna.mpeg-tts:DLNA.ORG_PN=MPEG_TS_SD_KO_T
http-get:*:video/vnd.dlna.mpeg-tts:DLNA.ORG_PN=MPEG_TS_SD_KO_XAC3
http-get:*:video/mpeg:DLNA.ORG_PN=MPEG_TS_SD_KO_XAC3_ISO
http-get:*:video/vnd.dlna.mpeg-tts:DLNA.ORG_PN=MPEG_TS_SD_KO_XAC3_T
http-get:*:video/vnd.dlna.mpeg-tts:DLNA.ORG_PN=MPEG_TS_SD_NA
http-get:*:video/mpeg:DLNA.ORG_PN=MPEG_TS_SD_NA_ISO
http-get:*:video/vnd.dlna.mpeg-tts:DLNA.ORG_PN=MPEG_TS_SD_NA_T
http-get:*:video/vnd.dlna.mpeg-tts:DLNA.ORG_PN=MPEG_TS_SD_NA_XAC3
http-get:*:video/mpeg:DLNA.ORG_PN=MPEG_TS_SD_NA_XAC3_ISO
http-get:*:video/vnd.dlna.mpeg-tts:DLNA.ORG_PN=MPEG_TS_SD_NA_XAC3_T
http-get:*:image/png:DLNA.ORG_PN=PNG_LRG
http-get:*:video/x-ms-wmv:DLNA.ORG_PN=WMDRM_WMVSPML_BASE
http-get:*:image/png:DLNA.ORG_PN=PNG_LRG_ICO
http-get:*:image/png:DLNA.ORG_PN=PNG_TN
http-get:*:video/x-ms-asf:DLNA.ORG_PN=VC1_ASF_AP_L1_WMA
http-get:*:video/x-ms-wmv:DLNA.ORG_PN=WMDRM_WMVSPLL_BASE
http-get:*:video/x-ms-asf:DLNA.ORG_PN=VC1_ASF_AP_L2_WMA
http-get:*:audio/x-ms-wma:DLNA.ORG_PN=WMABASE
http-get:*:video/x-ms-wmv:DLNA.ORG_PN=WMDRM_WMVMED_PRO
http-get:*:audio/x-ms-wma:DLNA.ORG_PN=WMAFULL
http-get:*:audio/x-ms-wma:DLNA.ORG_PN=WMALSL
http-get:*:audio/x-ms-wma:DLNA.ORG_PN=WMALSL_MULT5
http-get:*:audio/x-ms-wma:DLNA.ORG_PN=WMAPRO
http-get:*:video/x-ms-asf:DLNA.ORG_PN=WMDRM_VC1_ASF_AP_L1_WMA
http-get:*:video/x-ms-asf:DLNA.ORG_PN=WMDRM_VC1_ASF_AP_L2_WMA
http-get:*:audio/x-ms-wma:DLNA.ORG_PN=WMDRM_WMABASE
http-get:*:audio/x-ms-wma:DLNA.ORG_PN=WMDRM_WMAFULL
http-get:*:audio/x-ms-wma:DLNA.ORG_PN=WMDRM_WMALSL
http-get:*:audio/x-ms-wma:DLNA.ORG_PN=WMDRM_WMALSL_MULT5
http-get:*:audio/x-ms-wma:DLNA.ORG_PN=WMDRM_WMAPRO
http-get:*:video/x-ms-wmv:DLNA.ORG_PN=WMDRM_WMVHIGH_FULL
http-get:*:video/x-ms-wmv:DLNA.ORG_PN=WMDRM_WMVMED_FULL
http-get:*:video/x-ms-wmv:DLNA.ORG_PN=WMDRM_WMVHIGH_PRO
http-get:*:video/x-ms-wmv:DLNA.ORG_PN=WMDRM_WMVHM_BASE
http-get:*:video/x-ms-wmv:DLNA.ORG_PN=WMDRM_WMVMED_BASE
http-get:*:video/x-ms-wmv:*
http-get:*:audio/x-ms-wma:*
http-get:*:video/x-ms-asf:*
http-get:*:image/png:*
http-get:*:audio/mpeg:*
http-get:*:audio/L16:*
http-get:*:image/jpeg:*
http-get:*:video/mpeg:*
http-get:*:video/vnd.dlna.mpeg-tts:*
http-get:*:video/mp4:*
http-get:*:video/3gpp:*
http-get:*:audio/3gpp:*
http-get:*:audio/mp4:*
http-get:*:audio/vnd.dlna.adts:*
http-get:*:application/vnd.ms-search:*
http-get:*:application/vnd.ms-wpl:*
http-get:*:application/x-ms-wmd:*
http-get:*:application/x-ms-wmz:*
http-get:*:application/x-shockwave-flash:*
http-get:*:audio/3gpp2:*
http-get:*:audio/aiff:*
http-get:*:audio/basic:*
http-get:*:audio/l8:*
http-get:*:audio/mid:*
http-get:*:audio/wav:*
http-get:*:audio/x-mpegurl:*
http-get:*:audio/x-ms-wax:*
http-get:*:image/bmp:*
http-get:*:image/gif:*
http-get:*:image/vnd.ms-photo:*
http-get:*:video/3gpp2:*
http-get:*:video/avi:*
http-get:*:video/quicktime:*
http-get:*:video/x-ms-wm:*
http-get:*:video/x-ms-wmx:*
http-get:*:video/x-ms-wvx:*
http-get:*:video/x-msvideo:*
rtsp-rtp-udp:*:audio/L16:*
rtsp-rtp-udp:*:audio/L8:*
rtsp-rtp-udp:*:audio/mpeg:*
rtsp-rtp-udp:*:audio/x-ms-wma:*
rtsp-rtp-udp:*:video/x-ms-wmv:*
rtsp-rtp-udp:*:audio/x-asf-pf:*


*/
