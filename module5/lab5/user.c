#include <stdlib.h>
#include <string.h>
#include <stdio.h>
#include <sys/socket.h>
#include <linux/netlink.h>
#include <sys/types.h>
#include <sys/uio.h>
#include <unistd.h>

#define NETLINK_USER 31
#define MAX_PAYLOAD 1024 

int main(int argc, char *argv[])
{
    int sock_fd;
    struct sockaddr_nl src_addr, dest_addr;
    struct nlmsghdr *nlh = NULL;
    struct iovec iov;
    struct msghdr msg;

    sock_fd = socket(PF_NETLINK, SOCK_RAW, NETLINK_USER);
    if (sock_fd < 0) {
        perror("Failed to create socket");
        return EXIT_FAILURE;
    }

    memset(&src_addr, 0, sizeof(src_addr));
    src_addr.nl_family = AF_NETLINK;
    src_addr.nl_pid = getpid();
    if (bind(sock_fd, (struct sockaddr *)&src_addr, sizeof(src_addr)) < 0) {
        perror("Failed to bind socket");
        close(sock_fd);
        return EXIT_FAILURE;
    }

    memset(&dest_addr, 0, sizeof(dest_addr));
    dest_addr.nl_family = AF_NETLINK;
    dest_addr.nl_pid = 0;
    dest_addr.nl_groups = 0;

    nlh = (struct nlmsghdr *)malloc(NLMSG_SPACE(MAX_PAYLOAD));
    if (!nlh) {
        perror("Failed to allocate memory for message");
        close(sock_fd);
        return EXIT_FAILURE;
    }
    memset(nlh, 0, NLMSG_SPACE(MAX_PAYLOAD));
    nlh->nlmsg_len = NLMSG_SPACE(MAX_PAYLOAD);
    nlh->nlmsg_pid = getpid();
    nlh->nlmsg_flags = 0;

    strcpy(NLMSG_DATA(nlh),argv[1] ? argv[1] : "Hello from user!");

    iov.iov_base = (void *)nlh;
    iov.iov_len = nlh->nlmsg_len;

    memset(&msg, 0, sizeof(msg));
    msg.msg_name = (void *)&dest_addr;
    msg.msg_namelen = sizeof(dest_addr);
    msg.msg_iov = &iov;
    msg.msg_iovlen = 1;

    printf("Sending message to kernel: %s\n", (char *)NLMSG_DATA(nlh));
    if (sendmsg(sock_fd, &msg, 0) < 0) {
        perror("Failed to send message");
        free(nlh);
        close(sock_fd);
        return EXIT_FAILURE;
    }

    printf("Waiting for message from kernel\n");
    if (recvmsg(sock_fd, &msg, 0) < 0) {
        perror("Failed to receive message");
        free(nlh);
        close(sock_fd);
        return EXIT_FAILURE;
    }

    printf("Received message payload: %s\n", (char *)NLMSG_DATA(nlh));

    free(nlh);
    close(sock_fd);
    return EXIT_SUCCESS;
}