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

#ifndef TVSTRING_H_
#define TVSTRING_H_

#include <stddef.h>


typedef struct tvstring {
    char* c_str;
    size_t size;
}
tvstring_t;

void tvs_ctor( tvstring_t* tvs, const char* init );
tvstring_t* tvs_newp( const char* init );
void tvs_new( tvstring_t** tvs, const char* init );
void tvs_del( tvstring_t** tvs );

void tvs_assign( tvstring_t* tvs, const char* init );

void tvs_append( tvstring_t* tvs, const tvstring_t* app );
void tvs_cat( tvstring_t* tvs, const char* app );
void tvs_cat_int( tvstring_t* tvs, int app );

const char* tvs_cstr( const tvstring_t* tvs );
int tvs_atoi( const tvstring_t* tvs );

int tvs_casecmp( const tvstring_t* lv, const tvstring_t* rv );
int tvs_casecmp_chr( const tvstring_t* lv, const char* rv );

size_t tvs_size( const tvstring_t* tvs );

#endif /* TVSTRING_H_ */
