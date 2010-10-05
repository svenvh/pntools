/*
 * ppn.cc
 *
 *  Created on: Sep 30, 2010
 *      Author: Teddy Zhai, Sven van Haastregt
 */

#include "ppn.h"

using namespace ppn;
using namespace pdg;
/*using namespace std;*/
//using pdg::PDG;


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
    YAML_SEQ_FIELD(ppn_d, PPN, nodes, node);
    YAML_SEQ_FIELD(ppn_d, PPN, edges, edge);

    structure::register_type("perl/PPN", &typeid(PPN), &ppn_d.d);
}

void PPN::dump(emitter& e)
{
    yll_emitter_set_transfer(e.e, "perl/PPN");
    structure::dump(e);
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
    YAML_PTR_FIELD(edge_d, edge, name, char);
    YAML_PTR_FIELD(edge_d, edge, from_port, char);
    YAML_PTR_FIELD(edge_d, edge, to_port, char);
    YAML_PTR_FIELD(edge_d, edge, from_domain, isl_set);
    YAML_PTR_FIELD(edge_d, edge, to_domain, isl_set);
    YAML_PTR_FIELD(edge_d, edge, from_node, pdg::node);
	YAML_PTR_FIELD(edge_d, edge, to_node, pdg::node);


	YAML_PTR_FIELD(edge_d, edge, from_node, pdg::node);
	YAML_PTR_FIELD(edge_d, edge, to_node, pdg::node);

	YAML_SEQ_FIELD(edge_d, edge, from_access, pdg::access);
	YAML_SEQ_FIELD(edge_d, edge, to_access, pdg::access);

	YAML_PTR_FIELD(edge_d, edge, map, isl_mat);
	YAML_PTR_FIELD(edge_d, edge, array, pdg::array);

	YAML_INT_FIELD(edge_d, edge, reordering);
	YAML_INT_FIELD(edge_d, edge, multiplicity);

	YAML_PTR_FIELD(edge_d, edge, size, integer);

	YAML_INT_FIELD(edge_d, edge, nr);

	YAML_INT_FIELD(edge_d, edge, sticky);
	YAML_INT_FIELD(edge_d, edge, shift_register);


    structure::register_type("perl/edge", &typeid(ppn::edge), &edge_d.d);

}


PPN *import_ppn(PDG *pdg, std::vector<espam_edge*> edges) {
  PPN *ret = new PPN;

  for (int i = 0; i < pdg->nodes.size(); i++) {
    ret->nodes.push_back(pdg->nodes[i]);
  }

  for (int i = 0; i < edges.size(); i++) {
    edge *e = new edge();
    e->name         = edges[i]->name;
    e->from_port    = edges[i]->from_port;
    e->to_port      = edges[i]->to_port;
    e->from_domain  = edges[i]->from_domain;
    e->to_domain    = edges[i]->to_domain;
    e->from_node    = edges[i]->from_node;
    e->to_node      = edges[i]->to_node;
    
    ret->edges.push_back(e);
  }
}
