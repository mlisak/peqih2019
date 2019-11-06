
/*
TODO
- Implement die. (you can use err_sys and err_quit)
- Use TCP socket instead of UDP.
- Read from named domain sockets to receive new keys.
- Implement using a single select
- exec python
*/

#include <sys/types.h>          /* basic system data types */
#include <sys/socket.h>         /* basic socket definitions */
#include <sys/time.h>           /* high resolution wall clock time */
#include <sys/wait.h>
#include <time.h>               /* nanosleep */
#include <netinet/in.h>         /* Internet defns like sockaddr_in */
#include <arpa/inet.h>          /* inet(3) functions */
#include <errno.h>
#include <fcntl.h>              /* for nonblocking */
#include <netdb.h>
#include <signal.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/stat.h>           /* for S_xxx file mode constants */
#include <unistd.h>
#include <sys/ioctl.h>          /* To configure tun interface */
#include <linux/if.h>
#include <linux/if_tun.h>
#include <linux/netlink.h>
#include <argp.h>               /* Argument parsing */

#include "error.h"

#define IFNAME_FMT "qit%d"
#define TUN_CLONE_DEV "/dev/net/tun"

#define DEFAULT_PORT 10321
#define QKD_PORT 10000

#define IP_MAXPACKET 1<<16

static uint8_t tx_buf[IP_MAXPACKET];
static uint8_t rx_buf[IP_MAXPACKET];

struct tun_dev {
    struct in_addr addr;
    struct in_addr dstaddr;
    char name[IFNAMSIZ];
    int fd;
    int mtu;
};

#define MODE_CLIENT 0
#define MODE_SERVER 1

struct tun_sock {
    struct sockaddr_in bind;
    struct sockaddr_in peer;
    int mode;
    int fd;
};


#define KEY_LENGTH 32 // AES-256

struct qkd {
    uint8_t key[KEY_LENGTH];
    int fd;
};

struct tun {
    struct tun_dev dev;
    struct tun_sock sock;
    struct qkd qkd;
};


static struct tun T;

void
tun_sock_init (struct tun_sock *sock)
{
    //memset (sock, 0, sizeof(*sock));

#if 0
    sock->bind.sin_family = AF_INET;
    sock->bind.sin_port = htons(DEFAULT_PORT);

    sock->peer.sin_family = AF_INET;
    sock->peer.sin_port = htons(DEFAULT_PORT);

    if ((sock->fd = socket(AF_INET, SOCK_DGRAM, IPPROTO_UDP)) == -1)
        err_sys("could not open UDP socket file descriptor");

    if (bind(sock->fd, (struct sockaddr *) &sock->bind, sizeof(sock->bind)))
        err_sys("could not bind to UDP socket");

    err_msg("bound to (UDP) %s:%d", inet_ntoa(sock->bind.sin_addr), ntohs(sock->bind.sin_port));
    err_msg(" peer is (UDP) %s:%d", inet_ntoa(sock->peer.sin_addr), ntohs(sock->peer.sin_port));
#endif

    sock->bind.sin_family = AF_INET;
    sock->bind.sin_port = htons(DEFAULT_PORT);

    sock->peer.sin_family AF_INET;
    sock->peer.sin_port = htons(DEFAULT_PORT);

    if (T.sock.mode == MODE_SERVER) {
        int ssock;
        if ((ssock = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP)) == -1)
            err_sys("could not create server socket");
        
        if (bind(ssock, (struct sockaddr *) &sock->bind, sizeof(sock->bind))
            err_sys("could not bind to TCP socket");

        if (listen(ssock, 1))
            err_sys("could not listen");

        err_msg("listening on (TCP) %s:%d", inet_ntoa(sock->bind.sin_addr), ntohs(sock->bind.sin_port));

        if ((sock->fd = accept(ssock, (struct sockaddr *) &sock->peer, sizeof(sock->peer))) < 0)
            err_sys("server accept failed");

        err_msg("accepted client connection");

        close(ssock);
    } 

    else {
        if ((sock->fd = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP)) == -1)
            err_sys("could not create client socket");

        if (connect(sock->fd, (struct sockaddr *) &sock->peer, sizeof(sock->peer)))
            err_sys("could not connect to server");

        err_msg("connected to server");
    }

}

void 
tun_dev_init (struct tun_dev *dev)
{

    /* NOTE(oral)
    ioctl calls made to configure the netdevice need a socket file
    descriptor. Using the tun device does not work. man (7) netdevice
    says that "... any socket's file descriptor regardless of the
    family or type" can be used. My understanding is that, the reason
    for this is mainly historic. I create a new DGRAM INET socket to be
    used for this purpose. It is closed at the end of this funciton.
    */

    int sock;

    if ((sock = socket(AF_INET, SOCK_DGRAM, 0)) == -1)
        err_sys("Could not create ioctl socket");

    if ((dev->fd = open(TUN_CLONE_DEV, O_RDWR)) < 0)
        err_sys("Could not open tun device file descriptor");

    // Create the tun device
    {
        struct ifreq ifr;
        memset (&ifr, 0, sizeof(ifr));
        // NOTE(oral) Unless IFF_NO_PI is given, IP packets read from the
        // tun device are prepended with 4 bytes of information that we
        // don't care about.
        ifr.ifr_flags = IFF_TUN | IFF_NO_PI;
        strcpy(ifr.ifr_name, IFNAME_FMT);
        if (ioctl(dev->fd, TUNSETIFF, (void *) &ifr) < 0)
            err_sys("Could not create the tun device");
        // Copy the actual interface name with the resolved suffix
        strcpy(dev->name, ifr.ifr_name);
        err_msg("created tun device %s", dev->name);
    }

    // Bring tun device up
    // TODO(oral) ip addr show qit0 still shows device as down.
    {
        struct ifreq ifr;
        memset(&ifr, 0, sizeof(ifr));
        strcpy(ifr.ifr_name, dev->name);
        ifr.ifr_flags = IFF_UP | IFF_POINTOPOINT | IFF_NOARP | IFF_RUNNING;
        if (ioctl(sock, SIOCGIFFLAGS, (void *)&ifr) < 0)
            err_sys("Could not bring interface %s up", dev->name);
        err_msg("interface %s is up", dev->name);
    }

    // Assign IP address to tun device
    {
        struct ifreq ifr;
        struct sockaddr_in addr;
        char *addr_str = inet_ntoa(dev->addr);
        memset(&ifr, 0, sizeof(ifr));
        strcpy(ifr.ifr_name, dev->name);
        addr.sin_family = AF_INET;
        addr.sin_addr = dev->addr;
        memcpy(&ifr.ifr_addr, &addr, sizeof(addr));
        if (ioctl(sock, SIOCSIFADDR, (void *)&ifr) < 0)
             err_sys("Could not assign address %s to %s", addr_str, dev->name);
        err_msg("assigned address %s to %s", addr_str, dev->name);
    }

    // Assign destination IP address
    
    {
        struct ifreq ifr;
        struct sockaddr_in addr;
        char *addr_str = inet_ntoa(dev->dstaddr);
        memset(&ifr, 0, sizeof(ifr));
        strcpy(ifr.ifr_name, dev->name);
        addr.sin_family = AF_INET;
        addr.sin_addr = dev->dstaddr;
        memcpy(&ifr.ifr_dstaddr, &addr, sizeof(addr));
        if (ioctl(sock, SIOCSIFDSTADDR, (void *)&ifr) < 0)
             err_sys("Could not assign dst address %s to %s", addr_str, dev->name);
        err_msg("assigned destination address %s to %s", addr_str, dev->name);
    }

    // Set the tun device's MTU
    {
        struct ifreq ifr;
        memset(&ifr, 0, sizeof(ifr));
        strcpy(ifr.ifr_name, dev->name);
        ifr.ifr_mtu = dev->mtu;
        if (ioctl(sock, SIOCSIFMTU, (void *)&ifr) < 0)
             err_sys("Could not set MTU of %s", dev->name);
        err_msg("set MTU of %s to %d", dev->name, dev->mtu);
    }

    close(sock);

}

void 
tun_init(struct tun *T)
{
    tun_dev_init(&T->dev);
    tun_sock_init(&T->sock);
}

void
loop()
{
    // NOTE: This should be the largest fd - the one that was opened last.
    int bfd = T.sock.fd;
    fd_set rset;
    int ret;
    size_t nread;
    for (;;) {
        FD_ZERO(&rset);
        FD_SET(bfd, &rset);
        // TODO: Implement timeout
        if (select(bfd + 1, &rset, NULL, NULL, NULL) > 0) {
            if (FD_ISSET(T.dev.fd, &rset)) {
                if ((nread = read(T.dev.fd, tx_buf, IP_MAXPACKET)) < 0)
                    err_sys("Error reading from tun device");
                if (nread == 0)
                    err_quit("tun device EOF");
            }
            if (FD_ISSET(T.sock.fd, &rset)) {
                do {
                    ret = qvpn_recv(T.sock.fd, rx_buf, &nread);
                    if (ret != 0) {
                        err_quit("Error reading from socket");
                    }
                    write(T.dev.fd, rx_buf, nread);
                } while (nread > 0);
            } 
            if (FD_ISSET(T.qkd.fd, &rset)) { // TODO
                if ((nread = read(T.qkd.fd, &T.qkd.key, KEY_LENGTH)) < 0)
                    err_sys("Error reading new key");
                qvpn_rekey(&T.qkd.key);
            }
        }
    }

}

int
getenv_int(const char* var_name)
{
    int ret;
    const char* var_value_str = getenv(var_name);
    if(var_value_str == NULL)
        return -1;

    return atoi(var_value_str);
}

int
drop_permissions()
{
    int status = 0;

    uid_t uid = getuid();

    if(uid != 0) // Not running as root, no need to drop permissions
        return 0;

    uid_t sudo_uid = (uid_t) getenv_int("SUDO_UID");
    gid_t sudo_gid = (gid_t) getenv_int("SUDO_GID");

    if(sudo_uid < 0 || sudo_gid < 0) // Running as root, but not with sudo, PROLLY BAD
        err_quit("Running as root without sudo, not cool");

    setuid(sudo_uid);
    setgid(sudo_gid);
    return 0;
}

pid_t
init_qkd_script()
{
    // TODO(oral): It might be better to open file descriptors of the domain sockets before forking
    // so Python can directly write to the file descriptor that we already opened...
    pid_t pid;
    if ((pid = fork()) < 0) {
        err_sys("fork error");
    } else if (pid == 0) { // child
        drop_permissions(); // TODO(mlisak): It would be better to drop permissions right after creating network device, check if we can do that
        chdir("/usr/local/opt/qvpn");
        if (execl("/usr/local/opt/qvpn/run.sh", (char * )0) < 0)
            err_sys("execl error");
    } else { // parent
        int qkd_sock_fd = -1;
        for(int i = 0; i < 10; i++)
        {
            qkd_sock_fd = open("/tmp/qvpn.socket", O_RDWR);
            if(qkd_sock_fd > 0)
            {
                T.qkd.fd = qkd_sock_fd;
                break;
            }
        }
        if(qkd_sock_fd < 0)
            err_sys("failed to open communication socket");
    }

    return pid;
}


char procname[256];

int
main (int argc, char **argv)
{

    generate_network_json();

    sprintf(procname, "tun-test");
    err_setprogname(procname);

    inet_aton(argv[1], &T.dev.addr);
    inet_aton(argv[2], &T.dev.dstaddr);
    T.dev.mtu = 1500;

    inet_aton(argv[3], &T.sock.bind.sin_addr);
    inet_aton(argv[4], &T.sock.peer.sin_addr);

    if (argc > 4 && !strcmp(argv[5], "server")) {
        T.sock.mode = MODE_SERVER;
    } else {
        T.sock.mode = MODE_CLIENT;
    }

    tun_init(&T);

    for(;;) {
        sleep(1);
    }

}
