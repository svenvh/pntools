# Makefile for pntools
# Sven van Haastregt
# LIACS, Leiden University

SUBDIRS = .

ACLOCAL_AMFLAGS = -I m4

ISADIR = @ISA_DIR@
BARVINOK_DIR = $(ISADIR)/barvinok


### Libraries

lib_LTLIBRARIES = libppn.la
libppn_la_SOURCES = \
	ast.cc \
	ast.h \
	ppn.cc \
	ppn.h \
	tarjan.cc \
	tarjan.h
libppn_la_LIBADD = $(ISADIR)/libpdg.la $(ISADIR)/isl/libisl.la
libppn_la_LDFLAGS = -release @VERSION@


### Applications

pdg1ana_LDADD = $(ISADIR)/libpdg.la @SYCK_LIBS@
pdg1ana_SOURCES = \
	pdg1ana.cc

pdg1trans_LDADD = $(ISADIR)/libpdg.la $(BARVINOK_DIR)/libbarvinok.la @SYCK_LIBS@
pdg1trans_SOURCES = \
	pdg1trans.cc transgdal.cc transgdal.h

pn2ppn_LDADD = libppn.la $(ISADIR)/libpdg.la @SYCK_LIBS@ @CLOOG_LIBS@
pn2ppn_SOURCES = \
	pn2ppn_util.cc \
	pn2ppn.cc

ppnta_LDADD = libppn.la $(ISADIR)/libpdg.la $(BARVINOK_DIR)/libbarvinok.la @SYCK_LIBS@
ppnta_SOURCES = \
	ppnta.cc utility.cc utility.h

bin_PROGRAMS = pdg1ana pdg1trans ppnta pn2ppn

include local.am

