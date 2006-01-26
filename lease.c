#include <sys/types.h>
#include <unistd.h>
#include <syslog.h>
#include <stdlib.h>
#include <string.h>
#include <sys/socket.h>
#include <sys/queue.h>
#include <netinet/in.h>
#include "dhcp6.h"
#include "config.h"
#include "common.h"

#ifndef FALSE
#define FALSE 	0
#define TRUE	!FALSE
#endif

struct hash_entry {
	LIST_ENTRY(hash_entry) list;
	char val[];
};

LIST_HEAD(hash_head, hash_entry);

typedef unsigned int (*pfn_hash_t)(void *val) ;
typedef int (*pfh_hash_match_t)(void *val1, void *val2); 

struct hash_table {
	struct hash_head *table;
	unsigned int size;
	pfn_hash_t hash;
	pfh_hash_match_t match;
};

#ifndef DHCP6_LEASE_TABLE_SIZE
#define DHCP6_LEASE_TABLE_SIZE	256
#endif

static struct hash_table dhcp6_lease_table;

static unsigned int in6_addr_hash __P((void *));
static int in6_addr_match __P((void *, void *));

static int hash_table_init __P((struct hash_table *, unsigned int,
				pfn_hash_t, pfh_hash_match_t));
static void hash_table_cleanup __P((struct hash_table *));
static int hash_table_add __P((struct hash_table *, void *, unsigned int));
static int hash_table_remove __P((struct hash_table *, void *));
static struct hash_entry * hash_table_find __P((struct hash_table *, void *));

int
lease_init(void)
{
	dprintf(LOG_DEBUG, FNAME, "called");

	if (hash_table_init(&dhcp6_lease_table, DHCP6_LEASE_TABLE_SIZE,
		in6_addr_hash, in6_addr_match) != 0) {
		return (-1);
	}

	return (0);
}

void
lease_cleanup(void)
{
	hash_table_cleanup(&dhcp6_lease_table);
}

int
lease_address(addr)
	struct in6_addr *addr;
{
	if (!addr)
		return (FALSE);

	dprintf(LOG_DEBUG, FNAME, "addr=%s", in6addr2str(addr, 0));

	if (hash_table_find(&dhcp6_lease_table, addr)) {
		dprintf(LOG_WARNING, FNAME, "already leased: %s",
			in6addr2str(addr, 0));
		return (FALSE);
	}

	if (hash_table_add(&dhcp6_lease_table, addr, sizeof(*addr)) != 0) {
		return (FALSE);
	}

	return (TRUE);
}

void
release_address(addr)
	struct in6_addr *addr;
{
	if (!addr)
		return;

	dprintf(LOG_DEBUG, FNAME, "addr=%s", in6addr2str(addr, 0));

	if (hash_table_remove(&dhcp6_lease_table, addr) != 0) {
		dprintf(LOG_WARNING, FNAME, "not found: %s", in6addr2str(addr, 0));
	}
}

int
is_leased(addr)
	struct in6_addr *addr;
{
	return (hash_table_find(&dhcp6_lease_table, addr) != NULL);
}

static unsigned int
in6_addr_hash(val)
	void *val;
{
	u_int8_t *addr = ((struct in6_addr *)val)->s6_addr;
	unsigned int hash = 0;
	int i;

	for (i = 0; i < 16; i++) {
		hash += addr[i];
	}

	return (hash);
}

static int
in6_addr_match(val1, val2)
	void *val1, *val2;
{
	struct in6_addr * addr1 = val1;
	struct in6_addr * addr2 = val2;

	return (memcmp(addr1->s6_addr, addr2->s6_addr, 16) == 0);
}

/*
 * hash table
 */
static int
hash_table_init(table, size, hash, match)
	struct hash_table *table; 
	unsigned int size;
	pfn_hash_t hash;
	pfh_hash_match_t match;
{
	int i;

	if (!table || !hash || !match) {
		return (-1);
	}

	if ((table->table = malloc(sizeof(*table->table) * size)) == NULL) {
		return (-1);
	}

	for (i = 0; i < size; i++)
		LIST_INIT(&table->table[i]);

	table->size = size;
	table->hash = hash;
	table->match = match;

	return (0);
}

static void
hash_table_cleanup(table)
	struct hash_table *table; 
{
	int i;

	if (!table) {
		return;
	}

	for (i = 0; i < table->size; i++) {
		while (!LIST_EMPTY(&table->table[i])) {
			struct hash_entry *entry = LIST_FIRST(&table->table[i]);
			LIST_REMOVE(entry, list);
			free(entry);
		}
	}
	free(table->table);
	memset(table, 0, sizeof(*table));
}

static int
hash_table_add(table, val, size)
	struct hash_table *table; 
	void *val;
	unsigned int size;
{
	struct hash_entry *entry = NULL;
	int i = 0;

	if (!table || !val) {
		return (-1);
	}

	if ((entry = malloc(sizeof(*entry) + size)) == NULL) {
		return (-1);
	}
	memset(entry, 0, sizeof(*entry));
	memcpy(entry->val, val, size);

	i = table->hash(val) % table->size;
	LIST_INSERT_HEAD(&table->table[i], entry, list);

	return (0);
}

static int
hash_table_remove(table, val)
	struct hash_table *table; 
	void *val;
{
	struct hash_entry *entry;

	if (!table || !val) {
		return (-1);
	}

	if ((entry = hash_table_find(table, val)) == NULL) {
		return (-1);
	}

	LIST_REMOVE(entry, list);
	free(entry);

	return (0);
}

static struct hash_entry *
hash_table_find(table, val)
	struct hash_table *table; 
	void *val;
{
	struct hash_entry *entry;
	int i;

	if (!table || !val) {
		return (NULL);
	}

	i = table->hash(val) % table->size;
	LIST_FOREACH(entry, &table->table[i], list)
	{
		if (table->match(val, entry->val)) {
			return (entry);
		}
	}

	return (NULL);
}

