/*
 * pn2ppn_util.cc
 * pn2ppn utilities are kept in this file to reduce the dependence of the PPN class on external libs.
 *
 *    Created on: Sep 30, 2010
 *      Author: Teddy Zhai, Sven van Haastregt
 *      $Id: pn2ppn_util.cc,v 1.4 2011/03/21 15:48:33 svhaastr Exp $
 *
 */

#include <cloog/isl/cloog.h>
#include "cloog_util.cc"
#include "ppn.h"

using namespace ppn;

static ASTNode_Block *convert_stmt(clast_stmt *s);

static ASTNode_Stmt *convert_user_stmt(clast_user_stmt *u) {
  ASTNode_Stmt *ret = new ASTNode_Stmt;

  char *name = (char *)u->statement->name;
  if (name != NULL)
    ret->setName(new str(name));
  else 
    ret->setName(new str("NULL"));

  if (strchr(name, 'O')) {
    ret->setType(Stmt_OPD);
  }
  else if (strchr(name, 'I')) {
    ret->setType(Stmt_IPD);
  }
  else {
    ret->setType(Stmt_Function);
  }

  return ret;
}

static ASTNode_If *convert_guard(clast_guard *g) {
  ASTNode_If *ret = new ASTNode_If;

  ret->setSign(g->eq[0].sign);
  ret->setThen(convert_stmt(g->then));
  //TODO: handle expressions

  return ret;
}

static ASTNode_For *convert_for(clast_for *f) {
  ASTNode_For *ret = new ASTNode_For;

  ret->setIterator(new str(f->iterator));
  ret->setBody(convert_stmt(f->body));
  ret->setStride(isl_int_get_si(f->stride));
  //TODO: handle expressions

  return ret;
}


static ASTNode_Block *convert_stmt(clast_stmt *s) {
  ASTNode_Block *ret = new ASTNode_Block;

  for ( ; s; s = s->next) {
    if (CLAST_STMT_IS_A(s, stmt_user)) {
      ret->append(convert_user_stmt((clast_user_stmt *) s));
    } else if (CLAST_STMT_IS_A(s, stmt_for)) {
      ret->append(convert_for((clast_for*)s));
    } else if (CLAST_STMT_IS_A(s, stmt_guard)) {
      ret->append(convert_guard((clast_guard*)s));
    }
  }
  
  return ret;
}


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
  ret->setRoot(convert_stmt(stmt));

  cloog_clast_free(stmt);

  return ret;
}
