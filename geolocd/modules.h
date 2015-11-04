/*	$NetBSD:  $ */

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

#ifndef _GEOLOC_MODULES_H
#define _GEOLOC_MODULES_H		1

#ifdef GEOLOC_GEOIP
#include <modules/mod_geoip.h>
#endif
#ifdef GEOLOC_IP2LOCATION
#include <modules/mod_ip2location.h>
#endif

static inline void
init_modules(void) {
    log_info("modules loading");
    TAILQ_INIT(&backends);
#ifdef GEOLOC_GEOIP
    TAILQ_INSERT_TAIL(&backends, &geoip_backend, entry);
    log_info("geoip backend added");
#endif
#ifdef GEOLOC_IP2LOCATION
    TAILQ_INSERT_TAIL(&backends, &ip2location_backend, entry);
    log_info("ip2location backend added");
#endif
}

static inline void
dispose_modules(void)
{
	struct backend *bcurrent = NULL;
	while ((bcurrent = TAILQ_FIRST(&backends)) != NULL) {
		TAILQ_REMOVE(&backends, bcurrent, entry);
		log_info("%s backend unset", bcurrent->name);
	}
}

#endif
