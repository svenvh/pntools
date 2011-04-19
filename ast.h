/*
 * AST.h
 *
 *    Created on: Feb 2, 2011
 *      Author: Sven van Haastregt, Teddy Zhai
 *      $Id: ast.h,v 1.9 2011/04/19 15:36:29 svhaastr Exp $
 */

#ifndef AST_H_
#define AST_H_

#include <vector>
#include "yaml.h"

#include "global.h"


using namespace yaml;

namespace ppn {

class ASTExpression;

///////////////////////////////////////////////////////////////
typedef enum {EXPR_UNSET = -1, EXPR_NAME, EXPR_TERM, EXPR_BINOP, EXPR_REDUCTION} ASTExpressionType;
typedef enum {BINOP_UNSET = -1, BINOP_MODULO, BINOP_DIV, BINOP_FLOORDIV, BINOP_CEILDIV} ASTBinop_type;
typedef enum {RED_UNSET = -1, RED_SUM, RED_MIN, RED_MAX} ASTReduction_type;

// Hack/proxy class to deserialize using YAML:
class ASTExpression_YAMLProxy: public structure {
  public:
    ASTExpression_YAMLProxy();

    ASTExpression *fromYAMLProxy();

    ASTExpressionType type;

    str *name;
    
    int coeff;
    ASTExpression_YAMLProxy *var;

    ASTBinop_type binopType;
    ASTExpression_YAMLProxy *LHS;
    int RHS;

    ASTReduction_type redType;
    seq<ASTExpression_YAMLProxy> elts;

    static serialize *create(void *user) { return new ASTExpression_YAMLProxy(); }
    static void register_type();
};


// Pure virtual / Abstract
class ASTExpression {
public:
  virtual bool isConstant();
  virtual ASTExpression_YAMLProxy *toYAMLProxy() =0;
  virtual void dumpCProgram(FILE *out) =0;

private:

};


class ASTName: public ASTExpression {
private:
  str *name;

  //static serialize *create(void *user) { return new ASTName(); }
public:
  ASTName() {};
  ~ASTName() {};
  str * getName();
  void setName(str *newname);
  void dumpCProgram(FILE *out);
  ASTExpression_YAMLProxy *toYAMLProxy();
  //static void register_type();
};


class ASTTerm: public ASTExpression {
private:
  int coeff;
  ASTExpression *var;

  //static serialize *create(void *user) { return new ASTTerm(); }
public:
  ASTTerm() {};
  ~ASTTerm() {};
  int  getCoeff();
  void setCoeff(int newcoeff);
  ASTExpression * getVar();
  void setVar(ASTExpression *newvar);
  bool isConstant();
  void dumpCProgram(FILE *out);
  ASTExpression_YAMLProxy *toYAMLProxy();
  //static void register_type();
};



class ASTBinop: public ASTExpression {
private:
  ASTBinop_type type;
  ASTExpression *LHS;
  int RHS;

  //static serialize *create(void *user) { return new ASTBinop(); }
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
  ASTExpression_YAMLProxy *toYAMLProxy();
  //static void register_type();
};



class ASTReduction: public ASTExpression {
private:
  ASTReduction_type type;
  std::vector<ASTExpression*> elts;

  //static serialize *create(void *user) { return new ASTReduction(); }
public:
  ASTReduction();
  ~ASTReduction() {};
  ASTReduction_type getType();
  void setType(ASTReduction_type newtype);
  void append(ASTExpression* expr);
  void dumpCProgram(FILE *out);
  ASTExpression_YAMLProxy *toYAMLProxy();
  //static void register_type();
};


//////////////////////////////////////////////////////

typedef enum {NODE_UNSET = -1, NODE_BLOCK, NODE_IF, NODE_FOR, NODE_STMT} ASTNodeType;
typedef enum type {Stmt_unset = -1, Stmt_IPD, Stmt_OPD, Stmt_Function} Stmt_type;

class ASTNode;

// Hack/proxy class to deserialize using YAML:
class ASTNode_YAMLProxy: public structure {
  public:
    ASTNode_YAMLProxy();

    ASTNode *fromYAMLProxy();

    static serialize *create(void *user) { return new ASTNode_YAMLProxy(); }

    ASTNodeType nodetype;
    seq<ASTNode_YAMLProxy> stmts;

    ASTExpression_YAMLProxy *LHS;
    ASTExpression_YAMLProxy *RHS;
    int sign;
    ASTNode_YAMLProxy *then;

    str *iterator;
    ASTExpression_YAMLProxy *lb;
    ASTExpression_YAMLProxy *ub;
    int stride;
    ASTNode_YAMLProxy *body;

    str *name;
    Stmt_type type;

    //void dumpCProgram(FILE *out, int indent);
    static void register_type();
};


// Pure virtual / Abstract
class ASTNode {
protected:
  ASTNodeType nodetype;
private:
  ASTNode *parent;
public:
  ASTNode();
  ~ASTNode();
  ASTNodeType  getNodetype();
  void setNodetype(ASTNodeType newnodetype);
  virtual void dumpCProgram(FILE *out, int indent) =0;
  virtual ASTNode_YAMLProxy *toYAMLProxy() =0;
  //static void registerbase_ASTNode(struct_description &sd);
};

class ASTNode_Block: public ASTNode {
private:
  //seq<ASTNode> stmts;
  std::vector<ASTNode*> stmts;

  //static serialize *create(void *user) { printf("BLK\n"); return new ASTNode_Block(); }
public:
  ASTNode_Block();
  ~ASTNode_Block() {};
  void append(ASTNode* node);
  void dumpCProgram(FILE *out, int indent);
  ASTNode_YAMLProxy *toYAMLProxy();
  //static void register_type();
};


class ASTNode_If: public ASTNode {
private:
  ASTExpression *LHS;
  ASTExpression *RHS;
  int sign;
  ASTNode_Block *then;

  //static serialize *create(void *user) { printf("IF\n"); return new ASTNode_If(); }
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
  ASTNode_YAMLProxy *toYAMLProxy();
  //static void register_type();
};


class ASTNode_For: public ASTNode {
private:
  str *iterator;
  ASTExpression *lb;
  ASTExpression *ub;
  int stride;
  ASTNode_Block *body;

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

  ASTNode_YAMLProxy *toYAMLProxy();
  //static void register_type();
};



class ASTNode_Stmt: public ASTNode {
private:
  str *name;
  Stmt_type type;

  //static serialize *create(void *user) { return new ASTNode_Stmt(); }
public:
  ASTNode_Stmt();
  ~ASTNode_Stmt(){};
  str * getName();
  void setName(str *newname);
  Stmt_type getType();
  void setType(Stmt_type newtype);

  ASTNode_YAMLProxy *toYAMLProxy();
  void dumpCProgram(FILE *out, int indent);
  //static void register_type();
};


class AST: public structure {
public:
  ////////////////////////////////////////////////////////////////////////////
  //// Constructors/destructors
  AST();
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
  ASTNode_Block *realRoot;
  ASTNode_YAMLProxy *root;
  static serialize *create(void *user) { return new AST(); }
};


} // end namespace ppn
#endif /* AST_H_ */
