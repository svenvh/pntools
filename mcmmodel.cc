/*
 * mcmmodel.cc
 *
 * Purpose: derive an MCM modeling graph from a PDG (and later perhaps also ADG).
 * Authors: Hristo Nikolov, Sven van Haastregt
 *
 * Copyright (c) 2012 Leiden University (LERC group at LIACS).
 * All rights reserved.
 *
 */

#include <sstream>
#include <iostream>
#include <vector>

#include <isa/pdg.h>
#include "barvinok/barvinok.h"

#include "ImplementationTable.h"
#include "PDG_helper.h"
#include "utility.h"


using namespace pdg_helper;

// Classification of an edge in MCM model
enum McmChannelType {
  FEEDFORWARD,        // Connects an SCC to another SCC
  FEEDBACK,           // Connects two processes in the same SCC
  SELFLOOP            // Connects process to itself
};

//// MCM Model Dumper class
class McmModelDumper {
  public:
    McmModelDumper(ImplementationTable *t, PDG *pdg);
    ~McmModelDumper();
    void dump(std::ostream &strm);

  private:
    void determineChannelTypes(std::vector<McmChannelType> &channelTypes);
    int getFeedbackChannelSize(pdg::dependence const *dep);
    void writePort(pdg::dependence const *dep, std::string name, std::string type, std::ostream &strm);
    void writeChannel(std::ostream &strm);
    int getDependenceCardinality(pdg::dependence const *dep);
    unsigned getWCET(pdg::node const *node);
    unsigned getII(pdg::node const *node);
    std::string getPortName(int n, const char *direction);
    std::string getBackedgePortName(int n, const char *direction);

    PDG_helper *pdgHelper;
    ImplementationTable *implTable;
    PDG *pdg;

};


//// Implementation
// Constructor
McmModelDumper::McmModelDumper(ImplementationTable *t, PDG *pdg) {
  this->implTable = t;
  this->pdg = pdg;
  this->pdgHelper = new PDG_helper(pdg->ctx, pdg);
}


// Destructor
McmModelDumper::~McmModelDumper() {
  delete this->pdgHelper;
}


// Returns a vector containing for each dependence the type of the channel in the SDF graph.
void McmModelDumper::determineChannelTypes(std::vector<McmChannelType> &channelTypes) {
  channelTypes.clear();
  for (unsigned int j = 0; j < pdg->dependences.size(); j++) {
    pdg::dependence *dep = pdg->dependences[j];
    McmChannelType type = FEEDFORWARD;

    if (dep->from == dep->to) {
      type = SELFLOOP;
    }
    else if (this->pdgHelper->isInSCC(dep)) {
      type = FEEDBACK;
    }

    channelTypes.push_back(type);
  }
}


// Returns channel size for a feedback channel
int McmModelDumper::getFeedbackChannelSize(pdg::dependence const *dep) {
  bool putInitialTokens = this->pdgHelper->firesBefore(dep->from, dep->to);
  if (putInitialTokens) {
    pDeps_t reverseEdges = this->pdgHelper->getEdges(dep->to, dep->from);
    int maxRevEdge = 0;
    for (int i = 0; i < reverseEdges.size(); i++) {
      if (reverseEdges[i]->value_size->v > maxRevEdge) {
        maxRevEdge = reverseEdges[i]->value_size->v;
      }
    }
    return maxRevEdge;
  }
  else {
    return 0;
  }
}


// Dumps MCM model for PDG in SDF3 SDF format.
void McmModelDumper::dump(std::ostream& strm) {
  std::ostringstream sdfProps;
  std::ostringstream selfLoopChannels;

  std::vector<McmChannelType> channelTypes;
  determineChannelTypes(channelTypes);

  strm << "<?xml version=\"1.0\" encoding=\"UTF-8\"?>\n"
       << "<sdf3 type=\"sdf\" version=\"1.0\" xmlns:xsi=\"http://www.w3.org/2001/XMLSchema-instance\" xsi:noNamespaceSchemaLocation=\"http://www.es.ele.tue.nl/sdf3/xsd/sdf3-sdf.xsd\">\n"
       << "  <applicationGraph name='" << this->pdg->name->s << "'>\n"
       << "    <sdf name='" << this->pdg->name->s << "' type='mcmmodel'>\n";

  for (unsigned int i = 0; i < pdg->nodes.size(); i++){
    pdg::node *node = pdg->nodes[i];

    // Write list of actors and their ports
    strm << "      <actor name='" << node->name->s << "'"
         << " type='" << this->pdgHelper->getFunctionName(node) << "'>\n";

    // Introduce a selfloop to limit autoconcurrency
    writePort(NULL, "slout", "out", strm);
    writePort(NULL, "slin", "in", strm);
    selfLoopChannels << "      <channel name='sl_" << node->name->s << "' "
                     <<        "srcActor='" << node->name->s << "' "
                     <<        "srcPort='slout' "
                     <<        "dstActor='" << node->name->s << "' "
                     <<        "dstPort='slin' "
                     <<        "initialTokens='1' />\n";

    // Write the "real" ports of the actor
    for (unsigned int j = 0; j < pdg->dependences.size(); j++) {
      pdg::dependence *dep = pdg->dependences[j];
      if (dep->from == node) {
        writePort(dep, getPortName(j, "out"), "out", strm);
      }
      if (dep->to == node) {
        writePort(dep, getPortName(j, "in"), "in", strm);
      }
    }

    // Write the ports for the backedges connected to this actor
    for (unsigned int j = 0; j < pdg->dependences.size(); j++) {
      pdg::dependence *dep = pdg->dependences[j];

      // Skip if we don't want a backedge
      if (channelTypes[j] != FEEDFORWARD)
        continue;

      if (dep->from == node) {
        writePort(dep, getBackedgePortName(j, "in"), "in", strm);
      }
      if (dep->to == node) {
        writePort(dep, getBackedgePortName(j, "out"), "out", strm);
      }
    }
    strm << "      </actor>\n";

    // While we're iterating over the actors also fill SDF properties
    sdfProps << "      <actorProperties actor='" << node->name->s << "'>\n"
             << "        <processor type='proc_0' default='true'>\n"
             << "          <executionTime time='";
    int wcet = getWCET(node);
    isl_set *domain = node->source->get_isl_set();
    int points = getCardinality(domain);
    isl_set_free(domain);
    sdfProps << (points*wcet);
    sdfProps << "' />"
             << "  <!-- " << points << " * " << wcet << " -->\n"
             << "        </processor>\n"
             << "      </actorProperties>\n";
  }

  strm << "\n";

  // Write channels
  for (unsigned int i = 0; i < pdg->dependences.size(); i++) {
    pdg::dependence *dep = pdg->dependences[i];
    char channelName[16];
    int channelSize;

    if (dep->reordering == 0 && dep->value_size) {
      // FIFO with integer (non-parametric) size
      if (channelTypes[i] == SELFLOOP) {
        channelSize = dep->value_size->v * getDependenceCardinality(dep);
      }
      else if (channelTypes[i] == FEEDFORWARD) {
        channelSize = (1+dep->value_size->v) * getDependenceCardinality(dep);
      }
      else if (channelTypes[i] == FEEDBACK) {
        channelSize = (1+getFeedbackChannelSize(dep)) * getDependenceCardinality(dep);
      }
    }
    else {
      // Assume worst-case
      fprintf(stderr, "ED_%d: Assuming channel size of %d\n", i, getDependenceCardinality(dep));
      channelSize = getDependenceCardinality(dep) * getDependenceCardinality(dep);
    }

    // "Real" edge
    snprintf(channelName, sizeof(channelName), "ED_%d", i);
    strm << "      <channel name='" << channelName << "' "
         <<        "srcActor='" << dep->from->name->s << "' "
         <<        "srcPort='" << getPortName(i, "out") << "' "
         <<        "dstActor='" << dep->to->name->s << "' "
         <<        "dstPort='" << getPortName(i, "in") << "' "
         <<        "initialTokens='" << (channelTypes[i]==FEEDFORWARD ? 0 : channelSize) << "' "
         << "/>\n";

    if (channelTypes[i] == FEEDFORWARD) {
      // Corresponding backedge
      snprintf(channelName, sizeof(channelName), "BE_%d", i);
      strm << "      <channel name='" << channelName << "' "
           <<        "srcActor='" << dep->to->name->s << "' "
           <<        "srcPort='" << getBackedgePortName(i, "out") << "' "
           <<        "dstActor='" << dep->from->name->s << "' "
           <<        "dstPort='" << getBackedgePortName(i, "in") << "' "
           <<        "initialTokens='" << channelSize << "' "
           << "/>\n";
    }
  }

  // Write the selfloop channels
  strm << "\n"
       << selfLoopChannels.str();

  // Write the CSDF properties that we collected earlier
  strm << "    </sdf>\n"
       << "\n"
       << "    <sdfProperties>\n"
       << sdfProps.str()
       << "    </sdfProperties>\n"
       << "  </applicationGraph>\n"
       << "</sdf3>\n";
}


// Returns the number of points in the dependence relation
int McmModelDumper::getDependenceCardinality(pdg::dependence const *dep) {
  isl_map *rel = dep->relation->get_isl_map();
  isl_set *portDomain = isl_map_domain(rel);
  int card = getCardinality(portDomain);
  isl_set_free(portDomain);
  return card;
}


// Returns the WCET of a process
unsigned McmModelDumper::getWCET(pdg::node const *node) {
  return implTable->getMetric( IM_DELAY_WORST, this->pdgHelper->getFunctionName(node)) ;
}


// Returns the II of a process
unsigned McmModelDumper::getII(pdg::node const *node) {
  return implTable->getMetric(IM_II, this->pdgHelper->getFunctionName(node));
}


// Returns a port name of the form ED_n_direction
std::string McmModelDumper::getPortName(int n, const char *direction) {
  std::ostringstream portName;
  portName << "ED_" << n << "_" << direction;
  return portName.str();
}


// Returns a port name of the form BE_n_direction
std::string McmModelDumper::getBackedgePortName(int n, const char *direction) {
  std::ostringstream portName;
  portName << "BE_" << n << "_" << direction;
  return portName.str();
}


// Writes a port in SDF3 format
void McmModelDumper::writePort(pdg::dependence const *dep, std::string name, std::string type, std::ostream &strm) {
  // Set padding to nicely align columns regardless of whether it's "in" or "out".
  const char *padding = (type.compare("in") == 0 ? " " : "");

  // Port rate is the number of tokens transferred for regular channels and their backedges,
  // or 1 for autoconcurrency-limiting selfloops (for which no pdg::dependence exists).
  int rate = 1;
  if (dep) {
    rate = getDependenceCardinality(dep);
  }

  strm << "        <port type='" << type << "' " << padding
       <<          "name='" << name << "' " << padding
       <<          "rate='" << rate << "' />\n";
}


//// Other functions

void printUsage() {
  fprintf(stderr, "Usage: mcmmodel < XXXpn.yaml > XXX.sdf\n");
}


int parseCommandline(int argc, char ** argv)
{
  for (int i = 1; i < argc; i++) {
    fprintf(stderr, "Error: unrecognized command line option '%s'\n", argv[i]);
    return 1;
  }

  return 0;
}


int main(int argc, char * argv[])
{
  FILE *in = stdin;

  if (parseCommandline(argc, argv) != 0) {
    printUsage();
    exit(1);
  }

  isl_ctx *ctx = isl_ctx_alloc();

  // Load the PDG and perform some sanity checks
  PDG *pdg = PDG::Load(in, ctx);
  if (!pdg) {
    fprintf(stderr, "No PDG specified or PDG invalid.\n");
    exit(1);
  }

  if (pdg->params.size() > 0) {
    fprintf(stderr, "Specified PDG contains parameters.\n");
    exit(1);
  }

  if (pdg->dependences.size() == 0) {
    fprintf(stderr, "Specified PDG does not contain any dependences.\n");
    exit(1);
  }

  // Load function implementation data
  ImplementationTable *implTable = new ImplementationTable();
  if (!implTable->loadDefaultFile()) {
    fprintf(stderr, "Warning: Could not load implementation data from default files;\n"
                    "         please put impldata.xml in the current directory or in ~/.daedalus\n");
  }

  McmModelDumper *dumper = new McmModelDumper(implTable, pdg);
  dumper->dump(cout);

  // Cleanup
  delete dumper;
  delete implTable;
  pdg->free();
  delete pdg;

  isl_ctx_free(ctx);

  return 0;
}
