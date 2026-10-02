/*-
 * SPDX-License-Identifier: BSD-2-Clause
 */

/* TCP echo and fixed-response services for firewall tests. */
#include <sys/types.h>
#include <sys/socket.h>

#include <netinet/in.h>

#include <err.h>
#include <errno.h>
#include <netdb.h>
#include <signal.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

static int
send_all(int fd, const char *buf, size_t len)
{
	ssize_t n;

	while (len != 0) {
		n = send(fd, buf, len, 0);
		if (n < 0 && errno == EINTR)
			continue;
		if (n <= 0)
			return (-1);
		buf += n;
		len -= n;
	}
	return (0);
}

static void
serve(int fd, const char *reply)
{
	char buf[65536];
	ssize_t n;

	if (reply != NULL) {
		if (send_all(fd, reply, strlen(reply)) == 0)
			(void)send_all(fd, "\n", 1);
		return;
	}
	for (;;) {
		n = recv(fd, buf, sizeof(buf), 0);
		if (n < 0 && errno == EINTR)
			continue;
		if (n <= 0 || send_all(fd, buf, n) != 0)
			return;
	}
}

int
main(int argc, char **argv)
{
	struct addrinfo hints, *addr;
	FILE *ready;
	pid_t pid;
	int fd, client, error, one;

	if ((argc != 4 && argc != 5) ||
	    (strcmp(argv[1], "-4") != 0 && strcmp(argv[1], "-6") != 0))
		errx(1, "usage: tcp_server -4|-6 port readyfile [reply]");

	memset(&hints, 0, sizeof(hints));
	hints.ai_family = strcmp(argv[1], "-4") == 0 ? AF_INET : AF_INET6;
	hints.ai_socktype = SOCK_STREAM;
	hints.ai_flags = AI_PASSIVE | AI_NUMERICSERV;
	error = getaddrinfo(NULL, argv[2], &hints, &addr);
	if (error != 0)
		errx(1, "getaddrinfo: %s", gai_strerror(error));
	fd = socket(addr->ai_family, addr->ai_socktype, addr->ai_protocol);
	if (fd == -1)
		err(1, "socket");
	one = 1;
	if (setsockopt(fd, SOL_SOCKET, SO_REUSEADDR, &one, sizeof(one)) == -1)
		err(1, "SO_REUSEADDR");
	if (addr->ai_family == AF_INET6 &&
	    setsockopt(fd, IPPROTO_IPV6, IPV6_V6ONLY, &one, sizeof(one)) == -1)
		err(1, "IPV6_V6ONLY");
	if (bind(fd, addr->ai_addr, addr->ai_addrlen) == -1)
		err(1, "bind");
	freeaddrinfo(addr);
	if (listen(fd, SOMAXCONN) == -1)
		err(1, "listen");
	(void)signal(SIGCHLD, SIG_IGN);
	(void)signal(SIGPIPE, SIG_IGN);

	/* Publish readiness only after the listener has been established. */
	ready = fopen(argv[3], "w");
	if (ready == NULL)
		err(1, "readyfile");
	if (fclose(ready) == EOF)
		err(1, "readyfile");

	for (;;) {
		client = accept(fd, NULL, NULL);
		if (client == -1) {
			if (errno == EINTR)
				continue;
			err(1, "accept");
		}
		pid = fork();
		if (pid == -1)
			err(1, "fork");
		if (pid == 0) {
			(void)close(fd);
			serve(client, argc == 5 ? argv[4] : NULL);
			(void)close(client);
			_exit(0);
		}
		(void)close(client);
	}
}
