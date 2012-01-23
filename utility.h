/*
 * 		utility.h
 *
 *  	Created on: Jan 4, 2011
 *      Author: Teddy Zhai
 *      $Id: utility.h,v 1.4 2012/01/18 15:38:22 tzhai Exp $
 */

#ifndef UTILITY_H_
#define UTILITY_H_

#include "global.h"
#include "defs.h"


using pdg::PDG;
using namespace std;

//void toposort(PPN *ppn, pdg::node **topo) ;

// callback
int countCard(isl_set *set, isl_qpolynomial *qp, void *user);

int getCardinality(isl_ctx *ctx, pdg::UnionSet *s);

int getCardinality(isl_pw_qpolynomial *pw_qpoly);

__isl_give isl_set* getUnwrappedDomain(__isl_take isl_set *set);

#endif /* UTILITY_H_ */
