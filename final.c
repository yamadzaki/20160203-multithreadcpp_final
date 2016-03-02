/*
static const char* templ = "HTTP/1.0 200 OK\r\n"
		           "Content-length: %d\r\n"
		       	   "Connection: close\r\n"
		       	   "Content-Type: text/html\r\n"
		       	   "\r\n"
		       	   "%s";

static const char not_found[] = "HTTP/1.0 404 NOT FOUND\r\nContent-Type: text/html\r\n\r\n";
*/

#include <stdio.h>
#include <stdlib.h>

#include <unistd.h> 
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <string.h>

#define handle_error_en(en, msg) \
    do { errno = en; perror(msg); exit(EXIT_FAILURE); } while (0)

#define handle_error(msg) \
    do { perror(msg); exit(EXIT_FAILURE); } while (0)

void getoptions(int argc, char **argv, int *port, char *ip, char *dir) {
    // parse parameters
    int opt = 0;
    *port = 0;
    ip = NULL;
    dir = NULL;

    while ((opt = getopt(argc, argv, "hpd")) != -1) {
        switch (opt) {
            case 'h':
                ip = argv[optind];
                break;
            case 'p':
                *port = atoi(argv[optind]);
                break;
            case 'd':
                dir = argv[optind];
                break;
            default:
                fprintf(stderr, "Usage: %s -h <ip> -p <port> -d <dir>\n", argv[0]);
                exit(EXIT_FAILURE);
        }
    }


    if (argc != 7) {
        fprintf(stderr, "Usage: %s -h <ip> -p <port> -d <dir>\n", argv[0]);
        exit(EXIT_FAILURE);
    }

    return;
}


ssize_t sock_fd_write(int sock, void *buf, ssize_t buflen, int fd)
{
    ssize_t     size;
    struct msghdr   msg;
    struct iovec    iov;
    union {
        struct cmsghdr  cmsghdr;
        char        control[CMSG_SPACE(sizeof (int))];
    } cmsgu;
    struct cmsghdr  *cmsg;

    iov.iov_base = buf;
    iov.iov_len = buflen;

    msg.msg_name = NULL;
    msg.msg_namelen = 0;
    msg.msg_iov = &iov;
    msg.msg_iovlen = 1;

    if (fd != -1) {
        msg.msg_control = cmsgu.control;
        msg.msg_controllen = sizeof(cmsgu.control);

        cmsg = CMSG_FIRSTHDR(&msg);
        cmsg->cmsg_len = CMSG_LEN(sizeof (int));
        cmsg->cmsg_level = SOL_SOCKET;
        cmsg->cmsg_type = SCM_RIGHTS;

        printf ("passing fd %d\n", fd);
        *((int *) CMSG_DATA(cmsg)) = fd;
    } else {
        msg.msg_control = NULL;
        msg.msg_controllen = 0;
        printf ("not passing fd\n");
    }

    size = sendmsg(sock, &msg, 0);

    if (size < 0)
        perror ("sendmsg");
    return size;
}       


ssize_t sock_fd_read(int sock, void *buf, ssize_t bufsize, int *fd)
{
    ssize_t size;

    if (fd) {
        struct msghdr   msg;
        struct iovec    iov;
        union {
            struct cmsghdr  cmsghdr;
            char        control[CMSG_SPACE(sizeof (int))];
        } cmsgu;
        struct cmsghdr  *cmsg;

        iov.iov_base = buf;
        iov.iov_len = bufsize;

        msg.msg_name = NULL;
        msg.msg_namelen = 0;
        msg.msg_iov = &iov;
        msg.msg_iovlen = 1;
        msg.msg_control = cmsgu.control;
        msg.msg_controllen = sizeof(cmsgu.control);
        size = recvmsg (sock, &msg, 0);
        if (size < 0) {
            perror ("recvmsg");
            exit(1);
        }
        cmsg = CMSG_FIRSTHDR(&msg);
        if (cmsg && cmsg->cmsg_len == CMSG_LEN(sizeof(int))) {
            if (cmsg->cmsg_level != SOL_SOCKET) {
                fprintf (stderr, "invalid cmsg_level %d\n",
                     cmsg->cmsg_level);
                exit(1);
            }
            if (cmsg->cmsg_type != SCM_RIGHTS) {
                fprintf (stderr, "invalid cmsg_type %d\n",
                     cmsg->cmsg_type);
                exit(1);
            }

            *fd = *((int *) CMSG_DATA(cmsg));
            printf ("received fd %d\n", *fd);
        } else
            *fd = -1;
    } else {
        size = read (sock, buf, bufsize);
        if (size < 0) {
            perror("read");
            exit(1);
        }
    }
    return size;
}


void child(int sock)
{
    int SlaveSocket;
    char    buf[16];
    ssize_t size;

    sleep(1);
    for (;;) {
        size = sock_fd_read(sock, buf, sizeof(buf), &SlaveSocket);
        if (size <= 0)
            break;

        if (SlaveSocket != -1) {
            printf("Child recieve SlaveSocket: %d\n", SlaveSocket);
            
            while(1) {
                char Buffer[5] = {0, 0, 0, 0, 0};
                ssize_t msgsize = recv(SlaveSocket, Buffer, 4, MSG_NOSIGNAL);

                if (msgsize == -1) { // error
                    handle_error("Error in recv()\n");

                } else if (msgsize == 0) { //seems EOF
                    printf("Client correctly close connection\n");
                    if (shutdown(SlaveSocket, SHUT_RDWR) == -1) {
                        handle_error("Problem with shutdown() function\n");
                    }

                    if (close(SlaveSocket) == -1) {
                        handle_error("Problem with close() function\n");
                    }
                    break; //exit
                }

                printf("%s", Buffer);

                static const char *not_found = "HTTP/1.0 404 NOT FOUND\r\nContent-Type: text/html\r\n\r\n";

                if (send(SlaveSocket, not_found, strlen(not_found), MSG_NOSIGNAL) == -1) {
                    handle_error("Problems with send()\n");
                }

                printf("Server correctly close connection\n");
                if (shutdown(SlaveSocket, SHUT_RDWR) == -1) {
                    handle_error("Problem with shutdown() function\n");
                }

                if (close(SlaveSocket) == -1) {
                    handle_error("Problem with close() function\n");
                }
                break; //exit
            } 


           // write(fd, "hello, world\n", 13);
            //close(fd);
        }
    }
    printf("Child finish work\n");
}


void parent(int sock, int port)
{
    ssize_t size;

    int MasterSocket = socket(
        AF_INET,     //IPv4
        SOCK_STREAM, //TCP
        IPPROTO_TCP);

    struct sockaddr_in SockAddr;
    SockAddr.sin_family = AF_INET;
    SockAddr.sin_port = htons(port);
    SockAddr.sin_addr.s_addr = htonl(INADDR_ANY);

    if (bind(MasterSocket, (struct sockaddr *)(&SockAddr), sizeof(SockAddr)) != 0) {
        handle_error("Can not bind port\n");
    }

    if (listen(MasterSocket, SOMAXCONN) != 0) {
        handle_error("Can not listen() socket\n");
    }

    while(1) {
        int SlaveSocket = accept(MasterSocket, 0, 0);
        if (SlaveSocket >= 0) {
            printf("New client accepted\n");
        } else if (SlaveSocket == -1) {
            handle_error("Problem with accept()\n");
        }

        size = sock_fd_write(sock, "1", 1, SlaveSocket);
        printf ("Parent send SlaveSocket: %d\n", SlaveSocket);

        if (close(SlaveSocket) == -1) {
            handle_error("Problem with close() function\n");
        }
    }
}



int main(int argc, char **argv) {

    int port = 0;
    char *ip = NULL;
    char *dir = NULL;

    getoptions(argc, argv, &port, ip, dir);
    printf("ip=%s; port=%d; dir=%s\n", ip, port, dir);

//---------------
    int sv[2];
    int pid;


    if (socketpair(AF_LOCAL, SOCK_STREAM, 0, sv) < 0) {
        perror("socketpair");
        exit(1);
    }

    switch ((pid = fork())) {
    case 0:
        close(sv[0]);
        child(sv[1]);
        break;
    case -1:
        perror("fork");
        exit(1);
    default:
        close(sv[1]);
        parent(sv[0], port);
        break;
    }
    return 0;


////
/*
 */
    printf("%s\n", "Exit program");
    return 0;
}

