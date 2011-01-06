/*
 * 		utility.h
 *
 *  	Created on: Jan 4, 2011
 *      Author: teddyzhai
 *      $Id: utility.h,v 1.2 2011/01/06 14:13:53 tzhai Exp $
 */

#ifndef UTILITY_H_
#define UTILITY_H_

#include "ppn.h"

using pdg::PDG;
using ppn::PPN;
using namespace std;
using namespace ppn;

void toposort(PPN *ppn, pdg::node **topo) ;

int countCard(isl_set *set, isl_qpolynomial *qp, void *user);

int getCardinality(isl_ctx *ctx, pdg::UnionSet *s);


#endif /* UTILITY_H_ */
