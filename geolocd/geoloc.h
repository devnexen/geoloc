/* $NetBSD: $ */

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
 * WHATSOEVER RESULTING FROM LOSS OF USE, DATA OR PROFITS, WHETHER IN AN
 * ACTION OF CONTRACT, NEGLIGENCE OR OTHER TORTIOUS ACTION, ARISING OUT OF
 * OR IN CONNECTION WITH THE USE OR PERFORMANCE OF THIS SOFTWARE.
 */

#ifndef _GEOLOC_H_
#define _GEOLOC_H_          1

#include <sys/queue.h>
#include <sys/socket.h>
#include <net/if.h>
#include <netinet/in.h>

#include <stdarg.h>
#include <string.h>

#define GEOLOCD_SOCKET      "/var/run/geolocd.sock"
#define CONF_FILE           "/etc/geolocd.conf"
#define GEOLOCD_USER        "_geolocd"

enum msg_type {
    MSG_CTL_NONE               = 0,
    MSG_CTL_RELOAD             = 1,
    MSG_CTL_SHUTDOWN           = 2,
    MSG_CTL_BACKEND_INFO       = 3,
    MSG_CTL_PROPERTY           = 4,
    MSG_CTL_PROPERTY_KEY       = 5
};

enum msg_field {
    MSG_NONE                   = 0,
    /* Infos from backend set */
    MSG_BACKEND_NAME           = 1,
    MSG_BACKEND_DATAFILE       = 2,
    MSG_BACKEND_IPV6CAPABLE    = 3,
    /* Lookup properties */
    MSG_PROPERTY_CCODE         = 4,
    MSG_PROPERTY_ISP           = 5,
    MSG_PROPERTY_MNC           = 6,
    MSG_PROPERTY_MCC           = 7
};

enum lookup_info_type {
    GEOLOC_COUNTRY,
    GEOLOC_ISP,
    GEOLOC_MNC,
    GEOLOC_MCC
};

struct msg_ctl_req {
    enum msg_type       type;
    enum msg_field      field;
};

typedef void *(*backend_init_callback)(const char *);
typedef void *(*backend_lookup_init_callback)(void *, const char *, enum lookup_info_type, const char **);
typedef void (*backend_lookup_cleanup_callback)(void *, void *);
typedef void (*backend_shutdown_callback)(void *);

static TAILQ_HEAD(backends, backend) backends = TAILQ_HEAD_INITIALIZER(backends);

struct backend {
	TAILQ_ENTRY(backend)			entry;
    const char                      *name;
    const char                      *datafile;
    void                            *handler;
    
    backend_init_callback           gl_bic;
    backend_lookup_init_callback    gl_blic;
    backend_lookup_cleanup_callback gl_blcc;
    backend_shutdown_callback       gl_bsc;

    unsigned                        ipv6capable:1;
};

struct geolocd_conf {
    char                      *backend;
    char                      *datafile;
};

void usage(void);
struct geolocd_conf *parse_config(const char *);
void clear_config(struct geolocd_conf *);

/* log.c */
void log_init(int);
void log_warn(const char *, ...);
void log_warnx(const char *, ...);
void log_info(const char *, ...);
void log_debug(const char *, ...);
void vlog(int, const char *, va_list);
void fatal(const char *);
void fatalx(const char *);

#endif
