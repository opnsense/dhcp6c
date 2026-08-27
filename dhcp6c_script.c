/*
 * Copyright (C) 2003 WIDE Project.
 * Copyright (C) 2023-2026 Franco Fichtner <franco@opnsense.org>
 * All rights reserved.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions
 * are met:
 *
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
#include <sys/socket.h>
#include <sys/queue.h>
#include <sys/wait.h>
#include <sys/stat.h>

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

#include <netinet/in.h>

#include <fcntl.h>
#include <unistd.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <syslog.h>
#include <errno.h>

#include "dhcp6.h"
#include "config.h"
#include "dhcp6c.h"
#include "common.h"

#define DECLARE_LEN(lname)	int lname##_len = 0;
#define PDINFO_MAX		64

#define RENDER_IFNAME(lstr)	do { \
	char interface[PDINFO_MAX]; \
	snprintf(interface, sizeof(interface), "%s=%s", lstr, ifname); \
	if ((envp[i++] = strdup(interface)) == NULL) { \
		d_printf(LOG_NOTICE, FNAME, \
		    "failed to allocate interface string"); \
		ret = -1; \
		goto clean; \
	} \
} while (0)

#define RENDER_REASON(lstr)	do { \
	struct dhcp6_event ev; \
	char reason[PDINFO_MAX]; \
	ev.state = state; \
	snprintf(reason, sizeof(reason), "%s=%s", lstr, \
	    dhcp6_event_statestr(&ev)); \
	if ((envp[i++] = strdup(reason)) == NULL) { \
		d_printf(LOG_NOTICE, FNAME, \
		    "failed to allocate reason string"); \
		ret = -1; \
		goto clean; \
	} \
} while (0)

#define COUNT_LIST(lname, lip)	do { \
	struct dhcp6_listval *v; \
	TAILQ_FOREACH(v, &optinfo->lname##_list, link) { \
		/* one space separator plus address length or as is */ \
		lname##_len += 1 + ((lip) ? INET6_ADDRSTRLEN : v->val_vbuf.dv_len); \
	} \
	envc += lname##_len ? 1 : 0; \
} while (0)

#define COUNT_ADDR(lname)	COUNT_LIST(lname, 1)
#define COUNT_NAME(lname)	COUNT_LIST(lname, 0)

#define RENDER_LIST(lstr, lname, lip)	do { \
	if (lname##_len) { \
		/* "var=" + null char for termination */ \
		int slen = sizeof(lstr) + 2 + lname##_len; \
		struct dhcp6_listval *v; \
		char *sptr; \
		if ((sptr = envp[i++] = malloc(slen)) == NULL) { \
			d_printf(LOG_NOTICE, FNAME, \
			    "failed to allocate strings for %s", lstr); \
			ret = -1; \
			goto clean; \
		} \
		memset(sptr, 0, slen); \
		snprintf(sptr, slen, "%s=", lstr); \
		TAILQ_FOREACH(v, &optinfo->lname##_list, link) { \
			strlcat(sptr, (lip) ? in6addr2str(&v->val_addr6, 0) : \
			    v->val_vbuf.dv_buf, slen); \
			strlcat(sptr, " ", slen); \
		} \
	} \
} while (0)

#define RENDER_ADDR(lstr, lname)	RENDER_LIST(lstr, lname, 1)
#define RENDER_NAME(lstr, lname)	RENDER_LIST(lstr, lname, 0)

#define COUNT_IA(lname, ltype)	do { \
	struct dhcp6_listval *iav, *siav; \
	TAILQ_FOREACH(iav, &optinfo->lname##_list, link) { \
		TAILQ_FOREACH(siav, &iav->sublist, link) { \
			if (siav->type == (ltype)) { \
				/* one space separator plus prefix length max */ \
				lname##_len += 1 + PDINFO_MAX; \
			} \
		} \
	} \
	envc += lname##_len ? 1 : 0; \
} while (0)

#define COUNT_IANA(lname)	COUNT_IA(lname, DHCP6_LISTVAL_STATEFULADDR6)
#define COUNT_IAPD(lname)	COUNT_IA(lname, DHCP6_LISTVAL_PREFIX6)

#define RENDER_IA(lstr, lname, ltype)	do { \
	if (lname##_len) { \
		struct dhcp6_listval *iav, *siav; \
		char prefixinfo[PDINFO_MAX]; \
		char *sptr; \
		int slen = sizeof(lstr) + 2 + lname##_len; \
		if ((sptr = envp[i++] = malloc(slen)) == NULL) { \
			d_printf(LOG_NOTICE, FNAME, \
			    "failed to allocate prefixinfo strings"); \
			ret = -1; \
			goto clean; \
		} \
		memset(sptr, 0, slen); \
		snprintf(sptr, slen, "%s=", lstr); \
		TAILQ_FOREACH(iav, &optinfo->lname##_list, link) { \
			TAILQ_FOREACH(siav, &iav->sublist, link) { \
				if (siav->type != (ltype)) { \
					continue; \
				} \
				if (siav->type == DHCP6_LISTVAL_PREFIX6) { \
					snprintf(prefixinfo, sizeof(prefixinfo), \
					    "%s/%d", in6addr2str(&siav->val_prefix6.addr, 0), \
					    siav->val_prefix6.plen); \
					strlcat(sptr, prefixinfo, slen); \
					strlcat(sptr, " ", slen); \
				} else if (siav->type == DHCP6_LISTVAL_STATEFULADDR6) { \
					snprintf(prefixinfo, sizeof(prefixinfo), \
					    "%s/128", in6addr2str(&siav->val_statefuladdr6.addr, 0)); \
					strlcat(sptr, prefixinfo, slen); \
					strlcat(sptr, " ", slen); \
				} \
			} \
		} \
	} \
} while (0)

#define RENDER_IANA(lstr, lname)	RENDER_IA(lstr, lname, DHCP6_LISTVAL_STATEFULADDR6)
#define RENDER_IAPD(lstr, lname)	RENDER_IA(lstr, lname, DHCP6_LISTVAL_PREFIX6)

#define COUNT_RAWOPT(lname)	do { \
	struct rawoption *rawopt; \
	TAILQ_FOREACH(rawopt, &optinfo->lname##_list, link) { \
		lname##_len += 1; \
	} \
	envc += lname##_len; \
} while (0)

#define RENDER_RAWOPT(lstr, lname)	do { \
	if (lname##_len) { \
		const char *hex = "0123456789abcdef"; \
		struct rawoption *rawopt; \
		char val[3]; \
		int o; \
		TAILQ_FOREACH(rawopt, &optinfo->lname##_list, link) { \
			/* \
			 * max of 5 numbers after last underscore \
			 * (seems like max DHCPv6 option could be 65535) \
			 * then underscore, equal sign and null char plus \
			 * hex signs of each byte \
			 */ \
			int slen = sizeof(lstr) + 5 + 3 + (rawopt->datalen * 2); \
			char *sptr; \
			if ((sptr = envp[i++] = malloc(slen)) == NULL) { \
				d_printf(LOG_NOTICE, FNAME, \
				    "failed to allocate string for DHCPv6 option %d", \
				    rawopt->opnum); \
				ret = -1; \
				goto clean; \
			} \
			/* make raw options available as lstr_abcde=hexresponse */ \
			/* XXX creates and overwrites %d for repeated options */ \
			snprintf(sptr, slen, "%s_%d=", lstr, rawopt->opnum); \
			for (o = 0; o < rawopt->datalen; o++) { \
				val[0] = hex[(rawopt->data[o]>>4) & 0x0F]; \
				val[1] = hex[(rawopt->data[o]   ) & 0x0F]; \
				val[2] = 0x00; \
				strlcat(sptr, val, slen); \
			} \
		} \
	} \
} while (0)

int
client6_script(char *scriptpath, const char *ifname, int state,
    struct dhcp6_optinfo *optinfo)
{
	/* we at least include the interface, reason and the terminator */
	int envc = 3;
	int i, ret = 0;
	char **envp;
	pid_t pid, wpid;

	DECLARE_LEN(iana);
	DECLARE_LEN(iapd);
	DECLARE_LEN(dns);
	DECLARE_LEN(dnsname);
	DECLARE_LEN(ntp);
	DECLARE_LEN(sip);
	DECLARE_LEN(sipname);
	DECLARE_LEN(nis);
	DECLARE_LEN(nisname);
	DECLARE_LEN(nisp);
	DECLARE_LEN(nispname);
	DECLARE_LEN(bcmcs);
	DECLARE_LEN(bcmcsname);
	DECLARE_LEN(aftrname);
	DECLARE_LEN(rawopt);

	/* if a script is not specified, do nothing */
	if (scriptpath == NULL || strlen(scriptpath) == 0) {
		return (-1);
	}

	d_printf(LOG_DEBUG, FNAME, "executes %s", scriptpath);

	/* full environment only during runtime */
	if (state != DHCP6S_EXIT) {
		COUNT_IANA(iana);
		COUNT_IAPD(iapd);
		COUNT_ADDR(dns);
		COUNT_NAME(dnsname);
		COUNT_ADDR(ntp);
		COUNT_ADDR(sip);
		COUNT_NAME(sipname);
		COUNT_ADDR(nis);
		COUNT_NAME(nisname);
		COUNT_ADDR(nisp);
		COUNT_NAME(nispname);
		COUNT_ADDR(bcmcs);
		COUNT_NAME(bcmcsname);
		COUNT_NAME(aftrname);
		COUNT_RAWOPT(rawopt);
	}

	/* allocate an environments array and fill it */
	if ((envp = malloc(sizeof (char *) * envc)) == NULL) {
		d_printf(LOG_NOTICE, FNAME,
		    "failed to allocate environment buffer");
		return (-1);
	}
	memset(envp, 0, sizeof(char *) * envc);
	i = 0;

	RENDER_IFNAME("IFNAME");
	RENDER_REASON("REASON");
	RENDER_IANA("NAINFO", iana);
	RENDER_IAPD("PDINFO", iapd);
	RENDER_ADDR("new_domain_name_servers", dns);
	RENDER_NAME("new_domain_name", dnsname);
	RENDER_ADDR("new_ntp_servers", ntp);
	RENDER_ADDR("new_sip_servers", sip);
	RENDER_NAME("new_sip_name", sipname);
	RENDER_ADDR("new_nis_servers", nis);
	RENDER_NAME("new_nis_name", nisname);
	RENDER_ADDR("new_nisp_servers", nisp);
	RENDER_NAME("new_nisp_name", nispname);
	RENDER_ADDR("new_bcmcs_servers", bcmcs);
	RENDER_NAME("new_bcmcs_name", bcmcsname);
	RENDER_NAME("new_aftr_name", aftrname);
	RENDER_RAWOPT("new_raw_option", rawopt); /* XXX does not handle option duplication */

	/* launch the script */
	pid = fork();
	if (pid < 0) {
		d_printf(LOG_ERR, FNAME, "failed to fork: %s", strerror(errno));
		ret = -1;
		goto clean;
	} else if (pid) {
		int wstatus;

		do {
			wpid = wait(&wstatus);
		} while (wpid != pid && wpid > 0);

		if (wpid < 0)
			d_printf(LOG_ERR, FNAME, "wait: %s", strerror(errno));
		else {
			d_printf(LOG_DEBUG, FNAME,
			    "script \"%s\" terminated", scriptpath);
		}
	} else {
		char *argv[2];
		int fd;

		argv[0] = scriptpath;
		argv[1] = NULL;

		if (safefile(scriptpath)) {
			d_printf(LOG_ERR, FNAME,
			    "script \"%s\" cannot be executed safely",
			    scriptpath);
			exit(1);
		}

		if (foreground == 0 && (fd = open("/dev/null", O_RDWR)) != -1) {
			dup2(fd, STDIN_FILENO);
			dup2(fd, STDOUT_FILENO);
			dup2(fd, STDERR_FILENO);
			if (fd > STDERR_FILENO)
				close(fd);
		}

		execve(scriptpath, argv, envp);

		d_printf(LOG_ERR, FNAME, "child: exec failed: %s",
		    strerror(errno));
		exit(0);
	}

clean:
	for (i = 0; i < envc; i++) {
		free(envp[i]);
	}
	free(envp);

	return (ret);
}
