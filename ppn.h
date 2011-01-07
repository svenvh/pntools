/*
 * ppn.h
 *
 *    Created on: Sep 30, 2010
 *      Author: Teddy Zhai, Sven van Haastregt
 *      $Id: ppn.h,v 1.8 2011/01/07 16:32:52 svhaastr Exp $
 *
 */

#ifndef PPN_H_
#define PPN_H_

#include <algorithm>
#include <vector>

#include "yaml.h"
#include "pdg.h"

#include "isl_set_polylib.h"
#include "isl_map_polylib.h"
#include "isl_set.h"

#include "global.h"
#include "defs.h"

using namespace yaml;

// espam_edge; structure used during conversion of a pn2ppn. Original definition is in pn2espam.cc .
struct espam_edge {
    char    *name;
    char    *from_port;
    char    *to_port;
    isl_set  *from_domain;
    isl_set  *to_domain;
    pdg::node    *from_node;
    pdg::node    *to_node;
    std::vector<pdg::access *>    from_access;
    std::vector<pdg::access *>    to_access;
    isl_mat  *map;
    pdg::Matrix *map_stripped;
    pdg::array *array;
    int   reordering;
    int   multiplicity;
    integer * size;
    int   nr;
    bool  sticky;
    bool  shift_register;
};

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
  edge(const char *name, const char *from_port, const char *to_port,
      const isl_set *from_domain, const isl_set *to_domain,
      const pdg::node* from_node, const pdg::node* to_node,
      const seq<pdg::access>& from_access, const seq<pdg::access>& to_access,
      const isl_mat* map, const pdg::array* array,
      const int& reordering, const int& multiplicity, const integer* size, const int& nr,
      const int& sticky, const int& shift_register);
  ~edge() {};

  static void register_type();
};


typedef pdg::node node;

// PPN; serializable class
// This is the base class representing the PPN (graph).
class PPN:public structure{
  static serialize *create(void *user) { return new PPN(); }
private:
  seq<pdg::node> nodes;
  seq<edge> edges;

public:
  ////////////////////////////////////////////////////////////////////////////
  //// Constructors/destructors
  PPN() {}
  PPN(const seq<pdg::node>* nodes, const seq<edge>* edges);
  ~PPN(){};


  ////////////////////////////////////////////////////////////////////////////
  //// YAML stuff
  static PPN *Load(char *str, void *user = NULL);
  static PPN *Load(FILE *fp, void *user = NULL);
  static void register_type();
  void dump(emitter& e);


  ////////////////////////////////////////////////////////////////////////////
  //// Loading / importing
  // Constructs a PPN from a PDG and vector of espam_edges
  // Probably only pn2ppn will use this one
  void import_pn(pdg::PDG *pdg, std::vector<espam_edge*> edges);


  ////////////////////////////////////////////////////////////////////////////
  //// Data access
  // Returns the list of edges
  std::vector<edge*> getEdges();

  // Returns the list of nodes
  std::vector<pdg::node*> getNodes();

  // Returns pointer to Node with the specified nr
  node *getNodeFromNr(const int nr);

  ////////////////////////////////////////////////////////////////////////////
  //// Graph operations
  // Returns a node* array representing a topological sort of the PPN
  void toposort(pdg::node **topo);

  // Finds all SCCs of size >= 2
  PPNgraphSCCs findSCCs();

  // Finds all cycles in a PPN
  PPNgraphCycles findPPNgraphCycles();

  // Returns a list of processes that are adjacent to the given process
  PPNprocesses getAdjacentProcesses(const Process *process);

};

} //namespace


#endif /* PPN_H_ */
