/*	$KAME: dhcp6c.c,v 1.164 2006/01/10 02:46:09 jinmei Exp $	*/
/*
 * Copyright (C) 1998 and 1999 WIDE Project.
 * All rights reserved.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions
 * are met:
 * 1. Redistributions of source code must retain the above copyright
 *    notice, this list of conditions and the following disclaimer.
 * 2. Redistributions in binary form must reproduce the above copyright
 *    notice, this list of conditions and the following disclaimer in the
 *    documentation and/or other materials provided with the distribution.
 * 3. Neither the name of the project nor the names of its contributors
 *    may be used to endorse or promote products derived from this software
 *    without specific prior written permission.
 *
 * THIS SOFTWARE IS PROVIDED BY THE PROJECT AND CONTRIBUTORS ``AS IS'' AND
 * ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
 * IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE
 * ARE DISCLAIMED.  IN NO EVENT SHALL THE PROJECT OR CONTRIBUTORS BE LIABLE
 * FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL
 * DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS
 * OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION)
 * HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT
 * LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY
 * OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF
 * SUCH DAMAGE.
 */
#include <sys/types.h>
#include <sys/param.h>
#include <sys/socket.h>
#include <sys/uio.h>
#include <sys/queue.h>
#include <errno.h>
#include <limits.h>
#if TIME_WITH_SYS_TIME
# include <sys/time.h>
# include <time.h>
#else
# if HAVE_SYS_TIME_H
#  include <sys/time.h>
# else
#  include <time.h>
# endif
#endif
#include <net/if.h>
#ifdef __FreeBSD__
#include <net/if_var.h>
#endif

#include <netinet/in.h>
#ifdef __KAME__
#include <net/if_dl.h>
#include <netinet6/in6_var.h>
#endif

#include <arpa/inet.h>
#include <netdb.h>

#include <signal.h>
#include <stdio.h>
#include <stdarg.h>
#include <syslog.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <err.h>
#include <ifaddrs.h>
#include <fcntl.h>

#include "dhcp6.h"
#include "config.h"
#include "common.h"
#include "timer.h"
#include "dhcp6c.h"
#include "dhcp6c_ia.h"
#include "prefixconf.h"
#include "auth.h"

static int debug = 0;
static int exit_ok = 0;
static sig_atomic_t sig_flags = 0;
#define SIGF_TERM 0x1
#define SIGF_HUP 0x2
#define	SIGF_USR1 0x4

const dhcp6_mode_t dhcp6_mode = DHCP6_MODE_CLIENT;

static int sock;	/* inbound/outbound udp port */

#define CTLSKEW 300

static const char *conffile = DHCP6C_CONF;

static const struct sockaddr_in6 *sa6_allagent;
static struct duid client_duid;
static const char *pid_file = DHCP6C_PIDFILE;

static struct keyinfo *ctlkey = NULL;
static int ctldigestlen;

static int infreq_mode = 0;

int opt_norelease;

static void usage(void);
static void client6_init(void);
static void client6_startall(int);
static void client6_reset(struct dhcp6_if *);
static void free_resources(struct dhcp6_if *);
static void client6_mainloop(void);
static void check_exit(void);
static void process_signals(void);
static struct dhcp6_serverinfo *find_server(struct dhcp6_event *,
						 struct duid *);
static struct dhcp6_serverinfo *select_server(struct dhcp6_event *);
static void client6_recv(void);
static int client6_recvadvert(struct dhcp6_if *, struct dhcp6 *,
				   ssize_t, struct dhcp6_optinfo *);
static int client6_recvreply(struct dhcp6_if *, struct dhcp6 *,
				  ssize_t, struct dhcp6_optinfo *);
static void client6_signal(int);
static struct dhcp6_event *find_event_withid(struct dhcp6_if *,
						  uint32_t);
static int construct_confdata(struct dhcp6_if *, struct dhcp6_event *);
static int construct_reqdata(struct dhcp6_if *, struct dhcp6_optinfo *,
    struct dhcp6_event *);
static void destruct_iadata(struct dhcp6_eventdata *);
static void tv_sub(struct timeval *, struct timeval *, struct timeval *);
static struct dhcp6_timer *client6_expire_refreshtime(void *);
static int process_auth(struct authparam *, struct dhcp6 *dh6, ssize_t,
    struct dhcp6_optinfo *);
static int set_auth(struct dhcp6_event *, struct dhcp6_optinfo *);

struct dhcp6_timer *client6_timo(void *);
int client6_start(struct dhcp6_if *);
static void info_printf(const char *, ...);

static void init_cli_if(int argc, char **argv);

int use_all_config_if;
static int saved_cli_if_count;
static char **saved_cli_if;

#define MAX_ELAPSED_TIME 0xffff

int
main(int argc, char *argv[])
{
	int ch, pid;
	char *progname;
	FILE *pidfp;
	struct cf_namelist *ifnamep;

#ifndef HAVE_ARC4RANDOM
	srandom(time(NULL) & getpid());
#endif

	if ((progname = strrchr(*argv, '/')) == NULL)
		progname = *argv;
	else
		progname++;

	while ((ch = getopt(argc, argv, "c:dDfinp:")) != -1) {
		switch (ch) {
		case 'c':
			conffile = optarg;
			break;
		case 'd':
			debug = 1;
			break;
		case 'D':
			debug = 2;
			break;
		case 'f':
			foreground++;
			break;
		case 'i':
			infreq_mode = 1;
			break;
		case 'n':
			opt_norelease = 1;
			break;
		case 'p':
			pid_file = optarg;
			break;
		default:
			usage();
			exit(0);
		}
	}
	argc -= optind;
	argv += optind;

	if (foreground == 0)
		openlog(progname, LOG_NDELAY|LOG_PID, LOG_DAEMON);

	setloglevel(debug);

	client6_init();

	/*
	 * Doing away with the need for command line interfaces -- if this is set
	 * config.c initializes the interface after parsing it.	This makes cfparse.y
	 * have valid entries in dhcp6_if before it invokes configure_commit() at the
	 * end of it's parse. Only one parse pass needed now.
	 */
	use_all_config_if = (argc == 0);

	if (!use_all_config_if) {
		saved_cli_if = argv;
		saved_cli_if_count = argc;
		init_cli_if(saved_cli_if_count, saved_cli_if);

	}

	if (infreq_mode == 0 && cfparse(conffile)) {
		d_printf(LOG_ERR, FNAME, "failed to parse configuration file");
		exit(1);
	}

	if (foreground == 0 && infreq_mode == 0) {
		if (daemon(0, 0) < 0)
			err(1, "daemon");
	}

	/* dump current PID */
	pid = getpid();
	if ((pidfp = fopen(pid_file, "w")) != NULL) {
		fprintf(pidfp, "%d\n", pid);
		fclose(pidfp);
	}

	client6_startall(0);
	client6_mainloop();
	exit(0);
}

static void
usage()
{

	fprintf(stderr, "usage: dhcp6c [-c configfile] [-dDfin] "
	    "[-p pid-file] [interfaces...]\n");
}

/*------------------------------------------------------------*/

static void
init_cli_if(int argc, char **argv) {
	while (argc-- > 0) {
		if (ifinit(argv[0]) == NULL) {
			d_printf(LOG_ERR, FNAME, "failed to initialize %s",
			    argv[0]);
			exit(1);
		}
		argv++;
	}
}

void
client6_init(void)
{
	struct addrinfo hints, *res;
	static struct sockaddr_in6 sa6_allagent_storage;
	int error, on = 0;

	/* get our DUID */
	if (get_duid(DUID_FILE, &client_duid)) {
		d_printf(LOG_ERR, FNAME, "failed to get a DUID");
		exit(1);
	}

	memset(&hints, 0, sizeof(hints));
	hints.ai_family = PF_INET6;
	hints.ai_socktype = SOCK_DGRAM;
	hints.ai_protocol = IPPROTO_UDP;
	hints.ai_flags = AI_PASSIVE;
	error = getaddrinfo(NULL, DH6PORT_DOWNSTREAM, &hints, &res);
	if (error) {
		d_printf(LOG_ERR, FNAME, "getaddrinfo: %s",
		    gai_strerror(error));
		exit(1);
	}
	sock = socket(res->ai_family, res->ai_socktype, res->ai_protocol);
	if (sock < 0) {
		d_printf(LOG_ERR, FNAME, "socket");
		exit(1);
	}

	if ((on = fcntl(sock, F_GETFL, 0)) == -1) {
		d_printf(LOG_ERR, FNAME, "fctnl getflags");
		exit(1);
	}

	on |= FD_CLOEXEC;

	if ((on = fcntl(sock, F_SETFL, on)) == -1) {
		d_printf(LOG_ERR, FNAME, "fctnl setflags");
		exit(1);
	}

	on = 1;
	if (setsockopt(sock, SOL_SOCKET, SO_REUSEPORT,
		       &on, sizeof(on)) < 0) {
		d_printf(LOG_ERR, FNAME,
		    "setsockopt(SO_REUSEPORT): %s", strerror(errno));
		exit(1);
	}
#ifdef IPV6_RECVPKTINFO
	if (setsockopt(sock, IPPROTO_IPV6, IPV6_RECVPKTINFO, &on,
		       sizeof(on)) < 0) {
		d_printf(LOG_ERR, FNAME,
			"setsockopt(IPV6_RECVPKTINFO): %s",
			strerror(errno));
		exit(1);
	}
#else
	if (setsockopt(sock, IPPROTO_IPV6, IPV6_PKTINFO, &on,
		       sizeof(on)) < 0) {
		d_printf(LOG_ERR, FNAME,
		    "setsockopt(IPV6_PKTINFO): %s",
		    strerror(errno));
		exit(1);
	}
#endif
	if (setsockopt(sock, IPPROTO_IPV6, IPV6_MULTICAST_LOOP, &on,
		       sizeof(on)) < 0) {
		d_printf(LOG_ERR, FNAME,
		    "setsockopt(sock, IPV6_MULTICAST_LOOP): %s",
		    strerror(errno));
		exit(1);
	}
#ifdef IPV6_V6ONLY
	if (setsockopt(sock, IPPROTO_IPV6, IPV6_V6ONLY,
	    &on, sizeof(on)) < 0) {
		d_printf(LOG_ERR, FNAME, "setsockopt(IPV6_V6ONLY): %s",
		    strerror(errno));
		exit(1);
	}
#endif

	/*
	 * According RFC3315 2.2, only the incoming port should be bound to UDP
	 * port 546.  However, to have an interoperability with some servers,
	 * the outgoing port is also bound to the DH6PORT_DOWNSTREAM.
	 */
	if (bind(sock, res->ai_addr, res->ai_addrlen) < 0) {
		d_printf(LOG_ERR, FNAME, "bind: %s", strerror(errno));
		exit(1);
	}
	freeaddrinfo(res);

	memset(&hints, 0, sizeof(hints));
	hints.ai_family = PF_INET6;
	hints.ai_socktype = SOCK_DGRAM;
	hints.ai_protocol = IPPROTO_UDP;
	error = getaddrinfo(DH6ADDR_ALLAGENT, DH6PORT_UPSTREAM, &hints, &res);
	if (error) {
		d_printf(LOG_ERR, FNAME, "getaddrinfo: %s",
		    gai_strerror(error));
		exit(1);
	}
	memcpy(&sa6_allagent_storage, res->ai_addr, res->ai_addrlen);
	sa6_allagent = (const struct sockaddr_in6 *)&sa6_allagent_storage;
	freeaddrinfo(res);

	if (signal(SIGHUP, client6_signal) == SIG_ERR) {
		d_printf(LOG_WARNING, FNAME, "failed to set signal: %s",
		    strerror(errno));
		exit(1);
	}
	if (signal(SIGINT, client6_signal) == SIG_ERR) {
		d_printf(LOG_WARNING, FNAME, "failed to set signal: %s",
		    strerror(errno));
		exit(1);
	}
	if (signal(SIGTERM, client6_signal) == SIG_ERR) {
		d_printf(LOG_WARNING, FNAME, "failed to set signal: %s",
		    strerror(errno));
		exit(1);
	}
	if (signal(SIGUSR1, client6_signal) == SIG_ERR) {
		d_printf(LOG_WARNING, FNAME, "failed to set signal: %s",
		    strerror(errno));
		exit(1);
	}
}

int
client6_start(struct dhcp6_if *ifp)
{
	struct dhcp6_event *ev;

	/* make sure that the interface does not have a timer */
	if (ifp->timer != NULL) {
		d_printf(LOG_DEBUG, FNAME,
		    "removed existing timer on %s", ifp->ifname);
		dhcp6_remove_timer(&ifp->timer);
	}

	/* create an event for the initial delay */
	if ((ev = dhcp6_create_event(ifp, DHCP6S_INIT)) == NULL) {
		d_printf(LOG_NOTICE, FNAME, "failed to create an event");
		return (-1);
	}
	TAILQ_INSERT_TAIL(&ifp->event_list, ev, link);

	if ((ev->authparam = new_authparam(ifp->authproto,
	    ifp->authalgorithm, ifp->authrdm)) == NULL) {
		d_printf(LOG_WARNING, FNAME, "failed to allocate "
		    "authentication parameters");
		dhcp6_remove_event(ev);
		return (-1);
	}

	if ((ev->timer = dhcp6_add_timer(client6_timo, ev)) == NULL) {
		d_printf(LOG_NOTICE, FNAME, "failed to add a timer for %s",
		    ifp->ifname);
		dhcp6_remove_event(ev);
		return (-1);
	}
	dhcp6_reset_timer(ev);

	return (0);
}

static void
client6_reset(struct dhcp6_if *ifp)
{
	d_printf(LOG_NOTICE, FNAME, "resetting lease on %s", ifp->ifname);

	free_resources(ifp);
	ifreset(ifp);
	client6_start(ifp);
}

static void
client6_startall(int isrestart)
{
	struct dhcp6_if *ifp;

	for (ifp = dhcp6_if; ifp; ifp = ifp->next) {
		if (isrestart &&ifreset(ifp)) {
			d_printf(LOG_NOTICE, FNAME, "failed to reset %s",
			    ifp->ifname);
			continue; /* XXX: try to recover? */
		}
		if (client6_start(ifp))
			exit(1); /* initialization failure.  we give up. */
	}
}

static void
free_resources(struct dhcp6_if *freeifp)
{
	struct dhcp6_if *ifp;

	for (ifp = dhcp6_if; ifp; ifp = ifp->next) {
		struct dhcp6_event *ev, *ev_next;

		if (freeifp != NULL && freeifp != ifp)
			continue;

		/* release all IAs as well as send RELEASE message(s) */
		release_all_ia(ifp);

		/*
		 * Cancel all outstanding events for each interface except
		 * ones being released.
		 */
		for (ev = TAILQ_FIRST(&ifp->event_list); ev; ev = ev_next) {
			ev_next = TAILQ_NEXT(ev, link);

			if (ev->state == DHCP6S_RELEASE)
				continue; /* keep it for now */

			dhcp6_remove_event(ev);
		}
	}
}

static void
check_exit(void)
{
	struct dhcp6_if *ifp;

	if (!exit_ok)
		return;

	for (ifp = dhcp6_if; ifp; ifp = ifp->next) {
		/*
		 * Check if we have an outstanding event.  If we do, we cannot
		 * exit for now.
		 */
		if (!TAILQ_EMPTY(&ifp->event_list))
			return;
	}
	for (ifp = dhcp6_if; ifp; ifp = ifp->next)
		client6_script(ifp->scriptpath, DHCP6S_EXIT, NULL);

	/* We have no existing event.  Do exit. */
	d_printf(LOG_INFO, FNAME, "exiting");

	unlink(pid_file);

	if (foreground) {
		fflush(stdout);
		fflush(stderr);
	}

	exit(0);
}

static void
process_signals(void)
{
	struct cf_namelist *ifnamep;
	struct dhcp6_if *ifp;

	if ((sig_flags & SIGF_TERM)) {
		exit_ok = 1;
		free_resources(NULL);
		check_exit();
	}
	if ((sig_flags & SIGF_HUP)) {
		d_printf(LOG_INFO, FNAME, "restarting");
		free_resources(NULL);
		if (!use_all_config_if) {
			init_cli_if(saved_cli_if_count, saved_cli_if);
		}
		if (cfparse(conffile)) {
			d_printf(LOG_WARNING, FNAME,
			    "failed to reload configuration file");
		}
		client6_startall(1);
	}
	if ((sig_flags & SIGF_USR1)) {
		d_printf(LOG_INFO, FNAME, "exit without release");
		exit_ok = 1;
		opt_norelease = 1;
		free_resources(NULL);
		check_exit();
	}

	sig_flags = 0;
}

static void
client6_mainloop(void)
{
	struct timeval *w;
	int ret, maxsock;
	fd_set r;

	while(1) {
		if (sig_flags)
			process_signals();

		w = dhcp6_check_timer();

		FD_ZERO(&r);
		FD_SET(sock, &r);
		maxsock = sock;

		ret = select(maxsock + 1, &r, NULL, NULL, w);

		switch (ret) {
		case -1:
			if (errno != EINTR) {
				d_printf(LOG_ERR, FNAME, "select: %s",
				    strerror(errno));
				exit(1);
			}
			continue;
		case 0:	/* timeout */
			break;	/* dhcp6_check_timer() will treat the case */
		default:
			break;
		}
		if (FD_ISSET(sock, &r))
			client6_recv();
	}
}

static inline int
get_ifname(char **bpp, int *lenp, char *ifbuf, int ifbuflen)
{
	char *bp = *bpp;
	int len = *lenp, ifnamelen;
	uint32_t i32;

	if (get_val32(bpp, lenp, &i32))
		return (-1);
	ifnamelen = (int)i32;

	if (*lenp < ifnamelen || ifnamelen > ifbuflen)
		return (-1);

	memset(ifbuf, 0, ifbuflen);
	memcpy(ifbuf, *bpp, ifnamelen);
	if (ifbuf[ifbuflen - 1] != '\0')
		return (-1);	/* not null terminated */

	*bpp = bp + sizeof(i32) + ifnamelen;
	*lenp = len - (sizeof(i32) + ifnamelen);

	return (0);
}

static struct dhcp6_timer *
client6_expire_refreshtime(void *arg)
{
	struct dhcp6_if *ifp = arg;

	d_printf(LOG_DEBUG, FNAME,
	    "information refresh time on %s expired", ifp->ifname);

	dhcp6_remove_timer(&ifp->timer);
	client6_start(ifp);

	return (NULL);
}

struct dhcp6_timer *
client6_timo(void *arg)
{
	struct dhcp6_event *ev = (struct dhcp6_event *)arg;
	struct dhcp6_if *ifp;
	int state = ev->state;

	ifp = ev->ifp;
	ev->timeouts++;

	/*
	 * Unless MRC is zero, the message exchange fails once the client has
	 * transmitted the message MRC times.
	 * [RFC3315 14.]
	 */
	if (ev->max_retrans_cnt && ev->timeouts >= ev->max_retrans_cnt) {
		d_printf(LOG_INFO, FNAME, "no responses were received");
		dhcp6_remove_event(ev);

		if (state == DHCP6S_RELEASE)
			check_exit();

		return (NULL);
	}

	switch(ev->state) {
	case DHCP6S_INIT:
		ev->timeouts = 0; /* indicate to generate a new XID. */
		if ((ifp->send_flags & DHCIFF_INFO_ONLY) || infreq_mode)
			ev->state = DHCP6S_INFOREQ;
		else {
			ev->state = DHCP6S_SOLICIT;
			if (construct_confdata(ifp, ev)) {
				d_printf(LOG_ERR, FNAME, "can't send solicit");
				exit(1); /* XXX */
			}
		}
		dhcp6_set_timeoparam(ev); /* XXX */
		/* fall through */
	case DHCP6S_REQUEST:
	case DHCP6S_RELEASE:
	case DHCP6S_INFOREQ:
		client6_send(ev);
		break;
	case DHCP6S_RENEW:
	case DHCP6S_REBIND:
		if (!TAILQ_EMPTY(&ev->data_list))
			client6_send(ev);
		else {
			d_printf(LOG_INFO, FNAME,
			    "all information to be updated was canceled");
			dhcp6_remove_event(ev);
			return (NULL);
		}
		break;
	case DHCP6S_SOLICIT:
		if (ev->servers) {
			/*
			 * Send a Request to the best server.
			 * Note that when we set Rapid-commit in Solicit,
			 * but a direct Reply has been delayed (very much),
			 * the transition to DHCP6S_REQUEST (and the change of
			 * transaction ID) will invalidate the reply even if it
			 * ever arrives.
			 */
			ev->current_server = select_server(ev);
			if (ev->current_server == NULL) {
				/* this should not happen! */
				d_printf(LOG_NOTICE, FNAME,
				    "can't find a server");
				exit(1); /* XXX */
			}
			if (duidcpy(&ev->serverid,
			    &ev->current_server->optinfo.serverID)) {
				d_printf(LOG_NOTICE, FNAME,
				    "can't copy server ID");
				return (NULL); /* XXX: better recovery? */
			}
			ev->timeouts = 0;
			ev->state = DHCP6S_REQUEST;
			dhcp6_set_timeoparam(ev);

			if (ev->authparam != NULL)
				free(ev->authparam);
			ev->authparam = ev->current_server->authparam;
			ev->current_server->authparam = NULL;

			if (construct_reqdata(ifp,
			    &ev->current_server->optinfo, ev)) {
				d_printf(LOG_NOTICE, FNAME,
				    "failed to construct request data");
				break;
			}
		}
		client6_send(ev);
		break;
	}

	dhcp6_reset_timer(ev);

	return (ev->timer);
}

static int
construct_confdata(struct dhcp6_if *ifp, struct dhcp6_event *ev)
{
	struct ia_conf *iac;
	struct dhcp6_eventdata *evd = NULL;
	struct dhcp6_list *ial = NULL, pl;
	struct dhcp6_ia iaparam;

	TAILQ_INIT(&pl);	/* for safety */

	for (iac = TAILQ_FIRST(&ifp->iaconf_list); iac;
	    iac = TAILQ_NEXT(iac, link)) {
		/* ignore IA config currently used */
		if (!TAILQ_EMPTY(&iac->iadata))
			continue;

		evd = NULL;
		if ((evd = malloc(sizeof(*evd))) == NULL) {
			d_printf(LOG_NOTICE, FNAME,
			    "failed to create a new event data");
			goto fail;
		}
		memset(evd, 0, sizeof(*evd));

		memset(&iaparam, 0, sizeof(iaparam));
		iaparam.iaid = iac->iaid;
		switch (iac->type) {
		case IATYPE_PD:
			ial = NULL;
			if ((ial = malloc(sizeof(*ial))) == NULL)
				goto fail;
			TAILQ_INIT(ial);

			TAILQ_INIT(&pl);
			dhcp6_copy_list(&pl,
			    &((struct iapd_conf *)iac)->iapd_prefix_list);
			if (dhcp6_add_listval(ial, DHCP6_LISTVAL_IAPD,
			    &iaparam, &pl) == NULL) {
				goto fail;
			}
			dhcp6_clear_list(&pl);

			evd->type = DHCP6_EVDATA_IAPD;
			evd->data = ial;
			evd->event = ev;
			evd->destructor = destruct_iadata;
			TAILQ_INSERT_TAIL(&ev->data_list, evd, link);
			break;
		case IATYPE_NA:
			ial = NULL;
			if ((ial = malloc(sizeof(*ial))) == NULL)
				goto fail;
			TAILQ_INIT(ial);

			TAILQ_INIT(&pl);
			dhcp6_copy_list(&pl,
			    &((struct iana_conf *)iac)->iana_address_list);
			if (dhcp6_add_listval(ial, DHCP6_LISTVAL_IANA,
			    &iaparam, &pl) == NULL) {
				goto fail;
			}
			dhcp6_clear_list(&pl);

			evd->type = DHCP6_EVDATA_IANA;
			evd->data = ial;
			evd->event = ev;
			evd->destructor = destruct_iadata;
			TAILQ_INSERT_TAIL(&ev->data_list, evd, link);
			break;
		default:
			d_printf(LOG_ERR, FNAME, "internal error");
			exit(1);
		}
	}

	return (0);

  fail:
	if (evd)
		free(evd);
	if (ial)
		free(ial);
	dhcp6_remove_event(ev);	/* XXX */

	return (-1);
}

static int
construct_reqdata(struct dhcp6_if *ifp,
    struct dhcp6_optinfo *optinfo, struct dhcp6_event *ev)
{
	struct ia_conf *iac;
	struct dhcp6_eventdata *evd = NULL;
	struct dhcp6_list *ial = NULL;
	struct dhcp6_ia iaparam;

	/* discard previous event data */
	dhcp6_remove_evdata(ev);

	if (optinfo == NULL)
		return (0);

	for (iac = TAILQ_FIRST(&ifp->iaconf_list); iac;
	    iac = TAILQ_NEXT(iac, link)) {
		struct dhcp6_listval *v;

		/* ignore IA config currently used */
		if (!TAILQ_EMPTY(&iac->iadata))
			continue;

		memset(&iaparam, 0, sizeof(iaparam));
		iaparam.iaid = iac->iaid;

		ial = NULL;
		evd = NULL;

		switch (iac->type) {
		case IATYPE_PD:
			if ((v = dhcp6_find_listval(&optinfo->iapd_list,
			    DHCP6_LISTVAL_IAPD, &iaparam, 0)) == NULL)
				continue;

			if ((ial = malloc(sizeof(*ial))) == NULL)
				goto fail;

			TAILQ_INIT(ial);
			if (dhcp6_add_listval(ial, DHCP6_LISTVAL_IAPD,
			    &iaparam, &v->sublist) == NULL) {
				goto fail;
			}

			if ((evd = malloc(sizeof(*evd))) == NULL)
				goto fail;
			memset(evd, 0, sizeof(*evd));
			evd->type = DHCP6_EVDATA_IAPD;
			evd->data = ial;
			evd->event = ev;
			evd->destructor = destruct_iadata;
			TAILQ_INSERT_TAIL(&ev->data_list, evd, link);
			break;
		case IATYPE_NA:
			if ((v = dhcp6_find_listval(&optinfo->iana_list,
			    DHCP6_LISTVAL_IANA, &iaparam, 0)) == NULL)
				continue;

			if ((ial = malloc(sizeof(*ial))) == NULL)
				goto fail;

			TAILQ_INIT(ial);
			if (dhcp6_add_listval(ial, DHCP6_LISTVAL_IANA,
			    &iaparam, &v->sublist) == NULL) {
				goto fail;
			}

			if ((evd = malloc(sizeof(*evd))) == NULL)
				goto fail;
			memset(evd, 0, sizeof(*evd));
			evd->type = DHCP6_EVDATA_IANA;
			evd->data = ial;
			evd->event = ev;
			evd->destructor = destruct_iadata;
			TAILQ_INSERT_TAIL(&ev->data_list, evd, link);
			break;
		default:
			d_printf(LOG_ERR, FNAME, "internal error");
			exit(1);
		}
	}

	return (0);

  fail:
	if (evd)
		free(evd);
	if (ial)
		free(ial);
	dhcp6_remove_event(ev);	/* XXX */

	return (-1);
}

static void
destruct_iadata(struct dhcp6_eventdata *evd)
{
	struct dhcp6_list *ial;

	if (evd->type != DHCP6_EVDATA_IAPD && evd->type != DHCP6_EVDATA_IANA) {
		d_printf(LOG_ERR, FNAME, "assumption failure %d", evd->type);
		exit(1);
	}

	ial = (struct dhcp6_list *)evd->data;
	dhcp6_clear_list(ial);
	free(ial);
}

static struct dhcp6_serverinfo *
select_server(struct dhcp6_event *ev)
{
	struct dhcp6_serverinfo *s;

	/*
	 * pick the best server according to RFC3315 Section 17.1.3.
	 * XXX: we currently just choose the one that is active and has the
	 * highest preference.
	 */
	for (s = ev->servers; s; s = s->next) {
		if (s->active) {
			d_printf(LOG_DEBUG, FNAME, "picked a server (ID: %s)",
			    duidstr(&s->optinfo.serverID));
			return (s);
		}
	}

	return (NULL);
}

static void
client6_signal(int sig)
{

	switch (sig) {
	case SIGHUP:
		sig_flags |= SIGF_HUP;
		break;
	case SIGINT:
	case SIGTERM:
		sig_flags |= SIGF_TERM;
		break;
	case SIGUSR1:
		sig_flags |= SIGF_USR1;
		break;
	}
}

void
client6_send(struct dhcp6_event *ev)
{
	struct dhcp6_if *ifp;
	char buf[BUFSIZ];
	struct sockaddr_in6 dst;
	struct dhcp6 *dh6;
	struct dhcp6_optinfo optinfo;
	ssize_t optlen, len;
	struct dhcp6_eventdata *evd;

	ifp = ev->ifp;

	dh6 = (struct dhcp6 *)buf;
	memset(dh6, 0, sizeof(*dh6));

	switch(ev->state) {
	case DHCP6S_SOLICIT:
		dh6->dh6_msgtype = DH6_SOLICIT;
		d_printf(LOG_INFO, FNAME, "Sending Solicit");
		break;
	case DHCP6S_REQUEST:
		dh6->dh6_msgtype = DH6_REQUEST;
		d_printf(LOG_INFO, FNAME, "Sending Request");
		break;
	case DHCP6S_RENEW:
		dh6->dh6_msgtype = DH6_RENEW;
		d_printf(LOG_INFO, FNAME, "Sending Renew");
		break;
	case DHCP6S_REBIND:
		dh6->dh6_msgtype = DH6_REBIND;
		d_printf(LOG_INFO, FNAME, "Sending Rebind");
		break;
	case DHCP6S_RELEASE:
		dh6->dh6_msgtype = DH6_RELEASE;
		d_printf(LOG_INFO, FNAME, "Sending Release");
		break;
	case DHCP6S_INFOREQ:
		dh6->dh6_msgtype = DH6_INFORM_REQ;
		d_printf(LOG_INFO, FNAME, "Sending Information Request");
		break;
	default:
		d_printf(LOG_ERR, FNAME, "unexpected state");
		exit(1);	/* XXX */
	}

	if (ev->timeouts == 0) {
		/*
		 * A client SHOULD generate a random number that cannot easily
		 * be guessed or predicted to use as the transaction ID for
		 * each new message it sends.
		 *
		 * A client MUST leave the transaction-ID unchanged in
		 * retransmissions of a message. [RFC3315 15.1]
		 */
#ifdef HAVE_ARC4RANDOM
		ev->xid = arc4random() & DH6_XIDMASK;
#else
		ev->xid = random() & DH6_XIDMASK;
#endif
		d_printf(LOG_DEBUG, FNAME, "a new XID (%x) is generated",
		    ev->xid);
	}
	dh6->dh6_xid &= ~ntohl(DH6_XIDMASK);
	dh6->dh6_xid |= htonl(ev->xid);
	len = sizeof(*dh6);

	/*
	 * construct options
	 */
	dhcp6_init_options(&optinfo);

	/* server ID */
	switch (ev->state) {
	case DHCP6S_REQUEST:
	case DHCP6S_RENEW:
	case DHCP6S_RELEASE:
		if (duidcpy(&optinfo.serverID, &ev->serverid)) {
			d_printf(LOG_ERR, FNAME, "failed to copy server ID");
			goto end;
		}
		break;
	}

	/* client ID */
	if (duidcpy(&optinfo.clientID, &client_duid)) {
		d_printf(LOG_ERR, FNAME, "failed to copy client ID");
		goto end;
	}

	/* rapid commit (in Solicit only) */
	if (ev->state == DHCP6S_SOLICIT &&
	    (ifp->send_flags & DHCIFF_RAPID_COMMIT)) {
		optinfo.rapidcommit = 1;
	}

	/* elapsed time */
	if (ev->timeouts == 0) {
		gettimeofday(&ev->tv_start, NULL);
		optinfo.elapsed_time = 0;
	} else {
		struct timeval now, tv_diff;
		long et;

		gettimeofday(&now, NULL);
		tv_sub(&now, &ev->tv_start, &tv_diff);

		/*
		 * The client uses the value 0xffff to represent any elapsed
		 * time values greater than the largest time value that can be
		 * represented in the Elapsed Time option.
		 * [RFC3315 22.9.]
		 */
		if (tv_diff.tv_sec >= (MAX_ELAPSED_TIME / 100) + 1) {
			/*
			 * Perhaps we are nervous too much, but without this
			 * additional check, we would see an overflow in 248
			 * days (of no responses).
			 */
			et = MAX_ELAPSED_TIME;
		} else {
			et = tv_diff.tv_sec * 100 + tv_diff.tv_usec / 10000;
			if (et >= MAX_ELAPSED_TIME)
				et = MAX_ELAPSED_TIME;
		}
		optinfo.elapsed_time = (int32_t)et;
	}

	/* option request options */
	if (ev->state != DHCP6S_RELEASE &&
	    dhcp6_copy_list(&optinfo.reqopt_list, &ifp->reqopt_list)) {
		d_printf(LOG_ERR, FNAME, "failed to copy requested options");
		goto end;
	}

	/* configuration information specified as event data */
	for (evd = TAILQ_FIRST(&ev->data_list); evd;
	     evd = TAILQ_NEXT(evd, link)) {
		switch(evd->type) {
		case DHCP6_EVDATA_IAPD:
			if (dhcp6_copy_list(&optinfo.iapd_list,
			    (struct dhcp6_list *)evd->data)) {
				d_printf(LOG_NOTICE, FNAME,
				    "failed to add an IAPD");
				goto end;
			}
			break;
		case DHCP6_EVDATA_IANA:
			if (dhcp6_copy_list(&optinfo.iana_list,
			    (struct dhcp6_list *)evd->data)) {
				d_printf(LOG_NOTICE, FNAME,
				    "failed to add an IANA");
				goto end;
			}
			break;
		default:
			d_printf(LOG_ERR, FNAME, "unexpected event data (%d)",
			    evd->type);
			exit(1);
		}
	}

	/* authentication information */
	if (set_auth(ev, &optinfo)) {
		d_printf(LOG_INFO, FNAME,
		    "failed to set authentication option");
		goto end;
	}

	rawop_copy_list(&optinfo.rawops, &ifp->rawops);

	/* set options in the message */
	if ((optlen = dhcp6_set_options(dh6->dh6_msgtype,
	    (struct dhcp6opt *)(dh6 + 1),
	    (struct dhcp6opt *)(buf + sizeof(buf)), &optinfo)) < 0) {
		d_printf(LOG_INFO, FNAME, "failed to construct options");
		goto end;
	}
	len += optlen;

	/* calculate MAC if necessary, and put it to the message */
	if (ev->authparam != NULL) {
		switch (ev->authparam->authproto) {
		case DHCP6_AUTHPROTO_DELAYED:
			if (ev->authparam->key == NULL)
				break;

			if (dhcp6_calc_mac((unsigned char *)dh6, len,
			    optinfo.authproto, optinfo.authalgorithm,
			    optinfo.delayedauth_offset + sizeof(*dh6),
			    ev->authparam->key)) {
				d_printf(LOG_WARNING, FNAME,
				    "failed to calculate MAC");
				goto end;
			}
			break;
		default:
			break;	/* do nothing */
		}
	}

	/*
	 * Unless otherwise specified in this document or in a document that
	 * describes how IPv6 is carried over a specific type of link (for link
	 * types that do not support multicast), a client sends DHCP messages
	 * to the All_DHCP_Relay_Agents_and_Servers.
	 * [RFC3315 Section 13.]
	 */
	dst = *sa6_allagent;
	dst.sin6_scope_id = ifp->linkid;

	if (sendto(sock, buf, len, 0, (struct sockaddr *)&dst,
	    sysdep_sa_len((struct sockaddr *)&dst)) == -1) {
		d_printf(LOG_ERR, FNAME,
		    "transmit failed: %s", strerror(errno));
		goto end;
	}

	d_printf(LOG_DEBUG, FNAME, "send %s to %s",
	    dhcp6msgstr(dh6->dh6_msgtype), addr2str((struct sockaddr *)&dst));

  end:
	dhcp6_clear_options(&optinfo);
	return;
}

/* result will be a - b */
static void
tv_sub(struct timeval *a, struct timeval *b, struct timeval *result)
{
	if (a->tv_sec < b->tv_sec ||
	    (a->tv_sec == b->tv_sec && a->tv_usec < b->tv_usec)) {
		result->tv_sec = 0;
		result->tv_usec = 0;

		return;
	}

	result->tv_sec = a->tv_sec - b->tv_sec;
	if (a->tv_usec < b->tv_usec) {
		result->tv_usec = a->tv_usec + 1000000 - b->tv_usec;
		result->tv_sec -= 1;
	} else
		result->tv_usec = a->tv_usec - b->tv_usec;

	return;
}

static void
client6_recv(void)
{
	char rbuf[BUFSIZ], cmsgbuf[BUFSIZ];
	struct msghdr mhdr;
	struct iovec iov;
	struct sockaddr_storage from;
	struct dhcp6_if *ifp;
	struct dhcp6opt *p, *ep;
	struct dhcp6_optinfo optinfo;
	ssize_t len;
	struct dhcp6 *dh6;
	struct cmsghdr *cm;
	struct in6_pktinfo *pi = NULL;

	memset(&iov, 0, sizeof(iov));
	memset(&mhdr, 0, sizeof(mhdr));

	iov.iov_base = (caddr_t)rbuf;
	iov.iov_len = sizeof(rbuf);
	mhdr.msg_name = (caddr_t)&from;
	mhdr.msg_namelen = sizeof(from);
	mhdr.msg_iov = &iov;
	mhdr.msg_iovlen = 1;
	mhdr.msg_control = (caddr_t)cmsgbuf;
	mhdr.msg_controllen = sizeof(cmsgbuf);
	if ((len = recvmsg(sock, &mhdr, 0)) < 0) {
		d_printf(LOG_ERR, FNAME, "recvmsg: %s", strerror(errno));
		return;
	}

	/* detect receiving interface */
	for (cm = (struct cmsghdr *)CMSG_FIRSTHDR(&mhdr); cm;
	     cm = (struct cmsghdr *)CMSG_NXTHDR(&mhdr, cm)) {
		if (cm->cmsg_level == IPPROTO_IPV6 &&
		    cm->cmsg_type == IPV6_PKTINFO &&
		    cm->cmsg_len == CMSG_LEN(sizeof(struct in6_pktinfo))) {
			pi = (struct in6_pktinfo *)(void *)(CMSG_DATA(cm));
		}
	}
	if (pi == NULL) {
		d_printf(LOG_NOTICE, FNAME, "failed to get packet info");
		return;
	}

	if ((ifp = find_ifconfbyid((unsigned int)pi->ipi6_ifindex)) == NULL) {
		d_printf(LOG_INFO, FNAME, "unexpected interface (%d)",
		    (unsigned int)pi->ipi6_ifindex);
		return;
	}

	if ((size_t)len < sizeof(*dh6)) {
		d_printf(LOG_INFO, FNAME, "short packet (%d bytes)", len);
		return;
	}

	dh6 = (struct dhcp6 *)rbuf;

	d_printf(LOG_DEBUG, FNAME, "receive %s from %s on %s",
	    dhcp6msgstr(dh6->dh6_msgtype),
	    addr2str((struct sockaddr *)&from), ifp->ifname);

	/* get options */
	dhcp6_init_options(&optinfo);
	p = (struct dhcp6opt *)(dh6 + 1);
	ep = (struct dhcp6opt *)((char *)dh6 + len);
	if (dhcp6_get_options(p, ep, &optinfo) < 0) {
		d_printf(LOG_INFO, FNAME, "failed to parse options");
		return;
	}

	switch(dh6->dh6_msgtype) {
	case DH6_ADVERTISE:
		(void)client6_recvadvert(ifp, dh6, len, &optinfo);
		break;
	case DH6_REPLY:
		(void)client6_recvreply(ifp, dh6, len, &optinfo);
		break;
	default:
		d_printf(LOG_INFO, FNAME, "received an unexpected message (%s) "
		    "from %s", dhcp6msgstr(dh6->dh6_msgtype),
		    addr2str((struct sockaddr *)&from));
		break;
	}

	dhcp6_clear_options(&optinfo);
	return;
}

static int
client6_recvadvert(struct dhcp6_if *ifp, struct dhcp6 *dh6,
    ssize_t len, struct dhcp6_optinfo *optinfo)
{
	struct dhcp6_serverinfo *newserver, **sp;
	struct dhcp6_event *ev;
	struct dhcp6_eventdata *evd;
	struct authparam *authparam = NULL, authparam0;
	int have_ia = -1;

	/* find the corresponding event based on the received xid */
	ev = find_event_withid(ifp, ntohl(dh6->dh6_xid) & DH6_XIDMASK);
	if (ev == NULL) {
		d_printf(LOG_INFO, FNAME, "XID mismatch");
		return (-1);
	}

	/* packet validation based on Section 15.3 of RFC3315. */
	if (optinfo->serverID.duid_len == 0) {
		d_printf(LOG_INFO, FNAME, "no server ID option");
		return (-1);
	} else {
		d_printf(LOG_DEBUG, FNAME, "server ID: %s, pref=%d",
		    duidstr(&optinfo->serverID),
		    optinfo->pref);
	}
	if (optinfo->clientID.duid_len == 0) {
		d_printf(LOG_INFO, FNAME, "no client ID option");
		return (-1);
	}
	if (duidcmp(&optinfo->clientID, &client_duid)) {
		d_printf(LOG_INFO, FNAME, "client DUID mismatch");
		return (-1);
	}

	/* validate authentication */
	authparam0 = *ev->authparam;
	if (process_auth(&authparam0, dh6, len, optinfo)) {
		d_printf(LOG_INFO, FNAME, "failed to process authentication");
		return (-1);
	}

	/*
	 * The requesting router MUST ignore any Advertise message that
	 * includes a Status Code option containing the value NoPrefixAvail
	 * [RFC3633 Section 11.1].
	 * Likewise, the client MUST ignore any Advertise message that includes
	 * a Status Code option containing the value NoAddrsAvail.
	 * [RFC3315 Section 17.1.3].
	 * We only apply this when we are going to request an address or
	 * a prefix.
	 */
	for (evd = TAILQ_FIRST(&ev->data_list); evd;
	    evd = TAILQ_NEXT(evd, link)) {
		struct dhcp6_listval *lv, *slv;
		uint16_t stcode;
		const char *stcodestr;

		switch (evd->type) {
		case DHCP6_EVDATA_IAPD:
			stcode = DH6OPT_STCODE_NOPREFIXAVAIL;
			stcodestr = "NoPrefixAvail";
			break;
		case DHCP6_EVDATA_IANA:
			stcode = DH6OPT_STCODE_NOADDRSAVAIL;
			stcodestr = "NoAddrsAvail";
			break;
		default:
			continue;
		}

		if (dhcp6_find_listval(&optinfo->stcode_list,
		    DHCP6_LISTVAL_STCODE, &stcode, 0)) {
			d_printf(LOG_INFO, FNAME,
			    "advertise contains %s status", stcodestr);
			return (-1);
		}

		if (have_ia > 0 ||
		    TAILQ_EMPTY((struct dhcp6_list *)evd->data)) {
			continue;
		}

		have_ia = 0;

		/* parse list of IA_PD */
		if (evd->type == DHCP6_EVDATA_IAPD) {
			TAILQ_FOREACH(lv, (struct dhcp6_list *)evd->data, link) {
				slv = dhcp6_find_listval(&optinfo->iapd_list,
				    DHCP6_LISTVAL_IAPD, &lv->val_ia, 0);
				if (slv == NULL) {
					continue;
				}
				TAILQ_FOREACH(slv, &slv->sublist, link) {
					if (slv->type == DHCP6_LISTVAL_PREFIX6) {
						have_ia = 1;
						break;
					}
				}
			}
		}

		/* parse list of IA_NA */
		if (evd->type == DHCP6_EVDATA_IANA) {
			TAILQ_FOREACH(lv, (struct dhcp6_list *)evd->data, link) {
				slv = dhcp6_find_listval(&optinfo->iana_list,
				    DHCP6_LISTVAL_IANA, &lv->val_ia, 0);
				if (slv == NULL) {
					continue;
				}
				TAILQ_FOREACH(slv, &slv->sublist, link) {
					if (slv->type == DHCP6_LISTVAL_STATEFULADDR6) {
						have_ia = 1;
						break;
					}
				}
			}
		}
	}

	/*
	 * Ignore message with none of requested addresses and/or
	 * a prefixes as if NoAddrsAvail/NoPrefixAvail Status Code
	 * was included.
	 */
	if (have_ia == 0) {
		d_printf(LOG_INFO, FNAME, "advertise contains no address/prefix");
		return (-1);
	}

	if (ev->state != DHCP6S_SOLICIT ||
	    (ifp->send_flags & DHCIFF_RAPID_COMMIT) || infreq_mode) {
		/*
		 * We expected a reply message, but do actually receive an
		 * Advertise message.  The server should be configured not to
		 * allow the Rapid Commit option.
		 * We process the message as if we expected the Advertise.
		 * [RFC3315 Section 17.1.4]
		 */
		d_printf(LOG_INFO, FNAME, "unexpected advertise");
		/* proceed anyway */
	}

	/* ignore the server if it is known */
	if (find_server(ev, &optinfo->serverID)) {
		d_printf(LOG_INFO, FNAME, "duplicated server (ID: %s)",
		    duidstr(&optinfo->serverID));
		return (-1);
	}

	/* keep the server */
	if ((newserver = malloc(sizeof(*newserver))) == NULL) {
		d_printf(LOG_WARNING, FNAME,
		    "memory allocation failed for server");
		return (-1);
	}
	memset(newserver, 0, sizeof(*newserver));

	/* remember authentication parameters */
	newserver->authparam = ev->authparam;
	newserver->authparam->flags = authparam0.flags;
	newserver->authparam->prevrd = authparam0.prevrd;
	newserver->authparam->key = authparam0.key;

	/* allocate new authentication parameter for the soliciting event */
	if ((authparam = new_authparam(ev->authparam->authproto,
	    ev->authparam->authalgorithm, ev->authparam->authrdm)) == NULL) {
		d_printf(LOG_WARNING, FNAME, "memory allocation failed "
		    "for authentication parameters");
		free(newserver);
		return (-1);
	}
	ev->authparam = authparam;

	/* copy options */
	dhcp6_init_options(&newserver->optinfo);
	if (dhcp6_copy_options(&newserver->optinfo, optinfo)) {
		d_printf(LOG_ERR, FNAME, "failed to copy options");
		if (newserver->authparam != NULL)
			free(newserver->authparam);
		free(newserver);
		return (-1);
	}
	if (optinfo->pref != DH6OPT_PREF_UNDEF)
		newserver->pref = optinfo->pref;
	newserver->active = 1;
	for (sp = &ev->servers; *sp; sp = &(*sp)->next) {
		if ((*sp)->pref != DH6OPT_PREF_MAX &&
		    (*sp)->pref < newserver->pref) {
			break;
		}
	}
	newserver->next = *sp;
	*sp = newserver;

	if (newserver->pref == DH6OPT_PREF_MAX) {
		/*
		 * If the client receives an Advertise message that includes a
		 * Preference option with a preference value of 255, the client
		 * immediately begins a client-initiated message exchange.
		 * [RFC3315 Section 17.1.2]
		 */
		ev->current_server = newserver;
		if (duidcpy(&ev->serverid,
		    &ev->current_server->optinfo.serverID)) {
			d_printf(LOG_NOTICE, FNAME, "can't copy server ID");
			return (-1); /* XXX: better recovery? */
		}
		if (construct_reqdata(ifp, &ev->current_server->optinfo, ev)) {
			d_printf(LOG_NOTICE, FNAME,
			    "failed to construct request data");
			return (-1); /* XXX */
		}

		ev->timeouts = 0;
		ev->state = DHCP6S_REQUEST;

		free(ev->authparam);
		ev->authparam = newserver->authparam;
		newserver->authparam = NULL;

		client6_send(ev);

		dhcp6_set_timeoparam(ev);
		dhcp6_reset_timer(ev);
	} else if (ev->servers->next == NULL) {
		struct timeval *rest, elapsed, tv_rt, tv_irt, timo;

		/*
		 * If this is the first advertise, adjust the timer so that
		 * the client can collect other servers until IRT elapses.
		 * XXX: we did not want to do such "low level" timer
		 *      calculation here.
		 */
		rest = dhcp6_timer_rest(ev->timer);
		tv_rt.tv_sec = (ev->retrans * 1000) / 1000000;
		tv_rt.tv_usec = (ev->retrans * 1000) % 1000000;
		tv_irt.tv_sec = (ev->init_retrans * 1000) / 1000000;
		tv_irt.tv_usec = (ev->init_retrans * 1000) % 1000000;
		timeval_sub(&tv_rt, rest, &elapsed);
		if (TIMEVAL_LEQ(elapsed, tv_irt))
			timeval_sub(&tv_irt, &elapsed, &timo);
		else
			timo.tv_sec = timo.tv_usec = 0;

		d_printf(LOG_DEBUG, FNAME, "reset timer for %s to %d.%06d",
		    ifp->ifname, (int)timo.tv_sec, (int)timo.tv_usec);

		dhcp6_set_timer(&timo, ev->timer);
	}

	return (0);
}

static struct dhcp6_serverinfo *
find_server(struct dhcp6_event *ev, struct duid *duid)
{
	struct dhcp6_serverinfo *s;

	for (s = ev->servers; s; s = s->next) {
		if (duidcmp(&s->optinfo.serverID, duid) == 0)
			return (s);
	}

	return (NULL);
}

static int
client6_recvreply(struct dhcp6_if *ifp, struct dhcp6 *dh6,
    ssize_t len, struct dhcp6_optinfo *optinfo)
{
	struct dhcp6_listval *lv;
	struct dhcp6_event *ev;
	int state;

	/* find the corresponding event based on the received xid */
	ev = find_event_withid(ifp, ntohl(dh6->dh6_xid) & DH6_XIDMASK);
	if (ev == NULL) {
		d_printf(LOG_INFO, FNAME, "XID mismatch");
		return (-1);
	}

	state = ev->state;
	if (state != DHCP6S_INFOREQ &&
	    state != DHCP6S_REQUEST &&
	    state != DHCP6S_RENEW &&
	    state != DHCP6S_REBIND &&
	    state != DHCP6S_RELEASE &&
	    (state != DHCP6S_SOLICIT ||
	     !(ifp->send_flags & DHCIFF_RAPID_COMMIT))) {
		d_printf(LOG_INFO, FNAME, "unexpected reply");
		return (-1);
	}

	d_printf(LOG_INFO, FNAME, "Received REPLY for %s",
	    dhcp6_event_statestr(ev));

	/* A Reply message must contain a Server ID option */
	if (optinfo->serverID.duid_len == 0) {
		d_printf(LOG_INFO, FNAME, "no server ID option");
		return (-1);
	}

	/*
	 * DUID in the Client ID option (which must be contained for our
	 * client implementation) must match ours.
	 */
	if (optinfo->clientID.duid_len == 0) {
		d_printf(LOG_INFO, FNAME, "no client ID option");
		return (-1);
	}
	if (duidcmp(&optinfo->clientID, &client_duid)) {
		d_printf(LOG_INFO, FNAME, "client DUID mismatch");
		return (-1);
	}

	/* validate authentication */
	if (process_auth(ev->authparam, dh6, len, optinfo)) {
		d_printf(LOG_INFO, FNAME, "failed to process authentication");
		return (-1);
	}

	/*
	 * If the client included a Rapid Commit option in the Solicit message,
	 * the client discards any Reply messages it receives that do not
	 * include a Rapid Commit option.
	 * (should we keep the server otherwise?)
	 * [RFC3315 Section 17.1.4]
	 */
	if (state == DHCP6S_SOLICIT &&
	    (ifp->send_flags & DHCIFF_RAPID_COMMIT) &&
	    !optinfo->rapidcommit) {
		d_printf(LOG_INFO, FNAME, "no rapid commit");
		return (-1);
	}

	/*
	 * The client MAY choose to report any status code or message from the
	 * status code option in the Reply message.
	 * [RFC3315 Section 18.1.8]
	 */
	for (lv = TAILQ_FIRST(&optinfo->stcode_list); lv;
	     lv = TAILQ_NEXT(lv, link)) {
		d_printf(LOG_INFO, FNAME, "status code: %s",
		    dhcp6_stcodestr(lv->val_num16));

		if ((state == DHCP6S_RENEW || state == DHCP6S_REBIND) &&
		    lv->val_num16 == DH6OPT_STCODE_NOBINDING) {
			/* server has no memory of our lease :( */
			client6_reset(ifp);
			return (0);
		}

		/* XXX is it wise to continue with an error in the reply? */
	}

	if (!TAILQ_EMPTY(&optinfo->dns_list)) {
		struct dhcp6_listval *d;
		int i = 0;

		for (d = TAILQ_FIRST(&optinfo->dns_list); d;
		     d = TAILQ_NEXT(d, link), i++) {
			info_printf("nameserver[%d] %s",
			    i, in6addr2str(&d->val_addr6, 0));
		}
	}

	if (!TAILQ_EMPTY(&optinfo->dnsname_list)) {
		struct dhcp6_listval *d;
		int i = 0;

		for (d = TAILQ_FIRST(&optinfo->dnsname_list); d;
		     d = TAILQ_NEXT(d, link), i++) {
			info_printf("Domain search list[%d] %s",
			    i, d->val_vbuf.dv_buf);
		}
	}

	if (!TAILQ_EMPTY(&optinfo->ntp_list)) {
		struct dhcp6_listval *d;
		int i = 0;

		for (d = TAILQ_FIRST(&optinfo->ntp_list); d;
		     d = TAILQ_NEXT(d, link), i++) {
			info_printf("NTP server[%d] %s",
			    i, in6addr2str(&d->val_addr6, 0));
		}
	}

	if (!TAILQ_EMPTY(&optinfo->sip_list)) {
		struct dhcp6_listval *d;
		int i = 0;

		for (d = TAILQ_FIRST(&optinfo->sip_list); d;
		     d = TAILQ_NEXT(d, link), i++) {
			info_printf("SIP server address[%d] %s",
			    i, in6addr2str(&d->val_addr6, 0));
		}
	}

	if (!TAILQ_EMPTY(&optinfo->sipname_list)) {
		struct dhcp6_listval *d;
		int i = 0;

		for (d = TAILQ_FIRST(&optinfo->sipname_list); d;
		     d = TAILQ_NEXT(d, link), i++) {
			info_printf("SIP domain name[%d] %s",
			    i, d->val_vbuf.dv_buf);
		}
	}

	/*
	 * Set refresh timer for configuration information specified in
	 * information-request.  If the timer value is specified by the server
	 * in an information refresh time option, use it; use the protocol
	 * default otherwise.
	 */
	if (state == DHCP6S_INFOREQ) {
		int64_t refreshtime = DHCP6_IRT_DEFAULT;

		if (optinfo->refreshtime != DH6OPT_REFRESHTIME_UNDEF)
			refreshtime = optinfo->refreshtime;

		ifp->timer = dhcp6_add_timer(client6_expire_refreshtime, ifp);
		if (ifp->timer == NULL) {
			d_printf(LOG_WARNING, FNAME,
			    "failed to add timer for refresh time");
		} else {
			struct timeval tv;

			tv.tv_sec = (long)refreshtime;
			tv.tv_usec = 0;

			if (tv.tv_sec < 0) {
				/*
				 * XXX: tv_sec can overflow for an
				 * unsigned 32bit value.
				 */
				d_printf(LOG_WARNING, FNAME,
				    "refresh time is too large: %lu",
				    (uint32_t)refreshtime);
				tv.tv_sec = 0x7fffffff;	/* XXX */
			}

			dhcp6_set_timer(&tv, ifp->timer);
		}
	} else if (optinfo->refreshtime != DH6OPT_REFRESHTIME_UNDEF) {
		/*
		 * draft-ietf-dhc-lifetime-02 clarifies that refresh time
		 * is only used for information-request and reply exchanges.
		 */
		d_printf(LOG_INFO, FNAME,
		    "unexpected information refresh time option (ignored)");
	}

	/* update stateful configuration information */
	if (state != DHCP6S_RELEASE) {
		update_ia(IATYPE_PD, &optinfo->iapd_list, ifp,
		    &optinfo->serverID, ev->authparam);
		update_ia(IATYPE_NA, &optinfo->iana_list, ifp,
		    &optinfo->serverID, ev->authparam);
	}

	/*
	 * Call the configuration script, if specified, to handle various
	 * configuration parameters.
	 */
	client6_script(ifp->scriptpath, state, optinfo);

	dhcp6_remove_event(ev);

	if (state == DHCP6S_RELEASE) {
		/*
		 * When the client receives a valid Reply message in response
		 * to a Release message, the client considers the Release event
		 * completed, regardless of the Status Code option(s) returned
		 * by the server.
		 * [RFC3315 Section 18.1.8]
		 */
		check_exit();
	}

	d_printf(LOG_DEBUG, FNAME, "got an expected reply, sleeping.");

	if (infreq_mode) {
		exit_ok = 1;
		free_resources(NULL);
		check_exit();
	}
	return (0);
}

static struct dhcp6_event *
find_event_withid(struct dhcp6_if *ifp, uint32_t xid)
{
	struct dhcp6_event *ev;

	for (ev = TAILQ_FIRST(&ifp->event_list); ev;
	     ev = TAILQ_NEXT(ev, link)) {
		if (ev->xid == xid)
			return (ev);
	}

	return (NULL);
}

static int
process_auth(struct authparam *authparam, struct dhcp6 *dh6,
    ssize_t len, struct dhcp6_optinfo *optinfo)
{
	struct keyinfo *key = NULL;
	int authenticated = 0;

	switch (optinfo->authproto) {
	case DHCP6_AUTHPROTO_UNDEF:
		/* server did not provide authentication option */
		break;
	case DHCP6_AUTHPROTO_DELAYED:
		if ((optinfo->authflags & DHCP6OPT_AUTHFLAG_NOINFO)) {
			d_printf(LOG_INFO, FNAME, "server did not include "
			    "authentication information");
			break;
		}

		if (optinfo->authalgorithm != DHCP6_AUTHALG_HMACMD5) {
			d_printf(LOG_INFO, FNAME, "unknown authentication "
			    "algorithm (%d)", optinfo->authalgorithm);
			break;
		}

		if (optinfo->authrdm != DHCP6_AUTHRDM_MONOCOUNTER) {
			d_printf(LOG_INFO, FNAME,"unknown RDM (%d)",
			    optinfo->authrdm);
			break;
		}

		/*
		 * Replay protection.  If we do not know the previous RD value,
		 * we accept the message anyway (XXX).
		 */
		if ((authparam->flags & AUTHPARAM_FLAGS_NOPREVRD)) {
			d_printf(LOG_WARNING, FNAME, "previous RD value is "
			    "unknown (accept it)");
		} else {
			if (dhcp6_auth_replaycheck(optinfo->authrdm,
			    authparam->prevrd, optinfo->authrd)) {
				d_printf(LOG_INFO, FNAME,
				    "possible replay attack detected");
				break;
			}
		}

		/* identify the secret key */
		if ((key = authparam->key) != NULL) {
			/*
			 * If we already know a key, its identification should
			 * match that contained in the received option.
			 * (from Section 21.4.5.1 of RFC3315)
			 */
			if (optinfo->delayedauth_keyid != key->keyid ||
			    (size_t)optinfo->delayedauth_realmlen != key->realmlen ||
			    memcmp(optinfo->delayedauth_realmval, key->realm,
			    key->realmlen) != 0) {
				d_printf(LOG_INFO, FNAME,
				    "authentication key mismatch");
				break;
			}
		} else {
			key = find_key(optinfo->delayedauth_realmval,
			    optinfo->delayedauth_realmlen,
			    optinfo->delayedauth_keyid);
			if (key == NULL) {
				d_printf(LOG_INFO, FNAME, "failed to find key "
				    "provided by the server (ID: %x)",
				    optinfo->delayedauth_keyid);
				break;
			} else {
				d_printf(LOG_DEBUG, FNAME, "found key for "
				    "authentication: %s", key->name);
			}
			authparam->key = key;
		}

		/* check for the key lifetime */
		if (dhcp6_validate_key(key)) {
			d_printf(LOG_INFO, FNAME, "key %s has expired",
			    key->name);
			break;
		}

		/* validate MAC */
		if (dhcp6_verify_mac((unsigned char *)dh6, len, optinfo->authproto,
		    optinfo->authalgorithm,
		    optinfo->delayedauth_offset + sizeof(*dh6), key) == 0) {
			d_printf(LOG_DEBUG, FNAME, "message authentication "
			    "validated");
			authenticated = 1;
		} else {
			d_printf(LOG_INFO, FNAME, "invalid message "
			    "authentication");
		}

		break;
	default:
		d_printf(LOG_INFO, FNAME, "server sent unsupported "
		    "authentication protocol (%d)", optinfo->authproto);
		break;
	}

	if (authenticated == 0) {
		if (authparam->authproto != DHCP6_AUTHPROTO_UNDEF) {
			d_printf(LOG_INFO, FNAME, "message not authenticated "
			    "while authentication required");

			/*
			 * Right now, we simply discard unauthenticated
			 * messages.
			 */
			return (-1);
		}
	} else {
		/* if authenticated, update the "previous" RD value */
		authparam->prevrd = optinfo->authrd;
		authparam->flags &= ~AUTHPARAM_FLAGS_NOPREVRD;
	}

	return (0);
}

static int
set_auth(struct dhcp6_event *ev, struct dhcp6_optinfo *optinfo)
{
	struct authparam *authparam = ev->authparam;

	if (authparam == NULL)
		return (0);

	optinfo->authproto = authparam->authproto;
	optinfo->authalgorithm = authparam->authalgorithm;
	optinfo->authrdm = authparam->authrdm;

	switch (authparam->authproto) {
	case DHCP6_AUTHPROTO_UNDEF: /* we simply do not need authentication */
		return (0);
	case DHCP6_AUTHPROTO_DELAYED:
		if (ev->state == DHCP6S_INFOREQ) {
			/*
			 * In the current implementation, delayed
			 * authentication for Information-request and Reply
			 * exchanges doesn't work.  Specification is also
			 * unclear on this usage.
			 */
			d_printf(LOG_WARNING, FNAME, "delayed authentication "
			    "cannot be used for Information-request yet");
			return (-1);
		}

		if (ev->state == DHCP6S_SOLICIT) {
			optinfo->authflags |= DHCP6OPT_AUTHFLAG_NOINFO;
			return (0); /* no auth information is needed */
		}

		if (authparam->key == NULL) {
			d_printf(LOG_INFO, FNAME,
			    "no authentication key for %s",
			    dhcp6_event_statestr(ev));
			return (-1);
		}

		if (dhcp6_validate_key(authparam->key)) {
			d_printf(LOG_INFO, FNAME, "key %s is invalid",
			    authparam->key->name);
			return (-1);
		}

		if (get_rdvalue(optinfo->authrdm, &optinfo->authrd,
		    sizeof(optinfo->authrd))) {
			d_printf(LOG_ERR, FNAME, "failed to get a replay "
			    "detection value");
			return (-1);
		}

		optinfo->delayedauth_keyid = authparam->key->keyid;
		optinfo->delayedauth_realmlen = authparam->key->realmlen;
		optinfo->delayedauth_realmval =
		    malloc(optinfo->delayedauth_realmlen);
		if (optinfo->delayedauth_realmval == NULL) {
			d_printf(LOG_ERR, FNAME, "failed to allocate memory "
			    "for authentication realm");
			return (-1);
		}
		memcpy(optinfo->delayedauth_realmval, authparam->key->realm,
		    optinfo->delayedauth_realmlen);

		break;
	default:
		d_printf(LOG_ERR, FNAME, "unsupported authentication protocol "
		    "%d", authparam->authproto);
		return (-1);
	}

	return (0);
}

static void
info_printf(const char *fmt, ...)
{
	va_list ap;
	char logbuf[LINE_MAX];

	va_start(ap, fmt);
	vsnprintf(logbuf, sizeof(logbuf), fmt, ap);

	d_printf(LOG_DEBUG, FNAME, "%s", logbuf);
	if (infreq_mode)
		printf("%s\n", logbuf);

	return;
}
