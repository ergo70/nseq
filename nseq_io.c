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
#include "libpq/pqformat.h"	/* needed for send/recv functions */
#include "nseq.h"

Datum dna_in (PG_FUNCTION_ARGS);
Datum dna_in_text (PG_FUNCTION_ARGS);
Datum dna_in_varchar (PG_FUNCTION_ARGS);
//Datum dna_in_bytea (PG_FUNCTION_ARGS);
Datum nseq_out (PG_FUNCTION_ARGS);
Datum nseq_out_complement (PG_FUNCTION_ARGS);

Datum rna_in (PG_FUNCTION_ARGS);
Datum rna_in_text (PG_FUNCTION_ARGS);
Datum rna_in_varchar (PG_FUNCTION_ARGS);
//Datum rna_in_bytea (PG_FUNCTION_ARGS);

Datum nseq_recv (PG_FUNCTION_ARGS);
Datum nseq_send (PG_FUNCTION_ARGS);

Datum nseq_concat (PG_FUNCTION_ARGS);
static char *nseq_concat_slow (const NSEQ *a, const NSEQ *b);

static char *decode_nseq(const char* buffer, const size_t bufsize, const bool isRNA)
{
    char* retval = NULL;
    char tmp;
    const char cmp = 0x3;
    uint32 i, offset = 0;

    retval = (char*) palloc0(bufsize*BLOCKSIZE);

    while (offset < bufsize)
    {
        for(i = 0; i<BLOCKSIZE; i++)
        {
            tmp = buffer[offset] >> (i*2);

            //elog(INFO,"%i",offset);
            //elog(INFO,"%i",i);
            //elog(INFO,"%i",bufsize);

            switch(tmp & cmp)
            {
            case 0x0:
                retval[offset*BLOCKSIZE+i] = 'A';
                break;
            case 0x1:
                retval[offset*BLOCKSIZE+i] = 'C';
                break;
            case 0x2:
                retval[offset*BLOCKSIZE+i] = 'G';
                break;
            case 0x3:
                if(isRNA)
                {
                    retval[offset*BLOCKSIZE+i] = 'U';
                }
                else
                {
                    retval[offset*BLOCKSIZE+i] = 'T';
                }
                break;
            }
        }
        offset++;
    }

    return retval;
}

static char *decode_nseq_complement(const char* buffer, const size_t bufsize, const bool isRNA)
{
    char* retval = NULL;
    char tmp;
    const char cmp = 0x3;
    uint32 i, offset = 0;

    retval = (char*) palloc0(bufsize*BLOCKSIZE);

    while (offset < bufsize)
    {
        for(i = 0; i<BLOCKSIZE; i++)
        {
            tmp = buffer[offset] >> (i*2);

            //elog(INFO,"%i",offset);
            //elog(INFO,"%i",i);
            //elog(INFO,"%i",bufsize);

            switch(tmp & cmp)
            {
            case 0x3:
                retval[offset*BLOCKSIZE+i] = 'A';
                break;
            case 0x2:
                retval[offset*BLOCKSIZE+i] = 'C';
                break;
            case 0x1:
                retval[offset*BLOCKSIZE+i] = 'G';
                break;
            case 0x0:
                if(isRNA)
                {
                    retval[offset*BLOCKSIZE+i] = 'U';
                }
                else
                {
                    retval[offset*BLOCKSIZE+i] = 'T';
                }
                break;
            }
        }
        offset++;
    }

    return retval;
}

static NSEQ *make_nseq(const char* sequence, const size_t seqlen, const bool isRNA)
{
    NSEQ *retval = NULL;
    char *buffer = NULL;
    char tmp;
    uint32 i, offset = 0;
    uint32 bufsize = seqlen / BLOCKSIZE;
    //COMPRESSED_DATA *compressed_data;
    int32 histogram[HISTSZ] = {0,0,0,0};
    bool run = true;

    if((seqlen % BLOCKSIZE) != 0)
    {
        bufsize += 1;
    }

    buffer = (char*) palloc0(bufsize*sizeof(char));

    while (run && (offset < bufsize))
    {
        for(i = 0; i<BLOCKSIZE; i++)
        {
            tmp = toupper(sequence[(offset*BLOCKSIZE)+i]);

            switch(tmp)
            {
            case 'A':
                buffer[offset] |= (0x0 << (i*2));
                histogram[0]++;
                break;
            case 'C':
                buffer[offset] |= (0x1 << (i*2));
                histogram[1]++;
                break;
            case 'G':
                buffer[offset] |= (0x2 << (i*2));
                histogram[2]++;
                break;
            case 'U':
                if(isRNA)
                {
                    buffer[offset] |= (0x3 << (i*2));
                    histogram[3]++;
                }
                else
                {
                    elog(ERROR, "Unknown nucleotide for DNA: %c\n", tmp);
                    run = false;
                }
                break;
            case 'T':
                if(isRNA)
                {
                    elog(ERROR, "Unknown nucleotide for RNA: %c\n", tmp);
                    run = false;
                }
                else
                {
                    buffer[offset] |= (0x3 << (i*2));
                    histogram[3]++;
                }
                break;
            case '\0':
                run = false;
                break;
            }
        }
        offset++;
    }

    //compressed_data = compress_data(buffer, bufsize);

    retval = palloc(CALCDATASZ(bufsize));

    retval->rna = isRNA;
    retval->size = seqlen;
    retval->compressed_size = bufsize;
    memcpy(retval->histogram, &histogram, sizeof(histogram));

    memcpy(DATAPTR(retval), buffer, bufsize);

    SET_VARSIZE (retval,CALCDATASZ(bufsize));

    //elog(INFO,"make %d %d", seqlen, bufsize);

    return retval;
}

PG_FUNCTION_INFO_V1 (dna_in);

Datum
dna_in (PG_FUNCTION_ARGS)
{
    char *input = PG_GETARG_CSTRING (0);
    size_t size = strlen(input);

    //elog(INFO,"cstring %d", size);

    PG_RETURN_NSEQ_P (make_nseq(input,size,false));
}

PG_FUNCTION_INFO_V1 (dna_in_text);

Datum
dna_in_text (PG_FUNCTION_ARGS)
{
    text *x = PG_GETARG_TEXT_P (0);
    char *input = VARDATA(x);
    size_t size = VARSIZE(x)-VARHDRSZ;

    //elog(INFO,"text %d", size);

    PG_RETURN_NSEQ_P (make_nseq(input,size,false));
}

PG_FUNCTION_INFO_V1 (dna_in_varchar);

Datum
dna_in_varchar (PG_FUNCTION_ARGS)
{
    VarChar *x = PG_GETARG_VARCHAR_P (0);
    char *input = VARDATA(x);
    size_t size = VARSIZE(x)-VARHDRSZ;

    //elog(INFO,"varchar %d", size);

    PG_RETURN_NSEQ_P (make_nseq(input,size,false));
}

/*PG_FUNCTION_INFO_V1 (dna_in_bytea);

Datum
dna_in_bytea (PG_FUNCTION_ARGS)
{
    bytea *x = PG_GETARG_BYTEA_P (0);
    char *input = VARDATA(x);
    size_t size = VARSIZE(x)-VARHDRSZ;

    PG_RETURN_NSEQ_P (make_nseq(input,size,false));
}*/

/*
* Output a nseq in cstring form as molfile
*/
PG_FUNCTION_INFO_V1 (nseq_out);

Datum
nseq_out (PG_FUNCTION_ARGS)
{
    NSEQ *nseq = PG_GETARG_NSEQ_P (0);
    char *decoded = NULL;
    char *result = NULL;

    result = (char *) palloc0 (nseq->size+1);

    decoded = decode_nseq((const char*)nseq->data, nseq->compressed_size, nseq->rna);

    strncpy(result, decoded, nseq->size);

    //elog(INFO,"Uncompressed: %i -> Compressed: %i -> Ratio: %i:1",nseq->size, nseq->compressed_size,(nseq->size / nseq->compressed_size));

    PG_RETURN_CSTRING (result);
}

/*
* Output a nseq in cstring form as molfile
*/
PG_FUNCTION_INFO_V1 (nseq_out_complement);

Datum
nseq_out_complement (PG_FUNCTION_ARGS)
{
    NSEQ *nseq = PG_GETARG_NSEQ_P (0);
    char *decoded = NULL;
    char *result = NULL;

    result = (char *) palloc0 (nseq->size+sizeof(char));

    decoded = decode_nseq_complement((const char*)nseq->data, nseq->compressed_size, nseq->rna);

    strncpy(result, decoded, nseq->size);

    //elog(INFO,"Uncompressed: %i -> Compressed: %i -> Ratio: %i:1",nseq->size, nseq->compressed_size,(nseq->size / nseq->compressed_size));

    PG_RETURN_CSTRING (result);
}

PG_FUNCTION_INFO_V1 (rna_in);

Datum
rna_in (PG_FUNCTION_ARGS)
{
    char *input = PG_GETARG_CSTRING (0);
    size_t size = strlen(input);

    PG_RETURN_NSEQ_P (make_nseq(input,size,true));
}

PG_FUNCTION_INFO_V1 (rna_in_text);

Datum
rna_in_text (PG_FUNCTION_ARGS)
{
    text *x = PG_GETARG_TEXT_P (0);
    char *input = VARDATA(x);
    size_t size = VARSIZE(x)-VARHDRSZ;

    PG_RETURN_NSEQ_P (make_nseq(input,size,true));
}

PG_FUNCTION_INFO_V1 (rna_in_varchar);

Datum
rna_in_varchar (PG_FUNCTION_ARGS)
{
    VarChar *x = PG_GETARG_VARCHAR_P (0);
    char *input = VARDATA(x);
    size_t size = VARSIZE(x)-VARHDRSZ;

    PG_RETURN_NSEQ_P (make_nseq(input,size,true));
}

/*PG_FUNCTION_INFO_V1 (rna_in_bytea);

Datum
rna_in_bytea (PG_FUNCTION_ARGS)
{
    bytea *x = PG_GETARG_BYTEA_P (0);
    char *input = VARDATA(x);
    size_t size = VARSIZE(x)-VARHDRSZ;

    PG_RETURN_NSEQ_P (make_nseq(input,size,true));
}*/

/*****************************************************************************
 * Binary Input/Output functions
 *
 * These are optional.
 *****************************************************************************/

PG_FUNCTION_INFO_V1 (nseq_recv);

Datum
nseq_recv (PG_FUNCTION_ARGS)
{
    StringInfo buf = (StringInfo) PG_GETARG_POINTER (0);
    int len = buf->len;
    const char *str = pq_getmsgbytes (buf, len);
    NSEQ *result = (NSEQ *) palloc0 (len);

    memcpy (result, str, len);

    PG_RETURN_POINTER (result);
}

PG_FUNCTION_INFO_V1 (nseq_send);

Datum
nseq_send (PG_FUNCTION_ARGS)
{
    NSEQ *nseq = PG_GETARG_NSEQ_P (0);

    PG_RETURN_BYTEA_P(nseq);
}

PG_FUNCTION_INFO_V1 (nseq_concat);

Datum
nseq_concat (PG_FUNCTION_ARGS)
{
    NSEQ *a = PG_GETARG_NSEQ_P (0);
    NSEQ *b = PG_GETARG_NSEQ_P (1);
    NSEQ *retval = NULL;
    int32 totalsize, totalblocks, overflow;
    char *newval = NULL;
    int i;

    if(a->rna != b->rna)
    {
        elog(ERROR,"Cannot concatenate DNA and RNA");
    }

    totalsize = a->size+b->size;

    //elog(INFO, "%i", totalsize);

    overflow = totalsize % BLOCKSIZE;

    //elog(INFO, "%i", overflow);

    totalblocks = totalsize / BLOCKSIZE;

    //elog(INFO, "%i", totalblocks);

    if(overflow > 0)
    {
        newval = nseq_concat_slow(a,b);
        PG_RETURN_NSEQ_P (make_nseq(newval,strlen(newval),a->rna));
    }
    else
    {
        newval = (char *) palloc0 (totalblocks);

        memcpy(newval, a->data, a->compressed_size);
        memcpy(newval+(a->size / BLOCKSIZE), b->data, b->compressed_size);
    }

    retval = palloc(CALCDATASZ(a->compressed_size+b->compressed_size));

    retval->rna = a->rna;
    retval->size = totalsize;
    retval->compressed_size = a->compressed_size+b->compressed_size;

    for(i=0; i<HISTSZ; i++)
    {
        retval->histogram[i] = a->histogram[i]+b->histogram[i];
    }

    memcpy(DATAPTR(retval), newval, retval->compressed_size);

    SET_VARSIZE (retval,CALCDATASZ(retval->compressed_size));

    PG_RETURN_NSEQ_P (retval);
}

static char *nseq_concat_slow (const NSEQ *a, const NSEQ *b)
{
    char *decoded, *newval;

    newval = (char *) palloc0 (a->size+b->size+sizeof(char));

    decoded = decode_nseq((const char*)a->data, a->compressed_size, a->rna);

    strncpy(newval, decoded, a->size);

    //elog(INFO, "%s", newval);

    pfree(decoded);

    decoded = decode_nseq((const char*)b->data, b->compressed_size, b->rna);

    strncat(newval, decoded, b->size);

    //elog(INFO, "%s", newval);

    pfree(decoded);

    return newval;
}


