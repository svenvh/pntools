/*
 * AST.h
 *
 *    Created on: Feb 2, 2011
 *      Author: Sven van Haastregt, Teddy Zhai
 *      $Id: ast.h,v 1.8 2011/04/07 16:10:13 svhaastr Exp $
 */

#ifndef AST_H_
#define AST_H_

#include "yaml.h"

#include "global.h"


using namespace yaml;

namespace ppn {

///////////////////////////////////////////////////////////////

// Pure virtual / Abstract
class ASTExpression: public structure {
public:
  virtual bool isConstant();
  virtual void dumpCProgram(FILE *out) =0;

private:

};


class ASTName: public ASTExpression {
private:
  str *name;

  static serialize *create(void *user) { return new ASTName(); }
public:
  ASTName() {};
  ~ASTName() {};
  str * getName();
  void setName(str *newname);
  void dumpCProgram(FILE *out);
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
  int  getCoeff();
  void setCoeff(int newcoeff);
  ASTExpression * getVar();
  void setVar(ASTExpression *newvar);
  bool isConstant();
  void dumpCProgram(FILE *out);
  static void register_type();
};


typedef enum {BINOP_UNSET = -1, BINOP_MODULO, BINOP_DIV, BINOP_FLOORDIV, BINOP_CEILDIV} ASTBinop_type;

class ASTBinop: public ASTExpression {
private:
  ASTBinop_type type;
  ASTExpression *LHS;
  int RHS;

  static serialize *create(void *user) { return new ASTBinop(); }
public:
  ASTBinop();
  ~ASTBinop() {};
  ASTBinop_type getType();
  void setType(ASTBinop_type newtype);
  ASTExpression *getLHS();
  void setLHS(ASTExpression *newLHS);
  int getRHS();
  void setRHS(int newRHS);
  void dumpCProgram(FILE *out);
  static void register_type();
};


typedef enum {RED_UNSET = -1, RED_SUM, RED_MIN, RED_MAX} ASTReduction_type;

class ASTReduction: public ASTExpression {
private:
  ASTReduction_type type;
  seq<ASTExpression> elts;

  static serialize *create(void *user) { return new ASTReduction(); }
public:
  ASTReduction();
  ~ASTReduction() {};
  ASTReduction_type getType();
  void setType(ASTReduction_type newtype);
  void append(ASTExpression* expr);
  void dumpCProgram(FILE *out);
  static void register_type();
};


//////////////////////////////////////////////////////

typedef enum {NODE_UNSET = -1, NODE_BLOCK, NODE_IF, NODE_FOR, NODE_STMT} ASTNodeType;

// Pure virtual / Abstract
class ASTNode: public structure {
protected:
  ASTNodeType nodetype;
  static serialize *create(void *user);
private:
  ASTNode *parent;
public:
  ASTNode();
  ~ASTNode();
  ASTNodeType  getNodetype();
  void setNodetype(ASTNodeType newnodetype);
  virtual void dumpCProgram(FILE *out, int indent) =0;
  static void registerbase_ASTNode(struct_description &sd);
  static void register_type();
  //void init(SyckParser *p, SyckNode *n);
};

class ASTNode_Block: public ASTNode {
private:
  seq<ASTNode> stmts;
  ASTExpression *LHS;
  ASTExpression *RHS;
  int sign;
  ASTNode_Block *then;

  static serialize *create(void *user) { printf("BLK\n"); return new ASTNode_Block(); }
public:
  ASTNode_Block();
  ~ASTNode_Block() {};
  void append(ASTNode* node);
  void dumpCProgram(FILE *out, int indent);
  static void register_type();
};


class ASTNode_If: public ASTNode {
private:
  ASTExpression *LHS;
  ASTExpression *RHS;
  int sign;
  ASTNode_Block *then;

  static serialize *create(void *user) { printf("IF\n"); return new ASTNode_If(); }
public:
  ASTNode_If();
  ~ASTNode_If() {};
  ASTExpression * getLHS();
  void setLHS(ASTExpression *newLHS);
  ASTExpression * getRHS();
  void setRHS(ASTExpression *newRHS);
  int  getSign();
  void setSign(int newsign);
  ASTNode_Block * getThen();
  void setThen(ASTNode_Block *newthen);

  void dumpCProgram(FILE *out, int indent);
  static void register_type();
};


class ASTNode_For: public ASTNode {
private:
  str *iterator;
  ASTExpression *lb;
  ASTExpression *ub;
  int stride;
  ASTNode_Block *body;

  static serialize *create(void *user) { printf("FOR\n"); return new ASTNode_For(); }
public:
  ASTNode_For();
  ~ASTNode_For() {};

  str * getIterator();
  void setIterator(str *newiterator);
  ASTExpression * getLb();
  void setLb(ASTExpression *newlb);
  ASTExpression * getUb();
  void setUb(ASTExpression *newub);
  int  getStride();
  void setStride(int newstride);
  ASTNode_Block * getBody();
  void setBody(ASTNode_Block *newbody);

  void dumpCProgram(FILE *out, int indent);

  static void register_type();
};


typedef enum type {unset = -1, Stmt_IPD, Stmt_OPD, Stmt_Function} Stmt_type;

class ASTNode_Stmt: public ASTNode {
private:
  str *name;
  Stmt_type type;

  static serialize *create(void *user) { return new ASTNode_Stmt(); }
public:
  ASTNode_Stmt();
  ~ASTNode_Stmt(){};
  str * getName();
  void setName(str *newname);
  Stmt_type getType();
  void setType(Stmt_type newtype);

  void dumpCProgram(FILE *out, int indent);
  static void register_type();
};


class AST: public structure {
public:
  ////////////////////////////////////////////////////////////////////////////
  //// Constructors/destructors
  AST() {};
  ~AST() {};

  ////////////////////////////////////////////////////////////////////////////
  //// Accessors
  ASTNode_Block * getRoot();
  void setRoot(ASTNode_Block *newroot);

  ////////////////////////////////////////////////////////////////////////////
  //// Other methods
  void dumpCProgram(FILE *out);

  ////////////////////////////////////////////////////////////////////////////
  //// YAML stuff
  static void register_type();
  void dump(emitter& e);


private:
  ASTNode_Block *root;
  static serialize *create(void *user) { return new AST(); }
};

// Hack to deserialize using YAML:
class ASTNode_YAML: public ASTNode {
  public:
    seq<ASTNode> stmts;

    ASTExpression *LHS;
    ASTExpression *RHS;
    int sign;
    ASTNode_Block *then;

    str *iterator;
    ASTExpression *lb;
    ASTExpression *ub;
    int stride;
    ASTNode_Block *body;

    str *name;
    Stmt_type type;

    void dumpCProgram(FILE *out, int indent);
    static void register_type();
};

} // end namespace ppn
#endif /* AST_H_ */
