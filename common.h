/*	$KAME: common.h,v 1.42 2005/09/16 11:30:13 suz Exp $	*/
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

#ifndef	_COMMON_H_
#define	_COMMON_H_

#ifdef __KAME__
#define IN6_IFF_INVALID (IN6_IFF_ANYCAST|IN6_IFF_TENTATIVE|\
		IN6_IFF_DUPLICATED|IN6_IFF_DETACHED)
#else
#define IN6_IFF_INVALID (0)
#endif

#ifdef HAVE_ANSI_FUNC
#define FNAME __func__
#elif defined (HAVE_GCC_FUNCTION)
#define FNAME __FUNCTION__
#else
#define FNAME ""
#endif

/* XXX: bsdi4 does not have TAILQ_EMPTY */
#ifndef TAILQ_EMPTY
#define	TAILQ_EMPTY(head) ((head)->tqh_first == NULL)
#endif

/* and linux *_FIRST and *_NEXT */
#ifndef LIST_EMPTY
#define	LIST_EMPTY(head)	((head)->lh_first == NULL)
#endif
#ifndef LIST_FIRST
#define	LIST_FIRST(head)	((head)->lh_first)
#endif
#ifndef LIST_NEXT
#define	LIST_NEXT(elm, field)	((elm)->field.le_next)
#endif
#ifndef LIST_FOREACH
#define	LIST_FOREACH(var, head, field)					\
	for ((var) = LIST_FIRST((head));				\
	    (var);							\
	    (var) = LIST_NEXT((var), field))
#endif
#ifndef TAILQ_FIRST
#define	TAILQ_FIRST(head)	((head)->tqh_first)
#endif
#ifndef TAILQ_LAST
#define	TAILQ_LAST(head, headname)					\
	(*(((struct headname *)((head)->tqh_last))->tqh_last))
#endif
#ifndef TAILQ_PREV
#define	TAILQ_PREV(elm, headname, field)				\
	(*(((struct headname *)((elm)->field.tqe_prev))->tqh_last))
#endif
#ifndef TAILQ_NEXT
#define	TAILQ_NEXT(elm, field) ((elm)->field.tqe_next)
#endif
#ifndef TAILQ_FOREACH
#define	TAILQ_FOREACH(var, head, field)					\
	for ((var) = TAILQ_FIRST((head));				\
	    (var);							\
	    (var) = TAILQ_NEXT((var), field))
#endif
#ifdef HAVE_TAILQ_FOREACH_REVERSE_OLD
#undef TAILQ_FOREACH_REVERSE
#endif
#ifndef TAILQ_FOREACH_REVERSE
#define	TAILQ_FOREACH_REVERSE(var, head, headname, field)		\
	for ((var) = TAILQ_LAST((head), headname);			\
	    (var);							\
	    (var) = TAILQ_PREV((var), headname, field))
#endif


#ifndef SO_REUSEPORT
#define SO_REUSEPORT SO_REUSEADDR
#endif

/* s*_len stuff */
static __inline uint8_t
sysdep_sa_len (const struct sockaddr *sa)
{
#ifndef HAVE_SA_LEN
  switch (sa->sa_family)
    {
    case AF_INET:
      return sizeof (struct sockaddr_in);
    case AF_INET6:
      return sizeof (struct sockaddr_in6);
    }
  return sizeof (struct sockaddr_in);
#else
  return sa->sa_len;
#endif
}

extern int foreground;
extern int debug_thresh;
extern char *device;
extern int opt_norelease;

/* search option for dhcp6_find_listval() */
#define MATCHLIST_PREFIXLEN 0x1

/* common.c */
typedef enum { IFADDRCONF_ADD, IFADDRCONF_REMOVE } ifaddrconf_cmd_t;
int rawop_copy_list(struct rawop_list *, struct rawop_list *);
void rawop_clear_list(struct rawop_list *);
int dhcp6_copy_list(struct dhcp6_list *, struct dhcp6_list *);
void dhcp6_move_list(struct dhcp6_list *, struct dhcp6_list *);
void dhcp6_clear_list(struct dhcp6_list *);
void dhcp6_clear_listval(struct dhcp6_listval *);
struct dhcp6_listval *dhcp6_find_listval(struct dhcp6_list *,
    dhcp6_listval_type_t, void *, int);
struct dhcp6_listval *dhcp6_add_listval(struct dhcp6_list *,
    dhcp6_listval_type_t, void *, struct dhcp6_list *);
int dhcp6_vbuf_copy(struct dhcp6_vbuf *, struct dhcp6_vbuf *);
void dhcp6_vbuf_free(struct dhcp6_vbuf *);
int dhcp6_vbuf_cmp(struct dhcp6_vbuf *, struct dhcp6_vbuf *);
struct dhcp6_event *dhcp6_create_event(struct dhcp6_if *, int);
void dhcp6_remove_event(struct dhcp6_event *);
void dhcp6_remove_evdata(struct dhcp6_event *);
struct authparam *new_authparam(int, int, int);
struct authparam *copy_authparam(struct authparam *);
int dhcp6_auth_replaycheck(int, uint64_t, uint64_t);
int getifaddr(struct in6_addr *, char *, struct in6_addr *,
			  int, int, int);
int getifidfromaddr(struct in6_addr *, unsigned int *);
int transmit_sa(int, struct sockaddr *, char *, size_t);
long random_between(long, long);
int prefix6_mask(struct in6_addr *, int);
int sa6_plen2mask(struct sockaddr_in6 *, int);
char *addr2str(struct sockaddr *);
char *in6addr2str(struct in6_addr *, int);
int in6_addrscopebyif(struct in6_addr *, char *);
int in6_scope(struct in6_addr *);
void setloglevel(int);
void d_printf(int, const char *, const char *, ...);
int get_duid(const char *, struct duid *);
void dhcp6_init_options(struct dhcp6_optinfo *);
void dhcp6_clear_options(struct dhcp6_optinfo *);
int dhcp6_copy_options(struct dhcp6_optinfo *,
				   struct dhcp6_optinfo *);
int dhcp6_get_options(struct dhcp6opt *, struct dhcp6opt *,
				  struct dhcp6_optinfo *);
int dhcp6_set_options(int, struct dhcp6opt *, struct dhcp6opt *,
				  struct dhcp6_optinfo *);
void dhcp6_set_timeoparam(struct dhcp6_event *);
void dhcp6_reset_timer(struct dhcp6_event *);
const char *dhcp6optstr(int);
const char *dhcp6msgstr(int);
const char *dhcp6_stcodestr(uint16_t);
char *duidstr(struct duid *);
const char *dhcp6_event_statestr(struct dhcp6_event *);
int get_rdvalue(int, void *, size_t);
int duidcpy(struct duid *, struct duid *);
int duidcmp(struct duid *, struct duid *);
void duidfree(struct duid *);
int ifaddrconf(ifaddrconf_cmd_t, char *, struct sockaddr_in6 *,
			   int, int, int);
int safefile(const char *);

/* missing */
#ifndef HAVE_STRLCAT
size_t strlcat(char *, const char *, size_t);
#endif
#ifndef HAVE_STRLCPY
size_t strlcpy(char *, const char *, size_t);
#endif

int get_val32(char **bpp, int *lenp, uint32_t *valp);
int get_val(char **bpp, int *lenp, void *valp, size_t vallen);

#endif
