/*
 * Copyright (c) 2012 Leiden University (LERC group at LIACS).
 * All rights reserved.
 *
 * 		adg2csdf.cc
 *
 *		Created on: Dec 19, 2011
 *      Author: Teddy Zhai
 *
 *      History:	Feb. 7, 2012		Initial version
 *
 */

#include <sstream>
#include <iostream>
#include <vector>

#include "barvinok/barvinok.h"

#include "isl/ctx.h"
#include "isl/set.h"

#include "isa/adg.h"
#include "adg_parse.h"

#include "defs.h"
#include "ADG_helper.h"
#include "ImplementationTable.h"
#include "suffix_tree.h"
#include "utility.h"

using namespace adg_helper;

#define TABS(i) std::string((i), '\t')

isl_printer *PRINTER;

/// Csdf Dumper class
class CsdfDumper {
	ADG_helper *ppn;
	phases_t *phases;
	var_domain_t *var_domains;
	ImplementationTable *implTable;
	isl_ctx *ctx;

public:
    CsdfDumper(ADG_helper *adg_helper, ImplementationTable *t, isl_ctx *ctx);
    ~CsdfDumper();
    void DumpCsdf(std::ostream &strm);
    void dumpCsdf3(std::ostream &strm);


private:
    void writePortCsdf3(__isl_keep isl_id *name, std::string type, std::ostream &strm);
    unsigned getWCET(Process *process);

    bool checkPhaseValidity(const Process *process);

    void dumpChannels(std::ostream &strm);

};


/////// Implementation
CsdfDumper::CsdfDumper(ADG_helper *adg_helper, ImplementationTable *t, isl_ctx *ctx) {
  this->ppn = adg_helper;
  this->implTable = t;
  this->ctx = ctx;
}



//// Destructor
CsdfDumper::~CsdfDumper() {
}


void printTest(adg_domain *adgdomain){
	std::cout << "controls: " << adgdomain->controls.size() << std::endl;
	std::cout << "filters: " << adgdomain->filters.size() << std::endl;
}

// Dumps PPN in SDF3 CSDF format.
void CsdfDumper::dumpCsdf3(std::ostream& strm) {
  const Processes nodes = this->ppn->getNodes();
  const Channels edges = this->ppn->getEdges();

  std::ostringstream csdfProps;

  strm << "<?xml version=\"1.0\" encoding=\"UTF-8\"?>\n"
       << "<sdf3 type=\"csdf\" version=\"1.0\" xmlns:xsi=\"http://www.w3.org/2001/XMLSchema-instance\" xsi:noNamespaceSchemaLocation=\"http://www.es.ele.tue.nl/sdf3/xsd/sdf3-csdf.xsd\">\n"
       << "  <applicationGraph name='ppn2csdf'>\n"
       << "    <csdf name='graph' type='csdfgraph'>\n";

  for (unsigned int i = 0; i < nodes.size(); i++){
    Process* process = nodes[i];
    this->var_domains = this->ppn->findVariantDomain2(process);
    this->phases = this->ppn->computePhases(process);

    // Write list of actors and their ports
    strm << "      <actor name='" << isl_id_get_name(process->name) << "' type='x'>\n";
    Channels processEdges = this->ppn->getNodeEdges(process);
    for (PPNchIter eit = processEdges.begin(); eit != processEdges.end(); ++eit) {
      Edge *ch = *eit;
      if (isl_id_cmp(ch->from_node_name, process->name) == 0) {
        writePortCsdf3(ch->from_port_name, "out", strm);
      }
      if (ch->to_node_name == process->name) {
        writePortCsdf3(ch->to_port_name, "in", strm);
      }
    }
    strm << "      </actor>\n";

    // While we're iterating over the actors also fill CSDF properties
    csdfProps << "      <actorProperties actor='" << isl_id_get_name(process->name) << "'>\n"
              << "        <processor type='proc_0' default='true'>\n"
              << "          <executionTime time='";
    int wcet = getWCET(process);
    for (int xi = 0; xi < this->ppn->getPhaseLength((*var_domains)[process->name]); xi++) {
      if (xi != 0) csdfProps << ",";
      csdfProps << wcet;
    }
    csdfProps << "' />\n";
    csdfProps << "        </processor>\n"
              << "      </actorProperties>\n";
  }

  strm << "\n";

  // Write channels
  for (unsigned int i = 0; i < edges.size(); i++){
    Edge *e = edges[i];
    strm << "      <channel name='" << isl_id_get_name(e->name) << "' "
         <<        "srcActor='" << isl_id_get_name(e->from_node_name) << "' "
         <<        "srcPort='" << isl_id_get_name(e->from_port_name) << "' "
         <<        "dstActor='" << isl_id_get_name(e->to_node_name) << "' "
         <<        "dstPort='" << isl_id_get_name(e->to_port_name) << "' "
         <<        "initialTokens='0' "
         << "/>\n";
  }

  strm << "\n";

  // Write the CSDF properties that we collected earlier
  strm << "    </csdf>\n"
       << "    <csdfProperties>\n"
       << csdfProps.str()
       << "    </csdfProperties>\n"
       << "  </applicationGraph>\n"
       << "</sdf3>\n";
}


// Returns the WCET of a process
unsigned
CsdfDumper::getWCET(Process *process) {
  return implTable->getMetric( IM_DELAY_WORST, isl_id_get_name(process->function->name) );
}

// Writes a port in SDF3 format
void CsdfDumper::writePortCsdf3(__isl_keep isl_id *name, std::string type, std::ostream &strm) {
  strm << "        <port type='" << type << "' "
       << (type.compare("in") == 0 ? " " : "")
       <<          "name='" << isl_id_get_name(name) << "' "
       <<          "rate='";
  this->ppn->writePhase(name, strm, ',');
  strm << "' />\n";
}


/* valid phases of a port must at least have one 1*/
bool
CsdfDumper::checkPhaseValidity(const Process *process){

	Ports in_ports = process->input_ports;
	for (int i = 0; i < in_ports.size(); ++i) {
		isl_id *port_id = in_ports[i]->name;
		std::vector<short> *port_phases = (*this->phases)[port_id];

		if (find(port_phases->begin(), port_phases->end(), 1) == port_phases->end()) {
			return false;
		}
	}
	Ports out_ports = process->output_ports;
	for (int i = 0; i < out_ports.size(); ++i) {
		isl_id *port_id = out_ports[i]->name;
		std::vector<short> *port_phases = (*this->phases)[port_id];

		if (find(port_phases->begin(), port_phases->end(), 1) == port_phases->end()) {
			return false;
		}
	}

	return true;
}


void
CsdfDumper::dumpChannels(std::ostream& strm) {
	int indent = 0;
	adg_helper::Edges ppn_channels = this->ppn->getChannels();
	strm << TABS(indent) << "edge_number:" << ppn_channels.size() << "\n";
	// iterate over all channels
	unsigned int edge_ed = 0;
	for (PPNchIter eit = ppn_channels.begin();
			eit != ppn_channels.end();
			++eit)
	{
		Edge* ch = *eit;

		if (ppn->isSelfEdge(ch)) {
			continue;
		}

		strm << TABS(indent) << "edge:" << "\n";
		indent++;
		strm << TABS(indent) << "id:" << edge_ed++ << "\n";
		strm << TABS(indent) << "name:" << isl_id_get_name(ch->name) << "\n";
		strm << TABS(indent) << "src:" << ppn->getId(ch->from_node_name) <<
				" " << ppn->getId(ch->from_port_name) << "\n";
		strm << TABS(indent) << "dst:" << ppn->getId(ch->to_node_name) <<
				" " << ppn->getId(ch->to_port_name) << "\n";
		indent = 0;
	}
}

// print the csdf in the StreamIT-compatible format
void CsdfDumper::DumpCsdf(std::ostream& strm) {
	const Processes procs = this->ppn->getProcesses();

	unsigned int indent = 0;
	const unsigned int nd_nr = procs.size();

	strm<< TABS(indent) << "node_number:" << nd_nr << "\n";

	// iterate over all processes
	for (unsigned int i = 0; i < procs.size(); i++){
		Process* process = procs[i];

		strm<< TABS(indent) << "node:\n";
		indent++;

		strm << TABS(indent) << "id:" << ppn->getId(process) << "\n";
		strm << TABS(indent) << "name:" << isl_id_get_name(process->name) << "\n";

		// there are special cases in which simple patterns, such as [1],  can be derived without further processing
		bool isSimplePattern = this->ppn->checkSimplePattern(process);
////		bool isSimplePattern = false;
		if (isSimplePattern) {
			strm << TABS(indent) << "length:1"  << "\n";

			strm << TABS(indent) << "wcet:" << getWCET(process);
		} else { // simple pattern cannot be found
			this->var_domains = this->ppn->findVariantDomain2(process);

			this->phases = this->ppn->computePhases(process);

			strm << TABS(indent) << "length:" << this->ppn->getPhaseLength((*var_domains)[process->name]) << "\n";
			strm << TABS(indent) << "wcet:" ;
			int wcet = getWCET(process);
			for (unsigned int wc = 1; wc <= this->ppn->getPhaseLength((*var_domains)[process->name]); wc++) {
				strm << wcet;
				if (wc < this->ppn->getPhaseLength((*var_domains)[process->name])) strm << " ";
			}
		} // isSImplePattern

		strm << "\n";


		strm << TABS(indent) << "port_number:" << process->input_ports.size() + process->output_ports.size() <<"\n";
		// iterator over input ports of the process
		for (PortIter pit = process->input_ports.begin();
				pit != process->input_ports.end();
				++pit)
		{
			Port *port = *pit;

			// ignore the port associated with a self-edge
			if (ppn->isSelfEdge(ppn->getEdge(port->edge_name))) {
				continue;
			}

			strm << TABS(indent) << "port:\n";
			indent++;

			strm << TABS(indent) << "type:in" << "\n";
			strm << TABS(indent) << "id:" << ppn->getId(port) << "\n";
			strm << TABS(indent) << "rate:";
			this->ppn->writePhase(port->name, strm, ' ');
			strm << "\n";
			indent--;
		}

		// iterate over all output ports of the process
		for (PortIter pit = process->output_ports.begin();
				pit != process->output_ports.end();
				++pit)
		{
			Port *port = *pit;

			// ignore the port associated with a self-edge
			if (ppn->isSelfEdge(ppn->getEdge(port->edge_name))) {
				continue;
			}

			strm << TABS(indent) << "port:\n";
			indent++;

			strm << TABS(indent) << "type:out" << "\n";
			strm << TABS(indent) << "id:" << ppn->getId(port) << "\n";
			strm << TABS(indent) << "rate:";
			this->ppn->writePhase(port->name, strm, ' ');
			strm << "\n";
			indent--;
		}

		indent = 0;
	} // end processes

	// write channels
	dumpChannels(strm);
}

static int gOutputFormat; // 1 = StreamIT, 3 = SDF3

void printUsage() {
  fprintf(stderr, "Usage: adg2csdf [options] < [adg].yaml\n");
  fprintf(stderr, "Supported options:\n");
  fprintf(stderr, "  -3     Dump in SDF3 format\n");
  fprintf(stderr, "  -s     Dump in StreamIT format [default]\n");
}


int parseCommandline(int argc, char ** argv)
{
  gOutputFormat = 1;
  for (int i = 1; i < argc; i++) {
    if (strcmp(argv[i], "-3") == 0) {
      gOutputFormat = 3;
      return 0;
    }
    else if (strcmp(argv[i], "-s") == 0) {
      gOutputFormat = 1;
      return 0;
    }
    else {
      fprintf(stderr, "Error: unrecognized command line option '%s'\n", argv[i]);
      return 1;
    }
  }

  return 0;
}


int main(int argc, char * argv[])
{
	FILE *in = stdin;

	if (parseCommandline(argc, argv) == 1) {
		printUsage();
		exit(1);
	}

	isl_ctx *ctx;
	ctx = isl_ctx_alloc();

	adg *csdf_adg;
	csdf_adg = adg_parse(ctx, in);
	assert(csdf_adg);

	// initialize the global variable: PRINTER
	PRINTER = isl_printer_to_file(ctx, stdout);
	PRINTER = isl_printer_set_output_format(PRINTER, ISL_FORMAT_ISL);


	ADG_helper *adg_helper = new ADG_helper(csdf_adg, ctx);

	Parameters params = adg_helper->getParameters();
	if (params.size() > 0) {
		fprintf(stderr, "PPN with parameters has no equivalent CSDF\n");
		delete csdf_adg;
		isl_ctx_free(ctx);
		exit(1);
	}

	ImplementationTable *implTable = new ImplementationTable();
	if (!implTable->loadDefaultFile()) {
		fprintf(stderr, "Warning: Could not load implementation data from default files;\n"
				"         please put impldata.xml in the current directory or in ~/.daedalus\n");
	}

	CsdfDumper *dumper = new CsdfDumper(adg_helper, implTable, ctx);
	if (gOutputFormat == 3) {
		dumper->dumpCsdf3(cout);
	}
	else{
		dumper->DumpCsdf(cout);
	}

	delete dumper;
	delete implTable;

	isl_printer_free(PRINTER);
	delete csdf_adg;
	delete adg_helper;
	isl_ctx_free(ctx);

	return 0;
}

