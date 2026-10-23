/* SPDX-License-Identifier: BSD-2-Clause */
/* Run explicitly after installing the new kernel; no graphics mode changes. */
#include <sys/types.h>
#include <sys/ioctl.h>
#include <sys/sysctl.h>
#include <sys/consio.h>
#include <sys/stat.h>
#include <assert.h>
#include <errno.h>
#include <fcntl.h>
#include <stdio.h>
#include <unistd.h>
int main(void) {
    int count=0; size_t length=sizeof(count);
    if(sysctlbyname("kern.vt.console_count", &count, &length, NULL, 0)==-1 || count!=1) {
        fputs("SKIP: new single-console kernel marker is absent\n", stderr); return 77;
    }
    int fd=open("/dev/ttyv0", O_RDONLY|O_NOCTTY);
    if(fd==-1) { perror("/dev/ttyv0"); return 1; }
    int active=0;
    assert(ioctl(fd, VT_GETACTIVE, &active)==0 && active==1);
    const char digits[]="0123456789abcdefghijklmnopqrstuv";
    for(size_t i=1;i<sizeof(digits)-1;i++) {
        char path[32]; struct stat st;
        snprintf(path,sizeof(path),"/dev/ttyv%c",digits[i]);
        assert(lstat(path,&st)==-1 && errno==ENOENT);
    }
    int target=2;
    assert(ioctl(fd,VT_ACTIVATE,target)==-1 && errno==ENOTTY);
    assert(ioctl(fd,VT_WAITACTIVE,target)==-1 && errno==ENOTTY);
    assert(ioctl(fd,VT_OPENQRY,&target)==-1 && errno==ENOTTY);
    struct vt_mode mode={.mode=VT_PROCESS};
    assert(ioctl(fd,VT_SETMODE,&mode)==-1 && errno==ENOTTY);
    close(fd);
    puts("PASS native single-console inventory and removed switching ioctls");
    return 0;
}
