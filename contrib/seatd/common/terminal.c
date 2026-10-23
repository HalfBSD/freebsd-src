#include <assert.h>
#include <errno.h>
#include <fcntl.h>
#include <signal.h>
#include <stdbool.h>
#include <stdio.h>
#include <string.h>
#include <sys/ioctl.h>
#include <unistd.h>

#if defined(__linux__)
#include <linux/kd.h>
#define K_ENABLE  K_UNICODE
#define K_DISABLE K_OFF
#elif defined(__FreeBSD__)
#include <sys/consio.h>
#include <sys/kbio.h>
#include <termios.h>
#define K_ENABLE  K_XLATE
#define K_DISABLE K_RAW
#elif defined(__NetBSD__)
#include <dev/wscons/wsdisplay_usl_io.h>
#define K_ENABLE  K_XLATE
#define K_DISABLE K_RAW
#else
#error Unsupported platform
#endif

#include "log.h"
#include "terminal.h"

/* HalfBSD has one local console. No virtual-terminal selection or signals. */
int terminal_open(void) {
#if defined(__FreeBSD__)
	const char *path = "/dev/ttyv0";
#elif defined(__linux__)
	const char *path = "/dev/tty0";
#elif defined(__NetBSD__)
	const char *path = "/dev/ttyE0";
#else
#error Unsupported platform
#endif
	int fd = open(path, O_RDWR | O_NOCTTY | O_CLOEXEC);
	if (fd == -1) {
		log_errorf("Could not open console: %s", strerror(errno));
	}
	return fd;
}

int terminal_set_keyboard(int fd, bool enable) {
	log_debugf("Setting KD keyboard state to %d", enable);
	if (ioctl(fd, KDSKBMODE, enable ? K_ENABLE : K_DISABLE) == -1) {
		log_errorf("Could not set KD keyboard mode to %s: %s",
			   enable ? "enabled" : "disabled", strerror(errno));
		return -1;
	}
#if defined(__FreeBSD__)
	struct termios tios;
	if (tcgetattr(fd, &tios) == -1) {
		log_errorf("Could not set get terminal mode: %s", strerror(errno));
		return -1;
	}
	if (enable) {
		cfmakesane(&tios);
	} else {
		cfmakeraw(&tios);
	}
	if (tcsetattr(fd, TCSAFLUSH, &tios) == -1) {
		log_errorf("Could not set terminal mode to %s: %s", enable ? "sane" : "raw",
			   strerror(errno));
		return -1;
	}
#endif
	return 0;
}

int terminal_set_graphics(int fd, bool enable) {
	log_debugf("Setting KD graphics state to %d", enable);
	if (ioctl(fd, KDSETMODE, enable ? KD_GRAPHICS : KD_TEXT) == -1) {
		log_errorf("Could not set KD graphics mode to %s: %s", enable ? "graphics" : "text",
			   strerror(errno));
		return -1;
	}
	return 0;
}
