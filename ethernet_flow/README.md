# linux_custom_mod
This is a small network linux module that let us capture MAC addresses and ethernet flows statistic.
After you load this modoule to kernel. It creates some proc files under /proc/ethernet_flow
```
# ls -l /proc/ethernet_flow/
total 0
--w--w----    1 root     root             0 Jan 10 02:35 flow_clear
-rw-rw----    1 root     root             0 Jan 10 02:35 flow_size
-r--r--r--    1 root     root             0 Jan 10 02:35 flow_table
--w--w----    1 root     root             0 Jan 10 02:35 macs_clear
-r--r--r--    1 root     root             0 Jan 10 02:35 macs_table
-rw-rw----    1 root     root             0 Jan 10 02:35 monitor_dev
```
Run following command to register your network interface to this module:
```
echo eth0 > /proc/ethernet_flow/monitor_dev
```
You monitor the highest ethernet flow with following command: 
```
watch "cat /proc/ethernet_flow/flow_table  | sort -k 6 -r | head"
```
Example output:
```
# cat /proc/ethernet_flow/flow_table  | sort -k 6 -r | head
01: 00:1b:21:5c:ea:3d -> b8:27:eb:d0:15:87 : 00998263
19: 52:54:00:64:77:74 -> 01:00:5e:1e:01:01 : 00256752
08: 00:15:ad:3c:ce:34 -> 01:15:ad:00:00:00 : 00172154
09: 00:15:ad:3c:ce:7c -> 01:15:ad:00:00:00 : 00172132
06: 00:15:ad:3b:50:42 -> 01:15:ad:00:00:00 : 00172131
10: 00:15:ad:3b:2f:26 -> 01:15:ad:00:00:00 : 00172058
11: 00:15:ad:3b:2f:1a -> 01:15:ad:00:00:00 : 00172048
16: 00:15:ad:3b:50:90 -> 01:15:ad:00:00:00 : 00172043
12: 00:15:ad:3b:50:36 -> 01:15:ad:00:00:00 : 00166554
07: 00:15:ad:21:b6:0c -> 01:15:ad:00:00:00 : 00166542
```
