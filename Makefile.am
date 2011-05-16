# Makefile for pntools
# Sven van Haastregt
# LIACS, Leiden University

SUBDIRS = .

ACLOCAL_AMFLAGS = -I m4

ISADIR = @ISA_DIR@
ISALIB = $(ISADIR)/libpdg.la @SYCK_LIBS@
BARVINOKDIR = $(ISADIR)/barvinok
BARVINOKLIB = $(BARVINOKDIR)/libbarvinok.la


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

pdg1ana_LDADD = $(ISALIB)
pdg1ana_SOURCES = \
	pdg1ana.cc

pdg1trans_LDADD = $(ISALIB) $(BARVINOKLIB)
pdg1trans_SOURCES = \
	pdg1trans.cc transgdal.cc transgdal.h

pn2loopo_LDADD = $(ISALIB)
pn2loopo_SOURCES = \
	pn2loopo.cc

pn2ppn_LDADD = libppn.la $(ISALIB) @CLOOG_LIBS@
pn2ppn_SOURCES = \
	pn2ppn_util.cc \
	pn2ppn.cc

ppn2csdf_LDADD = libppn.la $(ISADIR)/libpdg.la @SYCK_LIBS@
ppn2csdf_SOURCES = \
	ImplementationTable.cc \
  ImplementationTable.h \
	ppn2csdf.cc

ppnta_LDADD = libppn.la $(ISALIB) $(BARVINOKLIB)
ppnta_SOURCES = \
	ppnta.cc utility.cc utility.h

bin_PROGRAMS = pdg1ana pdg1trans ppnta ppn2csdf pn2ppn pn2loopo

include local.am

