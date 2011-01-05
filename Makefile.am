# Makefile for pntools
# Sven van Haastregt
# LIACS, Leiden University

SUBDIRS = .

ACLOCAL_AMFLAGS = -I m4

ISADIR = @ISA_DIR@
BARVINOK_DIR = $(ISADIR)/barvinok

pdg1ana_LDADD = $(ISADIR)/libpdg.la @SYCK_LIBS@
pdg1ana_SOURCES = \
	pdg1ana.cc

pdg1trans_LDADD = $(ISADIR)/libpdg.la $(BARVINOK_DIR)/libbarvinok.la @SYCK_LIBS@
pdg1trans_SOURCES = \
	pdg1trans.cc transgdal.cc transgdal.h

pn2ppn_LDADD = $(ISADIR)/libpdg.la @SYCK_LIBS@ @OMEGA_LIBS@
pn2ppn_SOURCES = \
  $(ISADIR)/Exit.cc \
	$(ISADIR)/pdgutil.cc \
	pn2ppn.cc ppn.h ppn.cc

ppnta_LDADD = $(ISADIR)/libpdg.la $(BARVINOK_DIR)/libbarvinok.la @SYCK_LIBS@
ppnta_SOURCES = \
	ppnta.cc ppn.h ppn.cc utility.cc utility.h

bin_PROGRAMS = pdg1ana pdg1trans ppnta pn2ppn

include local.am

