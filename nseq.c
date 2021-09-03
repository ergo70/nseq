/************************************************************************
 * nseq - a storage efficient nucleotide sequence datatype for PostgreSQL
 *
 * Copyright (c) 2014,2015 by Ernst-G. Schmid
 *
 ************************************************************************/

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>
#include "postgres.h"
#include "fmgr.h"
#include "utils/array.h"
#include "utils/lsyscache.h"
#include "catalog/pg_type.h"
#include "nseq.h"

#ifdef PG_MODULE_MAGIC
PG_MODULE_MAGIC;
#endif

PG_FUNCTION_INFO_V1 (nseq_length);
PG_FUNCTION_INFO_V1 (nseq_size);
PG_FUNCTION_INFO_V1 (nseq_transscribe);
PG_FUNCTION_INFO_V1 (nseq_histogram);

Datum nseq_length (PG_FUNCTION_ARGS);

/*
* Return uncompressed length of nseq.
*/
Datum
nseq_length (PG_FUNCTION_ARGS)
{
    NSEQ *nseq = PG_GETARG_NSEQ_P (0);

    PG_RETURN_INT32 (nseq->size);
}

/*
* Return compressed size of nseq.
*/
Datum nseq_size (PG_FUNCTION_ARGS);

Datum
nseq_size (PG_FUNCTION_ARGS)
{
    NSEQ *nseq = PG_GETARG_NSEQ_P (0);

    PG_RETURN_INT32 (nseq->compressed_size + (((nseq->compressed_size % BLOCKSIZE) != 0) ? 1 : 0));
}

Datum nseq_transscribe (PG_FUNCTION_ARGS);

Datum
nseq_transscribe (PG_FUNCTION_ARGS)
{
    NSEQ *nseq = PG_GETARG_NSEQ_P (0);

    nseq->rna = !nseq->rna;

    PG_RETURN_NSEQ_P (nseq);
}

Datum nseq_histogram (PG_FUNCTION_ARGS);

Datum
nseq_histogram (PG_FUNCTION_ARGS)
{
    NSEQ *nseq = PG_GETARG_NSEQ_P (0);
    ArrayType  *result = NULL;
    Datum *result_data = (Datum *)palloc(4 * sizeof(Datum));
    int16       typlen;
    bool        typbyval;
    char        typalign;
    int8 i;

    get_typlenbyvalalign(INT4OID, &typlen, &typbyval, &typalign);

    for(i=0; i<HISTSZ; i++)
    {
        result_data[i] = Int32GetDatum((int32)nseq->histogram[i]);
    }
    result = construct_array((void *)result_data, HISTSZ, INT4OID, typlen, typbyval, typalign);

    PG_RETURN_ARRAYTYPE_P (result);
}

