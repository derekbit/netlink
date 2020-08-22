#include <linux/module.h>
#include <linux/netlink.h>
#include <net/sock.h>
#include <linux/string.h>
#include <uapi/linux/netlink.h>

#define NETLINK_TEST_PROTOCOL 31

static struct sock *nl_sk;

static void nlmsg_dump(struct nlmsghdr *nlh)
{
	printk(KERN_INFO "netlink msg hdr: "
		"len=%u, type=%u, flags=%u, seq=%u, pid=%u\n",
		nlh->nlmsg_len, nlh->nlmsg_type,
		nlh->nlmsg_flags, nlh->nlmsg_seq,
		nlh->nlmsg_pid);
}

static void netlink_recv_msg_fn(struct sk_buff *skb_in)
{
	unsigned char *data = skb_in->data;
	struct nlmsghdr *hdr = (struct nlmsghdr *)data;

	nlmsg_dump(hdr);
}

static struct netlink_kernel_cfg cfg = {
	.input = netlink_recv_msg_fn,
};

static int __init netlink_helloworld_init(void)
{
	printk(KERN_INFO "Netlink: Hello, World!\n");
    
	nl_sk = netlink_kernel_create(&init_net, NETLINK_TEST_PROTOCOL, &cfg);
	if (!nl_sk) {
		printk(KERN_INFO "Netlink: failed to create netlink socket protocol %u\n", NETLINK_TEST_PROTOCOL);
		return ENOMEM;
	}

	return 0;
}

static void __exit netlink_helloworld_exit(void)
{
	printk(KERN_INFO "Netlink: Bye Bye!\n");
	netlink_kernel_release(nl_sk);
	nl_sk = NULL;
}

module_init(netlink_helloworld_init);
module_exit(netlink_helloworld_exit);

MODULE_LICENSE("GPL");
