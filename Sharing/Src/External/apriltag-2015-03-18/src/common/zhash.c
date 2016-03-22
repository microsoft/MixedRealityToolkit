/* (C) 2013-2015, The Regents of The University of Michigan
All rights reserved.

This software may be available under alternative licensing
terms. Contact Edwin Olson, ebolson@umich.edu, for more information.

   Redistribution and use in source and binary forms, with or without
modification, are permitted provided that the following conditions are met:

1. Redistributions of source code must retain the above copyright notice, this
   list of conditions and the following disclaimer.
2. Redistributions in binary form must reproduce the above copyright notice,
   this list of conditions and the following disclaimer in the documentation
   and/or other materials provided with the distribution.

THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS" AND
ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED
WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE
DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT OWNER OR CONTRIBUTORS BE LIABLE FOR
ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES
(INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES;
LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND
ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
(INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS
SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.

The views and conclusions contained in the software and documentation are those
of the authors and should not be interpreted as representing official policies,
either expressed or implied, of the FreeBSD Project.
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <assert.h>

#include "zhash.h"

#define INITIAL_NBUCKETS 16
#define INITIAL_BUCKET_CAPACITY 4

// when the ratio of allocations to actual size drops below this
// ratio, we rehash. (Reciprocal of more typical load factor.)
#define REHASH_RATIO 2

struct bucket
{
    uint32_t size; // # entries in this bucket
    uint32_t alloc; // # entries allocated

    uint8_t *keys;
    uint8_t *values;
};

struct zhash
{
    size_t keysz, valuesz;

    uint32_t(*hash)(const void *a);

    // returns 1 if equal
    int(*equals)(const void *a, const void *b);

    int size; // # of items in hash table

    struct bucket *buckets;
    int nbuckets;
};

zhash_t *zhash_create(size_t keysz, size_t valuesz,
                      uint32_t(*hash)(const void *a), int(*equals)(const void *a, const void*b))
{
    assert(hash != NULL);
    assert(equals != NULL);

    zhash_t *zh = (zhash_t*) calloc(1, sizeof(zhash_t));
    zh->keysz = keysz;
    zh->valuesz = valuesz;
    zh->hash = hash;
    zh->equals = equals;

    zh->nbuckets = INITIAL_NBUCKETS;
    zh->buckets = (struct bucket*) calloc(zh->nbuckets, sizeof(struct bucket));
    return zh;
}

void zhash_destroy(zhash_t *zh)
{
    if (zh == NULL)
        return;

    for (int i = 0; i < zh->nbuckets; i++) {
        free(zh->buckets[i].keys);
        free(zh->buckets[i].values);
    }

    free(zh->buckets);
    memset(zh, 0, sizeof(zhash_t));
    free(zh);
}

zhash_t * zhash_copy(const zhash_t * orig)
{
    assert(orig != NULL);

    zhash_t * out = zhash_create(orig->keysz, orig->valuesz, orig->hash, orig->equals);
    zhash_iterator_t itr;
    zhash_iterator_init_const(orig, &itr);
    void * key_ptr = calloc(1,orig->keysz);
    void * val_ptr = calloc(1,orig->valuesz);
    while (zhash_iterator_next(&itr, key_ptr, val_ptr))
        zhash_put(out, key_ptr, val_ptr, NULL, NULL);

    free(key_ptr);
    free(val_ptr);
    return out;
}

int zhash_size(const zhash_t *zh)
{
    assert(zh != NULL);

    return zh->size;
}

int zhash_contains(const zhash_t *zh, const void *key)
{
    assert(zh != NULL);
    assert(key != NULL);

    return zhash_get(zh, key, NULL);
}

int zhash_get(const zhash_t *zh, const void *key, void *out_value)
{
    assert(zh != NULL);
    assert(key != NULL);

    uint32_t hash = zh->hash(key);
    int idx = hash % zh->nbuckets;

    struct bucket *bucket = &zh->buckets[idx];
    for (int i = 0; i < bucket->size; i++) {
        void *this_key = &bucket->keys[zh->keysz * i];

        if (zh->equals(key, this_key)) {
            void *this_value = &bucket->values[zh->valuesz * i];
            if (out_value != NULL)
                memcpy(out_value, this_value, zh->valuesz);
            return 1;
        }
    }

    return 0;
}

int zhash_get_volatile(const zhash_t *zh, const void *key, void *out_value)
{
    assert(zh != NULL);
    assert(key != NULL);

    uint32_t hash = zh->hash(key);
    int idx = hash % zh->nbuckets;

    struct bucket *bucket = &zh->buckets[idx];
    for (int i = 0; i < bucket->size; i++) {
        void *this_key = &bucket->keys[zh->keysz * i];

        if (zh->equals(key, this_key)) {
            void *this_value = &bucket->values[zh->valuesz * i];
            if (out_value != NULL)
                *((void**) out_value) = this_value;
            return 1;
        }
    }

    return 0;
}

// returns 1 if there was an oldkey/oldvalue.
static inline int zhash_put_real(zhash_t *zh, struct bucket *buckets, int nbuckets,
                                 const void *key, const void *value, void *old_key, void *old_value)
{
    assert(zh != NULL);
    assert(buckets != NULL);
    assert(key != NULL);
    assert(zh->valuesz == 0 || value != NULL);

    uint32_t hash = zh->hash(key);

    int idx = hash % nbuckets;
    struct bucket *bucket = &buckets[idx];

    // replace an existing key if it exists.
    for (int i = 0; i < bucket->size; i++) {
        void *this_key = &bucket->keys[zh->keysz * i];
        if (zh->equals(key, this_key)) {
            void *this_value = &bucket->values[zh->valuesz * i];

            if (old_key)
                memcpy(old_key, this_key, zh->keysz);

            if (old_value)
                memcpy(old_value, this_value, zh->valuesz);

            memcpy(this_key, key, zh->keysz);
            memcpy(this_value, value, zh->valuesz);
            return 1;
        }
    }

    // enlarge bucket?
    if (bucket->size == bucket->alloc) {
        if (bucket->alloc == 0)
            bucket->alloc = INITIAL_BUCKET_CAPACITY;
        else
            bucket->alloc *= 2;

        bucket->keys = realloc(bucket->keys, bucket->alloc * zh->keysz);
        bucket->values = realloc(bucket->values, bucket->alloc * zh->valuesz);
    }

    // add!
    void *this_key = &bucket->keys[zh->keysz * bucket->size];
    void *this_value = &bucket->values[zh->valuesz * bucket->size];

    memcpy(this_key, key, zh->keysz);
    memcpy(this_value, value, zh->valuesz);
    bucket->size++;

    return 0;
}

int zhash_remove(zhash_t *zh, const void *key, void *old_key, void *old_value)
{
    assert(zh != NULL);
    assert(key != NULL);

    uint32_t hash = zh->hash(key);

    int idx = hash % zh->nbuckets;
    struct bucket *bucket = &zh->buckets[idx];

    // replace an existing key if it exists.
    for (int i = 0; i < bucket->size; i++) {
        void *this_key = &bucket->keys[zh->keysz * i];

        if (zh->equals(key, this_key)) {
            if (old_key)
                memcpy(old_key, this_key, zh->keysz);

            void *this_value = &bucket->values[zh->valuesz * i];
            if (old_value)
                memcpy(old_value, this_value, zh->valuesz);

            // shuffle remove.

            // If we're removing the last element in the list,
            // then the shuffle remove would rewrite the last element
            // with itself. (That's forbidden with memcpy, so
            // explicitly test for it.)
            if (i != bucket->size - 1) {
                // shuffle remove.
                void *last_key = &bucket->keys[zh->keysz * (bucket->size - 1)];
                void *last_value = &bucket->values[zh->valuesz * (bucket->size - 1)];

                memcpy(this_key, last_key, zh->keysz);
                memcpy(this_value, last_value, zh->valuesz);
            }
            bucket->size--;
            zh->size--;

            return 1;
        }
    }

    return 0;
}

void zhash_clear(zhash_t *zh) {
    assert(zh != NULL);
    if (zhash_size(zh) == 0)
        return;

    // free current contents
    for (int i = 0; i < zh->nbuckets; i++) {
        free(zh->buckets[i].keys);
        free(zh->buckets[i].values);
    }
    free(zh->buckets);

    // recreate
    zh->size = 0;
    zh->nbuckets = INITIAL_NBUCKETS;
    zh->buckets = (struct bucket*) calloc(zh->nbuckets, sizeof(struct bucket));
}

int zhash_put(zhash_t *zh, const void *key, const void *value, void *oldkey, void *oldvalue)
{
    assert(zh != NULL);
    assert(key != NULL);
    assert(zh->valuesz == 0 || value != NULL);

    if (zh->nbuckets * REHASH_RATIO < zh->size) {

        // resize
        int new_nbuckets = zh->nbuckets*2;
        struct bucket *new_buckets = calloc(new_nbuckets, sizeof(struct bucket));

        // put all our existing elements into the new hash table
        zhash_iterator_t zit;
        zhash_iterator_init(zh, &zit);
        void *key, *value;
        while (zhash_iterator_next_volatile(&zit, &key, &value)) {
            zhash_put_real(zh, new_buckets, new_nbuckets, key, value, NULL, NULL);
        }

        // free the old elements
        for (int i = 0; i < zh->nbuckets; i++) {
            free(zh->buckets[i].keys);
            free(zh->buckets[i].values);
        }
        free(zh->buckets);

        // switch to the new elements
        zh->nbuckets = new_nbuckets;
        zh->buckets = new_buckets;
    }

    int has_oldkey = zhash_put_real(zh, zh->buckets, zh->nbuckets, key, value, oldkey, oldvalue);
    if (!has_oldkey)
        zh->size++;

    return has_oldkey;
}

void zhash_iterator_init(zhash_t *zh, zhash_iterator_t *zit)
{
    assert(zh != NULL);
    assert(zit != NULL);

    zit->zh = zh;
    zit->czh = zh;
    zit->bucket = 0;
    zit->idx = 0;
}

void zhash_iterator_init_const(const zhash_t *zh, zhash_iterator_t *zit)
{
    assert(zh != NULL);
    assert(zit != NULL);

    zit->zh = NULL;
    zit->czh = zh;
    zit->bucket = 0;
    zit->idx = 0;
}

int zhash_iterator_next_volatile(zhash_iterator_t *zit, void *outkey, void *outvalue)
{
    assert(zit != NULL);

    const zhash_t *zh = zit->czh;

    while (zit->bucket < zh->nbuckets) {
        struct bucket *bucket = &zh->buckets[zit->bucket];

        if (zit->idx < bucket->size) {
            void *this_key = &bucket->keys[zh->keysz * zit->idx];
            void *this_value = &bucket->values[zh->valuesz * zit->idx];

            if (outkey != NULL)
                *((void**) outkey) = this_key;
            if (outvalue != NULL)
                *((void**) outvalue) = this_value;
            zit->idx++;

            return 1;
        }
        zit->bucket++;
        zit->idx = 0;
    }

    return 0;
}

int zhash_iterator_next(zhash_iterator_t *zit, void *outkey, void *outvalue)
{
    assert(zit != NULL);

    void *outkeyp, *outvaluep;

    int res = zhash_iterator_next_volatile(zit, &outkeyp, &outvaluep);
    if (res) {
        if (outkey != NULL)
            memcpy(outkey, outkeyp, zit->czh->keysz);
        if (outvalue != NULL)
            memcpy(outvalue, outvaluep, zit->czh->valuesz);
    }
    return res;
}

void zhash_iterator_remove(zhash_iterator_t *zit)
{
    assert(zit != NULL);
    assert(zit->zh != NULL && "Must initialize zhash iterator with non-const _init() to use with _remove()");

    zhash_t *zh = zit->zh;

    struct bucket *bucket = &zh->buckets[zit->bucket];
    void *this_key = &bucket->keys[zh->keysz * (zit->idx - 1)];

    zhash_remove(zh, this_key, NULL, NULL);
    zit->idx--;
}

void zhash_map_keys(zhash_t * zh, void (*f)())
{
    assert(zh != NULL);
    if (f == NULL)
        return;

    zhash_iterator_t itr;
    zhash_iterator_init(zh, &itr);

    void *key, *value;

    while(zhash_iterator_next_volatile(&itr, &key, &value)) {
        f(key);
    }
}

void zhash_vmap_keys(zhash_t * zh, void (*f)())
{
    assert(zh != NULL);
    if (f == NULL)
        return;

    zhash_iterator_t itr;
    zhash_iterator_init(zh, &itr);

    void *key, *value;

    while(zhash_iterator_next_volatile(&itr, &key, &value)) {
        void *p = *(void**) key;
        f(p);
    }
}

void zhash_map_values(zhash_t * zh, void (*f)())
{
    assert(zh != NULL);
    if (f == NULL)
        return;

    zhash_iterator_t itr;
    zhash_iterator_init(zh, &itr);

    void *key, *value;
    while(zhash_iterator_next_volatile(&itr, &key, &value)) {
        f(value);
    }
}

void zhash_vmap_values(zhash_t * zh, void (*f)())
{
    assert(zh != NULL);
    if (f == NULL)
        return;

    zhash_iterator_t itr;
    zhash_iterator_init(zh, &itr);

    void *key, *value;
    while(zhash_iterator_next_volatile(&itr, &key, &value)) {
        void *p = *(void**) value;
        f(p);
    }
}

zarray_t *zhash_keys(const zhash_t *zh)
{
    assert(zh != NULL);

    zarray_t *za = zarray_create(zh->keysz);

    zhash_iterator_t itr;
    zhash_iterator_init_const(zh, &itr);

    void *key, *value;
    while(zhash_iterator_next_volatile(&itr, &key, &value)) {
        zarray_add(za, key);
    }

    return za;
}

zarray_t *zhash_values(const zhash_t *zh)
{
    assert(zh != NULL);

    zarray_t *za = zarray_create(zh->valuesz);

    zhash_iterator_t itr;
    zhash_iterator_init_const(zh, &itr);

    void *key, *value;
    while(zhash_iterator_next_volatile(&itr, &key, &value)) {
        zarray_add(za, value);
    }

    return za;
}


uint32_t zhash_uint32_hash(const void *_a)
{
    assert(_a != NULL);

    uint32_t a = *((uint32_t*) _a);
    return a;
}

int zhash_uint32_equals(const void *_a, const void *_b)
{
    assert(_a != NULL);
    assert(_b != NULL);

    uint32_t a = *((uint32_t*) _a);
    uint32_t b = *((uint32_t*) _b);

    return a==b;
}

uint32_t zhash_uint64_hash(const void *_a)
{
    assert(_a != NULL);

    uint64_t a = *((uint64_t*) _a);
    return (uint32_t) (a ^ (a >> 32));
}

int zhash_uint64_equals(const void *_a, const void *_b)
{
    assert(_a != NULL);
    assert(_b != NULL);

    uint64_t a = *((uint64_t*) _a);
    uint64_t b = *((uint64_t*) _b);

    return a==b;
}


union uintpointer
{
    const void *p;
    uint32_t i;
};

uint32_t zhash_ptr_hash(const void *a)
{
    assert(a != NULL);

    union uintpointer ip;
    ip.p = * (void**)a;

    // compute a hash from the lower 32 bits of the pointer (on LE systems)
    uint32_t hash = ip.i;
    hash ^= (hash >> 7);

    return hash;
}


int zhash_ptr_equals(const void *a, const void *b)
{
    assert(a != NULL);
    assert(b != NULL);

    const void * ptra = * (void**)a;
    const void * ptrb = * (void**)b;
    return  ptra == ptrb;
}


int zhash_str_equals(const void *_a, const void *_b)
{
    assert(_a != NULL);
    assert(_b != NULL);

    char *a = * (char**)_a;
    char *b = * (char**)_b;

    return !strcmp(a, b);
}

uint32_t zhash_str_hash(const void *_a)
{
    assert(_a != NULL);

    char *a = * (char**)_a;

    int64_t hash = 0;
    while (*a != 0) {
        hash += *a;
        hash = (hash << 7) + (hash >> 23);
        a++;
    }

    return (uint32_t) hash;
}
