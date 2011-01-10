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
//
#include "transgdal.h"

using pdg::PDG;
using namespace std;



int main(int argc, char * argv[])
{
  FILE *in = stdin, *out = stdout;
  int c, ind = 0;
  PDG *pdg;
  bool evaluate = true;

  if (argc != 2) {
    fprintf(stderr, "Usage: pdg1trans file.yaml > file2.yaml\n");
    exit(1);
  }
  char *input = (char*) malloc(strlen(argv[1])+1);
  strcpy(input, argv[1]);
  in = fopen(input, "r");

  pdg = yaml::Load<PDG>(in);
  if (!pdg) {
    fprintf(stderr, "No PDG specified or PDG invalid.\n");
    fprintf(stderr, "Usage: pdg1trans file.yaml > file2.yaml\n");
    exit(1);
  }

  if (pdg->dependences.size() != 0) {
    fprintf(stderr, "Error: input already contains dependence information.\n");
    exit(1);
  }

  string inputfile = input;
  handle partitioning(pdg, inputfile.substr(0,inputfile.length() - 5));
  partitioning.main_handle();

  pdg->add_history_line("pdg1trans", argc, argv);
  pdg->Dump(out);

  pdg->free();
  delete pdg;
  free(input);

  return 0;
}
