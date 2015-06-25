/************************************************************************
 * nseq - a storage efficient nucleotide sequence datatype for PostgreSQL
 *
 * Copyright (c) 2014,2015 by Ernst-G. Schmid
 *
 ************************************************************************/

#include <stddef.h>
#include "postgres.h"

#ifndef ___NSEQ_H__
#define ___NSEQ_H__

#define BLOCKSIZE 4*sizeof(char)
#define HISTSZ 4

typedef struct NSEQ
{
	int32		        vl_len_;		    /* varlena header (do not touch directly!) */
	bool                rna;                /* DNA or RNA */
	int32               histogram[HISTSZ];  /* nucleodide histogram */
	uint32		        size;			    /* number of characters when expanded */
	uint32		        compressed_size;	/* number of characters when compressed */
	char		        data[1];		    /* variable length data */
} NSEQ;

#define DATAPTR(x)		 	         (x->data)
#define CALCDATASZ(compressed_size)  (sizeof(NSEQ) + compressed_size)

#define DatumGetNseqP(n)         ((NSEQ *) PG_DETOAST_DATUM(n))
#define PG_GETARG_NSEQ_P(n)      DatumGetNseqP(PG_GETARG_DATUM(n))
#define PG_RETURN_NSEQ_P(n)      PG_RETURN_POINTER(n)

#endif   /* __NSEQ_H__ */
