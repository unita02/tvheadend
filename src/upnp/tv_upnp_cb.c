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
#include "tv_upnp_cb.h"
#include "tv_upnp_tools.h"
#include "tv_upnp_browse.h"
#include "ixml.h"
#include <stdlib.h>
#include <stdio.h>
#include <strings.h>

/*----------------------------------------------------------------------------*/

int upnp_act_browse( struct Upnp_Action_Request* uar );
int upnp_act_get_search_caps( struct Upnp_Action_Request* uar );
int upnp_act_get_sort_caps( struct Upnp_Action_Request* uar );
int upnp_act_get_sys_upd_ID( struct Upnp_Action_Request* uar );

int tv_upnp_event_subscription( struct Upnp_Subscription_Request* usr );
int tv_upnp_control_action( struct Upnp_Action_Request* uar );
void tv_upnp_specchar2entity(const tvstring_t* Data, tvstring_t* NewData);

/*----------------------------------------------------------------------------*/

int tv_upnp_callback(Upnp_EventType eventtype, void *event, void *cookie)
{
    int ret = UPNP_E_BAD_REQUEST;
    if( event == NULL )
        return ret;

    switch ( eventtype ) {
    case UPNP_EVENT_SUBSCRIPTION_REQUEST:
        ret = tv_upnp_event_subscription( (struct Upnp_Subscription_Request *)event );
        break;

    case UPNP_CONTROL_ACTION_REQUEST:
        ret = tv_upnp_control_action( (struct Upnp_Action_Request *)event );
        break;

    case UPNP_DISCOVERY_ADVERTISEMENT_ALIVE:
    case UPNP_DISCOVERY_SEARCH_RESULT:
    case UPNP_DISCOVERY_SEARCH_TIMEOUT:
    case UPNP_DISCOVERY_ADVERTISEMENT_BYEBYE:
    case UPNP_CONTROL_ACTION_COMPLETE:
    case UPNP_CONTROL_GET_VAR_COMPLETE:
    case UPNP_EVENT_RECEIVED:
    case UPNP_EVENT_SUBSCRIBE_COMPLETE:
    case UPNP_EVENT_UNSUBSCRIBE_COMPLETE:
    case UPNP_EVENT_RENEWAL_COMPLETE:
    case UPNP_EVENT_AUTORENEWAL_FAILED:
    case UPNP_EVENT_SUBSCRIPTION_EXPIRED:
    case UPNP_CONTROL_GET_VAR_REQUEST:
        break;
    }
    return ret;
}

/*
tv_upnp_subs*
tv_upnp_find_subs( char* ServiceId, char* Sid )
{

}
*/

//if( ++upnp_event_subscr_count_g > upnp_event_subscr_max_g )

int tv_upnp_event_subscription( struct Upnp_Subscription_Request* usr )
{
    printf( "upnp_event_subscription - ID: %s, Sid: %s, UDN: %s\n", usr->ServiceId, usr->Sid, usr->UDN );

//    if( tv_upnp_store_subs )
//        return UPNP_E_BAD_REQUEST;


    printf( "\n" );

    return UPNP_E_SUCCESS;
}

int tv_upnp_control_action( struct Upnp_Action_Request* uar )
{
    if( (uar==NULL) || (uar->ActionName==NULL) )
        return UPNP_E_INVALID_ACTION;

    if( !strcasecmp( uar->ActionName, "Browse" ))
    {
        return upnp_act_browse(uar);
    }
    else
    if( !strcasecmp( uar->ActionName, "GetSearchCapabilities" ))
    {
        return upnp_act_get_search_caps(uar);
    }
    else
    if( !strcasecmp( uar->ActionName, "GetSortCapabilities" ))
    {
        return upnp_act_get_sort_caps(uar);
    }
    else
    if( !strcasecmp( uar->ActionName, "GetSystemUpdateID" ))
    {
        return upnp_act_get_sys_upd_ID(uar);
    }

    printf( "control action '%s' not supported!\n", uar->ActionName );
    return UPNP_E_INVALID_ACTION;

}

int upnp_act_browse( struct Upnp_Action_Request* uar )
{
    int node_id = 0;
    int children = 0;
    int nd_start = 0;
    int nr_count = 0;
    int nr_ttl = 0;

    tvstring_t* obj_id_str = tv_upnp_xml_nd1_val( uar->ActionRequest, "ObjectID" );
    tvstring_t* br_flag_str = tv_upnp_xml_nd1_val( uar->ActionRequest, "BrowseFlag" );
    tvstring_t* filter_str = tv_upnp_xml_nd1_val( uar->ActionRequest, "Filter" );
    tvstring_t* sidx_str = tv_upnp_xml_nd1_val( uar->ActionRequest, "StartingIndex" );
    tvstring_t* cnt_str = tv_upnp_xml_nd1_val( uar->ActionRequest, "RequestedCount" );
    tvstring_t* sort_str = tv_upnp_xml_nd1_val( uar->ActionRequest, "SortCriteria" );

    node_id = tvs_atoi(obj_id_str);
    nd_start = tvs_atoi(sidx_str);
    nr_count = tvs_atoi(cnt_str);

    if( 0 == tvs_casecmp_chr( br_flag_str, "BrowseDirectChildren" ))
        children = 1;

    printf( "upnp_control_action - ActionName %s, DevUDN %s, ErrCode %d, ErrStr %s, ServiceID %s, Socket %d\n", uar->ActionName, uar->DevUDN, uar->ErrCode, uar->ErrStr, uar->ServiceID, uar->Socket );
    printf( "upnp_control_action - Filter %s, \n", tvs_cstr( filter_str ) );

    // TODO del
    DOMString ds = ixmlPrintDocument(uar->ActionRequest);
    if( ds != NULL ) {
        printf( "%s\n", ds );
        ixmlFreeDOMString( ds );
    }

    tvstring_t* res = tvs_newp(
            "<u:BrowseResponse xmlns:u=\"urn:schemas-upnp-org:service:ContentDirectory:1\">"
            "<Result>" );

    tvstring_t* didl = tvs_newp(
            "<DIDL-Lite xmlns=\"urn:schemas-upnp-org:metadata-1-0/DIDL-Lite/\" xmlns:dc=\"http:/"
            "/purl.org/dc/elements/1.1/\" xmlns:upnp=\"urn:schemas-upnp-org:metadata-1-0/upnp/\">" );
    tv_upnp_browse_tv_tree_node( didl, node_id, children, nd_start, &nr_count, &nr_ttl );
    tvs_cat( didl,
            "</DIDL-Lite>" );
    tv_upnp_specchar2entity( didl, res );

    tvs_cat( res,
            "</Result> "
            "<NumberReturned>" );
    tvs_cat_int( res, nr_count );
    tvs_cat( res,
            "</NumberReturned> "
            "<TotalMatches>" );
    tvs_cat_int( res, nr_ttl );
    tvs_cat( res,
            "</TotalMatches> "
            "<UpdateID>0</UpdateID> "
            "</u:BrowseResponse>" );

    ixmlParseBufferEx( res->c_str,&(uar->ActionResult));

    tvs_del( &res );
    tvs_del( &didl );

    tvs_del( &obj_id_str );
    tvs_del( &br_flag_str );
    tvs_del( &filter_str );
    tvs_del( &sidx_str );
    tvs_del( &cnt_str );
    tvs_del( &sort_str );

    return UPNP_E_SUCCESS;
}

int upnp_act_get_search_caps( struct Upnp_Action_Request* uar )
{
    return UPNP_E_SUCCESS;
}

int upnp_act_get_sort_caps( struct Upnp_Action_Request* uar )
{
    return UPNP_E_SUCCESS;
}

int upnp_act_get_sys_upd_ID( struct Upnp_Action_Request* uar )
{
    return UPNP_E_SUCCESS;
}
