/*
 * 		utility.h
 *
 *  	Created on: Jan 4, 2011
 *      Author: teddyzhai
 *      $Id: utility.h,v 1.1 2011/01/05 09:05:03 tzhai Exp $
 */

#ifndef UTILITY_H_
#define UTILITY_H_

using pdg::PDG;
using ppn::PPN;
using namespace std;


void toposort(PPN *ppn, pdg::node **topo) ;

int countCard(isl_set *set, isl_qpolynomial *qp, void *user);

int getCardinality(isl_ctx *ctx, pdg::UnionSet *s);

#endif /* UTILITY_H_ */
