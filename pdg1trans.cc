//
//
//
//
// Transform PDG1 (PDG without dep info)
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
#include "isl_set_polylib.h"
#include "isl_constraint.h"
#include "barvinok/barvinok.h"
#include "transgdal.h"

using pdg::PDG;
using namespace std;

int evalTerm(__isl_take isl_term *term, void *user) {
  isl_int n, d;
  isl_int_init(n);
  isl_int_init(d);

  int nvars = isl_term_dim(term, isl_dim_set);
  int nparams = isl_term_dim(term, isl_dim_param);
  // Only params are allowed:
  assert(nvars == 0);

  // Allocate space to hold the coefficients
  if (*(int**)user == NULL) {
    *(int**)user = new int[nparams+2];
  }
  int *coeffs = *(int**)user;

  for (int i = 0; i < nparams; i++) {
    printf("par[%d] %d ", i, isl_term_get_exp(term, isl_dim_param, i));
  }

  isl_term_get_num(term, &n);
  isl_term_get_den(term, &d);
  isl_int_print(stdout, n, 3);
  printf("/");
  isl_int_print(stdout, d, 3);
  printf("\n");

  isl_term_free(term);
  isl_int_clear(n);
  isl_int_clear(d);
  return 0;
}

int evalPiece(__isl_take isl_set *set, __isl_take isl_qpolynomial *qp, void *user) {
  assert(*(void**)user == NULL); // qp should consist of only a single piece, so no result is available yet
  isl_qpolynomial_foreach_term(qp, evalTerm, user);
  isl_set_free(set);
  isl_qpolynomial_free(qp);
  return 0;
}

// Determine the maximum number of iterations for a loop
void getLoopBound(isl_set *dom, int it) {
  int *coefflist = NULL;

  printf("Domain:\n");
  isl_set_dump(dom, stdout, 1);
  isl_set *s = isl_set_copy(dom);
  isl_dim *d = isl_set_get_dim(s);
  int lastn = isl_dim_size(d, isl_dim_set)-it-1;
  printf("Projecting out: %d to %d  and  %d to %d\n", 0, it, it+1, it+lastn);
  s = isl_set_project_out(s, isl_dim_set, 0, it);
  s = isl_set_project_out(s, isl_dim_set, it+1, lastn);
  isl_dim_free(d);
  printf("After projection:\n");
  isl_set_dump(s, stdout, 1);
  isl_pw_qpolynomial *pwpol = isl_set_card(s);
  isl_pw_qpolynomial_print(pwpol, stdout, 0);
  printf("\n");
  isl_pw_qpolynomial_foreach_piece(pwpol, evalPiece, (void*)&coefflist);
  isl_pw_qpolynomial_free(pwpol);
  //isl_dim *poldim = isl_pw_qpolynomial_get_dim(pwpol);
  //isl_point *pnt = isl_point_zero(poldim);
  //isl_qpolynomial *pol = isl_pw_qpolynomial_eval(pwpol, pnt);
  //isl_qpolynomial_print(pol, stdout, 0);
  //printf("\n");
  //isl_qpolynomial_free(pol);

  if (coefflist) {
    delete [] coefflist;
  }
}

void transtest(PDG *pdg) {
  int i;
  int stmt = 2;   // Statement to partition
  int it = 1;     // Iterator on which to partition
  int factor = 2; // Splitting factor

  isl_ctx *ctx = isl_ctx_alloc();
  isl_int v;
  for (i = 0; i < pdg->nodes.size(); i++) {
    pdg::node *node = pdg->nodes[i];
    if (node->nr == stmt) {
      isl_int_init(v);
      isl_set *dom = node->source->get_isl_set(ctx);
      getLoopBound(dom, it);
      // TODO: getLoopBound should return the "chunksize", which is a list of coefficients
      // TODO: then additional constraints can be constructed to split the domain:
      //       it >= 0              &&  it <= chunksize
      //       it >= chunksize+1    &&  it <= 2*chunksize
      //       it >= 2*chunksize+1  &&  it <= 3*chunksize
      //       etc.

      isl_set_free(dom);
/*
      isl_dim *d = isl_set_get_dim(dom);
      isl_basic_set *bs = isl_basic_set_universe(isl_dim_copy(d));
      isl_constraint *c = isl_inequality_alloc(d);
      isl_int_set_si(v, 2);
      isl_constraint_set_constant(c, v);
      isl_int_set_si(v, -1);
      isl_constraint_set_coefficient(c, isl_dim_set, it, v);
      bs = isl_basic_set_add_constraint(bs, c);
      isl_set *cs = isl_set_from_basic_set(bs);
      isl_set *s = isl_set_intersect(dom, cs);

      printf("After:\n");*/
      //isl_set_dump(s, stdout, 1);
      isl_int_clear(v);
    }
  }
  isl_ctx_free(ctx);
}


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
/*
  string inputfile = input;
  handle partitioning(pdg, inputfile.substr(0,inputfile.length() - 5));
  partitioning.main_handle();
*/
  pdg->add_history_line("pdg1trans", argc, argv);
  //pdg->Dump(out);

  transtest(pdg);

  pdg->free();
  delete pdg;
  free(input);

  return 0;
}
