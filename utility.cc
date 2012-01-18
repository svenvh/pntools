/*
 * 		utility.cc
 *
 *  	Created on: Jan 4, 2011
 *      Author: Sven van Haastregt, Teddy Zhai
 *     $Id: utility.cc,v 1.6 2012/01/18 15:38:22 tzhai Exp $
 */


#include "barvinok/barvinok.h"

#include "isa/yaml.h"
#include "isa/pdg.h"

#include "defs.h"
#include "ADG_helper.h"
#include "utility.h"

using pdg::PDG;
//using ppn::PPN;
using namespace std;

/* Helper function (callback) for getCardinality
 * This function is mainly used for non-parameterized case,
 * in which only a single piece and a constant are desired
 * */
int countCard(isl_set *set, isl_qpolynomial *qp, void *user) {
  int *count = (int*)user;
  isl_int n, d;
  isl_int_init(n);
  isl_int_init(d);

  /* The resulting pw_qpolynomial should consist of only a single piece,
   * therefore, it should equal to initial vaule -1
   */
  assert(*count == -1);

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
  int count = -1;
  isl_pw_qpolynomial *pwqp = isl_set_card(s->get_isl_set(ctx));
  isl_pw_qpolynomial_foreach_piece(pwqp, &countCard, &count);
  isl_pw_qpolynomial_free(pwqp);
  return count;
}

int getCardinality(__isl_keep isl_pw_qpolynomial *pwqp){
	int count = -1;

	isl_pw_qpolynomial_foreach_piece(pwqp, &countCard, &count);

	assert(count >= 0);
	return count;
}




