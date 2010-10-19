/*
 * ppn.h
 *
 *  Created on: Sep 30, 2010
 *      Author: Teddy Zhai, Sven van Haastregt
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
//#include "isl_map.h"

using namespace yaml;

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

class edge:public structure{
	static serialize *create(void *user) { return new edge(); }
public:
	str	*name;
	str	*from_port;
	str	*to_port;
	pdg::UnionSet *from_domain;
	pdg::UnionSet *to_domain;
	pdg::node    *from_node;
	pdg::node    *to_node;
	seq<pdg::access>  from_access;
	seq<pdg::access>  to_access;
	pdg::Matrix *map;
	pdg::array *array;
	int		reordering;
	int		multiplicity;
	//integer *	size;
	int size;
	int		nr;
	int	sticky;
	int	shift_register;

	edge();
	edge(const char *name, const char *from_port, const char *to_port,
			const isl_set *from_domain, const isl_set	*to_domain,
			const pdg::node* from_node, const pdg::node* to_node,
			const seq<pdg::access>& from_access, const seq<pdg::access>& to_access,
			const isl_mat* map, const pdg::array* array,
			const int& reordering, const int& multiplicity, const integer* size, const int& nr,
			const int& sticky, const int& shift_register);
	~edge() {};

	 static void register_type();
};


class PPN:public structure{
	static serialize *create(void *user) { return new PPN(); }
public:
	seq<pdg::node> nodes;
	seq<edge> edges;

	PPN() {}
	PPN(const seq<pdg::node>* nodes, const seq<edge>* edges);
	~PPN(){};

	static PPN *Load(char *str, void *user = NULL);
	static PPN *Load(FILE *fp, void *user = NULL);
	static void register_type();
	void dump(emitter& e);
};



PPN *import_ppn(pdg::PDG *pdg, std::vector<espam_edge*> edges);

static pdg::UnionSet *isl_set_to_UnionSet(isl_set *s) ;
} //namespace



#endif /* PPN_H_ */
