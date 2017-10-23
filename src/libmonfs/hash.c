/** This file is part of MonFS. **
 * 
 * MonFS: File system for monitoring file I/O operations
 * 
 * Copyright (C) 2010 Hitoshi Sato <hitoshi.sato@gmail.com>
 * 
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
 * This program can be distributed under the terms of the GNU GPL.
 * See the file COPYING.
 */

#include <stddef.h>
#include <stdlib.h>
#include <string.h>
#include <monfs.h>

#define ALIGNMENT 16
#define ALIGN(p) (((unsigned long)(p) + ALIGNMENT - 1) & ~(ALIGNMENT - 1))

int
hash_default(const void *key, int keylen)
{
	int i;
	unsigned int hash = 0, g;

	for (i = 0; i < keylen; i++) {
		hash = (hash << 4) + ((unsigned char *)key)[i];
		/* this assumes size of `int` is 32 bit */
		if (( g = hash & 0xf0000000) != 0) {
			hash ^= g >> 24;
			hash ^= g;
		}
	}

	return (hash);
}

int
hash_key_equal_default(const void *key1, int key1len, const void *key2, int key2len)
{
	return (key1len == key2len && memcmp(key1, key2, key1len) == 0);
}

struct hash_entry {
	struct hash_entry *next;
	int key_length;
	int data_length;
	double key_stub;
};

#define HASH_KEY(entry) \
	(((char *)(entry)) + \
	ALIGN(offsetof(struct hash_entry, key_stub)))

#define HASH_DATA(entry) \
	(HASH_KEY(entry) + ALIGN((entry)->key_length))

struct hash_table {
	int table_size;

	int (*hash)(const void *, int);
	int (*equal)(const void *, int, const void *, int);

	struct hash_entry *buckets[1];
};

static size_t
size_mul(int *overflowp, size_t a, size_t b)
{
	size_t product = a * b;

	if (b != 0 && product / b != a)
		*overflowp = 1;

	return product;
}

static size_t
size_add(int *overflowp, size_t a, size_t b)
{
	size_t sum = a + b;

	if (sum < a)
		*overflowp = 1;
	return sum;
}

struct hash_table *
hash_table_alloc(int size,
		int (*hash)(const void *, int),
		int (*equal)(const void *, int, const void *, int))
{
	struct hash_table *ht;
	size_t alloc_size;
	int overflow = 0;

	alloc_size = size_add(&overflow,
			sizeof(struct hash_table),
			size_mul(&overflow,
					sizeof(struct hash_entry *), size - 1));
	if (overflow)
		return NULL;

	ht = malloc(alloc_size);
	if (ht == NULL)
		return NULL;

	ht->table_size = size;
	ht->hash = hash;
	ht->equal = equal;
	memset(ht->buckets, 0, sizeof(struct hash_entry *) * size);
	return (ht);
}

void
hash_table_free(struct hash_table *ht)
{
	int i;
	struct hash_entry *p, *np;

	for (i = 0; i < ht->table_size; i++) {
		for (p = ht->buckets[i]; p != NULL; p = np) {
			np = p->next;
			free(p);
		}
	}
	free(ht);
}

static struct hash_entry **
hash_lookup_internal_search(struct hash_table *ht, struct hash_entry **pp, const void *key, int keylen)
{
	struct hash_entry *p;
	int (*equal)(const void *, int, const void *, int) = ht->equal;

	for (p = *pp; p != NULL; pp = &p->next, p = *pp) {
		if ((*equal)(HASH_KEY(p), p->key_length, key, keylen))
			break;
	}


	return pp;
}

#define HASH_BUCKET(ht, key, keylen) \
	((*ht->hash)(key, keylen) % ht->table_size)

#define HASH_LOOKUP_INTERNAL(ht, key, keylen) \
	hash_lookup_internal_search(ht, \
			&ht->buckets[HASH_BUCKET(ht, key, keylen)], \
			key, keylen)

struct hash_entry *
hash_lookup(struct hash_table *ht, const void *key, int keylen)
{
	struct hash_entry **pp =
		HASH_LOOKUP_INTERNAL(ht, key, keylen);

	return *pp;
}

struct hash_entry *
hash_enter(struct hash_table *ht, const void *key, int keylen, int datalen, int *createdp)
{
	struct hash_entry *p, **pp =
		HASH_LOOKUP_INTERNAL(ht, key, keylen);
	size_t hash_entry_size;
	int overflow = 0;

	if (createdp != NULL)
		*createdp = 0;

	if (*pp != NULL)
		return *pp;

	/*
	 * create if not found
	 */
	hash_entry_size =
		size_add(&overflow,
			size_add(&overflow,
					ALIGN(offsetof(struct hash_entry, key_stub)),
					ALIGN(keylen)),
				datalen);
	if (overflow)
		return NULL;

	p = malloc(hash_entry_size); /* size is already cheecked */
	if (p == NULL)
		return NULL;

	*pp = p;

	p->next = NULL;
	p->key_length = keylen;
	p->data_length = datalen;
	memcpy(HASH_KEY(p), key, keylen);

	if (createdp != NULL)
		*createdp = 1;
	return p;
}

int
hash_purge(struct hash_table *ht, const void *key, int keylen)
{
	struct hash_entry *p, **pp =
		HASH_LOOKUP_INTERNAL(ht, key, keylen);

	p = *pp;
	if (p == NULL)
		return (0); /* key is not found */
	*pp = p->next;
	free(p);
	return 1;
}

void *
hash_entry_key(struct hash_entry *entry)
{
	return (HASH_KEY(entry));
}

int
hash_entry_key_length(struct hash_entry *entry)
{
	return (entry->key_length);
}

void *
hash_entry_data(struct hash_entry *entry)
{
	return (HASH_DATA(entry));
}

int
hash_entry_data_length(struct hash_entry *entry)
{
	return (entry->data_length);
}
