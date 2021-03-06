/*
 * Copyright (c) 2012 Leiden University (LERC group at LIACS).
 * All rights reserved.
 *
 * 		utility.h
 *
 *  	Created on: Jan 4, 2011
 *      Author: Teddy Zhai
 *
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

int getCardinality(__isl_keep isl_set *set);

/* get the set  from a wrapped set without nested space.
 * a wrapped set = [set] -> local[...] */
__isl_give isl_set* getUnwrappedDomain(__isl_take isl_set *set);

// retrieve the domain (without nested space, but with existential variables) from adg_domain.
// Tuple is preserved.
__isl_give isl_set* getPDGDomain(const adg_domain *adgDomain);

/* check if the dimensions starting from "first" and consecutive "n" in set "set1" is equal to
 * those in set "set2"
 * NOTE: for non-parameterized sets only and both sets are assumed to live in the same space. */
bool isDimsEqual(__isl_keep isl_set *set1, __isl_keep isl_set *set2,
		unsigned int first, unsigned int n);

int isl_id_cmp(__isl_keep isl_id *s1, __isl_keep isl_id *s2);

#endif /* UTILITY_H_ */
