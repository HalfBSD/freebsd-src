/* Test-only socket interposition. Public upstreams are redirected to loopback;
 * every other non-loopback destination aborts the test. No production switches. */
#define _GNU_SOURCE
#include <arpa/inet.h>
#include <dlfcn.h>
#include <errno.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/socket.h>
#include <sys/time.h>
#include <time.h>

#ifdef __FreeBSD__
typedef struct timezone test_timezone;
#else
typedef void test_timezone;
#endif

static const char* ips[] = {"1.1.1.1", "1.0.0.1", "8.8.8.8", "8.8.4.4",
    "208.67.222.222", "208.67.220.220"};

static void ports(unsigned p[6], long* offset)
{
    FILE* f = fopen(getenv("UNBOUND_TEST_MAP"), "r");
    if(!f || fscanf(f, "%u %u %u %u %u %u %ld", p, p+1, p+2, p+3,
        p+4, p+5, offset) != 7) abort();
    fclose(f);
}

static int translate(const struct sockaddr* a, socklen_t len,
    struct sockaddr_storage* copy)
{
    unsigned p[6], i;
    long offset;
    struct in_addr ip;
    struct sockaddr_in* b = (struct sockaddr_in*)copy;
    memcpy(copy, a, len);
    if(a->sa_family != AF_INET) return 0;
    if((ntohl(b->sin_addr.s_addr) >> 24) == 127) return 0;
    ports(p, &offset);
    for(i=0; i<6; i++) {
        inet_pton(AF_INET, ips[i], &ip);
        if(ip.s_addr == b->sin_addr.s_addr) {
            if(!p[i]) break;
            b->sin_addr.s_addr = htonl(INADDR_LOOPBACK);
            b->sin_port = htons(p[i]);
            return 0;
        }
    }
    if(i == 6) abort(); /* Any attempted recursive destination is a test failure. */
    errno = ENETUNREACH;
    return -1;
}

static void restore(struct sockaddr* a)
{
    unsigned p[6], i;
    long offset;
    struct sockaddr_in* b = (struct sockaddr_in*)a;
    if(!a || a->sa_family != AF_INET ||
        b->sin_addr.s_addr != htonl(INADDR_LOOPBACK)) return;
    ports(p, &offset);
    for(i=0; i<6; i++) if(p[i] && ntohs(b->sin_port) == p[i]) {
        inet_pton(AF_INET, ips[i], &b->sin_addr);
        b->sin_port = htons(53);
        return;
    }
}

int connect(int fd, const struct sockaddr* a, socklen_t len)
{
    struct sockaddr_storage copy;
    int (*real)(int,const struct sockaddr*,socklen_t) = dlsym(RTLD_NEXT,"connect");
    if(translate(a,len,&copy)) return -1;
    return real(fd,(struct sockaddr*)&copy,len);
}

ssize_t sendto(int fd,const void* data,size_t n,int flags,
    const struct sockaddr* a,socklen_t len)
{
    struct sockaddr_storage copy;
    ssize_t (*real)(int,const void*,size_t,int,const struct sockaddr*,socklen_t) =
        dlsym(RTLD_NEXT,"sendto");
    if(a && translate(a,len,&copy)) return -1;
    return real(fd,data,n,flags,a?(struct sockaddr*)&copy:NULL,len);
}

ssize_t recvfrom(int fd,void* data,size_t n,int flags,
    struct sockaddr* a,socklen_t* len)
{
    ssize_t (*real)(int,void*,size_t,int,struct sockaddr*,socklen_t*) =
        dlsym(RTLD_NEXT,"recvfrom");
    ssize_t r = real(fd,data,n,flags,a,len);
    if(r >= 0) restore(a);
    return r;
}

ssize_t sendmsg(int fd,const struct msghdr* msg,int flags)
{
    struct msghdr m = *msg;
    struct sockaddr_storage copy;
    ssize_t (*real)(int,const struct msghdr*,int) = dlsym(RTLD_NEXT,"sendmsg");
    if(m.msg_name) {
        if(translate(m.msg_name,m.msg_namelen,&copy)) return -1;
        m.msg_name = &copy;
    }
    return real(fd,&m,flags);
}

ssize_t recvmsg(int fd,struct msghdr* msg,int flags)
{
    ssize_t (*real)(int,struct msghdr*,int) = dlsym(RTLD_NEXT,"recvmsg");
    ssize_t r = real(fd,msg,flags);
    if(r >= 0) restore(msg->msg_name);
    return r;
}

int gettimeofday(struct timeval* tv, test_timezone* tz)
{
    unsigned p[6];
    long offset;
    int (*real)(struct timeval*,test_timezone*) = dlsym(RTLD_NEXT,"gettimeofday");
    int r = real(tv,tz);
    ports(p,&offset);
    if(!r) tv->tv_sec += offset;
    return r;
}

time_t time(time_t* out)
{
    struct timeval tv;
    gettimeofday(&tv,NULL);
    if(out) *out = tv.tv_sec;
    return tv.tv_sec;
}
