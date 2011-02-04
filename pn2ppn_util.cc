/*
 * pn2ppn_util.cc
 * pn2ppn utilities are kept in a separate file to reduce the dependence of the PPN class on external libs.
 *
 *    Created on: Sep 30, 2010
 *      Author: Teddy Zhai, Sven van Haastregt
 *      $Id: pn2ppn_util.cc,v 1.1 2011/02/04 16:31:36 svhaastr Exp $
 *
 */

#include <cloog/isl/cloog.h>
#include "cloog_util.cc"
#include "ppn.h"

using namespace ppn;

AST *cloog_clast_to_AST(CloogInput *input, int dim, CloogOptions *options) {
  printf("Hi there! Please implement me!\n"); // TODO
  AST *ret = new AST;

  options->override = 1;
  options->f = -1;
  options->l = dim;
  options->esp = 1;
  options->sh = 1;

  //cloog_input_dump_cloog(stdout, input, options);
/*
  struct clast_stmt *stmt = cloog_clast_create_from_input(input, options);
  cloog_util_simple_guards(stmt);
  stmt = cloog_util_hoist_ifs(stmt);

  //TODO: traverse stmt

  cloog_clast_free(stmt);
*/
  printf("Done\n");

  return ret;
}
