# Makefile for pntools
# Sven van Haastregt
# LIACS, Leiden University

SUBDIRS = .

ACLOCAL_AMFLAGS = -I m4

ISADIR = @ISA_DIR@
ISALIB = $(ISADIR)/libpdg.la
ADGLIB = $(ISADIR)/libadg.la
ISLDIR = $(ISADIR)/isl
ISLLIB = $(ISLDIR)/libisl.la
BARVINOKDIR = $(ISADIR)/barvinok
BARVINOKLIB = $(BARVINOKDIR)/libbarvinok.la


### Libraries

## comment tarjan temporarily
#	tarjan.cc \
#	tarjan.h
lib_LTLIBRARIES = libadg_helper.la
libadg_helper_la_SOURCES = \
	ADG_helper.cc \
	ADG_helper.h 
libadg_helper_la_LIBADD = $(ISADIR)/libpdg.la $(ISLLIB) $(ADGLIB)
libadg_helper_la_LDFLAGS = -release @VERSION@


### Applications

# pdg1ana_LDADD = $(ISALIB)
# pdg1ana_SOURCES = \
# 	pdg1ana.cc

# pdg1trans_LDADD = $(ISALIB) $(BARVINOKLIB)
# pdg1trans_SOURCES = \
# 	pdg1trans.cc #transgdal.cc transgdal.h

# pn2ppn_LDADD = libppn.la $(ISALIB) @CLOOG_LIBS@
# pn2ppn_SOURCES = \
# 	pn2ppn_util.cc \
# 	pn2ppn.cc

adg2csdf_LDADD = libadg_helper.la $(ISALIB) $(ADGLIB) @libxml2_LIBS@ @SYCK_LIBS@ $(BARVINOKLIB)
adg2csdf_SOURCES = \
	suffix_tree.cc suffix_tree.h \
	ImplementationTable.cc \
	ImplementationTable.h \
	utility.cc utility.h \
	adg2csdf.cc

adgstat_LDADD = $(ADGLIB)
adgstat_SOURCES = adgstat.cc

pdgtrans_LDADD = $(ISALIB) @SYCK_LIBS@ $(BARVINOKLIB)
pdgtrans_SOURCES = \
	TransReport.cc TransReport.h \
	SetDivider.cc SetDivider.h \
	SplitCommand.cc SplitCommand.h \
	TransCLParser.cc TransCLParser.h \
	pdgtrans.cc

visset_LDADD = $(ISLLIB)
visset_SOURCES = visset.cc

#ppnta_LDADD = libppn.la $(ISALIB) $(BARVINOKLIB)
#ppnta_SOURCES = \
#	#ppnta.cc utility.cc utility.h

plamapgen_SOURCES = plamapgen.cc



bin_PROGRAMS = adg2csdf adgstat pdgtrans visset plamapgen

include local.am

