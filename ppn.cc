/*
 * ppn.cc
 *
 *  Created on: Sep 30, 2010
 *      Author: Teddy Zhai, Sven van Haastregt
 */

//#include "isl_set_polylib.h"
#include "ppn.h"
#include "yaml.h"
#include "tarjan.h"

using namespace ppn;
//using namespace pdg;
/*using namespace std;*/
using pdg::PDG;


PPN::PPN(const seq<pdg::node>* nodes, const seq<edge>* edges){
	this->nodes = *nodes;
	this->edges = *edges;
}

PPN *PPN::Load(char *str, void *user)
{
    return yaml::Load<PPN>(str, user);
}

PPN *PPN::Load(FILE *fp, void *user)
{
    return yaml::Load<PPN>(fp, user);
}

static at_init register_ppn(PPN::register_type);

void PPN::register_type()
{
    static struct_description ppn_d = { create };
    YAML_SEQ_FIELD(ppn_d, PPN, edges, edge);
    YAML_SEQ_FIELD(ppn_d, PPN, nodes, node);

    structure::register_type("perl/PPN", &typeid(PPN), &ppn_d.d);
}

void PPN::dump(emitter& e)
{
    yll_emitter_set_transfer(e.e, "perl/PPN");
    structure::dump(e);
}


edge::edge() {
  name = NULL;
  from_port = NULL;
  to_port = NULL;
  from_domain = NULL;
  to_domain = NULL;
  from_node = NULL;
  to_node = NULL;
  map = NULL;
  array = NULL;
  reordering = 0;
  multiplicity = 0;
  size = NULL;
  nr = -1;
  sticky = 0;
  shift_register = 0;
}

edge::edge(const char *name, const char *from_port, const char *to_port,
			const isl_set *from_domain, const isl_set	*to_domain,
			const pdg::node* from_node, const pdg::node* to_node,
			const seq<pdg::access>& from_access, const seq<pdg::access>& to_access,
			const isl_mat* map, const pdg::array* array,
			const int& reordering, const int& multiplicity, const integer* size, const int& nr,
			const int& sticky, const int& shift_register){
/*	this->name = name;
	this->from_port = from_port;
	this->to_port = to_port;
	this->from_domain = from_domain;
	this->to_domain = to_domain;
	this->from_node = from_node;
	this->to_node = to_node;
	this->from_access = from_access;
	this->to_access = to_access;
	this->map = map;
	this->array = array;
	this->reordering = reordering;
	this->multiplicity = multiplicity;
	this->size = size;
	this->nr = nr;
	this->sticky = sticky;
	this->shift_register = shift_register;*/
}

static at_init register_edge(edge::register_type);

void edge::register_type()
{
  static struct_description edge_d = { create };
  YAML_PTR_FIELD(edge_d, edge, name, str);
  YAML_PTR_FIELD(edge_d, edge, from_port, str);
  YAML_PTR_FIELD(edge_d, edge, to_port, str);
  YAML_PTR_FIELD(edge_d, edge, from_domain, pdg::UnionSet);
  YAML_PTR_FIELD(edge_d, edge, to_domain, pdg::UnionSet);

  YAML_PTR_FIELD(edge_d, edge, from_node, pdg::node);
  YAML_PTR_FIELD(edge_d, edge, to_node, pdg::node);

  YAML_SEQ_FIELD(edge_d, edge, from_access, pdg::access);
  YAML_SEQ_FIELD(edge_d, edge, to_access, pdg::access);

  YAML_PTR_FIELD(edge_d, edge, map, pdg::Matrix);
  YAML_PTR_FIELD(edge_d, edge, array, pdg::array);

  YAML_INT_FIELD(edge_d, edge, reordering);
  YAML_INT_FIELD(edge_d, edge, multiplicity);

  //YAML_PTR_FIELD(edge_d, edge, size, integer);
  YAML_INT_FIELD(edge_d, edge, size);

  YAML_INT_FIELD(edge_d, edge, nr);

  YAML_INT_FIELD(edge_d, edge, sticky);
  YAML_INT_FIELD(edge_d, edge, shift_register);

  structure::register_type("perl/edge", &typeid(ppn::edge), &edge_d.d);
}


static pdg::UnionSet *isl_set_to_UnionSet(isl_set *s) {
  pdg::UnionSet *ret = new pdg::UnionSet();
  Polyhedron *D = isl_set_to_polylib(s);
  for (Polyhedron *P = D; P; P = P->next) {
    pdg::Matrix *M = new pdg::Matrix(P);
    ret->constraints.push_back(M);
  }
  ret->dim = D->Dimension;
  return ret;
}


static pdg::Matrix *isl_mat_to_PdgMatrix(isl_mat *m) {
  pdg::Matrix *ret = new pdg::Matrix;

  for (int i = 0; i < m->n_row; ++i) {
    std::vector<int> v;
    for (int j = 0; j < m->n_col; ++j)
      v.push_back(isl_int_get_si(m->row[i][j]));
    ret->el.push_back(v);
  }

  return ret;
}


static void copy_accesses(seq<pdg::access> *dst, std::vector<pdg::access * > *src) {
  std::vector<pdg::access *>::iterator i;
  for (i = src->begin(); i != src->end(); i++) {
    dst->v.push_back(*i);
  }
}


void
PPN::import_pn(PDG *pdg, std::vector<espam_edge*> edges) {
  for (int i = 0; i < pdg->nodes.size(); i++) {
    this->nodes.push_back(pdg->nodes[i]);
  }

  for (int i = 0; i < edges.size(); i++) {
    edge *e = new edge();
    e->name         = new str(edges[i]->name);
    e->from_port    = new str(edges[i]->from_port);
    e->to_port      = new str(edges[i]->to_port);
    e->from_domain  = isl_set_to_UnionSet(edges[i]->from_domain);
    e->to_domain    = isl_set_to_UnionSet(edges[i]->to_domain);
    e->from_node    = edges[i]->from_node;
    e->to_node      = edges[i]->to_node;
    copy_accesses(&(e->from_access), &(edges[i]->from_access));
    copy_accesses(&(e->to_access), &(edges[i]->to_access));
//    e->map          = isl_mat_to_PdgMatrix(edges[i]->map);
    e->map          = edges[i]->map_stripped;
    e->array        = edges[i]->array;
    e->reordering   = edges[i]->reordering;
    e->multiplicity = edges[i]->multiplicity;
    e->size         = (edges[i]->size) ? edges[i]->size->v : -1;
    e->nr           = edges[i]->nr;
    e->sticky       = edges[i]->sticky;
    e->shift_register = edges[i]->shift_register;

    this->edges.push_back(e);
  }

}

std::vector<edge*>
PPN::getEdges(){
		return this->edges.v;
}

std::vector<pdg::node*>
PPN::getNodes(){
	return this->nodes.v;
}

AST *PPN::getAST() {
  return &ast;
}

void
PPN::toposort(pdg::node **topo) {
  int n = this->nodes.size();

  bool *marks = new bool[n];
  int prev, ins = 0;

  for (int i = 0; i < n; i++) {
    marks[i] = false;
  }

  while (ins < n) {
    prev = ins;
    //for (int i = n-1; i >= 0; i--) {
    for (int i = 0; i < n; i++) {
      bool haspred = false;
      if (!marks[i]) {
        for (int r = 0; r < this->edges.size(); r++) {
          ppn::edge *e = this->edges[r];
          if (e->from_node && e->to_node && e->to_node->nr == i) {
            if (marks[e->from_node->nr] == false) {
              haspred = true;
              break;
            }
          }
        }
        if (!haspred) {
          topo[ins++] = this->nodes[i];
          marks[i] = true;
        }
      }
    }
/*    for (int p=0; p < n; p++) {
      printf("%d ", marks[p]);
    }
    printf("\n");*/
    if (prev == ins) {
      fprintf(stderr, "Toposort not making any progress, perhaps your PPN is cyclic?\n");
      exit(1);
    }
  }

  delete[] marks;
}


node *PPN::getNodeFromNr(int nr) {
 	int size = this->getNodes().size();
 	for (int i = 0; i < size; ++i) {
    if (this->getNodes()[i]->nr == nr)
      return this->getNodes()[i];
  }
}


PPNgraphCycles
PPN::findPPNgraphCycles(){

	PPNgraphCycles cycles;

// 	int size = ppn->nodes.size();
// 	for (int i = 0; i < size; ++i) {
// 		const pdg::node *proc = ppn->nodes[i];
//
// 		PPNprocesses adj_processes = getAdjacentProcesses(ppn, proc);
//
// 	} // all nodes


	return cycles;
}


PPNprocesses
PPN::getAdjacentProcesses(const Process *process){
	PPNprocesses processes;

// 	PPNprocessIter pit = find(this->nodes.v.begin(), this->nodes.v.end(), process);
// 	assert(pit != this->nodes.v.end());
// 	const Process* procs = *pit;
// 	int procsNr = procs->nr;
//
// 	//
// 	for (int i = 0; i < this->edges.v.size(); ++i) {
// 		this->edges.v[i];
// 	}

	// get input/output ports


	return processes;
}


PPNgraphSCCs PPN::findSCCs() {
  PPNgraphSCCs ret;
  Tarjan::TarjanClass tc;
  ret = tc.runTarjansAlgorithm(this);

  return ret;
}
