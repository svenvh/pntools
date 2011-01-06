/*
 * 		utility.cc
 *
 *  	Created on: Jan 4, 2011
 *      Author: Sven van Haastregt, Teddy Zhai
 *     $Id: utility.cc,v 1.2 2011/01/06 14:13:53 tzhai Exp $
 */

#include <iostream>

#include "barvinok/barvinok.h"

#include "yaml.h"
#include "pdg.h"
#include "ppn.h"
#include "defs.h"
#include "utility.h"

using pdg::PDG;
using ppn::PPN;
using namespace std;

// Helper function (callback) for getCardinality
int countCard(isl_set *set, isl_qpolynomial *qp, void *user) {
  int *count = (int*)user;
  isl_int n, d;
  isl_int_init(n);
  isl_int_init(d);

  // The resulting pw_qpolynomial should consist of only a single piece:
  assert(*count == 0);

  if (isl_qpolynomial_is_cst(qp, &n, &d) == 1) {
    if (isl_int_get_si(d) != 1) {
      fprintf(stderr, "Warning: Set cardinality seems fractional!\n");
    }
    *count = isl_int_get_si(n);
  }
  else {
    fprintf(stderr, "Warning: At least one qpolynomial is not constant!\n");
  }
  isl_int_clear(n);
  isl_int_clear(d);
  isl_qpolynomial_free(qp);
  isl_set_free(set);
  return 0;
}


// Count number of points in UnionSet s
int getCardinality(isl_ctx *ctx, pdg::UnionSet *s) {
  int count = 0;
  isl_pw_qpolynomial *pwqp = isl_set_card(s->get_isl_set(ctx));
  isl_pw_qpolynomial_foreach_piece(pwqp, &countCard, &count);
  isl_pw_qpolynomial_free(pwqp);
  return count;
}

