//
//
//
//
// Analyze PDG1 (PDG without dep info)
//
//
//
//
#include <iostream>
//#include <set>

//#include <isl_set_polylib.h>

#include "yaml.h"
#include "pdg.h"
//extern "C" {
//#include "isl_util.h"
//}

using pdg::PDG;
using namespace std;



int main(int argc, char * argv[])
{
  FILE *in = stdin, *out = stdout;
  int c, ind = 0;
  PDG *pdg;
  bool evaluate = true;

  pdg = yaml::Load<PDG>(in);
  if (!pdg) {
    fprintf(stderr, "No PDG specified or PDG invalid.\n");
    fprintf(stderr, "Usage: pdg1ana < file.yaml > file2.yaml\n");
    exit(1);
  }

  if (pdg->dependences.size() != 0) {
    fprintf(stderr, "Error: input already contains dependence information.\n");
    exit(1);
  }

  int i;
  for (i = 0; i < pdg->nodes.size(); i++) {
    pdg::node *node = pdg->nodes[i];
    printf("Node: %2d  line %3d %30s  pre:", node->nr, node->statement->line, node->statement->top_function->name->s.c_str());
    for (int j = 0; j < node->prefix.size(); j++) {
      printf("%s%2d", j==0?"":", ", node->prefix[j]);
    }
    printf("\n");
  }

  //pdg->add_history_line("pdg1ana", argc, argv);
  //pdg->Dump(out);

  pdg->free();
  delete pdg;

  return 0;
}
