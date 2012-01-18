/*
 * ppn.h
 *
 *    	Created on: Sep 30, 2010
 *      Author: Teddy Zhai, Sven van Haastregt
 *
 *		This class builds on top of adg struct in isa. It intends to provides functions which are not
 *		implemented in adg struct.
 *
 *
 *      $Id: ppn.h,v 1.17 2012/01/18 15:38:22 tzhai Exp $
 *
 *      30. Nov. 2011: adapted ppn to adg data structure in isa
 *
 */

#ifndef PPN_H_
#define PPN_H_

#include "global.h"
#include "defs.h"

#include "isa/yaml.h"
#include "isa/pdg.h"
#include "isa/adg.h"

#include "isl/mat.h"
#include "isl/set.h"

#include "ast.h"

using namespace yaml;
//using pdg::PDG;


namespace ppn{

// Edge; serializable class
// After pn2ppn conversion, pdg::dependences are transformed into edges. This edge class contains additional fields (such as
// from_domain, to_domain representing OPD, IPD resp.) that are used during further PPN analysis.
class edge:public structure{
  static serialize *create(void *user) { return new edge(); }
public:
  str *name;
  str *from_port;
  str *to_port;
  pdg::UnionSet *from_domain;
  pdg::UnionSet *to_domain;
  pdg::node    *from_node;
  pdg::node    *to_node;
  seq<pdg::access>  from_access;
  seq<pdg::access>  to_access;
  pdg::Matrix *map;
  pdg::array *array;
  int   reordering;
  int   multiplicity;
  //integer * size;
  int size;
  int   nr;
  int sticky;
  int shift_register;

  edge();
/*  edge(const char *name, const char *from_port, const char *to_port,
      const isl_set *from_domain, const isl_set *to_domain,
      const pdg::node* from_node, const pdg::node* to_node,
      const seq<pdg::access>& from_access, const seq<pdg::access>& to_access,
      const isl_mat* map, const pdg::array* array,
      const int& reordering, const int& multiplicity, const integer* size, const int& nr,
      const int& sticky, const int& shift_register);*/
  ~edge();

  static void register_type();
};


typedef pdg::node node;



// PPN; serializable class
// This is the base class representing the PPN (graph).
class PPN:public structure{
//	seq<pdg::node> nodes;
//	seq<edge> edges;
//	AST *ast;
//	PDG *pdg;
	adg *pn_adg;

private:
//	static serialize *create(void *user) { return new PPN(); }

public:
  ////////////////////////////////////////////////////////////////////////////
  //// Constructors/destructors
  PPN() {}
  PPN(const seq<pdg::node>* nodes, const seq<edge>* edges, AST *ast);
  ~PPN(){};


  ////////////////////////////////////////////////////////////////////////////
  //// YAML stuff
//  static PPN *Load(char *str, void *user = NULL);
  void Load(FILE *fp, isl_ctx *ctx);
  static void register_type();
  void dump(emitter& e);
  void dump(FILE *fp);


//  ////////////////////////////////////////////////////////////////////////////
//  //// Loading / importing
//  // Constructs a PPN from a PDG and vector of espam_edges
//  // Probably only pn2ppn will use this one
//  void import_pn(pdg::PDG *pdg, std::vector<espam_edge*> edges, AST *ast);
//
//  ////////////////////////////////////////////////////////////////////////////
//  //// Data access
//  // Returns the list of edges
  std::vector<edge*> getEdges();

  std::vector<edge*> getEdges(const Process *process);

  // Returns the list of nodes
  std::vector<pdg::node*> getNodes();
//
  // Returns pointer to AST
  AST *getAST();
//
//  // Returns pointer to Node with the specified nr
//  node *getNodeFromNr(const int nr);
//
//  // get name of the process
//  std::string getProcessName(const Process* process);
//
//  ////////////////////////////////////////////////////////////////////////////
//  //// Graph operations
//  // Returns a node* array representing a topological sort of the PPN
//  void toposort(pdg::node **topo);
//
//  // Finds all SCCs of size >= 2
//  PPNgraphSCCs findSCCs();
//
//  // Finds all cycles in a PPN
//  PPNgraphCycles findPPNgraphCycles();
//
//  // Returns a list of processes that are adjacent to the given process
//  PPNprocesses getAdjacentProcesses(const Process* process);


};

class PPN_ADG{
	isl_ctx *ctx;
	adg *pn_adg;

public:
	PPN_ADG(adg *pn_adg, isl_ctx *ctx);
	~PPN_ADG();
	void Load(FILE *fp, isl_ctx *ctx);

	//  ////////////////////////////////////////////////////////////////////////////
	//  //// Data access
	//  // Returns the list of edges
	  Channels getChannels();
	  Channels getChannels(const Process *process);

	  Processes GetProcesses();
	  Domain* getProcessDomain(Process *process);
	  __isl_give isl_set* getProcessDomainBound(const Process *process);


	  Ports getInPorts(const Process *process);
	  Ports getOutPorts(const Process *process);

	  __isl_give isl_set* getPortDomainBound(const Port *port);

	  std::vector<adg_param*> GetParameters();
};



} //namespace


#endif /* PPN_H_ */
