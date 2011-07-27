//
// Convert PPN to CSDF
// Sven van Haastregt, Teddy Zhai, May 2011
// LERC, LIACS, Leiden University
// $Id: ppn2csdf.cc,v 1.10 2011/07/27 13:11:22 svhaastr Exp $
//
#include <sstream>
#include <iostream>

#include "barvinok/barvinok.h"

#include "isa/yaml.h"
#include "isa/pdg.h"
#include "ppn.h"
#include "defs.h"
#include "ImplementationTable.h"

using pdg::PDG;
using ppn::PPN;
//using namespace std;
using namespace ppn;

#define TABS(i) std::string((i), '\t')

typedef std::map<std::string, std::vector<short>* > phaseMap_t;

///// Csdf Dumper class
// Currently only used by ppn2csdf, so declaration is inside this file.
class CsdfDumper {
  public:
    CsdfDumper(PPN *newPpn, ImplementationTable *t);
    ~CsdfDumper();
    void dumpCsdf(std::ostream &strm);
    void dumpCsdf3(std::ostream &strm);

  private:
    void writePortCsdf3(std::string name, std::string type, std::ostream &strm);
    void computePhases();
    void writePhase(const std::string &portName, std::ostream &strm, char sep);
    void extendPhase(unsigned int nodenr);
    unsigned int getPhaseLength(unsigned int nodenr);
    void processTrace(FILE *fin);
    unsigned int getPortNr(const std::string &portname);
    int getWCET(Process *process);
    PPN *ppn;
    phaseMap_t phases;
    ImplementationTable *implTable;

};



///// Implementation

CsdfDumper::CsdfDumper(PPN *newPpn, ImplementationTable *t) {
  this->ppn = newPpn;
  this->implTable = t;
  
  // Allocate vectors for each port
  for (unsigned int i = 0; i < this->ppn->getEdges().size(); ++i) {
    edge *ch = this->ppn->getEdges()[i];
    phases[ch->from_port->s] = new std::vector<short>;
    phases[ch->to_port->s] = new std::vector<short>;
  }
}



// Destructor
CsdfDumper::~CsdfDumper() {
  for (phaseMap_t::iterator it = phases.begin(); it != phases.end(); it++) {
    delete it->second;
  }
}


// Returns the WCET of a process
int CsdfDumper::getWCET(Process *process) {
  return implTable->getMetric(IM_DELAY_WORST, process->statement->top_function->name->s);
}

// Dumps PPN in SDF3 CSDF format.
void CsdfDumper::dumpCsdf3(std::ostream& strm) {
  const PPNprocesses nodes = this->ppn->getNodes();
  const PPNchannels edges = this->ppn->getEdges();

  std::ostringstream csdfProps;

  strm << "<?xml version=\"1.0\" encoding=\"UTF-8\"?>\n"
       << "<sdf3 type=\"csdf\" version=\"1.0\" xmlns:xsi=\"http://www.w3.org/2001/XMLSchema-instance\" xsi:noNamespaceSchemaLocation=\"http://www.es.ele.tue.nl/sdf3/xsd/sdf3-csdf.xsd\">\n"
       << "  <applicationGraph name='ppn2csdf'>\n"
       << "    <csdf name='graph' type='csdfgraph'>\n";

  computePhases();

  for (unsigned int i = 0; i < nodes.size(); i++){
    Process* process = nodes[i];
    strm << "      <actor name='ND_" << process->nr << "' type='x'>\n";
    PPNchannels processEdges = this->ppn->getEdges(process);
    for (PPNchIter eit = processEdges.begin(); eit != processEdges.end(); ++eit) {
      edge *ch = *eit;
      if (ch->from_node->nr == process->nr) {
        writePortCsdf3(ch->from_port->s, "out", strm);
      }
      if (ch->to_node->nr == process->nr) {
        writePortCsdf3(ch->to_port->s, "in", strm);
      }
    }
    strm << "      </actor>\n";

    // While we're iterating over the actors also fill CSDF properties
    csdfProps << "      <actorProperties actor='ND_" << process->nr << "'>\n"
              << "        <processor type='proc_0' default='true'>\n"
              << "          <executionTime time='";
    int wcet = getWCET(process);
    for (int xi = 0; xi < getPhaseLength(process->nr); xi++) {
      if (xi != 0) csdfProps << ",";
      csdfProps << wcet;
    }
    csdfProps << "' />\n";
    csdfProps << "        </processor>\n"
              << "      </actorProperties>\n";
  }

  strm << "\n";

  for (unsigned int i = 0; i < edges.size(); i++){
    edge *e = edges[i];
    strm << "      <channel name='" << e->name->s << "' "
         <<        "srcActor='ND_" << e->from_node->nr << "' "
         <<        "srcPort='" << e->from_port->s << "' "
         <<        "dstActor='ND_" << e->to_node->nr << "' "
         <<        "dstPort='" << e->to_port->s << "' "
         <<        "initialTokens='0' "
         << "/>\n";
  }

  strm << "\n";

  strm << "    </csdf>\n"
       << "    <csdfProperties>\n"
       << csdfProps.str()
       << "    </csdfProperties>\n"
       << "  </applicationGraph>\n"
       << "</sdf3>\n";
}


void CsdfDumper::dumpCsdf(std::ostream& strm) {
  const PPNprocesses nodes = this->ppn->getNodes();

  unsigned int indent = 0;
  const unsigned int nd_nr = nodes.size();

  computePhases();

  strm<< TABS(indent) << "node_number:" << nd_nr << "\n";

  // print all nodes
  for (unsigned int i = 0; i < nodes.size(); i++){
    Process* process = nodes[i];

    strm<< TABS(indent) << "node:\n";
    indent++;

    strm << TABS(indent) << "id:" << process->nr << "\n";
    strm << TABS(indent) << "name:ND_" << process->nr << "\n";
    //strm << TABS(indent) << "name:" << process->statement->top_function->name->s << "\n";
    strm << TABS(indent) << "length:" << getPhaseLength(process->nr) << "\n";
    strm << TABS(indent) << "wcet:";
    int wcet = getWCET(process);
    for (unsigned int wc = 1; wc <= getPhaseLength(process->nr); wc++) {
      	strm << wcet;
		if (wc < getPhaseLength(process->nr)) strm << " ";
    }
    strm << "\n";

    std::vector<edge*> edges_process = this->ppn->getEdges(process);
    strm << TABS(indent) << "port_number:" << edges_process.size() <<"\n";


    // iterator over all ports
    for (PPNchIter eit = edges_process.begin();
        eit != edges_process.end();
        ++eit) {
      edge* ch = *eit;

      std::string type;
      unsigned int port_id;
      if (ch->from_node->nr == process->nr) {
        type = "out";
        port_id = getPortNr(ch->from_port->s);;
        strm << TABS(indent) << "port:\n";
        indent++;
        strm << TABS(indent) << "type:" << type << "\n";
        strm << TABS(indent) << "id:" << port_id << "\n";
        strm << TABS(indent) << "rate:";
        writePhase(ch->from_port->s, strm, ' ');
        strm << "\n";
        indent--;
      }
      if (ch->to_node->nr == process->nr){
        type = "in";
        port_id = getPortNr(ch->to_port->s);;
        strm << TABS(indent) << "port:\n";
        indent++;
        strm << TABS(indent) << "type:" << type << "\n";
        strm << TABS(indent) << "id:" << port_id << "\n";
        strm << TABS(indent) << "rate:";
        writePhase(ch->to_port->s, strm, ' ');
        strm << "\n";
        indent--;
      }
    }

    indent = 0;
  } // end nodes

  // write edges
  indent = 0;
  std::vector<edge*> ppn_edges = this->ppn->getEdges();
  strm << TABS(indent) << "edge_number:" << ppn_edges.size() << "\n";

  // iterate over all edges
  unsigned int edge_ed = 0;
  for (PPNchIter eit = ppn_edges.begin();
      eit != ppn_edges.end();
      ++eit)
  {
    edge* ch = *eit;

    strm << TABS(indent) << "edge:" << "\n";
    indent++;
    strm << TABS(indent) << "id:" << edge_ed++ << "\n";
    strm << TABS(indent) << "name:" << ch->name->s << "\n";
    strm << TABS(indent) << "src:" << ch->from_node->nr << " " << getPortNr(ch->from_port->s) << "\n";
    strm << TABS(indent) << "dst:" << ch->to_node->nr << " " << getPortNr(ch->to_port->s) <<  "\n";

    indent = 0;
  }
}


void CsdfDumper::writePortCsdf3(std::string name, std::string type, std::ostream &strm) {
  strm << "        <port type='" << type << "' "
       << (type.compare("in") == 0 ? " " : "")
       <<          "name='" << name << "' "
       <<          "rate='";
  writePhase(name, strm, ',');
  strm << "' />\n";
}


unsigned int CsdfDumper::getPortNr(const std::string &portname) {
  std::string p = "TAGNAME_ED_0_0_V_0";
  size_t pos; // position of "ED"

  pos = portname.find("ED");
  std::string name = portname.substr (pos);

  const char *name_c  = name.c_str();

  int nr1, nr2, nr3;
  if (sscanf(name_c, "ED_%d_%d_V_%d", &nr1, &nr2, &nr3) != 3){
    printf("WARNING: port number might be incorrect\n");
  }
  assert(nr1 <= 9 && nr2 <= 9 && nr3 <= 9); // Otherwise it is not unique(?)

  return nr1*100 + nr2 * 10 + nr3;
}


// Returns the phase length for given node
unsigned int CsdfDumper::getPhaseLength(unsigned int nodenr) {
  std::ostringstream oss;
  oss << "ND_" << nodenr;
  std::string nodePrefix = oss.str();
  for (phaseMap_t::iterator it = phases.begin(); it != phases.end(); it++) {
    string pName = it->first;
    if (pName.compare(0, nodePrefix.length(), nodePrefix) == 0) {
      // Take the first port we can find and return (all phases for ports of the same node should be equal)
      return it->second->size();
    }
  }
}


// Extends phase for all ports of given node by one (initialized to zero)
void CsdfDumper::extendPhase(unsigned int nodenr) {
  std::ostringstream oss;
  oss << "ND_" << nodenr;
  std::string nodePrefix = oss.str();
  for (phaseMap_t::iterator it = phases.begin(); it != phases.end(); it++) {
    string pName = it->first;
    if (pName.compare(0, nodePrefix.length(), nodePrefix) == 0) {
      it->second->push_back(0);
    }
  }
}


// Processes the trace produced by AST execution into the phases for all nodes.
void CsdfDumper::processTrace(FILE *fin) {
  char lineBuffer[128];
  bool stop = false;
  unsigned int nodenr;
  char porttype;
  int *status = new int[this->ppn->getNodes().size()];

  for (int i = 0; i < this->ppn->getNodes().size(); ++i) {
    extendPhase(i);
    status[i] = 0;
  }

  while (!stop) {
    fgets(lineBuffer, 128, fin);
    int linelen = strlen(lineBuffer);

    if (lineBuffer[linelen-1] == '\n') {
      lineBuffer[linelen-1] = '\0';
    }
    if (strcmp(lineBuffer, STR_CPROG_FINISHED) == 0) {
      stop = true;
      break;
    }
    else if (strcmp(lineBuffer, STR_CPROG_NEXTITER) == 0) {
      // Next iteration of body, extend vectors of all nodes that have fired their function.
      for (int i = 0; i < this->ppn->getNodes().size(); ++i) {
        if (status[i] == 1) {
          extendPhase(i);
          status[i] = 0;
        }
      }
      continue;
    }

    switch (sscanf(lineBuffer, "ND_%d%cP", &nodenr, &porttype)) {
      case 1: // Node execution
        status[nodenr] = 1;
        break;
      case 2: // IPD/OPD statement
        //status[nodenr] = 1;
        /*if (status[nodenr] == 1 && porttype == 'I') {
          // We've made a transition from OPDs to IPDs for this node (i.e. next iteration)
          //extendPhase(nodenr);
          status[nodenr] = 0;
        }*/
        assert(phases[lineBuffer]->back() == 0);  // if this fails, we missed a step...
        phases[lineBuffer]->back() = 1;
        break;
      default: // Line not in expected format
        assert(0);
    }
  }

  // Remove phase extension for all (source) nodes that we didn't touch since the last extendPhase.
  for (int i = 0; i < this->ppn->getNodes().size(); ++i) {
    if (status[i] == 0) {
      std::ostringstream oss;
      oss << "ND_" << i;
      std::string nodePrefix = oss.str();
      for (phaseMap_t::iterator it = phases.begin(); it != phases.end(); it++) {
        string pName = it->first;
        if (pName.compare(0, nodePrefix.length(), nodePrefix) == 0) {
          assert(it->second->back() == 0);
          it->second->pop_back();
        }
      }

    }
  }

  delete[] status;
}


// Only needs to be run once; computes all phases and stores result in an internal map.
void CsdfDumper::computePhases() {
  int ret;
  char tmpFile[] = "phasesXXXXXX";
  char cmdBuffer[100];

  // Dump AST as C program and compile it.
  int fTmp = mkstemp(tmpFile);
  assert(tmpFile);
  snprintf(cmdBuffer, 100, "gcc -x c -O2 - -o %s", tmpFile);
  FILE *cProg = popen(cmdBuffer, "w");
  ppn->getAST()->dumpCProgram(cProg);
  ret = pclose(cProg);
  assert(WEXITSTATUS(ret) == 0);
  close(fTmp);

  // Run it and catch the output.
  snprintf(cmdBuffer, 100, "./%s", tmpFile);
  FILE *fTrace = popen(cmdBuffer, "r");
  processTrace(fTrace);
  ret = pclose(fTrace);
  assert(WEXITSTATUS(ret) == 0);
  unlink(tmpFile);
}


// Write phase belonging to given port to ostream.
// NOTE: computePhases() needs to be called once prior to calling this one!
void CsdfDumper::writePhase(const std::string &portName, std::ostream &strm, char sep) {
  std::vector<short> *phase = phases[portName];
  int phaseLen = phase->size();
  assert(phaseLen); // if length == 0, computePhases was probably not called

  strm << (*phase)[0];
  for (int i = 1; i < phaseLen; i++) {
    strm << sep << (*phase)[i];
  }
}






static int gOutputFormat; // 1 = StreamIT, 3 = SDF3

void printUsage() {
  fprintf(stderr, "Usage: ppn2csdf [options] < file.ppn\n");
  fprintf(stderr, "Supported options:\n");
  fprintf(stderr, "  -3     Dump in SDF3 format [default]\n");
  fprintf(stderr, "  -s     Dump in StreamIT format\n");
}


int parseCommandline(int argc, char ** argv)
{
  gOutputFormat = 3;
  for (int i = 1; i < argc; i++) {
    if (strcmp(argv[i], "-3") == 0) {
      gOutputFormat = 3;
    }
    else if (strcmp(argv[i], "-s") == 0) {
      gOutputFormat = 1;
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

  if (parseCommandline(argc, argv) != 0) {
    printUsage();
    exit(1);
  }

  PPN *ppn = new PPN;
  ppn = yaml::Load<PPN>(in);

  if (!ppn) {
    fprintf(stderr, "No PPN specified or PPN invalid.\n");
    printUsage();
    exit(1);
  }

  ImplementationTable *implTable = new ImplementationTable();
  if (!implTable->loadDefaultFile()) {
    fprintf(stderr, "Warning: Could not load implementation data from default files;\n"
                    "         please put impldata.xml in the current directory or in ~/.daedalus\n");
  }

  CsdfDumper *dumper = new CsdfDumper(ppn, implTable);
  if (gOutputFormat == 3) {
    dumper->dumpCsdf3(cout);
  }
  else if (gOutputFormat == 1) {
    dumper->dumpCsdf(cout);
  }

  delete dumper;
  delete implTable;
  ppn->free();
  delete ppn;

  return 0;
}
