/*
 * Transforms a PDG without dependence information according to the options given on
 * the command line.
 * Authors: Wouter de Zwijger,
 *          Sven van Haastregt
 */

#include "TransCLParser.h"

#include <sstream>

using pdg::PDG;
using namespace std;


int main(int argc, char * argv[])
{
  FILE *in = stdin, *out = stdout;

  isl_ctx *ctx = isl_ctx_alloc();
  PDG *pdg;

  pdg = PDG::Load(in, ctx);
  if (!pdg) {
    fprintf(stderr, "No PDG specified or PDG invalid.\n");
    fprintf(stderr, "Usage: pdgtrans < file.yaml > file2.yaml\n");
    exit(1);
  }

  if (pdg->dependences.size() != 0) {
    fprintf(stderr, "Error: input already contains dependence information.\n");
    exit(1);
  }

  // execute all given arguments
  TransCLParser parser(pdg);
  parser.parseAll(argc, argv);

  pdg->add_history_line("pdgtrans", argc, argv);
  
  // dump new tree in case no error has occured
  if(!TransError::print()){
    pdg->Dump(out);
  }

  pdg->free();
  delete pdg;

  isl_ctx_free(ctx);

  return TransError::isFound();
}
