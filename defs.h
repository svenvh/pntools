/*
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

//struct adg_edge;
typedef adg_edge Channel;
typedef std::vector<Channel*> Channels;
typedef std::vector<Channel*>::iterator PPNchIter;
typedef std::vector<Channel*>::const_iterator PPNchCIter;

typedef adg_node Process;
typedef std::vector<Process*> Processes;
//typedef std::vector<Process*>::iterator PPNprocessIter;
//typedef std::vector<Process*>::const_iterator PPNprocessCIter;

typedef adg_domain Domain;

typedef adg_port Port;
typedef std::vector<adg_port*> Ports;
typedef std::vector<adg_port*>::iterator PPNportIter;
typedef std::vector<adg_port*>::iterator PPNportCIter;


typedef adg_param Parameter;
typedef std::vector<adg_param*> Parameters;

//typedef std::vector<pdg::node*> PPNgraphSCC;
//typedef PPNgraphSCC::iterator PPNgraphSCCIter;
//typedef PPNgraphSCC::const_iterator PPNgraphSCCCIter;
//
//typedef std::vector<PPNprocesses> PPNgraphSCCs;
//typedef PPNgraphSCCs::iterator PPNgraphSCCsIter;
//typedef PPNgraphSCCs::const_iterator PPNgraphSCCsCIter;
//
//
//typedef std::vector<pdg::node*> PPNgraphCycle;
//typedef PPNgraphCycle::iterator PPNgraphCycleIter;
//typedef PPNgraphCycle::const_iterator PPNgraphCycleCIter;
//
//typedef vector<PPNprocesses> PPNgraphCycles;
//typedef PPNgraphCycles::iterator PPNgraphIter;
//typedef PPNgraphCycles::const_iterator PPNgraphCIter;

// partitioning
typedef enum PartType{Modulo, PlainCut} PartType ;

// Direction vector used for plain-cut: indicate how to cut the iteration domain
typedef vector<int> Direction;

} // namespace


#endif /* DEFS_H_ */
