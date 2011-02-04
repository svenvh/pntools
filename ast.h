/*
 * AST.h
 *
 *  	Created on: Feb 2, 2011
 *      Author: Sven van Haastregt, Teddy Zhai
 *      $Id: ast.h,v 1.2 2011/02/04 16:31:36 svhaastr Exp $
 */

#ifndef AST_H_
#define AST_H_

#include "yaml.h"

#include "global.h"


using namespace yaml;

namespace ppn {

///////////////////////////////////////////////////////////////// Pure virtual / Abstract
class ASTExpression: public structure {
public:
  bool isConstant();

private:

};


class ASTName: public ASTExpression {
private:
  str *name;

  static serialize *create(void *user) { return new ASTName(); }
public:
  ASTName() {};
  ~ASTName() {};
  static void register_type();
};


class ASTTerm: public ASTExpression {
private:
  int coeff;
  ASTExpression *var;

  static serialize *create(void *user) { return new ASTTerm(); }
public:
  ASTTerm() {};
  ~ASTTerm() {};
  static void register_type();
};


class ASTBinop: public ASTExpression {
private:
  enum type {unset = -1, OP_MODULO, OP_DIV, OP_FLOORDIV, OP_CEILDIV} type;
  ASTTerm *LHS;
  ASTTerm *RHS;

  static serialize *create(void *user) { return new ASTBinop(); }
public:
  ASTBinop() {};
  ~ASTBinop() {};
  static void register_type();
};


class ASTReduction: public ASTExpression {
private:
  enum type {unset = -1, RED_SUM, RED_MIN, RED_MAX} type;
  int n;
  seq<ASTExpression> elts;

  static serialize *create(void *user) { return new ASTReduction(); }
public:
  ASTReduction() {};
  ~ASTReduction() {};
  static void register_type();
};


//////////////////////////////////////////////////////// Pure virtual / Abstract
class ASTNode: public structure {
private:
  ASTNode *parent;
};


class ASTNode_Block: public ASTNode {
private:
	seq<ASTNode> stmts;

	static serialize *create(void *user) { return new ASTNode_Block(); }
public:
	ASTNode_Block(){};
	~ASTNode_Block() {};
	static void register_type();
};


class ASTNode_If: public ASTNode {
private:
  ASTExpression *LHS;
  ASTExpression *RHS;
  int sign;
  ASTNode_Block *then;

	static serialize *create(void *user) { return new ASTNode_If(); }
public:
	ASTNode_If(){};
	~ASTNode_If() {};
	static void register_type();
};


class ASTNode_For: public ASTNode {
private:
  str *iterator;
  ASTExpression *lb;
  ASTExpression *ub;
  int stride;
  ASTNode_Block *body;

  static serialize *create(void *user) { return new ASTNode_For(); }
public:
	ASTNode_For(){};
	~ASTNode_For() {};
	static void register_type();
};


class ASTNode_Stmt: public ASTNode {
private:
  str *name;
  enum type {unset = -1, Stmt_IPD, Stmt_OPD, Stmt_exec} type;

  static serialize *create(void *user) { return new ASTNode_Stmt(); }
public:
  ASTNode_Stmt(){};
  ~ASTNode_Stmt(){};
  static void register_type();
};


class AST: public structure {
public:
  ////////////////////////////////////////////////////////////////////////////
  //// Constructors/destructors
  AST() {};
  ~AST() {};

  ////////////////////////////////////////////////////////////////////////////
  //// YAML stuff
/*  static AST* Load(char *str, void *user = NULL);
  static AST* Load(FILE *fp, void *user = NULL);*/
  static void register_type();
  void dump(emitter& e);


private:
  ASTNode_Block *root;
  static serialize *create(void *user) { return new AST(); }
};

} // end namespace ppn
#endif /* AST_H_ */
