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

#include <sys/socket.h>
#include <sys/un.h>

#include <err.h>
#include <pwd.h>
#include <getopt.h>
#include <errno.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <strings.h>
#include <stdio.h>

#include <geoloc.h>

struct geolocd_conf *conf = NULL;
void usage(void);

void
usage(void)
{
    extern char *__progname;

    fprintf(stderr, "usage: %s -r <backend|property> (-f <field info requested> -p <value for property lookup> -c <config file path>)\n", __progname);
    exit(1);
}

int
main(int argc, char *argv[])
{
    int c;
    int ctl_fd; 
    const char *reqarg = NULL, *fieldarg = NULL, *proparg = NULL;
    const char *conffile = CONF_FILE;
    char resdata[1024];
    struct msg_ctl_req req;
    extern char         *__progname;

    resdata[0] = '\0';
    explicit_memset(&req, 0, sizeof(req));
    req.type = MSG_CTL_NONE;
    req.field = MSG_NONE;

    while ((c = getopt(argc, argv, "r:f:p:c:")) != -1) {
        switch(c) {
        case 'r':
            reqarg = optarg;
            if (strcasecmp(reqarg, "backend") == 0) {
                req.type = MSG_CTL_BACKEND_INFO;
                if (req.field == MSG_NONE)
                    req.field = MSG_BACKEND_NAME;
            } else if (strcasecmp(reqarg, "property") == 0) {
                req.type = MSG_CTL_PROPERTY;
                if (req.field == MSG_NONE)
                    req.field = MSG_PROPERTY_CCODE;
            } else if (strcasecmp(reqarg, "shutdown") == 0) {
                req.type = MSG_CTL_SHUTDOWN;
                req.field = MSG_NONE;
            } else if (strcasecmp(reqarg, "restart") == 0) {
                req.type = MSG_CTL_RELOAD;
                req.field = MSG_NONE;
            } else {
                fprintf(stderr, "invalid request\n");
                exit(-1);
            }
            break;
        case 'f':
            fieldarg = optarg;
            if (strcasecmp(fieldarg, "name") == 0) {
                req.field = MSG_BACKEND_NAME;
                req.type = MSG_CTL_BACKEND_INFO;
            } else if (strcasecmp(fieldarg, "datafile") == 0) {
                req.field = MSG_BACKEND_DATAFILE;
                req.type = MSG_CTL_BACKEND_INFO;
            } else if (strcasecmp(fieldarg, "ipv6capable") == 0) {
                req.field = MSG_BACKEND_IPV6CAPABLE;
                req.type = MSG_CTL_BACKEND_INFO;
            } else if (strcasecmp(fieldarg, "ccode") == 0) {
                req.field = MSG_PROPERTY_CCODE;
                req.type = MSG_CTL_PROPERTY;
            } else if (strcasecmp(fieldarg, "isp") == 0) {
                req.field = MSG_PROPERTY_ISP;
                req.type = MSG_CTL_PROPERTY;
            } else if (strcasecmp(fieldarg, "mnc") == 0) {
                req.field = MSG_PROPERTY_MNC;
                req.type = MSG_CTL_PROPERTY;
            } else if (strcasecmp(fieldarg, "mcc") == 0) {
                req.field = MSG_PROPERTY_MCC;
                req.type = MSG_CTL_PROPERTY;
            } else {
                fprintf(stderr, "invalid field\n");
                exit(-1);
            }
            break;
        case 'p':
            proparg = optarg;
            break;
        case 'c':
            conffile = optarg;
            break;
        default:
            fprintf(stderr, "invalid arguments");
            exit(-1);
        } 
    }

	argc -= optind;
	argv += optind;
	if (argc > 0 || reqarg == NULL ||
        (req.type == MSG_CTL_PROPERTY && proparg == NULL))
		usage();

    if ((conf = parse_config(conffile)) == NULL)
        exit(1);

    if (geteuid())
        errx(1, "need root privileges");

    if (getpwnam(GEOLOCD_USER) == NULL)
        errx(1, "unknown user %s", GEOLOCD_USER);

    struct sockaddr_un sun;
    explicit_memset(&sun, 0, sizeof(sun)); 
    sun.sun_family = AF_UNIX;
    strlcpy(sun.sun_path, GEOLOCD_SOCKET, sizeof(sun.sun_path));

    if ((ctl_fd = socket(AF_UNIX, SOCK_STREAM, 0)) == -1) {
        fprintf(stderr, "cannot create ctl socket\n");
        exit(1);
    }

    if (connect(ctl_fd, (struct sockaddr *)&sun, sizeof(sun)) == -1) {
        fprintf(stderr, "cannot create ctl connect\n");
        goto shutdown;
    }

    printf("Config file %s used\n", conffile);

    switch (req.type) {
    case MSG_CTL_RELOAD:
        printf("Restart request\n");
        break;
    case MSG_CTL_SHUTDOWN:
        printf("Shutdown request\n");
        break;
    case MSG_CTL_BACKEND_INFO:
        printf("Backend information request\n");
        break;
    case MSG_CTL_PROPERTY:
        printf("Property lookup request\n");
        break;
    default:
        break;
    }

    switch (req.field) {
    case MSG_BACKEND_NAME:
        printf("Name\n");
        break;
    case MSG_BACKEND_DATAFILE:
        printf("Data's file\n");
        break;
    case MSG_BACKEND_IPV6CAPABLE:
        printf("IPv6 handling\n");
        break;
    case MSG_PROPERTY_CCODE:
        printf("Country code\n");
        break;
    case MSG_PROPERTY_ISP:
        printf("ISP\n");
        break;
    case MSG_PROPERTY_MCC:
        printf("MCC\n");
        break;
    case MSG_PROPERTY_MNC:
        printf("MCC\n");
        break;
    default:
        break;
    }

    if (proparg != NULL)
        printf("With property %s\n", proparg);

    send(ctl_fd, &req, sizeof(struct msg_ctl_req), 0); 
    if (proparg != NULL)
        send(ctl_fd, proparg, strlen(proparg) + 1, 0);

    recv(ctl_fd, resdata, sizeof(resdata), 0);

    printf("%s\n", resdata);

shutdown:
    close(ctl_fd); 

    return (0);
}
