//
//
//
//
// Analyze PDG1
//
//
//
//
#include <iostream>
//#include <set>

//#include <isl_set_polylib.h>

#include "isa/yaml.h"
#include "isa/pdg.h"
#include "isl/set.h"
#include "isl/map.h"
//extern "C" {
//#include "isl_util.h"
//}

using pdg::PDG;
using namespace std;


void dump_distances(PDG *pdg) {
  isl_ctx *ctx = pdg->get_isl_ctx();
  fprintf(stderr, "Dependence distances etc for non-reuse channels:\n");

  for (int i = 0; i < pdg->dependences.size(); ++i) {
    pdg::dependence *dep = pdg->dependences[i];
    // Not a real dependence
    if (dep->type == pdg::dependence::uninitialized)
      continue;
    // Not a selfloop
//    if (dep->from != dep->to)
//      continue;
    if (dep->from_access->type == pdg::access::read) {
      // It's a reuse channel
      continue;
    }
    int weight = dep->type == pdg::dependence::anti ? 0 : 1;
    pdg::node* from = dep->from;
    pdg::node* to = dep->to;
    isl_set *ddv;
    isl_map *scat;
		isl_map *dep_map = dep->relation->get_isl_map(ctx);
		scat = from->schedule->get_isl_map(ctx);
		dep_map = isl_map_apply_domain(dep_map, scat);
		scat = to->schedule->get_isl_map(ctx);
		dep_map = isl_map_apply_range(dep_map, scat);
		ddv = isl_map_deltas(dep_map);

    fprintf(stderr, "ED_%-2d %12s->%12s var=%-2s size=%2s   ", i, dep->from->statement->top_function->name->s.c_str(), dep->to->statement->top_function->name->s.c_str(), dep->array->name->s.c_str(), dep->size ? dep->size->s->c_str() : "?");
    isl_set_dump(ddv);
    isl_set_free(ddv);
  }
}


int main(int argc, char * argv[])
{
  FILE *in = stdin, *out = stdout;
  int c, ind = 0;
  PDG *pdg;
  bool evaluate = true;

  isl_ctx *ctx = isl_ctx_alloc();
  pdg = PDG::Load(in, ctx);
//  pdg = yaml::Load<PDG>(in);
  if (!pdg) {
    fprintf(stderr, "No PDG specified or PDG invalid.\n");
    fprintf(stderr, "Usage: pdg1ana < file.yaml > file2.yaml\n");
    exit(1);
  }

  /*if (pdg->dependences.size() != 0) {
    fprintf(stderr, "Error: input already contains dependence information.\n");
    exit(1);
  }*/

  int j;
  int nMult = 0, nReorder = 0;
  for (j = 0; j < pdg->dependences.size(); j++) {
    pdg::dependence *edge = pdg->dependences[j];
    if (edge->multiplicity) {
      nMult++;
    }
    if (edge->reordering) {
      nReorder++;
    }
  }
  printf("Nodes: %d\n", pdg->nodes.size());
  printf("Edges: %d of which:\n", pdg->dependences.size());
  printf("       %d multiplicity\n", nMult);
  printf("       %d reordering\n", nReorder);
  printf("\n");

  int i;
  for (i = 0; i < pdg->nodes.size(); i++) {
    pdg::node *node = pdg->nodes[i];
    printf("Node: %2d  line %3d  dim %d  %s", node->nr, node->statement->line, node->source->dim(), node->statement->top_function->name->s.c_str());
    // To print the prefix:
    /*printf("Node: %2d  line %3d %30s  pre:", node->nr, node->statement->line, node->statement->top_function->name->s.c_str());
    for (int j = 0; j < node->prefix.size(); j++) {
      printf("%s%2d", j==0?"":", ", node->prefix[j]);
    }*/
    printf("\n");
  }

  dump_distances(pdg);


  //pdg->add_history_line("pdg1ana", argc, argv);
  //pdg->Dump(out);

  pdg->free();
  delete pdg;
  isl_ctx_free(ctx);

  return 0;
}
