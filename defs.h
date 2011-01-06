/*
 * def.h
 *
 *  	Created on: Oct 6, 2010
 *      Author: Teddy Zhai, Sven van Haastregt
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

typedef pdg::node Process;

typedef std::list<pdg::node> PPNprocesses;
typedef std::list<pdg::node>::iterator PPNprocessIter;
typedef std::list<pdg::node>::const_iterator PPNprocessCIter;

typedef list<PPNgraphCycle*> PPNgraphCycles;
typedef PPNgraphCycles::iterator PPNgraphIter;
typedef PPNgraphCycles::const_iterator PPNgraphCIter;

} // namespace


#endif /* DEF_H_ */
