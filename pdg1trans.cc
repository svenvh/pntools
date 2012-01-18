//
// Analyze PDG1 (PDG without dep info)
//	Author: Teddy Zhai, Sven van Haastregt
// $Id: pdg1trans.cc,v 1.5 2012/01/18 15:38:22 tzhai Exp $
//
//
#include <iostream>
//#include <set>

//#include <isl_set_polylib.h>

#include "isa/yaml.h"
#include "isa/pdg.h"
//extern "C" {
//#include "isl_util.h"
//}
//
#include "transgdal.h"
#include "TrafoPartitioning.h"

using namespace pdg;
using namespace std;

bool
checkProcessName(const PDG *pdg, const string *procName){
	bool ret = false;

	// Traverse the vector to check the process name
	for (int i = 0; i < pdg->nodes.v.size(); ++i) {
		const node *nd = pdg->nodes.v[i];

		if (procName == (string*) nd->statement->top_function->name) {
			ret = true;
		}
	}

	return ret;
}


bool
checkIterName(const PDG *pdg, const string *iterName){

}


int main(int argc, char * argv[])
{
	FILE *in = stdin, *out = stdout;
	int c, ind = 0;
	PDG *pdg;

#if 0
  if (argc != 2) {
    fprintf(stderr, "Usage: pdg1trans file.yaml > file2.yaml\n");
    exit(1);
  }
  char *input = (char*) malloc(strlen(argv[1])+1);
  strcpy(input, argv[1]);
  in = fopen(input, "r");
#endif

  /////////////////////////////////////////////////////////////////////////////////////////
  /// we want to give parameters directly at command line
  // trans <input pdg> <process name> <partition type> <iterator name> <param>
  // if it is modolo, <param> indicates unfolding factor
  // TODO: if it is plain-cut, how can we give parameters
  int factor = 0;
  if (argc != 6){
	  //TODO:
//	  fprintf(stderr, "Usage: pdg1trans \n");
	  exit(1);
  }

  // load pdg
  char *input = (char*) malloc(strlen(argv[1])+1);
  strcpy(input, argv[1]);
  in = fopen(input, "r");
  pdg = yaml::Load<PDG>(in);
  if (!pdg) {
	  fprintf(stderr, "No PDG specified or PDG invalid.\n");
	  fprintf(stderr, "Usage: pdg1trans file.yaml > file2.yaml\n");
	  exit(1);
  }

  string procName(argv[2]);
//  if (checkProcessName(pdg, procName) == false) {
//	  fprintf(stderr, "there is no process with given name in the PPN!\n");
//	  exit(1);
//  }

  string iterName(argv[3]);
//  if (checkIterName(pdg, iterName) == false) {
//	  fprintf(stderr, "there is no iterator with given name in the PPN!\n");
//	  exit(1);
//  }

  if (pdg->dependences.size() != 0) {
	  fprintf(stderr, "Error: input already contains dependence information.\n");
	  exit(1);
  }

#if 0
  string inputfile = input;
  handle partitioning(pdg, inputfile.substr(0,inputfile.length() - 5));
  partitioning.main_handle();
#endif

#if 0
  // Re-implementation of process partitioning (Teddy Zhai)
  TrafoPartitioning (pdg, factor);

#endif


  pdg->add_history_line("pdg1trans", argc, argv);
  pdg->Dump(out);

  pdg->free();
  delete pdg;
  free(input);

  return 0;
}



