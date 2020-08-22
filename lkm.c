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
	struct nlmsghdr *nlh_recv, *nlh_reply;
	int user_space_process_port_id;
	char *user_space_data;
	int user_space_data_len;
	char kernel_reply[256];
	struct sk_buff *skb_out;
	int ret;

	nlh_recv = (struct nlmsghdr *)(skb_in->data);
	nlmsg_dump(nlh_recv);

	user_space_process_port_id = nlh_recv->nlmsg_pid;

	printk(KERN_INFO "netlink: port id of the user-space process = %u\n",
		user_space_process_port_id);

	user_space_data = (char *)nlmsg_data(nlh_recv);
	user_space_data_len = skb_in->len;

	printk(KERN_INFO "netlink: recv from user-space process = %s, "
		"skb_in->len = %u, nlh_recv->nlmsg_len = %u\n",
		user_space_data, user_space_data_len, nlh_recv->nlmsg_len);

	/* Reply to user-space process if flags have NLM_F_ACK. */
	if (nlh_recv->nlmsg_flags & NLM_F_ACK) {
		memset(kernel_reply, 0, sizeof(kernel_reply));
		snprintf(kernel_reply, sizeof(kernel_reply),
			"Msg from Process %u has been processed by kernel",
			nlh_recv->nlmsg_pid);

		skb_out = nlmsg_new(sizeof(kernel_reply), 0);

		/* Send msg out.*/
		nlh_reply = nlmsg_put(skb_out,
				      0, /* port id is 0 because it is sent by kernel */
				      nlh_recv->nlmsg_seq,
				      NLMSG_DONE,
				      sizeof(kernel_reply),
				      0);

		/* Copy payload to nlh_reply */
		memcpy(nlmsg_data(nlh_reply), kernel_reply, sizeof(kernel_reply));
		ret = nlmsg_unicast(nl_sk, skb_out, user_space_process_port_id);
		if (ret < 0) {
			printk(KERN_INFO "Error while sending the data back to user-space process\n");
			kfree_skb(skb_out);
		}
	}
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
