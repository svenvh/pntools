/*
 * Copyright (c) 2012 Leiden University (LERC group at LIACS).
 * All rights reserved.
 *
 * adgromgen.cc - ADG ROM Generation
 *
 * Created on: Feb 29, 2012
 * Author: Sven van Haastregt
 */

#include <iostream>

#include <adg_parse.h>

#include "ADG_helper.h"
#include "utility.h"

using namespace adg_helper;

//// ADG ROM Generator class
class ADGROMGenerator {
  public:
    ADGROMGenerator(ADG_helper *adg_helper);
    ~ADGROMGenerator();

    // Writes ROMs for all nodes of the associated ADG to an ostream.
    void generateROMs(std::ostream &strm);

  private:
    Ports sortPorts(const Args &args, Ports &ports);
    bool isReusePort(const Port *port);
    void appendReusePorts(Ports &sortedList, Ports &outPorts);

    ADG_helper *adg;                  // Associated ADG
    phases_t *phases;                 // Computed phases
    void dumpVhdlROM(std::ostream &strm, Node *node, Ports &ports, bool isWriteUnit);
};


//// ADGROMGenerator Implementation
// Constructor
ADGROMGenerator::ADGROMGenerator(ADG_helper *adg_helper) {
  this->adg = adg_helper;
}


// Destructor
ADGROMGenerator::~ADGROMGenerator() {
}


// Prints ROM in VHDL for a list of ports
void ADGROMGenerator::dumpVhdlROM(std::ostream &strm, Node *node, Ports &ports, bool isWriteUnit) {
  const char *portCountVarName = isWriteUnit ? "N_OUT_PORTS" : "N_IN_PORTS";
  if (ports.size() > 0) {
    // Compute number of times that ROM pattern should be repeated during runtime
    int phaseLength = (*this->phases)[ports[0]->name]->size();
    isl_set *node_domain = getPDGDomain(node->domain);
    int domainCard = getCardinality(node_domain);
    isl_set_free(node_domain);
    int phaseRepeat = domainCard / phaseLength;

    // Write VHDL declarations
    strm << "  -- ROM Address range\n"
         << "  constant rom_LOW    : natural := 0;\n"
         << "  constant rom_HIGH   : natural := " << phaseLength-1 << ";\n"
         << "  constant outer_LOW  : natural := 0;\n"
         << "  constant outer_HIGH : natural := " << phaseRepeat-1 << ";\n"
         << "  signal rom_addr : integer range rom_LOW to rom_HIGH;\n"
         << "  signal outer_i  : integer range outer_LOW to outer_HIGH;\n"
         << "  signal rom_data : std_logic_vector(" << portCountVarName << "-1 downto 0);\n"
         << "  signal sl_done  : std_logic;\n"
         << "  type ctrl_rom_type is array (rom_LOW to rom_HIGH) of std_logic_vector(" << portCountVarName << "-1 downto 0);\n"
         << "  signal ctrl_rom : ctrl_rom_type := (";

    // Generate a word in the ROM for each phase.
    // Each word contains a control bit for all ports.
    for (unsigned int j = 0; j < phaseLength; j++) {
      if (j) {
        strm << ", ";               // Print separator if it's not the first element
      }
      if (j % 10 == 0) {
        strm << "\n    ";           // Force newline after 10 elements
      }

      strm << "\"";
      // Iterate over ports in reverse order. This causes the control bit for the
      // last port to be the leftmost bit.
      for (Ports::reverse_iterator pit = ports.rbegin(); pit != ports.rend(); ++pit) {
        Port *port = *pit;
        strm << (*this->phases)[port->name]->operator[](j);
      }
      strm << "\"";
    }
    strm << "\n";
    strm << "  );\n";

    // Dump order of ports
    // This is used as a safety check by ESPAM, to ensure that this tool and ESPAM use the same port order.
    strm << "  -- Order:";
    for (Ports::reverse_iterator pit = ports.rbegin(); pit != ports.rend(); ++pit) {
      strm << " " << isl_id_get_name((*pit)->name);
    }
    strm << "\n";
  }
  strm << "END\n";
}


// Sorts the ports of a node in the same way as ESPAM's CompaanHWNodeIseVisitor.
// We sort by iterating over the list of arguments and finding the corresponding ports for
// each argument.
Ports ADGROMGenerator::sortPorts(const Args &args, Ports &ports) {
  Ports r;
  for (Args::const_iterator vi = args.begin(); vi != args.end(); ++vi) {
    const Arg *arg = *vi;
    isl_id *argName = isl_pw_multi_aff_get_tuple_id(arg->var->access, isl_dim_out);

    for (Ports::iterator pi = ports.begin(); pi != ports.end(); ++pi) {
      Port *port = *pi;
      assert(port->vars.size() == 1);
      isl_id *portArgName = isl_pw_multi_aff_get_tuple_id(port->vars[0]->access, isl_dim_out);
      if (argName == portArgName) {
        r.push_back(port);
      }
      isl_id_free(portArgName);
    }
    isl_id_free(argName);
  }
  return r;
}


// Returns true if port is a reuse port.
// That is, if the associated var starts with "in", then it is probably a reuse port.
bool ADGROMGenerator::isReusePort(const Port *port) {
  bool ret = false;
  isl_id *varName = isl_pw_multi_aff_get_tuple_id(port->vars[0]->access, isl_dim_out);
  if (strncmp(isl_id_get_name(varName), "in", 2) == 0) {
    ret = true;
  }
  isl_id_free(varName);
  return ret;
}


// Appends the reuse ports to the already sorted list of output ports.
// This resembles the order of ports derived by ESPAM's CompaanHWNodeIseVisitor.
void ADGROMGenerator::appendReusePorts(Ports &sortedList, Ports &outPorts) {
  for (Ports::iterator pi = outPorts.begin(); pi != outPorts.end(); ++pi) {
    Port *port = *pi;
    assert(port->vars.size() == 1);
    if (isReusePort(port)) {
      sortedList.push_back(port);
    }
  }
}


// Generate ROMs for all nodes
void ADGROMGenerator::generateROMs(std::ostream &strm) {
  const Nodes procs = this->adg->getNodes();
  const unsigned int nNodes = procs.size();
  for (unsigned int i = 0; i < nNodes; i++){
    Node *node = procs[i];

    // Compute phases for current node
    this->adg->findVariantDomain2(node);
    this->phases = this->adg->computePhases(node);

    // Write tag and dump ROM for read unit
    strm << isl_id_get_name(node->name) << ".EVAL_LOGIC_RD:" << "\n";
    Ports inports = sortPorts(node->function->in, node->input_ports);
    dumpVhdlROM(strm, node, inports, false);

    // Write tag and dump ROM for write unit
    strm << isl_id_get_name(node->name) << ".EVAL_LOGIC_WR:" << "\n";
    Ports outports = sortPorts(node->function->out, node->output_ports);
    appendReusePorts(outports, node->output_ports);
    dumpVhdlROM(strm, node, outports, true);
  }
}


//// Main functions

void printUsage() {
  fprintf(stderr, "Usage: adgromgen < file.adg\n");
}


int main(int argc, char * argv[]) {
  FILE *in = stdin;

  isl_ctx *ctx;
  ctx = isl_ctx_alloc();

  adg *graph;
  graph = adg_parse(ctx, in);
  if (!graph) {
    fprintf(stderr, "Input is not a valid ADG.\n");
    printUsage();
    exit(1);
  }

  ADG_helper *adg_helper = new ADG_helper(graph, ctx);

  Parameters params = adg_helper->getParameters();
  if (params.size() > 0) {
    fprintf(stderr, "ADG Contains parameters; ROM generation not supported.\n");
    delete graph;
    isl_ctx_free(ctx);
    exit(1);
  }

  ADGROMGenerator gen(adg_helper);
  gen.generateROMs(cout);

  delete graph;
  delete adg_helper;
  isl_ctx_free(ctx);

  return 0;
}
