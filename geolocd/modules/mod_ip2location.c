/*	$NetBSD: $ */

/*
 *
 * Copyright (c) 2014, 2015 David Carlier <devnexen@gmail.com>
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

#include <IP2Location.h>

#include "mod_ip2location.h"
#include <string.h>

void *
ip2location_init_callback(const char *datafile)
{
    IP2Location *il = NULL;
    char *datafile_ = strdup(datafile);
    if (datafile == NULL)
        return (NULL);
    il = IP2Location_open(datafile_);
    free(datafile_);
    if (il != NULL) {
        if (IP2Location_open_mem(il, IP2LOCATION_CACHE_MEMORY) != 0) {
            IP2Location_close(il);
            return (NULL);
        }
    }

    return ((void *)il);
}

void *
ip2location_lookup_init_callback(void *ptr, const char *addr,
                                 enum lookup_info_type lit, const char **info)
{
    IP2Location *il = (IP2Location *)ptr;
    IP2LocationRecord *rec = NULL;
    char *addr_ = NULL;

    if (il != NULL && addr != NULL && info != NULL) {
        addr_ = strdup(addr);
        if (addr_ == NULL)
            return (NULL);
        if ((rec = IP2Location_get_all(il, addr_)) != NULL) {
            switch(lit) {
            case GEOLOC_COUNTRY:
                *info = rec->country_short;
                break;
            case GEOLOC_ISP:
                *info = rec->isp;
                break;
            case GEOLOC_MNC:
                *info = rec->mnc;
                break;
            case GEOLOC_MCC:
                *info = rec->mcc;
                break;
            default:
                log_warn("wrong lookup info type");
            }

        }

        free(addr_);
    }

    return (rec);
}

void
ip2location_lookup_cleanup_callback(void *arg, void *ptr)
{
    IP2LocationRecord *rec = (IP2LocationRecord *)ptr;
    if (rec != NULL)
        IP2Location_free_record(rec);
}

void
ip2location_shutdown_callback(void *ptr)
{
    IP2Location *il = (IP2Location *)ptr;
    if (il != NULL) {
        IP2Location_close(il);
        il = NULL;
    }
}

struct backend ip2location_backend = {
    .name       = "ip2location",
    .gl_bic     = ip2location_init_callback,
    .gl_blic    = ip2location_lookup_init_callback,
    .gl_blcc    = ip2location_lookup_cleanup_callback,
    .gl_bsc     = ip2location_shutdown_callback,
    .ipv6capable= 1
};

