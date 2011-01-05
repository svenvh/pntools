/*
 * 		utility.cc
 *
 *  	Created on: Jan 4, 2011
 *      Author: Sven van Haastregt, Teddy Zhai
 *     $Id: utility.cc,v 1.1 2011/01/05 09:05:03 tzhai Exp $
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


// Topological sort
void toposort(PPN *ppn, pdg::node **topo) {
  int n = ppn->nodes.size();

  bool *marks = new bool[n];
  int prev, ins = 0;

  for (int i = 0; i < n; i++) {
    marks[i] = false;
  }

  while (ins < n) {
    prev = ins;
    //for (int i = n-1; i >= 0; i--) {
    for (int i = 0; i < n; i++) {
      bool haspred = false;
      if (!marks[i]) {
        for (int r = 0; r < ppn->edges.size(); r++) {
          ppn::edge *e = ppn->edges[r];
          if (e->from_node && e->to_node && e->to_node->nr == i) {
            if (marks[e->from_node->nr] == false) {
              haspred = true;
              break;
            }
          }
        }
        if (!haspred) {
          topo[ins++] = ppn->nodes[i];
          marks[i] = true;
        }
      }
    }
/*    for (int p=0; p < n; p++) {
      printf("%d ", marks[p]);
    }
    printf("\n");*/
    if (prev == ins) {
      fprintf(stderr, "Toposort not making any progress, perhaps your PPN is cyclic?\n");
      exit(1);
    }
  }

  delete[] marks;
}


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
