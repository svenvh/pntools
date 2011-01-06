//
// Implementation of throughput analysis
// Sven van Haastregt, September-December 2010
// LERC, LIACS, Leiden University
// $Id: ppnta.cc,v 1.10 2011/01/06 16:15:03 svhaastr Exp $
//
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

// Aggregate all incoming FIFOs of a node
THR_t aggregateFifos(PPN *ppn, pdg::node *node, THR_t tFifo[]) {
  int nAccesses = node->statement->accesses.size();
  THR_t *tArgs = new THR_t[nAccesses];
  THR_t tFAggr = THR_INF;

  // For each read access of this node, find the associated FIFOs and add up the FIFO throughputs.
  for (int a = 0; a < nAccesses; a++) {
    tArgs[a] = 0.0;
    pdg::access *acc = node->statement->accesses[a];
    if (acc->type == pdg::access::read) {
      printf(" Aggregating arg %d with incoming FIFOs: ", a);
      for (int c = 0; c < ppn->getEdges().size(); c++) {
        ppn::edge *e = ppn->getEdges()[c];
        if (e->from_node && e->to_node && e->to_node->nr == node->nr) {
          assert(e->to_access.size() == 1);  // [svh] I didn't test multiple to_accesses
          if (e->to_access[0] == acc) {
            tArgs[a] += tFifo[c];
            printf("%d ", c);
          }
        }
      }
      printf("\n");
    }
  }

  // Now find the minimum of all incoming FIFOs
  for (int a = 0; a < nAccesses; a++) {
    if (tArgs[a] > 0.0) {
      if (tArgs[a] < tFAggr) {
        tFAggr = tArgs[a];
      }
    }
  }

  delete[] tArgs;

  return tFAggr;
}


// Implementation of Algorithm 1 in Sjoerd Meijer's thesis.
void throughput(PPN *ppn) {
  int workload[10] = {5, 8, 6, 20, 10, 10, 10, 10, 10, 10};

  int n = ppn->getNodes().size();
  pdg::node **topo = new pdg::node*[n];

  isl_ctx *ctx = isl_ctx_alloc();
  assert(ctx);

  ppn->toposort(topo);
  printf("Topological sort:\n");
  for (int i = 0; i < n; i++) {
    printf("%2d %s\n", topo[i]->nr, topo[i]->statement->top_function->name->s.c_str());
  }
  printf("\n");

  // Storage for intermediate results
  THR_t *tIso = new THR_t[n];
  THR_t *tP = new THR_t[n];
  THR_t *tFifo_Rd = new THR_t[ppn->getEdges().size()];
  THR_t *tFifo_Wr = new THR_t[ppn->getEdges().size()];
  THR_t *tFifo = new THR_t[ppn->getEdges().size()];

  // Process nodes in topological order
  for (int i = 0; i < n; i++) {
    pdg::node *node = topo[i];     // Current node respecting topological order
    printf("---- Process %d (%s) ----\n", i, node->statement->top_function->name->s.c_str());

    // Step 1:
    tIso[node->nr] = 1.0 / workload[node->nr];

    for (int c = 0; c < ppn->getEdges().size(); c++) {
      ppn::edge *e = ppn->getEdges()[c];
      if (e->from_node && e->to_node && e->to_node->nr == node->nr) {
        // Step 2:
        // (4.7) t_Rd = |IPD| / |D| * t_isolated
        tFifo_Rd[c] = ((double)getCardinality(ctx, e->from_domain) / (double)getCardinality(ctx, node->source)) * tIso[node->nr];
        printf(" Edge %-2d  %2d->%-2d  tFifo_Rd = 1/%f\n", c, e->from_node->nr, e->to_node->nr, 1.0/tFifo_Rd[c]);

        // Step 3:
        // (4.5) t_F = min(t_Rd, t_Wr)
        tFifo[c] = thr_min(tFifo_Rd[c], tFifo_Wr[c]);
        printf("                  tFifo    = 1/%f\n", 1.0/tFifo[c]);
      }
    }

    
    // Step 4
    // Aggregate the incoming FIFO channels
    THR_t tFAggr = aggregateFifos(ppn, node, tFifo);
    printf(" tFAggr = 1/%f\n", 1.0/tFAggr);


    // Step 5:
    // tP = min(tIso, TFaggr)
    tP[node->nr] = thr_min(tIso[node->nr], tFAggr);
    printf(" tIso   = 1/%f\n", 1.0/tIso[node->nr]);
    printf(" tP_%-2d  = 1/%f\n", node->nr, 1.0/tP[node->nr]);


    // Step 6:
    for (int c = 0; c < ppn->getEdges().size(); c++) {
      ppn::edge *e = ppn->getEdges()[c];
      if (e->from_node && e->to_node && e->from_node->nr == node->nr) {
        // (4.6) t_Wr = |OPD| / |D| * t_node
        tFifo_Wr[c] = ((double)getCardinality(ctx, e->to_domain) / (double)getCardinality(ctx, node->source)) * tP[node->nr];
        printf(" Edge %-2d  %2d->%-2d  tFifo_Wr = 1/%f\n", c, e->from_node->nr, e->to_node->nr, 1.0/tFifo_Wr[c]);
      }
    }
    printf("\n");
  }

  isl_ctx_free(ctx);

  delete[] topo;
  delete[] tFifo_Rd;
  delete[] tFifo_Wr;
  delete[] tFifo;
  delete[] tIso;
  delete[] tP;
}


int main(int argc, char * argv[])
{
  FILE *in = stdin, *out = stdout;
  int c, ind = 0;
  bool evaluate = true;

  PPN *ppn = new PPN;
  ppn = yaml::Load<PPN>(in);

  if (!ppn) {
    fprintf(stderr, "No PPN specified or PPN invalid.\n");
    fprintf(stderr, "Usage: ppnta < file.ppn\n");
    exit(1);
  }

  throughput(ppn);

  ppn->free();
  delete ppn;

  return 0;
}
