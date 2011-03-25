/*
 * pn2ppn_util.cc
 * pn2ppn utilities are kept in this file to reduce the dependence of the PPN class on external libs.
 *
 *    Created on: Sep 30, 2010
 *      Author: Teddy Zhai, Sven van Haastregt
 *      $Id: pn2ppn_util.cc,v 1.5 2011/03/25 13:06:34 svhaastr Exp $
 *
 */

#include <cloog/isl/cloog.h>
#include "cloog_util.cc"
#include "ppn.h"

using namespace ppn;


/////////// CLAST-to-AST Conversion functions ///////////

static ASTExpression *convert_expr(clast_expr *e);

static ASTName *convert_expr_name(clast_name *n) {
  ASTName *ret = new ASTName;

  ret->setName(new str(n->name));

  return ret;
}

static ASTTerm *convert_expr_term(clast_term *t) {
  ASTTerm *ret = new ASTTerm;
  ret->setCoeff(isl_int_get_si(t->val));
  if (t->var) {
    ret->setVar(convert_expr(t->var));
  }
  else {
    ret->setVar(NULL);
  }
  return ret;
}

static ASTReduction *convert_expr_reduction(clast_reduction *r) {
  ASTReduction *ret = new ASTReduction;

  switch (r->type) {
    case clast_red_sum: ret->setType(RED_SUM); break;
    default: assert(0);
  }

  for (int i = 0; i < r->n; i++) {
    ret->append(convert_expr(r->elts[i]));
  }

  return ret;
}

static ASTBinop *convert_expr_binop(clast_binary *b) {
  ASTBinop *ret = new ASTBinop;

  switch (b->type) {
    case clast_bin_mod: ret->setType(BINOP_MODULO); break;
    case clast_bin_div: ret->setType(BINOP_DIV); break;
    case clast_bin_fdiv: ret->setType(BINOP_FLOORDIV); break;
    case clast_bin_cdiv: ret->setType(BINOP_CEILDIV); break;
    default: assert(0);
  }

  ret->setLHS(convert_expr(b->LHS));
  ret->setRHS(isl_int_get_si(b->RHS));

  return ret;
}

static ASTExpression *convert_expr(clast_expr *e) {
  ASTExpression *ret = NULL;
  assert(e);

  if (!e)
    return NULL;

  switch (e->type) {
    case clast_expr_name:
      ret = convert_expr_name((clast_name*) e);
      break;
    case clast_expr_term:
      ret = convert_expr_term((clast_term*) e);
      break;
    case clast_expr_red:
      ret = convert_expr_reduction((clast_reduction*) e);
      break;
    case clast_expr_bin:
      ret = convert_expr_binop((clast_binary*) e);
      break;
    default:
      assert(0);
  }
  return ret;
}



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

  assert(g->n == 1); // >1 equation not supported currently

  ret->setSign(g->eq[0].sign);
  ret->setThen(convert_stmt(g->then));
  ret->setLHS(convert_expr(g->eq[0].LHS));
  ret->setRHS(convert_expr(g->eq[0].RHS));

  return ret;
}

static ASTNode_For *convert_for(clast_for *f) {
  ASTNode_For *ret = new ASTNode_For;

  ret->setIterator(new str(f->iterator));
  ret->setBody(convert_stmt(f->body));
  ret->setStride(isl_int_get_si(f->stride));
  ret->setLb(convert_expr(f->LB));
  ret->setUb(convert_expr(f->UB));

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


// Translates a CLooG clast into a PPN AST (which can be dumped to a YAML file)
AST *cloog_clast_to_AST(CloogInput *input, int dim, CloogOptions *options) {
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
