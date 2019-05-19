 WIDE DHCPv6 client
=============================
*This is the WIDE-DHCPv6 client. Orginally maintained by the OPNsense project.*

 Improvements include the following:
--------
- Reload the client configuration on SIGHUP
- Removed all unused binaries except dhcp6c
- Raw option send and receive support

It is based on a patched version of WIDE-DHCPv6 maintained by hrs@FreeBSD.org.

 Improvements include the follwowing:
---

- Increased log verbosity.
- Fix socket leakage by setting FD_CLOEXEC.
- Call a configuration script after addresses and prefixes are set
  on an interface.
- Update ifid on interface restart, it may have changed.
- "-n" flag to prevent address release from being sent to the DHCP server
  upon restart.

 Installation Instructions:
--------------------------
### Requirements:
You need the following Packages installed:
```bash
sudo apt install gcc flex libfl-dev bison
```

### Installation

To install the packages first you need to configure:
```bash
./configure
```

Maybe you have to set custom CFLAGS:
```bash
./configure CFLAGS="-D _GNU_SOURCE"
```

Next you can execute the make and sudo make install
```
make
make install
```

### Usage
For the configuration you can use the provided ``dhcp6c.conf.sample`` file.

To test your config, your command may look like this:
```bash
/usr/local/sbin/dhcp6c -c /etc/wide-dhcpv6/dhcp6c.conf ppp0 -Ddf
# /etc/wide-dhcpv6/dhcp6c.conf is your local configuration
# ppp0 is your interface you want to run the client
# -Ddf are parameter for console output
```

 Original README:
-------------------
```txt
$KAME: README,v 1.3 2005/01/12 06:06:10 suz Exp $

WIDE-DHCPv6

	WIDE-DHCPv6 is an open-source implementation of Dynamic Host
	Configuration Protocol for IPv6 (DHCPv6) developed by the KAME
	project.

	The implementation mainly conforms to the following standards:

	- RFC3315: Dynamic Host Configuration Protocol for IPv6 (DHCPv6)
	- RFC3319: Dynamic Host Configuration Protocol (DHCPv6) Options 
         	   for Session Initiation Protocol (SIP) Servers
	- RFC3633: IPv6 Prefix Options for Dynamic Host Configuration
	           Protocol (DHCP)
	- RFC3646: DNS Configuration options for Dynamic Host
	           Configuration Protocol for IPv6 (DHCPv6)
	- RFC4075: Simple Network Time Protocol (SNTP) Configuration
	           Option for DHCPv6
	- RFC4242: Information Refresh Time Option for Dynamic Host
	           Configuration Protocol for IPv6 (DHCPv6)

	All the components of the main protocol are provided, i.e.,
	DHCPv6 clients, servers, and relay agents.

	Supported message types are as follows:
	  Solicit, Advertise, Request, Renew, Rebind, Release, Reply,
	  Information-request, Relay-forward and Relay-reply.

	Note that the current implementation does not support IPv6
	temporary address allocation by DHCPv6, and there is no plan to
	implement that feature at the moment.

Bug Reports and Mailing Lists

	Bugs reports should be put to the Bug Tracking System at 
	sourceforge.net
		http://sourceforge.net/projects/wide-dhcpv6/

	Information on the mailing list, including how to subscribe
	and browse archives, is available at the following URL:

		http://sourceforge.net/mail/?group_id=153668
```
