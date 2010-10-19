//
// Implementation of throughput analysis
// Sven van Haastregt, Teddy Zhai
// September 2010
// LERC, LIACS, Leiden University
//
#include <iostream>
//#include <isl_set_polylib.h>

#include "barvinok/barvinok.h"

#include "defs.h"
#include "yaml.h"
#include "pdg.h"
#include "ppn.h"
#include "isl_set.h"
//extern "C" {
//#include "isl_util.h"
//}

#define DEBUG_PPNTA

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
              continue;
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


int countCard(isl_set *set, isl_qpolynomial *qp, void *user) {
  int *count = (int*)user;
  isl_int n, d;
  isl_int_init(n);
  isl_int_init(d);

  // The resulting pw_qpolynomial should consist of only a single piece:
  assert(*count == 0);

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
  int n = ppn->nodes.size();
  pdg::node **topo = new pdg::node*[n];

  isl_ctx *ctx = isl_ctx_alloc();
  assert(ctx);

  // TODO: workload and cost for communication need to be read from external files.
  // For now, the size of the array shoul be equal to the number of processes
  const THR_t workload[10] = {10, 10, 10, 20, 10, 10, 10, 10, 10, 10};
  const THR_t commu_cost[10] = {10, 10, 10, 20, 10, 10, 10, 10, 10, 10};

//  barvinok_options *b_options = barvinok_options_new_with_defaults();
//  Value bres;
//  value_init(bres);

  toposort(ppn, topo);
  printf("Topological sort:\n");
  for (int i = 0; i < n; i++) {
    printf("%2d %s\n", topo[i]->nr, topo[i]->statement->top_function->name->s.c_str());
  }


  for (int i = 0; i < n; i++) {
    pdg::node *node = topo[i];     // Current node respecting topological order
    printf("-- Process %d (%s)\n", i, node->statement->top_function->name->s.c_str());
    pdg::UnionSet* d_node = isl_set_to_UnionSet(node->source);
#ifdef DEBUG_PPNTA
    cout << "node domain:" <<endl;\
    d_node->get_isl_set(ctx);
    isl_set_print(d_node->get_isl_set(ctx), stdout, 1);
#endif

    /********************************************************************************
     * Step 1
     */
    vector<THR_t>  ppn_iso_thrs(ppn->nodes.v.size(), 0.0);
    // get # input arguments
    THR_t x = ppn->nodes[i]->statement->top_function->arguments.size();
#ifdef DEBUG_PPNTA
    cout<<"nr input arguments: " << endl;
#endif
    // get # output ports
    THR_t y = ppn->nodes[i]->statement->accesses.v.size();
#ifdef DEBUG_PPNTA
    cout<<"nr ouput ports: " << endl;
#endif
    THR_t c_iso = workload[node->nr] + x * commu_cost[node->nr] + y * commu_cost[node->nr];
    ppn_iso_thrs[node->nr] = 1.0 / c_iso;


    /********************************************************************************
     * Step 2
     */
    for (int c = 0; c < ppn->edges.size(); c++) {
      ppn::edge *e = ppn->edges[c];
      if (e->from_node && e->to_node && e->to_node->nr == node->nr) {
        printf(" Edge %d -> %d; |IPD| = %d\n",
        		e->from_node->nr, e->to_node->nr, getCardinality(ctx, e->from_domain));
        // TODO: (4.7) t_Rd = |IPD| * t_isolated / |D|
        THR_t t_rd = getCardinality(ctx, e->from_domain) * c_iso / getCardinality(ctx, d_node);

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
  delete[] topo;
}


// Implementation of Algorithm 1 in Sjoerd Meijer's thesis.
// 
// OBSOLETE, PDG-based
//
/*void throughput(PDG *pdg) {
  int n = pdg->nodes.size();
  pdg::node **topo = new pdg::node*[n];

  // workload and cost for communication need to be read from external files
  THR_t workload[10] = {10, 10, 10, 20, 10, 10, 10, 10, 10, 10};


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
    	  isl_set_card();
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
