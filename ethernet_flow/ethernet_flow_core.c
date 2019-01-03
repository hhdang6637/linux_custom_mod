#include <linux/module.h>
#include <linux/printk.h>
#include <linux/proc_fs.h>
#include <linux/netdevice.h>
#include <linux/rtnetlink.h>
#include <linux/version.h>

MODULE_DESCRIPTION("ethernetflow module");
MODULE_LICENSE("GPL v2");

extern void ethernet_flow_init(void);
extern void ethernet_flow_cleanup(void);
extern void ethernetflow_add_frame(const unsigned char macs[]);

extern void ethernet_flow_macs_collection_init(void);
extern void ethernet_flow_macs_collection_cleanup(void);
extern int ethernet_flow_macs_collection_find(const unsigned char mac[]);
extern void ethernet_flow_macs_collection_add(const unsigned char mac[]);

static rx_handler_result_t ethernetFlowStatistics_handle_frame(struct sk_buff **pskb) {

	struct sk_buff *skb = *pskb;

	const unsigned char *dest = eth_hdr(skb)->h_dest;
	const unsigned char *src = eth_hdr(skb)->h_source;

	if (!ethernet_flow_macs_collection_find(src)) {
		ethernet_flow_macs_collection_add(src);
	}

	ethernetflow_add_frame(dest);

	return RX_HANDLER_PASS;
}

static struct net_device *monitor_dev;

static int ethernetflow_add_if(struct net_device *dev) {
	int err;

	err = netdev_rx_handler_register(dev, ethernetFlowStatistics_handle_frame, 0);
	if (err) {
		printk(KERN_WARNING "%s fail on %s\n", __FUNCTION__, dev->name);
		return -EINVAL;
	}

	err = dev_set_promiscuity(dev, 1);
	if (err)
		return 0;

	return 0;
}

static int ethernetflow_remove_if(struct net_device *dev) {

	netdev_rx_handler_unregister(dev);

	dev_set_promiscuity(dev, -1);

	return 0;
}

static int ethernetflow_device_event(struct notifier_block *unused, unsigned long event, void *ptr) {
#if LINUX_VERSION_CODE <= KERNEL_VERSION(4,0,0)
	struct net_device *dev = ptr;
#else
	struct net_device *dev = netdev_notifier_info_to_dev(ptr);
#endif

	switch (event) {
	case NETDEV_CHANGEMTU:
		break;

	case NETDEV_CHANGEADDR:
		break;

	case NETDEV_CHANGE:
		break;

	case NETDEV_FEAT_CHANGE:
		break;

	case NETDEV_DOWN:
		break;

	case NETDEV_UP:
		break;

	case NETDEV_UNREGISTER:
		if (monitor_dev == dev) {
			ethernetflow_remove_if(dev);
			monitor_dev = NULL;
		}
		break;

	case NETDEV_CHANGENAME:
		break;

	case NETDEV_PRE_TYPE_CHANGE:
		/* Forbid underlaying device to change its type. */
		return NOTIFY_BAD;
	}

	return NOTIFY_DONE;
}

static struct notifier_block ethernetflow_device_notifier = {
	.notifier_call = ethernetflow_device_event
};

static ssize_t monitor_dev_read(struct file *file, char __user *ubuf,size_t count, loff_t *ppos) {
	char buff[64];
	int len;
	int pos = 0;

	if(*ppos > 0) {
		return 0;
	}

	if (monitor_dev == NULL) {
		return 0;
	}

	len = sprintf(buff, "%s", monitor_dev->name);

	pos += len;

	if (pos <= count) {

		if(copy_to_user(ubuf + (pos - len), buff, len)) {
			return -EFAULT;
		}

	}

	*ppos = pos;
	return pos;
}

static ssize_t monitor_dev_ops_write(struct file *file, const char __user *buf,
				    size_t size, loff_t *_pos)
{
	char str[32];
	int n;
	int val;
	struct net_device *dev = NULL;

	val = 0;

	n = size > sizeof(str) - 1 ? sizeof(str) - 1 : size;

	if (copy_from_user (str, buf, n) != 0) {
		return ENOMEM;
	}

	*_pos = n;

	str[n] = '\0';

	dev = __dev_get_by_name(&init_net, str);

	if (dev) {

		rtnl_lock();
		if (monitor_dev != NULL) {
			ethernetflow_remove_if(monitor_dev);
			monitor_dev = NULL;
		}

		if (ethernetflow_add_if(dev) == 0) {
			monitor_dev = dev;
		}
		rtnl_unlock();

		return n;
	}

	return -EINVAL;
}

static struct file_operations monitor_dev_ops =
{
	.owner = THIS_MODULE,
	.read = monitor_dev_read,
	.write = monitor_dev_ops_write,
};

struct proc_dir_entry *ethernetflow_root;
static struct proc_dir_entry *ethernet_flow_monitor_dev;

static int ethernetflow_init(void)
{

	ethernetflow_root = proc_mkdir("ethernet_flow", NULL);

	if(ethernetflow_root == NULL)
	{
		printk(KERN_ERR "Cannot proc_mkdir ethernet_flow");
		return -1;
	}

	ethernet_flow_init();

	ethernet_flow_macs_collection_init();

	if (register_netdevice_notifier(&ethernetflow_device_notifier)) {
		printk(KERN_WARNING "register_netdevice_notifier failed\n");
	}

	ethernet_flow_monitor_dev = proc_create("monitor_dev", 0660, ethernetflow_root, &monitor_dev_ops);

	return 0;
}

static void ethernetflow_cleanup(void)
{
	unregister_netdevice_notifier(&ethernetflow_device_notifier);

	if (monitor_dev != NULL) {
		rtnl_lock();
		ethernetflow_remove_if(monitor_dev);
		rtnl_unlock();
		monitor_dev = NULL;
	}

	ethernet_flow_cleanup();

	ethernet_flow_macs_collection_cleanup();

	if (ethernet_flow_monitor_dev)
		remove_proc_entry("monitor_dev", ethernetflow_root);

	if (ethernetflow_root)
		remove_proc_entry("ethernet_flow", NULL);
}

module_init(ethernetflow_init);
module_exit(ethernetflow_cleanup);

