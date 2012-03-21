/*
 * Copyright (c) 2012 Leiden University (LERC group at LIACS).
 * All rights reserved.
 *
 * def.h
 *
 *  	Created on: Oct 6, 2010
 *      Author: Teddy Zhai, Sven van Haastregt
 */

#ifndef DEFS_H_
#define DEFS_H_

#include "global.h"
#include <limits>
#include "isa/pdg.h"
#include "isa/adg.h"

using namespace std;


typedef double THR_t;

#define thr_min(a,b) ((a)>=(b) ? (b) : (a))

#define THR_INF (std::numeric_limits<double>::max())

namespace adg_helper {

typedef unsigned int id_t;
typedef std::map<isl_id*, unsigned> NameId_t;

//struct adg_edge;
typedef adg_edge Channel;
typedef std::vector<Channel*> Channels;
typedef std::vector<Channel*>::iterator PPNchIter;
typedef std::vector<Channel*>::const_iterator PPNchCIter;

typedef adg_edge Edge;
typedef std::vector<adg_edge*> Edges;

typedef adg_node Node;
typedef std::vector<adg_node*> Nodes;
typedef std::vector<adg_node*>::iterator NodeIter;
typedef std::vector<adg_node*>::const_iterator NodeCIter;

typedef adg_node Process;
typedef std::vector<Process*> Processes;


typedef std::vector<adg_node*> CDNode;
// one unique id in the original ADG as the key, and a set of ids in the original ADG
// as the value
typedef std::vector<std::vector<unsigned int> > CDNodeIds;

typedef adg_domain Domain;

typedef adg_port Port;
typedef std::vector<adg_port*> Ports;
typedef std::vector<adg_port*>::iterator PortIter;
typedef std::vector<adg_port*>::iterator PortCIter;

typedef adg_arg Arg;
typedef std::vector<adg_arg *> Args;

typedef adg_param Parameter;
typedef std::vector<adg_param*> Parameters;

typedef std::vector<adg_node*> ADGgraphSCC;
typedef ADGgraphSCC::iterator ADGgraphSCCIter;
typedef ADGgraphSCC::const_iterator ADGgraphSCCCIter;

typedef std::vector<Nodes> ADGgraphSCCs;
typedef ADGgraphSCCs::iterator ADGgraphSCCsIter;
typedef ADGgraphSCCs::const_iterator ADGgraphSCCsCIter;

typedef std::vector<adg_node*> ADGgraphCycle;
typedef ADGgraphCycle::iterator ADGgraphCycleIter;
typedef ADGgraphCycle::const_iterator ADGgraphCycleCIter;


} // namespace


namespace pdg_helper{

typedef pdg::node pNode_t;
typedef std::vector<pdg::node*> pNodes_t;

typedef pdg::dependence pDep_t;
typedef std::vector<pdg::dependence*> pDeps_t;
typedef std::vector<isl_map*> pRelations_t;
typedef std::vector<isl_id*> pIds_t;

typedef std::vector<pdg::node*> PDGgraphSCC;
typedef std::vector<pNodes_t> PDGgraphSCCs;

}
#endif /* DEFS_H_ */
