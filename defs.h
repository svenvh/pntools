/*
 * def.h
 *
 *  	Created on: Oct 6, 2010
 *      Author: Teddy Zhai, Sven van Haastregt
 *      $Id: defs.h,v 1.7 2011/05/02 12:35:58 tzhai Exp $
 *
 */

#ifndef DEFS_H_
#define DEFS_H_

#include "global.h"
#include <limits>
#include "pdg.h"


using namespace std;


typedef double THR_t;

#define thr_min(a,b) ((a)>=(b) ? (b) : (a))

#define THR_INF (std::numeric_limits<double>::max())

namespace ppn {

class edge;

typedef pdg::node Process;

typedef std::vector<pdg::node*> PPNprocesses;
typedef std::vector<pdg::node*>::iterator PPNprocessIter;
typedef std::vector<pdg::node*>::const_iterator PPNprocessCIter;
typedef std::vector<edge*>::iterator PPNchIter;
typedef std::vector<edge*>::const_iterator PPNchCIter;


typedef std::vector<pdg::node*> PPNgraphSCC;
typedef PPNgraphSCC::iterator PPNgraphSCCIter;
typedef PPNgraphSCC::const_iterator PPNgraphSCCCIter;

typedef std::vector<PPNprocesses> PPNgraphSCCs;
typedef PPNgraphSCCs::iterator PPNgraphSCCsIter;
typedef PPNgraphSCCs::const_iterator PPNgraphSCCsCIter;


typedef std::vector<pdg::node*> PPNgraphCycle;
typedef PPNgraphCycle::iterator PPNgraphCycleIter;
typedef PPNgraphCycle::const_iterator PPNgraphCycleCIter;

typedef vector<PPNprocesses> PPNgraphCycles;
typedef PPNgraphCycles::iterator PPNgraphIter;
typedef PPNgraphCycles::const_iterator PPNgraphCIter;

// partitioning
typedef enum PartType{Modulo, PlainCut} PartType ;

// Direction vector used for plain-cut: indicate how to cut the iteration domain
typedef vector<int> Direction;

} // namespace


#endif /* DEFS_H_ */
