#include <linux/module.h>
#include <net/sock.h>
#include <linux/netlink.h>
#include <linux/skbuff.h>
#include <net/net_namespace.h>

#define NETLINK_USER 31

static struct sock * netlink_socket = NULL;

static void handle_incoming_message(struct sk_buff *skb){
    struct nlmsghdr *nlh;
    int sender_pid; // PID отправителя
    struct sk_buff * reply_skb;
    int msg_size;
    char *reply_msg = "Hello from kernel!";
    int res;

    printk(KERN_INFO,"Entering: %s\n", __FUNCTION__);

    msg_size = strlen(reply_msg);

    nlh = (struct nlmsghdr *)skb->data;

    sender_pid = nlh->nlmsg_pid; 

    printk(KERN_INFO "Netlink received msg payload from process %d: %s\n", 
           sender_pid, (char *)nlmsg_data(nlh));
    
    reply_skb = nlmsg_new(msg_size, 0);
    if (!reply_skb) {
        printk(KERN_ERR "Failed to allocate skb for reply\n");
        return;
    }

    nlh = nlmsg_put(reply_skb, 0, 0, NLMSG_DONE, msg_size, 0);
    if (!nlh) {
        printk(KERN_ERR "Failed to put nlmsghdr in skb\n");
        kfree_skb(reply_skb);
        return;
    }

    NETLINK_CB(reply_skb).dst_group = 0;

    memcpy(nlmsg_data(nlh), reply_msg, msg_size);

    res = nlmsg_unicast(netlink_socket, reply_skb, sender_pid);
    if (res < 0) {
        printk(KERN_INFO "Error while sending reply to user: %d\n", res);
    }
}

struct netlink_kernel_cfg cfg = {
    .groups = 1,  
    .input = handle_incoming_message, 
};

static int __init my_init(void)
{
    printk(KERN_INFO "Entering: %s\n", __FUNCTION__);
    
    netlink_socket = netlink_kernel_create(&init_net, NETLINK_USER, &cfg);
    if (!netlink_socket) {
        printk(KERN_ALERT "Error creating Netlink socket.\n");
        return -EIO; 
    }
    
    return 0;
}

static void __exit my_exit(void)
{
    printk(KERN_INFO "Exiting: %s\n", __FUNCTION__);
    
    if (netlink_socket) {
        netlink_kernel_release(netlink_socket);
        netlink_socket = NULL;
    }
}

module_init(my_init);
module_exit(my_exit);

MODULE_LICENSE("GPL");
MODULE_AUTHOR("Dudkina Mariya");