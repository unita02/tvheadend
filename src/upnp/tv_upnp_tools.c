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

#include "tv_upnp_tools.h"

tvstring_t* tv_upnp_xml_nd1_val( IXML_Document* doc, const char* name )
{
    tvstring_t* ret = NULL;
    IXML_NodeList* list = NULL;
    IXML_Node* node = NULL;

    if( NULL != (list = ixmlDocument_getElementsByTagName( doc, name ))) {
        if( ixmlNodeList_length(list))
            node = ixmlNodeList_item(list, 0);
        if( node != NULL ) {
            IXML_Node* child = ixmlNode_getFirstChild(node);
            if (child != 0 && ixmlNode_getNodeType(child) == eTEXT_NODE)
                ret = tvs_newp( ixmlNode_getNodeValue(child) );
        }
        ixmlNodeList_free(list);
    }
    return ret;
}

void tv_upnp_specchar2entity( const tvstring_t* Data, tvstring_t* NewData )
{
    char bf[2] = {0};
    unsigned int i = 0;

    if( Data==NULL ) return;
    if( NewData==NULL ) return;

    for( i = 0; i < tvs_size( Data ); ++i ) {
        bf[0] = Data->c_str[i];
        switch( (int)(bf[0]) ) {
            case L'€': tvs_cat( NewData, "&euro;" ); break;
            case L'"': tvs_cat( NewData, "&quot;" ); break;
            case L'&': tvs_cat( NewData, "&amp;" ); break;
            case L'<': tvs_cat( NewData, "&lt;" ); break;
            case L'>': tvs_cat( NewData, "&gt;" ); break;
            case L'¡': tvs_cat( NewData, "&iexcl;" ); break;
            case L'¢': tvs_cat( NewData, "&cent;" ); break;
            case L'£': tvs_cat( NewData, "&pound;" ); break;
            case L'¤': tvs_cat( NewData, "&curren;" ); break;
            case L'¥': tvs_cat( NewData, "&yen;" ); break;
            case L'¦': tvs_cat( NewData, "&brvbar;" ); break;
            case L'§': tvs_cat( NewData, "&sect;" ); break;
            case L'¨': tvs_cat( NewData, "&uml;" ); break;
            case L'©': tvs_cat( NewData, "&copy;" ); break;
            case L'ª': tvs_cat( NewData, "&ordf;" ); break;
            case L'¬': tvs_cat( NewData, "&not;" ); break;
            case L'­': tvs_cat( NewData, "&shy;" ); break;
            case L'®': tvs_cat( NewData, "&reg;" ); break;
            case L'¯': tvs_cat( NewData, "&macr;" ); break;
            case L'°': tvs_cat( NewData, "&deg;" ); break;
            case L'±': tvs_cat( NewData, "&plusmn;" ); break;
            case L'²': tvs_cat( NewData, "&sup2;" ); break;
            case L'³': tvs_cat( NewData, "&sup3;" ); break;
            case L'´': tvs_cat( NewData, "&acute;" ); break;
            case L'µ': tvs_cat( NewData, "&micro;" ); break;
            case L'¶': tvs_cat( NewData, "&para;" ); break;
            case L'·': tvs_cat( NewData, "&middot;" ); break;
            case L'¸': tvs_cat( NewData, "&cedil;" ); break;
            case L'¹': tvs_cat( NewData, "&sup1;" ); break;
            case L'º': tvs_cat( NewData, "&ordm;" ); break;
            case L'»': tvs_cat( NewData, "&raquo;" ); break;
            case L'«': tvs_cat( NewData, "&laquo;" ); break;
            case L'¼': tvs_cat( NewData, "&frac14;" ); break;
            case L'½': tvs_cat( NewData, "&frac12;" ); break;
            case L'¾': tvs_cat( NewData, "&frac34;" ); break;
            case L'¿': tvs_cat( NewData, "&iquest;" ); break;
            case L'À': tvs_cat( NewData, "&Agrave;" ); break;
            case L'Á': tvs_cat( NewData, "&Aacute;" ); break;
            case L'Â': tvs_cat( NewData, "&Acirc;" ); break;
            case L'Ã': tvs_cat( NewData, "&Atilde;" ); break;
            case L'Ä': tvs_cat( NewData, "&Auml;" ); break;
            case L'Å': tvs_cat( NewData, "&Aring;" ); break;
            case L'Æ': tvs_cat( NewData, "&AElig;" ); break;
            case L'Ç': tvs_cat( NewData, "&Ccedil;" ); break;
            case L'È': tvs_cat( NewData, "&Egrave;" ); break;
            case L'É': tvs_cat( NewData, "&Eacute;" ); break;
            case L'Ê': tvs_cat( NewData, "&Ecirc;" ); break;
            case L'Ë': tvs_cat( NewData, "&Euml;" ); break;
            case L'Ì': tvs_cat( NewData, "&Igrave;" ); break;
            case L'Í': tvs_cat( NewData, "&Iacute;" ); break;
            case L'Î': tvs_cat( NewData, "&Icirc;" ); break;
            case L'Ï': tvs_cat( NewData, "&Iuml;" ); break;
            case L'Ð': tvs_cat( NewData, "&ETH;" ); break;
            case L'Ñ': tvs_cat( NewData, "&Ntilde;" ); break;
            case L'Ò': tvs_cat( NewData, "&Ograve;" ); break;
            case L'Ó': tvs_cat( NewData, "&Oacute;" ); break;
            case L'Ô': tvs_cat( NewData, "&Ocirc;" ); break;
            case L'Õ': tvs_cat( NewData, "&Otilde;" ); break;
            case L'Ö': tvs_cat( NewData, "&Ouml;" ); break;
            case L'×': tvs_cat( NewData, "&times;" ); break;
            case L'Ø': tvs_cat( NewData, "&Oslash;" ); break;
            case L'Ù': tvs_cat( NewData, "&Ugrave;" ); break;
            case L'Ú': tvs_cat( NewData, "&Uacute;" ); break;
            case L'Û': tvs_cat( NewData, "&Ucirc;" ); break;
            case L'Ü': tvs_cat( NewData, "&Uuml;" ); break;
            case L'Ý': tvs_cat( NewData, "&Yacute;" ); break;
            case L'Þ': tvs_cat( NewData, "&THORN;" ); break;
            case L'ß': tvs_cat( NewData, "&szlig;" ); break;
            case L'à': tvs_cat( NewData, "&agrave;" ); break;
            case L'á': tvs_cat( NewData, "&aacute;" ); break;
            case L'â': tvs_cat( NewData, "&acirc;" ); break;
            case L'ã': tvs_cat( NewData, "&atilde;" ); break;
            case L'ä': tvs_cat( NewData, "&auml;" ); break;
            case L'å': tvs_cat( NewData, "&aring;" ); break;
            case L'æ': tvs_cat( NewData, "&aelig;" ); break;
            case L'ç': tvs_cat( NewData, "&ccedil;" ); break;
            case L'è': tvs_cat( NewData, "&egrave;" ); break;
            case L'é': tvs_cat( NewData, "&eacute;" ); break;
            case L'ê': tvs_cat( NewData, "&ecirc;" ); break;
            case L'ë': tvs_cat( NewData, "&euml;" ); break;
            case L'ì': tvs_cat( NewData, "&igrave;" ); break;
            case L'í': tvs_cat( NewData, "&iacute;" ); break;
            case L'î': tvs_cat( NewData, "&icirc;" ); break;
            case L'ï': tvs_cat( NewData, "&iuml;" ); break;
            case L'ð': tvs_cat( NewData, "&eth;" ); break;
            case L'ñ': tvs_cat( NewData, "&ntilde;" ); break;
            case L'ò': tvs_cat( NewData, "&ograve;" ); break;
            case L'ó': tvs_cat( NewData, "&oacute;" ); break;
            case L'ô': tvs_cat( NewData, "&ocirc;" ); break;
            case L'õ': tvs_cat( NewData, "&otilde;" ); break;
            case L'ö': tvs_cat( NewData, "&ouml;" ); break;
            case L'÷': tvs_cat( NewData, "&divide;" ); break;
            case L'ø': tvs_cat( NewData, "&oslash;" ); break;
            case L'ù': tvs_cat( NewData, "&ugrave;" ); break;
            case L'ú': tvs_cat( NewData, "&uacute;" ); break;
            case L'û': tvs_cat( NewData, "&ucirc;" ); break;
            case L'ü': tvs_cat( NewData, "&uuml;" ); break;
            case L'ý': tvs_cat( NewData, "&yacute;" ); break;
            case L'þ': tvs_cat( NewData, "&thorn;" ); break;
            default:
                tvs_cat( NewData, bf ); break;
        }
    }
}
