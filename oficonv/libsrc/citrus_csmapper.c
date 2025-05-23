/*-
 * Copyright (c)2003 Citrus Project,
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
 *
 * THIS SOFTWARE IS PROVIDED BY THE AUTHOR AND CONTRIBUTORS ``AS IS'' AND
 * ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
 * IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE
 * ARE DISCLAIMED.  IN NO EVENT SHALL THE AUTHOR OR CONTRIBUTORS BE LIABLE
 * FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL
 * DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS
 * OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION)
 * HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT
 * LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY
 * OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF
 * SUCH DAMAGE.
 */

#include "dcmtk/config/osconfig.h"
#include "citrus_csmapper.h"


#ifdef HAVE_SYS_TYPES_H
#include <sys/types.h>
#endif
#ifdef HAVE_SYS_QUEUE_H
#include <sys/queue.h>
#else
#include "dcmtk/oficonv/queue.h"
#endif


#include <errno.h>
#include <limits.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "citrus_bcs.h"
#include "citrus_types.h"
#include "citrus_region.h"
#include "citrus_lock.h"
#include "citrus_memstream.h"
#include "citrus_mmap.h"
#include "citrus_module.h"
#include "citrus_hash.h"
#include "citrus_mapper.h"
#include "citrus_pivot_file.h"
#include "citrus_db.h"
#include "citrus_db_hash.h"
#include "citrus_lookup.h"
#include "oficonv_strlcpy.h"

static struct _citrus_mapper_area   *maparea = NULL;

#ifdef WITH_THREADS
#ifdef HAVE_WINDOWS_H
static SRWLOCK ma_lock = SRWLOCK_INIT;
#elif defined(HAVE_PTHREAD_H)
static pthread_rwlock_t ma_lock = PTHREAD_RWLOCK_INITIALIZER;
#endif
#endif

#define CS_ALIAS_FILENAME "charset.alias"
#define CS_PIVOT_FILENAME "charset.pivot"

/* ---------------------------------------------------------------------- */

/* write the full path to the CSMAPPER directory and the given filename (which may be NULL)
 * to the path_out buffer, which is expected to be OFICONV_PATH_MAX in size.
 */
static void getCSMapperPath(char *path_out, size_t path_size, const char *filename)
{
    get_data_path(path_out, path_size, OFICONV_CSMAPPER_DIR, filename);
}


static int
get32(struct _citrus_region *r, uint32_t *rval)
{

    if (_citrus_region_size(r) != 4)
        return (EFTYPE);

    memcpy(rval, _citrus_region_head(r), (size_t)4);
    *rval = be32toh(*rval);

    return (0);
}

static int
open_subdb(struct _citrus_db **subdb, struct _citrus_db *db, const char *src)
{
    struct _citrus_region r;
    int ret;

    ret = _citrus_db_lookup_by_string(db, src, &r, NULL);
    if (ret)
        return (ret);
    ret = _citrus_db_open(subdb, &r, _CITRUS_PIVOT_SUB_MAGIC, _citrus_db_hash_std, NULL);
    if (ret)
        return (ret);

    return (0);
}


#define NO_SUCH_FILE    EOPNOTSUPP
static int
find_best_pivot_pvdb(const char *src, const char *dst, char *pivot,
    size_t pvlen, unsigned long *rnorm)
{
    struct _citrus_db *db1, *db2, *db3;
    struct _citrus_region fr, r1, r2;
    char buf[LINE_MAX];
    char mapper_path[OFICONV_PATH_MAX];
    uint32_t val32;
    unsigned long norm;
    int i, num, ret;

    getCSMapperPath(mapper_path, sizeof(mapper_path), CS_PIVOT_FILENAME ".pvdb");
    ret = _citrus_map_file(&fr, mapper_path);
    if (ret) {
        if (ret == ENOENT)
            ret = NO_SUCH_FILE;
        return (ret);
    }
    ret = _citrus_db_open(&db1, &fr, _CITRUS_PIVOT_MAGIC, _citrus_db_hash_std, NULL);
    if (ret)
        goto quit1;
    ret = open_subdb(&db2, db1, src);
    if (ret)
        goto quit2;

    num = _citrus_db_get_number_of_entries(db2);
    *rnorm = ULONG_MAX;
    for (i = 0; i < num; i++) {
        /* iterate each pivot */
        ret = _citrus_db_get_entry(db2, i, &r1, &r2);
        if (ret)
            goto quit3;
        /* r1:pivot name, r2:norm among src and pivot */
        ret = get32(&r2, &val32);
        if (ret)
            goto quit3;
        norm = val32;
        snprintf(buf, sizeof(buf), "%.*s",
             (int)_citrus_region_size(&r1), (char *)_citrus_region_head(&r1));
        /* buf: pivot name */
        ret = open_subdb(&db3, db1, buf);
        if (ret)
            goto quit3;
        if (_citrus_db_lookup_by_string(db3, dst, &r2, NULL) != 0)
            /* don't break the loop, test all src/dst pairs. */
            goto quit4;
        /* r2: norm among pivot and dst */
        ret = get32(&r2, &val32);
        if (ret)
            goto quit4;
        norm += val32;
        /* judge minimum norm */
        if (norm < *rnorm) {
            *rnorm = norm;
            strlcpy(pivot, buf, pvlen);
        }
quit4:
        _citrus_db_close(db3);
        if (ret)
            goto quit3;
    }
quit3:
    _citrus_db_close(db2);
quit2:
    _citrus_db_close(db1);
quit1:
    _citrus_unmap_file(&fr);
    if (ret)
        return (ret);

    if (*rnorm == ULONG_MAX)
        return (ENOENT);

    return (0);
}

/* ---------------------------------------------------------------------- */

struct zone {
    const char *begin, *end;
};

struct parse_arg {
    char dst[OFICONV_PATH_MAX];
    unsigned long norm;
};

static int
parse_line(struct parse_arg *pa, struct _citrus_region *r)
{
    struct zone z1, z2;
    char buf[20];
    size_t len;

    len = _citrus_region_size(r);
    z1.begin = _citrus_bcs_skip_ws_len(_citrus_region_head(r), &len);
    if (len == 0)
        return (EFTYPE);
    z1.end = _citrus_bcs_skip_nonws_len(z1.begin, &len);
    if (len == 0)
        return (EFTYPE);
    z2.begin = _citrus_bcs_skip_ws_len(z1.end, &len);
    if (len == 0)
        return (EFTYPE);
    z2.end = _citrus_bcs_skip_nonws_len(z2.begin, &len);

    /* z1 : dst name, z2 : norm */
    snprintf(pa->dst, sizeof(pa->dst),
        "%.*s", (int)(z1.end-z1.begin), z1.begin);
    snprintf(buf, sizeof(buf),
        "%.*s", (int)(z2.end-z2.begin), z2.begin);
    pa->norm = _citrus_bcs_strtoul(buf, NULL, 0);

    return (0);
}

static int
find_dst(struct parse_arg *pasrc, const char *dst)
{
    struct _citrus_lookup *cl;
    struct parse_arg padst;
    struct _citrus_region data;
    int ret;
    char mapper_path[OFICONV_PATH_MAX];

    getCSMapperPath(mapper_path, sizeof(mapper_path), CS_PIVOT_FILENAME);
    ret = _citrus_lookup_seq_open(&cl, mapper_path, _CITRUS_LOOKUP_CASE_IGNORE);
    if (ret)
        return (ret);

    ret = _citrus_lookup_seq_lookup(cl, pasrc->dst, &data);
    while (ret == 0) {
        ret = parse_line(&padst, &data);
        if (ret)
            break;
        if (strcmp(dst, padst.dst) == 0) {
            pasrc->norm += padst.norm;
            break;
        }
        ret = _citrus_lookup_seq_next(cl, NULL, &data);
    }
    _citrus_lookup_seq_close(cl);

    return (ret);
}

static int
find_best_pivot_lookup(const char *src, const char *dst, char *pivot,
    size_t pvlen, unsigned long *rnorm)
{
    struct _citrus_lookup *cl;
    struct _citrus_region data;
    struct parse_arg pa;
    char pivot_min[OFICONV_PATH_MAX];
    unsigned long norm_min;
    int ret;
    char mapper_path[OFICONV_PATH_MAX];

    getCSMapperPath(mapper_path, sizeof(mapper_path), CS_PIVOT_FILENAME);
    ret = _citrus_lookup_seq_open(&cl, mapper_path, _CITRUS_LOOKUP_CASE_IGNORE);
    if (ret)
        return (ret);

    norm_min = ULONG_MAX;

    /* find pivot code */
    ret = _citrus_lookup_seq_lookup(cl, src, &data);
    while (ret == 0) {
        ret = parse_line(&pa, &data);
        if (ret)
            break;
        ret = find_dst(&pa, dst);
        if (ret)
            break;
        if (pa.norm < norm_min) {
            norm_min = pa.norm;
            strlcpy(pivot_min, pa.dst, sizeof(pivot_min));
        }
        ret = _citrus_lookup_seq_next(cl, NULL, &data);
    }
    _citrus_lookup_seq_close(cl);

    if (ret != ENOENT)
        return (ret);
    if (norm_min == ULONG_MAX)
        return (ENOENT);
    strlcpy(pivot, pivot_min, pvlen);
    if (rnorm)
        *rnorm = norm_min;

    return (0);
}

static int
find_best_pivot(const char *src, const char *dst, char *pivot, size_t pvlen,
    unsigned long *rnorm)
{
    int ret;

    ret = find_best_pivot_pvdb(src, dst, pivot, pvlen, rnorm);
    if (ret == NO_SUCH_FILE)
        ret = find_best_pivot_lookup(src, dst, pivot, pvlen, rnorm);

    return (ret);
}

static __inline int
open_serial_mapper(struct _citrus_mapper_area * ma,
    struct _citrus_csmapper * * rcm,
    const char *src, const char *pivot, const char *dst)
{
    char buf[OFICONV_PATH_MAX];

#if ( defined(__GNUC__) && (__GNUC__ > 7 || (__GNUC__ == 7 && __GNUC_MINOR__ >= 1 ) ) )
#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wformat-truncation"
#endif

    snprintf(buf, sizeof(buf), "%s/%s,%s/%s", src, pivot, pivot, dst);

#if ( defined(__GNUC__) && (__GNUC__ > 7 || (__GNUC__ == 7 && __GNUC_MINOR__ >= 1 ) ) )
#pragma GCC diagnostic pop
#endif

    return (_citrus_mapper_open_direct(ma, rcm, "mapper_serial", buf));
}

static struct _citrus_csmapper *csm_none = NULL;
static int
get_none(struct _citrus_mapper_area * ma,
    struct _citrus_csmapper * * rcsm)
{
    int ret;

    WLOCK(&ma_lock);
    if (csm_none) {
        *rcsm = csm_none;
        ret = 0;
        goto quit;
    }

    ret = _citrus_mapper_open_direct(ma, &csm_none, "mapper_none", "");
    if (ret)
        goto quit;
    _citrus_mapper_set_persistent(csm_none);

    *rcsm = csm_none;
    ret = 0;
quit:
    UNLOCK(&ma_lock);
    return (ret);
}

int
_citrus_csmapper_open(struct _citrus_csmapper * * rcsm,
    const char * src, const char * dst, uint32_t flags,
    unsigned long *rnorm)
{
    const char *realsrc, *realdst;
#ifdef DCMTK_USE_OFICONV_CHARSET_ALIAS_FILE
    char buf1[OFICONV_PATH_MAX], buf2[OFICONV_PATH_MAX];
    char alias_path[OFICONV_PATH_MAX];
#endif
    char key[OFICONV_PATH_MAX], pivot[OFICONV_PATH_MAX];
    char mapper_path[OFICONV_PATH_MAX];
    unsigned long norm;
    int ret;
    norm = 0;

    getCSMapperPath(mapper_path, sizeof(mapper_path), NULL);
    ret = _citrus_mapper_create_area(&maparea, mapper_path);
    if (ret)
        return (ret);

#ifdef DCMTK_USE_OFICONV_CHARSET_ALIAS_FILE
    // Look up alias names in csmapper/charset.alias.db or csmapper/charset.alias
    getCSMapperPath(alias_path, sizeof(alias_path), CS_ALIAS_FILENAME);
    realsrc = _citrus_lookup_alias(alias_path, src, buf1, sizeof(buf1), _CITRUS_LOOKUP_CASE_IGNORE);
    realdst = _citrus_lookup_alias(alias_path, dst, buf2, sizeof(buf2), _CITRUS_LOOKUP_CASE_IGNORE);
#else
    // Don't use the alias files csmapper/charset.alias.db or csmapper/charset.alias
    realsrc = src;
    realdst = dst;
#endif
    if (!strcmp(realsrc, realdst)) {
        ret = get_none(maparea, rcsm);
        if (ret == 0 && rnorm != NULL)
            *rnorm = 0;
        return (ret);
    }

    snprintf(key, sizeof(key), "%s/%s", realsrc, realdst);

    ret = _citrus_mapper_open(maparea, rcsm, key);
    if (ret == 0) {
        if (rnorm != NULL)
            *rnorm = 0;
        return (0);
    }
    if (ret != ENOENT || (flags & _CITRUS_CSMAPPER_F_PREVENT_PIVOT)!=0)
        return (ret);

    ret = find_best_pivot(realsrc, realdst, pivot, sizeof(pivot), &norm);
    if (ret)
        return (ret);

    ret = open_serial_mapper(maparea, rcsm, realsrc, pivot, realdst);
    if (ret == 0 && rnorm != NULL)
        *rnorm = norm;

    return (ret);
}

void
_citrus_csmapper_free(void)
{
    if (maparea)
    {
        _citrus_mapper_delete_area(&maparea);
    }
}
