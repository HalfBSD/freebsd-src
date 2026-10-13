/* SPDX-License-Identifier: BSD-2-Clause
 * Disposable UDP/TCP DNS readiness fixture. Only ephemeral IPv4 loopback.
 */
#include <sys/socket.h>
#include <netinet/in.h>
#include <errno.h>
#include <poll.h>
#include <stdio.h>
#include <string.h>
#include <unistd.h>

int
main(void)
{
	int tcp = socket(AF_INET, SOCK_STREAM | SOCK_CLOEXEC, 0);
	int udp = socket(AF_INET, SOCK_DGRAM | SOCK_CLOEXEC, 0);
	struct sockaddr_in address;
	memset(&address, 0, sizeof(address));
	address.sin_family = AF_INET;
#ifdef __FreeBSD__
	address.sin_len = sizeof(address);
#endif
	address.sin_addr.s_addr = htonl(INADDR_LOOPBACK);
	socklen_t size = sizeof(address);
	if (tcp < 0 || udp < 0 || bind(tcp, (struct sockaddr *)&address, size) < 0 ||
	    getsockname(tcp, (struct sockaddr *)&address, &size) < 0 ||
	    bind(udp, (struct sockaddr *)&address, size) < 0 || listen(tcp, 16) < 0)
		return 1;
	printf("%u\n", (unsigned int)ntohs(address.sin_port)); fflush(stdout);
	struct pollfd events[] = {{tcp, POLLIN, 0}, {udp, POLLIN, 0}};
	for (;;) {
		int result = poll(events, 2, 5000);
		if (result < 0 && errno == EINTR) continue;
		if (result <= 0) break;
		if (events[0].revents & POLLIN) {
			int fd = accept(tcp, NULL, NULL);
			if (fd >= 0) close(fd);
		}
		if (events[1].revents & POLLIN) {
			unsigned char packet[1024]; struct sockaddr_in peer;
			socklen_t length = sizeof(peer);
			ssize_t count = recvfrom(udp, packet, sizeof(packet) - 16, 0,
			    (struct sockaddr *)&peer, &length);
			if (count < 12) continue;
			packet[2] = 0x81; packet[3] = 0x80;
			packet[6] = 0; packet[7] = 1;
			static const unsigned char answer[] =
			    {0xc0, 0x0c, 0, 1, 0, 1, 0, 0, 0, 1, 0, 4, 127, 0, 0, 1};
			memcpy(packet + count, answer, sizeof(answer));
			(void)sendto(udp, packet, (size_t)count + sizeof(answer), 0,
			    (struct sockaddr *)&peer, length);
		}
	}
	close(tcp); close(udp); return 0;
}
