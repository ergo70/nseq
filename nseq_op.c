/************************************************************************
 * nseq - a storage efficient nucleotide sequence datatype for PostgreSQL
 *
 * Copyright (c) 2014,2015 by Ernst-G. Schmid
 *
 ************************************************************************/

#include "postgres.h"
#include "fmgr.h"
#include "nseq.h"

PG_FUNCTION_INFO_V1 (nseq_equals);

Datum nseq_equals (PG_FUNCTION_ARGS);

/*
* Check if a query nseq is equal to a predicate nseq by comparing their compressed buffers.
*/
Datum
nseq_equals (PG_FUNCTION_ARGS)
{
    NSEQ *query = PG_GETARG_NSEQ_P (0);
    NSEQ *predicate = PG_GETARG_NSEQ_P (1);
    //DECOMPRESSED_DATA *q, *p;

    //if(query->rna != predicate->rna) PG_RETURN_BOOL(false);
    if(query->size != predicate->size) PG_RETURN_BOOL(false);

    //q = decompress_data(DATAPTR(query), query->compressed_size, query->size);
    //p = decompress_data(DATAPTR(predicate), predicate->compressed_size, predicate->size);

    if(memcmp(DATAPTR(query), DATAPTR(predicate), predicate->compressed_size) != 0) PG_RETURN_BOOL(false);

    PG_RETURN_BOOL (true);
}

