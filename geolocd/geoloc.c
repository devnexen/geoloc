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

#include <sys/types.h>
#include <sys/wait.h>
#include <sys/socket.h>
#include <sys/queue.h>
#include <sys/time.h>
#include <sys/sysctl.h>

#include <netinet/in.h>
#include <arpa/inet.h>

#include <err.h>
#include <errno.h>
#include <pwd.h>
#include <fcntl.h>
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <signal.h>
#include <unistd.h>
#include <poll.h>

#include "log.h"
#include "control.h"
#include "geoloc.h"
#include "modules.h"

void geolocd_shutdown(int);
void sighandler(int);

volatile sig_atomic_t   die = 0;
int                     ctl_fd;
struct geolocd_conf     *conf = NULL;
static struct backend   *backend = NULL;
int geoloc_msg_dispatch(int);
int geoloc_msg_backend(int, struct msg_ctl_req);
int geoloc_msg_property(int, struct msg_ctl_req, const char *);

void
usage(void)
{
    extern char *__progname;

    fprintf(stderr, "usage: %s [-dv][-f file]\n",
        __progname);
    exit(1);
}

void
sighandler(int sig)
{
    switch(sig) {
    case SIGTERM:
    case SIGINT:
        die = 1;
        break;
    default:
        fatalx("unexpected signal");
        /* NOTRECHED */
    }
}

int
main(int argc, char *argv[])
{
	int				    ch;
    int                 ndfs;
	int				    debug = 0;
	int				    verbose = 0;
	const char		    *conffile;
    struct pollfd       pfd[1];
    struct passwd       *pw = NULL;
    void                *handler = NULL;
    struct backend      *bcurrent = NULL;

	conffile = CONF_FILE;

	log_init(1);
	log_verbose(1);

	while ((ch = getopt(argc, argv, "df:v")) != -1) {
		switch (ch) {
		case 'd':
			debug = 1;
			break;
		case 'f':
			conffile = optarg;
			break;
		case 'v':
			verbose = 1;
			break;
		default:
			usage();
			/* NOTREACHED */
		}
	}

    argc -= optind;
    argv += optind;

    if ((conf = parse_config(conffile)) == NULL)
        exit(1);

    if (getuid())
        errx(1, "need root privileges");

    log_init(debug);
    log_verbose(verbose);

    if (!debug)
        daemon(1, 0);

    if ((ctl_fd = control_init()) == -1)
        fatalx("control socket init failed");
    if (control_listen(ctl_fd) == -1)
        fatalx("control socket listen failed");

    log_info("geolocd starting");

    setproctitle("geolocd");

    signal(SIGTERM, sighandler);
    signal(SIGINT, sighandler);

    init_modules();

    TAILQ_FOREACH(bcurrent, &backends, entry) {
        if (strcasecmp(conf->backend, bcurrent->name) == 0) {
            backend = bcurrent;
            break;
        }
    }

    if (backend == NULL) {
        log_warn("cannot init the backend %s, datafile %s", conf->backend, conf->datafile);
        goto shutdown;
    }

    handler = backend->gl_bic(conf->datafile);
    if (handler == NULL) {
        log_warn("backend handler alloc failure");
        goto shutdown;
    }

    if ((pw = getpwnam(GEOLOCD_USER)) == NULL) {
        log_warn("unknown user %s", GEOLOCD_USER);
        goto shutdown;
    }

    if (chroot(pw->pw_dir) == -1) {
        log_warn("chroot failed");
        goto shutdown;
    }

    if (chdir("/") == -1) {
        log_warn("chdir failed");
        goto shutdown;
    }

    if (setgroups(1, &pw->pw_gid) || 
        setegid(pw->pw_gid) == -1 ||
        seteuid(pw->pw_uid) == -1) {
        log_warn("drop privilege failed");
        goto shutdown;
    }

    backend->datafile = conf->datafile;
    backend->handler = handler;

    log_info("'%s' backend with '%s' data's file", backend->name, backend->datafile);

    bzero(pfd, sizeof(pfd));

    while (die == 0) {
        pfd[0].fd = control_accept(ctl_fd);
        pfd[0].events = POLLIN;
    
        ndfs = poll(pfd, 1, 60);

        if (ndfs && pfd[0].revents & POLLIN) {
            if (geoloc_msg_dispatch(pfd[0].fd) == -1)
                die = 1;
            close(pfd[0].fd);
        }

        if (ndfs == -1 && pfd[0].revents & (POLLERR|POLLHUP|POLLNVAL)) {
            log_warn("socket error");
            die = 1;
        }
    }

shutdown:
    if (backend != NULL)
        backend->gl_bsc(backend->handler);
    control_shutdown(ctl_fd);
    control_cleanup();

    dispose_modules();

    log_info("geolocd shutdown");

    return (0);
}

int
geoloc_msg_dispatch(int fd)
{
    struct msg_ctl_req  req;
    int                 n = -1;
    char property_key[125];
    property_key[0] = '\0';
    bzero(&req, sizeof(struct msg_ctl_req));

    if ((n = recv(fd, &req, sizeof(req), 0)) == -1) {
        log_warn("geoloc_msg_dispatch socket error");
        return (-1);
    }

    switch (req.type) {
    case MSG_CTL_BACKEND_INFO:
        return (geoloc_msg_backend(fd, req));
    case MSG_CTL_PROPERTY:
        recv(fd, property_key, sizeof(property_key), 0); 
        return (geoloc_msg_property(fd, req, property_key));
    case MSG_CTL_SHUTDOWN:
        die = 1;
        return (0);
    default:
        return (0);
    }

    return (0);
}

int 
geoloc_msg_backend(int fd, struct msg_ctl_req req)
{
    const char  *info = NULL;

    switch (req.field) {
    case MSG_BACKEND_NAME:
        info = backend->name;
        break;
    case MSG_BACKEND_DATAFILE:
        info = backend->datafile;
        break;
    case MSG_BACKEND_IPV6CAPABLE:
        info = (backend->ipv6capable ? "yes" : "no");
        break;
    default:
        info = "invalid request";
        break;
    }

    send(fd, info, strlen(info) + 1, 0);

    return (0);
}

int
geoloc_msg_property(int fd, struct msg_ctl_req req, const char *property_key)
{
    const char              *info = NULL;
    void                    *ptr = NULL;
    enum lookup_info_type   li;

    switch (req.field) {
    case MSG_PROPERTY_CCODE:
        li = GEOLOC_COUNTRY;
        break;
    case MSG_PROPERTY_ISP:
        li = GEOLOC_ISP;
        break;
    case MSG_PROPERTY_MNC:
        li = GEOLOC_MNC;
        break;
    case MSG_PROPERTY_MCC:
        li = GEOLOC_MCC;
        break;
    default:
        info = "invalid request";
        send(fd, info, strlen(info) + 1, 0);
        return (-1);
    }

    ptr = backend->gl_blic(backend->handler, property_key, li, &info);
    
    if (info == NULL)
        info = "";
    send(fd, info, strlen(info) + 1, 0);

    if (ptr != NULL)
        backend->gl_blcc(backend->handler, ptr);

    return (0);
}
