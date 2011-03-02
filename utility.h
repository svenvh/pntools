/*
 * 		utility.h
 *
 *  	Created on: Jan 4, 2011
 *      Author: teddyzhai
 *      $Id: utility.h,v 1.3 2011/03/02 10:22:15 tzhai Exp $
 */

#ifndef UTILITY_H_
#define UTILITY_H_

#include "global.h"
#include "defs.h"
#include "ppn.h"


using pdg::PDG;
using ppn::PPN;
using namespace std;
using namespace ppn;

void toposort(PPN *ppn, pdg::node **topo) ;

int countCard(isl_set *set, isl_qpolynomial *qp, void *user);

int getCardinality(isl_ctx *ctx, pdg::UnionSet *s);

void writePLA(ofstream& pla_file, signed int nr_proc);

#endif /* UTILITY_H_ */
