# contrib/nseq/Makefile
MODULE_big=nseq
OBJS=nseq_op.o nseq_io.o nseq.o

EXTENSION=nseq
DATA=nseq--1.0.sql nseq--unpackaged--1.0.sql

REGRESS=nseq

PG_CPPFLAGS=-O2 

ifdef USE_PGXS
PG_CONFIG = pg_config
PGXS := $(shell $(PG_CONFIG) --pgxs)
include $(PGXS)
else
subdir = contrib/nseq
top_builddir = ../..
include $(top_builddir)/src/Makefile.global
include $(top_srcdir)/contrib/contrib-global.mk
endif
