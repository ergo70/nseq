DROP TYPE IF EXISTS dna CASCADE;
DROP TYPE IF EXISTS rna CASCADE;

CREATE TYPE dna;
CREATE TYPE rna;

CREATE OR REPLACE FUNCTION dna_in(cstring)
    RETURNS dna
    AS 'nseq'
    LANGUAGE C IMMUTABLE STRICT;

    CREATE OR REPLACE FUNCTION dna_in(text)
    RETURNS dna
    AS 'nseq'
    LANGUAGE C IMMUTABLE STRICT;


CREATE OR REPLACE FUNCTION nseq_out(dna)
    RETURNS cstring
   AS 'nseq'
    LANGUAGE C IMMUTABLE STRICT;

    CREATE OR REPLACE FUNCTION rna_in(cstring)
    RETURNS rna
    AS 'nseq'
    LANGUAGE C IMMUTABLE STRICT;

     CREATE OR REPLACE FUNCTION rna_in(text)
    RETURNS rna
    AS 'nseq'
    LANGUAGE C IMMUTABLE STRICT;

CREATE OR REPLACE FUNCTION nseq_out(rna)
    RETURNS cstring
   AS 'nseq'
    LANGUAGE C IMMUTABLE STRICT;


CREATE TYPE dna (
   input = dna_in,
   output = nseq_out,
   alignment = int4,
   --receive = molecule_recv,
   --send = molecule_send,
   internallength = VARIABLE,
   storage = MAIN
   );

   CREATE TYPE rna (
   input = rna_in,
   output = nseq_out,
   alignment = int4,
   --receive = molecule_recv,
   --send = molecule_send,
   internallength = VARIABLE,
   storage = MAIN
   );

   CREATE OR REPLACE FUNCTION nseq_equals(dna,dna)
RETURNS bool
AS 'nseq'
LANGUAGE C STRICT;

   CREATE OR REPLACE FUNCTION nseq_equals(rna,rna)
RETURNS bool
AS 'nseq'
LANGUAGE C STRICT;

CREATE OPERATOR = (
		 LEFTARG = dna,
		 RIGHTARG = dna,
		 PROCEDURE = nseq_equals,
		 COMMUTATOR = =,
		 RESTRICT = eqsel,
		 JOIN = eqjoinsel
);

CREATE OPERATOR = (
		 LEFTARG = rna,
		 RIGHTARG = rna,
		 PROCEDURE = nseq_equals,
		 COMMUTATOR = =,
		 RESTRICT = eqsel,
		 JOIN = eqjoinsel
);

   CREATE OR REPLACE FUNCTION nseq_concat(dna,dna)
RETURNS dna
AS 'nseq'
LANGUAGE C STRICT;

   CREATE OR REPLACE FUNCTION nseq_concat(rna,rna)
RETURNS rna
AS 'nseq'
LANGUAGE C STRICT;

CREATE OPERATOR || (
		 LEFTARG = dna,
		 RIGHTARG = dna,
		 PROCEDURE = nseq_concat
);

CREATE OPERATOR || (
		 LEFTARG = rna,
		 RIGHTARG = rna,
		 PROCEDURE = nseq_concat
);

CREATE OR REPLACE FUNCTION length(dna)
    RETURNS int4
   AS 'nseq','nseq_length'
    LANGUAGE C IMMUTABLE STRICT;

    CREATE OR REPLACE FUNCTION length(rna)
    RETURNS int4
   AS 'nseq','nseq_length'
    LANGUAGE C IMMUTABLE STRICT;

    CREATE OR REPLACE FUNCTION size(dna)
    RETURNS int4
   AS 'nseq','nseq_size'
    LANGUAGE C IMMUTABLE STRICT;

    CREATE OR REPLACE FUNCTION size(rna)
    RETURNS int4
   AS 'nseq','nseq_size'
    LANGUAGE C IMMUTABLE STRICT;

    CREATE OR REPLACE FUNCTION transscribe(dna)
    RETURNS rna
   AS 'nseq','nseq_transscribe'
    LANGUAGE C IMMUTABLE STRICT;

    CREATE OR REPLACE FUNCTION transscribe(rna)
    RETURNS dna
   AS 'nseq','nseq_transscribe'
    LANGUAGE C IMMUTABLE STRICT;

    CREATE OR REPLACE FUNCTION complement(rna)
    RETURNS cstring
   AS 'nseq','nseq_out_complement'
    LANGUAGE C IMMUTABLE STRICT;

      CREATE OR REPLACE FUNCTION complement(dna)
    RETURNS cstring
   AS 'nseq','nseq_out_complement'
    LANGUAGE C IMMUTABLE STRICT;
    

    CREATE OR REPLACE FUNCTION reverse(rna)
  RETURNS rna AS
$BODY$
DECLARE
BEGIN
    RETURN reverse($1::text)::rna;
END;
$BODY$
  LANGUAGE plpgsql IMMUTABLE STRICT;

      CREATE OR REPLACE FUNCTION reverse(dna)
  RETURNS dna AS
$BODY$
DECLARE
BEGIN
    RETURN reverse($1::text)::dna;
END;
$BODY$
  LANGUAGE plpgsql IMMUTABLE STRICT;

    CREATE OR REPLACE FUNCTION histogram(dna)
    RETURNS int4[]
   AS 'nseq','nseq_histogram'
    LANGUAGE C IMMUTABLE STRICT;

     CREATE OR REPLACE FUNCTION histogram(rna)
    RETURNS int4[]
   AS 'nseq','nseq_histogram'
    LANGUAGE C IMMUTABLE STRICT;

  CREATE OR REPLACE FUNCTION histogram_rel(dna)
  RETURNS float[4] AS
$BODY$
DECLARE
l float;
h int[4];
r float[4];
BEGIN
l := length($1);
h := histogram($1);

FOR i IN 1..4 LOOP
	r[i] = h[i]/l;
END LOOP;

    RETURN r;
END;
$BODY$
  LANGUAGE plpgsql IMMUTABLE STRICT;

   CREATE OR REPLACE FUNCTION histogram_rel(rna)
  RETURNS float[4] AS
$BODY$
DECLARE
l float;
h int[4];
r float[4];
BEGIN
l := length($1);
h := histogram($1);

FOR i IN 1..4 LOOP
	r[i] = h[i]/l;
END LOOP;

    RETURN r;
END;
$BODY$
  LANGUAGE plpgsql IMMUTABLE STRICT;

CREATE OR REPLACE FUNCTION random_dna(integer)
  RETURNS dna AS
$BODY$
SELECT array_to_string(
    ARRAY (
        SELECT substring(
            'ACGT'
            FROM (ceil(random()*4))::int FOR 1
        )
        FROM generate_series(1, $1)
    ), 
    ''
)::dna
$BODY$
  LANGUAGE sql VOLATILE STRICT
  COST 1000;

CREATE OR REPLACE FUNCTION random_rna(integer)
  RETURNS rna AS
$BODY$
SELECT array_to_string(
    ARRAY (
        SELECT substring(
            'ACGU'
            FROM (ceil(random()*4))::int FOR 1
        )
        FROM generate_series(1, $1)
    ), 
    ''
)::rna
$BODY$
  LANGUAGE sql VOLATILE STRICT
  COST 1000;

