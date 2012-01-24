/*
 * Dump some statistics of an ADG.
 *
 * Sven van Haastregt
 * Copyright LIACS, Leiden University, 2012.
 * All rights reserved.
 */

#include <cstdio>
#include <math.h>
#include <assert.h>
#include "adg_parse.h"            // TODO: this file should be in include/isa

using namespace std;

static const int token_bits = 32; // Number of bits per token

// Collected statistics about channels
struct channel_statistics {
  // Constructor
  channel_statistics() {
    n_srl       = 0;
    n_bram18k   = 0;
    n_mult      = 0;
    n_reorder   = 0;
  }

  // Members
  int n_srl;                      // Number of SRL16-implementable edges
  int n_bram18k;                  // Number of 18Kbit BRAMs needed
  int n_mult;                     // Number of multiplicity channels
  int n_reorder;                  // Number of reordering channels
};


// Analyzes `edge` and updates channel statistics.
static void analyze_edge(adg_edge *edge, channel_statistics *stats) {
  int buffer_size = isl_int_get_si(edge->value_size);
  if (edge->type == adg_edge_generic) {
    stats->n_reorder++;
  }
  else if (edge->type == adg_edge_broadcast) {
    stats->n_mult++;
  }
  else {
    assert(buffer_size > 0);
    if (buffer_size < 16) {
      // Implemented using SRL16s
      stats->n_srl++;
    }
    else {
      // Implemented using BRAMs
      const int bits_per_bram = 16384;
      int total_bits = buffer_size * token_bits;
      stats->n_bram18k += (int)ceil((double)total_bits/(double)bits_per_bram);
    }
  }
}


// Analyzes an ADG and dumps a report to `out`.
static void analyze(adg *graph, FILE *out) {
  channel_statistics stats;
  for (int i = 0; i < graph->edges.size(); i++) {
    analyze_edge(graph->edges[i], &stats);
  }
  fprintf(out, "Channel statistics: (assuming a token size of %d bits)\n", token_bits);
  fprintf(out, "  SRLs:       %4d\n", stats.n_srl);
  fprintf(out, "  BRAMs18k:   %4d\n", stats.n_bram18k);
  fprintf(out, "  Multiplic.: %4d\n", stats.n_mult);
  fprintf(out, "  Reorder:    %4d\n", stats.n_reorder);
}


int main(int argc, char *argv[]) {
  isl_ctx *ctx;
  adg *graph;
  FILE *in = stdin, *out = stdout;

  ctx = isl_ctx_alloc();

  graph = adg_parse(ctx, in);
  if (!graph) {
    fprintf(stderr, "No ADG specified or ADG invalid.\n");
    fprintf(stderr, "Usage: adgstat < file.adg\n");
    exit(1);
  }

  analyze(graph, out);

  delete graph;
  isl_ctx_free(ctx);

  return 0;
}
