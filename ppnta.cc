//
// Implementation of throughput analysis
// Sven van Haastregt, September 2010
// LERC, LIACS, Leiden University
//
#include <iostream>
//#include <set>

//#include <isl_set_polylib.h>

#include "barvinok/barvinok.h"

#include "yaml.h"
#include "pdg.h"
#include "ppn.h"
//extern "C" {
//#include "isl_util.h"
//}

using pdg::PDG;
using ppn::PPN;
using namespace std;




// Topological sort
void toposort(PDG *pdg, pdg::node **topo) {
  int n = pdg->nodes.size();
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
        for (int r = 0; r < pdg->dependences.size(); r++) {
          pdg::dependence *dep = pdg->dependences[r];
          if (dep->from && dep->to && dep->to->nr == i) {
            if (marks[dep->from->nr] == false) {
              haspred = true;
              continue;
            }
          }
        }
        if (!haspred) {
          topo[ins++] = pdg->nodes[i];
          marks[i] = true;
        }
      }
    }
/*    for (int p=0; p < n; p++) {
      printf("%d ", marks[p]);
    }
    printf("\n");*/
    if (prev == ins) {
      fprintf(stderr, "Toposort not making any progress, perhaps your PDG is cyclic?\n");
      exit(1);
    }
  }

  delete[] marks;
}


// TODO: needs to be tested with pw_qpolynomial with at least 2 pieces
int countCard(isl_set *set, isl_qpolynomial *qp, void *user) {
  int *count = (int*)user;
  isl_int n, d;
  isl_int_init(n);
  isl_int_init(d);

  if (isl_qpolynomial_is_cst(qp, &n, &d) == 1) {
    if (isl_int_get_si(d) != 1) {
      fprintf(stderr, "Set cardinality seems fractional!\n");
    }
    *count = isl_int_get_si(n);
  }
  else {
    fprintf(stderr, "At least one qpolynomial is not constant!\n");
  }
  return 0;
}

int getCardinality(isl_ctx *ctx, pdg::UnionSet *s) {
  int count = 0;
  isl_pw_qpolynomial *pwqp = isl_set_card(s->get_isl_set(ctx));
  isl_pw_qpolynomial_foreach_piece(pwqp, &countCard, &count);
  return count;
}


// Implementation of Algorithm 1 in Sjoerd Meijer's thesis.
void throughput(PPN *ppn) {
  isl_ctx *ctx = isl_ctx_alloc();
  assert(ctx);

  int workload[10] = {10, 10, 10, 20, 10, 10, 10, 10, 10, 10};
  barvinok_options *b_options = barvinok_options_new_with_defaults();
  Value bres;
  value_init(bres);

  // TODO:
  //toposort(pdg, topo);

  int n = ppn->nodes.size();
  for (int i = 0; i < n; i++) {
    printf("-- Process %d (%s)\n", i, ppn->nodes[i]->statement->top_function->name->s.c_str());

    // Step 1:
    int t_isolated = workload[i];


    // Step 2:
    for (int c = 0; c < ppn->edges.size(); c++) {
      ppn::edge *e = ppn->edges[c];
      if (e->from_node && e->to_node && e->to_node->nr == i) {
        printf(" Edge %d -> %d; |IPD| = %d\n", e->from_node->nr, e->to_node->nr, getCardinality(ctx, e->from_domain));
        // TODO: (4.7) t_Rd = |IPD| / |D| * t_isolated

        /* // Ye olde way of counting with barvinok:
         Polyhedron *pol = *(e->from_domain);
        barvinok_count_with_options(pol, &bres, b_options);
        value_print(stdout, P_VALUE_FMT"\n", bres);*/
      }
    }

/*
    // Step 3:
    for (int r = 0; r < pdg->dependences.size(); r++) {
      pdg::dependence *dep = pdg->dependences[r];
      if (dep->from && dep->to && dep->to->nr == i) {
        // TODO:       t_f = min(t_Rd)
      }
    }


    // Step 4:
    // TODO: compute F_aggr


    // Step 5:
    // TODO: T_i = min(t_isolated, F_aggr)


    // Step 6:
    for (int r = 0; r < pdg->dependences.size(); r++) {
      pdg::dependence *dep = pdg->dependences[r];
      if (dep->from && dep->to && dep->from->nr == i) {
        // TODO:       t_Wr = ...
      }
    }*/
  }
}


// Implementation of Algorithm 1 in Sjoerd Meijer's thesis.
// 
// OBSOLETE, PDG-based
//
/*void throughput(PDG *pdg) {
  int n = pdg->nodes.size();
  pdg::node **topo = new pdg::node*[n];

  int workload[10] = {10, 10, 10, 20, 10, 10, 10, 10, 10, 10};

  toposort(pdg, topo);

  printf("Topological sort:\n");
  for (int i = 0; i < n; i++) {
    printf("%2d %s\n", topo[i]->nr, topo[i]->statement->top_function->name->s.c_str());
  }

  int *t_iso = new int[n];       // Isolated throughputs
  int *t_P = new int[n];         // Process throughput
  vector<vector<int> > t_Rd(n);
  Value bres;
  value_init(bres);
  barvinok_options *b_options = barvinok_options_new_with_defaults();

  for (int i = 0; i < n; i++) {
    printf("-- Process %d (%s)\n", i, topo[i]->statement->top_function->name->s.c_str());
    t_Rd[i].push_back(33);
    
    // Step 1:
    int t_isolated = workload[i];


    // Step 2:
    for (int r = 0; r < pdg->dependences.size(); r++) {
      pdg::dependence *dep = pdg->dependences[r];
      if (dep->from && dep->to && dep->to->nr == i) {
        // TODO: (4.7) t_Rd = |IPD| / |D| * t_isolated
        //TODO: dep->from->source->constraints is NOT the IPD!!!!
        // TODO: use isl_set_card 
        Polyhedron *pol = *(dep->from->source);
        barvinok_count_with_options(pol, &bres, b_options);
        value_print(stdout, P_VALUE_FMT"\n", bres);
        //printf("%ld\n", bres);
      }
    }


    // Step 3:
    for (int r = 0; r < pdg->dependences.size(); r++) {
      pdg::dependence *dep = pdg->dependences[r];
      if (dep->from && dep->to && dep->to->nr == i) {
        // TODO:       t_f = min(t_Rd)
      }
    }


    // Step 4:
    // TODO: compute F_aggr


    // Step 5:
    // TODO: T_i = min(t_isolated, F_aggr)


    // Step 6:
    for (int r = 0; r < pdg->dependences.size(); r++) {
      pdg::dependence *dep = pdg->dependences[r];
      if (dep->from && dep->to && dep->from->nr == i) {
        // TODO:       t_Wr = ...
      }
    }
  }

  delete[] topo;
}
*/

int main(int argc, char * argv[])
{
  FILE *in = stdin, *out = stdout;
  int c, ind = 0;
  bool evaluate = true;

  PPN *ppn = new PPN;
  ppn = yaml::Load<PPN>(in);

  if (!ppn) {
    fprintf(stderr, "No PPN specified or PPN invalid.\n");
    fprintf(stderr, "Usage: ppnta < file.yaml\n");
    exit(1);
  }

  throughput(ppn);

  ppn->free();
  delete ppn;

  return 0;
}
