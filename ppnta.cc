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


// Implementation of Algorithm 1 in Sjoerd Meijer's thesis.
void throughput(PDG *pdg) {
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


int main(int argc, char * argv[])
{
  FILE *in = stdin, *out = stdout;
  int c, ind = 0;
  PDG *pdg;
  bool evaluate = true;

  PPN *ppn = new PPN;
  ppn->Dump(stdout);
  printf("Bullshit follows now\n");

  pdg = yaml::Load<PDG>(in);
  if (!pdg) {
    fprintf(stderr, "No PPN specified or PPN invalid.\n");
    fprintf(stderr, "Usage: ppnta < file.yaml\n");
    exit(1);
  }

  if (pdg->dependences.size() == 0) {
    fprintf(stderr, "Error: input does not contain dependence information.\n");
    exit(1);
  }

  throughput(pdg);

  pdg->free();
  delete pdg;

  return 0;
}
