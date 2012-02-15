#include "tvstring.h"

#include <string.h>
#include <stdlib.h>
#include <stdio.h>

const char def_nil[1] = {0};

void tvs_ctor( tvstring_t* tvs, const char* init )
{
    if( tvs != NULL ) {
        //if( (NULL!=init) && (init[0]!=0) ) {
        if( NULL != init ) {
            tvs->c_str = strdup(init);
            tvs->size = strlen(init);
        }
        else {
            tvs->c_str = NULL;
            tvs->size = 0;
        }
    }
}

tvstring_t* tvs_newp( const char* init )
{
    tvstring_t* tvs = (tvstring_t*)malloc(sizeof(tvstring_t));
    tvs_ctor( tvs, init );
    if( tvs != NULL ) {
        if( (NULL!=init) && (init[0]!=0) ) {
            tvs->c_str = strdup(init);
            tvs->size = strlen(init);
        }
        else {
            tvs->c_str = NULL;
            tvs->size = 0;
        }
    }
    return tvs;
}

void tvs_new( tvstring_t** atvs, const char* init ) {
    tvs_del( atvs );
    (*atvs) = tvs_newp( init );
}

void tvs_del( tvstring_t** atvs ) {
    if( atvs == NULL ) return;
    if( (*atvs) == NULL ) return;
    if( (*atvs)->c_str != NULL )
        free( (*atvs)->c_str );
    free( (*atvs) );
    (*atvs) = NULL;
}

void tvs_assign( tvstring_t* tvs, const char* init )
{
    if( (tvs != NULL) && (tvs->c_str != NULL ))
        free( tvs->c_str );
    tvs_ctor( tvs, init );
}

void tvs_append( tvstring_t* tvs, const tvstring_t* app )
{
    if( tvs == NULL ) return;
    if( app == NULL ) return;
    if( tvs_size(app) < 1 ) return;

    size_t newl = (tvs_size(tvs) + tvs_size(app) + 1);
    char* news = (char*)malloc( newl );
    if( tvs_size(tvs) > 0 ) {
        strcpy( news, tvs->c_str );
        strcat( news, app->c_str );
    }
    else {
        strcpy( news, app->c_str );
    }
    tvs_assign( tvs, news );
}

void tvs_cat( tvstring_t* tvs, const char* app )
{
    if( tvs == NULL ) return;
    if( app == NULL ) return;
    if( app[0] == 0 ) return;

    size_t newl = (tvs_size(tvs) + strlen(app) + 1);
    char* news = (char*)malloc( newl );
    if( tvs_size(tvs) > 0 ) {
        strcpy( news, tvs->c_str );
        strcat( news, app );
    }
    else {
        strcpy( news, app );
    }
    tvs_assign( tvs, news );
}

void tvs_cat_int( tvstring_t* tvs, int app )
{
    if( tvs != NULL )
    {
        char bf[sizeof(int) * 4 + 1] = {0};
        sprintf( bf, "%d", app );
        tvs_cat( tvs, bf );
    }
}

/**
 * return the string inside
 * @param tvs - string-"object"
 * @return always a valid non-null pointer to a 0-terminated string
 */
const char* tvs_cstr( const tvstring_t* tvs )
{
    if(tvs == NULL) return def_nil;
    if(tvs->c_str == NULL) return def_nil;
    return tvs->c_str;
}

int tvs_atoi( const tvstring_t* tvs )
{
    if( tvs == NULL ) return 0;
    if( tvs->c_str == NULL ) return 0;
    return atoi(tvs->c_str);
}

int tvs_casecmp( const tvstring_t* lv, const tvstring_t* rv )
{
    return tvs_casecmp_chr( lv, (rv==NULL)?NULL:rv->c_str );
}

int tvs_casecmp_chr( const tvstring_t* lv, const char* rv )
{
    int rvln = 0;
    int lvln = tvs_size(lv);
    if(rv != NULL) rvln = strlen(rv);
    if( (rvln == 0) && (lvln == 0) ) return 0;
    if( rvln == 0) return lvln;
    if( lvln == 0) return rvln;
    return strcasecmp( lv->c_str, rv );
}

size_t tvs_size( const tvstring_t* tvs )
{
    if(tvs == NULL) return 0;
    if(tvs->c_str == NULL) return 0;
    return tvs->size;
}
