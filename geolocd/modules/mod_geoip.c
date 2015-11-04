/*	$NetBSD: $ */

/*
 * Copyright (c) 2014, 2015 David Carlier <devnexen@gmail.com>
 *
 * Permission to use, copy, modify, and distribute this software for any
 * purpose with or without fee is hereby granted, provided that the above
 * copyright notice and this permission notice appear in all copies.
 *
 * THE SOFTWARE IS PROVIDED "AS IS" AND THE AUTHOR DISCLAIMS ALL WARRANTIES
 * WITH REGARD TO THIS SOFTWARE INCLUDING ALL IMPLIED WARRANTIES OF
 * MERCHANTABILITY AND FITNESS. IN NO EVENT SHALL THE AUTHOR BE LIABLE FOR
 * ANY SPECIAL, DIRECT, INDIRECT, OR CONSEQUENTIAL DAMAGES OR ANY DAMAGES
 * WHATSOEVER RESULTING FROM LOSS OF MIND, USE, DATA OR PROFITS, WHETHER
 * IN AN ACTION OF CONTRACT, NEGLIGENCE OR OTHER TORTIOUS ACTION, ARISING
 * OUT OF OR IN CONNECTION WITH THE USE OR PERFORMANCE OF THIS SOFTWARE.
 */

#ifdef	GEOLOC_GEOIP
#include <GeoIP.h>

#include "mod_geoip.h"

void *
geoip_init_callback(const char *datafile)
{
    GeoIP *gi = NULL;
    gi = GeoIP_open(datafile, GEOIP_MEMORY_CACHE);

    return ((void *)gi); 
}

void *
geoip_lookup_init_callback(void *ptr, const char *addr, 
                           enum lookup_info_type lit, const char **info)
{
    GeoIP *gi = (GeoIP *)ptr;
    if (gi != NULL && addr != NULL && info != NULL) {
        switch(lit) {
        case GEOLOC_COUNTRY:
            *info = GeoIP_country_code_by_addr(gi, addr); 
            break;
        case GEOLOC_ISP:
            *info = GeoIP_org_by_addr(gi, addr);
            break;
        case GEOLOC_MNC:
        case GEOLOC_MCC:
            *info = "GeoIP does not handle this information";
            break;
        default:
            /* should not happen */
            log_warn("bad lookup info");
        }
    }

    /* GeoIP module has no cleanup callback */
    return (NULL);
}

void
geoip_shutdown_callback(void *ptr)
{
    GeoIP *gi = (GeoIP *)ptr;
    if (gi != NULL) {
        GeoIP_delete(gi);
        gi = NULL;
    }
}

struct backend geoip_backend = {
    .name       = "geoip",
    .gl_bic     = geoip_init_callback,
    .gl_blic    = geoip_lookup_init_callback,
    .gl_bsc     = geoip_shutdown_callback,
    .ipv6capable= 1
};
#endif
