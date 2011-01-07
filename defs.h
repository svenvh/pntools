/*
 * def.h
 *
 *  	Created on: Oct 6, 2010
 *      Author: Teddy Zhai, Sven van Haastregt
 *      $Id: defs.h,v 1.5 2011/01/07 13:29:36 tzhai Exp $
 *
 */

#ifndef DEF_H_
#define DEF_H_

#include "global.h"
#include <limits>

using namespace std;

typedef double THR_t;

#define thr_min(a,b) ((a)>=(b) ? (b) : (a))

#define THR_INF (std::numeric_limits<double>::max())

namespace ppn {
struct PPNgraphCycle{

};

typedef pdg::node* Process;

typedef std::vector<pdg::node*> PPNprocesses;
typedef std::vector<pdg::node*>::iterator PPNprocessIter;
typedef std::vector<pdg::node*>::const_iterator PPNprocessCIter;

typedef vector<PPNprocesses> PPNgraphSCCs;
typedef PPNgraphSCCs::iterator PPNgraphSCCIter;
typedef PPNgraphSCCs::const_iterator PPNgraphSCCCIter;

typedef vector<PPNprocesses> PPNgraphCycles;
typedef PPNgraphCycles::iterator PPNgraphIter;
typedef PPNgraphCycles::const_iterator PPNgraphCIter;

} // namespace


#endif /* DEF_H_ */
