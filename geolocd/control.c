/*	$NetBSD: $ */

/*
 * Copyright (c) 2014, 2015 David Carlier <devnexen@gmail.com>
 * Copyright (c) 2003, 2004 Henning Brauer <henning@openbsd.org>
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

#include <sys/types.h>
#include <sys/stat.h>
#include <sys/socket.h>
#include <sys/un.h>
#ifdef  HAVE_NO_BSDFUNCS
#include <bsd/string.h>
#endif
#include <errno.h>
#include <fcntl.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <unistd.h>
#include <err.h>

#include "control.h"

int
control_init(void)
{
    struct sockaddr_un  sun;
    int                 fd;
    mode_t              old_umask;

    if ((fd = socket(AF_UNIX, SOCK_STREAM, 0)) == -1) {
        log_warn("control_init: socket");
        return (-1);
    }

    bzero(&sun, sizeof(sun));
    sun.sun_family = AF_UNIX;
    strlcpy(sun.sun_path, GEOLOCD_SOCKET, sizeof(sun.sun_path));

    if (unlink(GEOLOCD_SOCKET) == -1)
        if (errno != ENOENT) {
            log_warn("control_init: unlink %s", GEOLOCD_SOCKET);
            close(fd);
            return (-1);
        }

    old_umask = umask(S_IXUSR|S_IXGRP|S_IWOTH|S_IROTH|S_IXOTH);
    if (bind(fd, (struct sockaddr *)&sun, sizeof(sun)) == -1) {
        log_warn("control_init: bind: %s", GEOLOCD_SOCKET);
        close(fd);
        umask(old_umask);
        return (-1);
    }
    umask(old_umask);

    if (chmod(GEOLOCD_SOCKET, S_IRUSR|S_IWUSR|S_IRGRP|S_IWGRP) == -1) {
        log_warn("control_init: chmod");
        close(fd);
        (void)unlink(GEOLOCD_SOCKET);
        return (-1);
    }

    session_socket_blockmode(fd, BM_NONBLOCK);

    return (fd);
}

int
control_listen(int fd)
{
    if (fd != -1 && listen(fd, CONTROL_BACKLOG)) {
        log_warn("control_listen: listen");
        return (-1);
    }

    return (0);
}



void
control_shutdown(int fd)
{
    close(fd);
}

void
control_cleanup(void)
{
    unlink(GEOLOCD_SOCKET);
}

int
control_accept(int fd)
{
    int                 connfd;
    socklen_t           len;
    struct sockaddr_un  sun;

    len = sizeof(sun);
    if ((connfd = accept(fd, (struct sockaddr *)&sun, &len)) != -1)
        session_socket_blockmode(connfd, BM_NONBLOCK);

    return (connfd);
}

int
control_close(int fd)
{
    close(fd);
    return (1);
}

void
session_socket_blockmode(int fd, enum blockmodes bm)
{
    int flags;

    if ((flags = fcntl(fd, F_GETFL, 0)) == -1)
        fatal("cannot get fnctl flags");

    if (bm == BM_NONBLOCK)
        flags |= O_NONBLOCK;
    else
        flags &= ~O_NONBLOCK;

    if ((flags = fcntl(fd, F_SETFL, flags)) == -1)
        fatal("cannot set fnctl flags");
}
