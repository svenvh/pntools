/*
 * pn2ppn_util.cc
 * pn2ppn utilities are kept in this file to reduce the dependence of the PPN class on external libs.
 *
 *    Created on: Sep 30, 2010
 *      Author: Teddy Zhai, Sven van Haastregt
 *      $Id: pn2ppn_util.cc,v 1.3 2011/02/07 09:55:34 svhaastr Exp $
 *
 */

#include <cloog/isl/cloog.h>
#include "cloog_util.cc"
#include "ppn.h"

using namespace ppn;

AST *cloog_clast_to_AST(CloogInput *input, int dim, CloogOptions *options) {
  fprintf(stderr, "[cloog_clast_to_AST] Hi there! Please implement me!\n"); // TODO

  options->override = 1;
  options->f = -1;
  options->l = dim;
  options->esp = 1;
  options->sh = 1;

  struct clast_stmt *stmt = cloog_clast_create_from_input(input, options);
  cloog_util_simple_guards(stmt);
  stmt = cloog_util_hoist_ifs(stmt);

  AST *ret = new AST;
  //TODO: traverse stmt

  cloog_clast_free(stmt);

  return ret;
}
