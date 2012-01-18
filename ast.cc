/*
 * AST.cc
 *
 *    Created on: Feb 2, 2011
 *      Author: Sven van Haastregt, Teddy Zhai
 *      $Id: ast.cc,v 1.12 2012/01/18 15:38:22 tzhai Exp $
 */

#include <limits>
#include "global.h"
#include "defs.h"

#include "ast.h"
#include "isa/yaml.h"

// General idea: we cannot handle inheritance hierarchies using YAML, because we cannot instantiate the corresponding derived
// class during deserialization. Therefore, we serialize/deserialize using YAML through a _YAMLProxy class. On serialization,
// the proper OOP-style (with inheritance) AST is first converted into this proxy. Then the proxy is Dump()ed. On
// deserialization, the proxy instances are converted back to the OOP-style classes.

using namespace ppn;

static at_init register_astexpr_yaml(ASTExpression_YAMLProxy::register_type);

void ASTExpression_YAMLProxy::register_type(){
  static struct_description sd = { create };

  static const char *type_names[5];
  type_names[EXPR_NAME] = "EXPR_NAME";
  type_names[EXPR_TERM] = "EXPR_TERM";
  type_names[EXPR_BINOP] = "EXPR_BINOP";
  type_names[EXPR_REDUCTION] = "EXPR_REDUCTION";
  YAML_ENUM_FIELD(sd, ASTExpression_YAMLProxy, type, type_names);

  YAML_PTR_FIELD(sd, ASTExpression_YAMLProxy, name, str);

  YAML_INT_FIELD(sd, ASTExpression_YAMLProxy, coeff);
  YAML_PTR_FIELD(sd, ASTExpression_YAMLProxy, var, ASTExpression_YAMLProxy);

  static const char *op_names[5];
  op_names[BINOP_MODULO] = "BINOP_MODULO";
  op_names[BINOP_DIV] = "BINOP_DIV";
  op_names[BINOP_FLOORDIV] = "BINOP_FLOORDIV";
  op_names[BINOP_CEILDIV] = "BINOP_CEILDIV";

  YAML_ENUM_FIELD(sd, ASTExpression_YAMLProxy, binopType, op_names);
  YAML_PTR_FIELD(sd, ASTExpression_YAMLProxy, LHS, ASTExpression_YAMLProxy);
  YAML_INT_FIELD(sd, ASTExpression_YAMLProxy, RHS);

  static const char *reduc_names[4];
  reduc_names[RED_SUM] = "RED_SUM";
  reduc_names[RED_MIN] = "RED_MIN";
  reduc_names[RED_MAX] = "RED_MAX";

  YAML_ENUM_FIELD(sd, ASTExpression_YAMLProxy, redType, reduc_names);
  YAML_SEQ_FIELD(sd, ASTExpression_YAMLProxy, elts, ASTExpression_YAMLProxy);

  structure::register_type("perl/ast_expr", &typeid(ASTExpression_YAMLProxy), &sd.d);
}

/*static at_init register_ast_name(ASTName::register_type);

void ASTName::register_type(){
  static struct_description ast_name = { create };
  YAML_PTR_FIELD(ast_name, ASTName, name, str);

  structure::register_type("perl/ast_name", &typeid(ASTName), &ast_name.d);
}


static at_init register_ast_term(ASTTerm::register_type);

void ASTTerm::register_type(){
  static struct_description ast_term = { create };
  YAML_INT_FIELD(ast_term, ASTTerm, coeff);
  YAML_PTR_FIELD(ast_term, ASTTerm, var, ASTExpression);

  structure::register_type("perl/ast_term", &typeid(ASTTerm), &ast_term.d);
}


static at_init register_ast_binop(ASTBinop::register_type);

void ASTBinop::register_type(){
  static struct_description ast_binop = { create };

  static const char *op_names[5];
  op_names[BINOP_MODULO] = "BINOP_MODULO";
  op_names[BINOP_DIV] = "BINOP_DIV";
  op_names[BINOP_FLOORDIV] = "BINOP_FLOORDIV";
  op_names[BINOP_CEILDIV] = "BINOP_CEILDIV";

  YAML_ENUM_FIELD(ast_binop, ASTBinop, type, op_names);
  YAML_PTR_FIELD(ast_binop, ASTBinop, LHS, ASTExpression);
  YAML_INT_FIELD(ast_binop, ASTBinop, RHS);

  structure::register_type("perl/ast_binop", &typeid(ASTBinop), &ast_binop.d);
}


static at_init register_ast_reduc(ASTReduction::register_type);

void ASTReduction::register_type(){
  static struct_description ast_reduc = { create };

  static const char *reduc_names[4];
  reduc_names[RED_SUM] = "RED_SUM";
  reduc_names[RED_MIN] = "RED_MIN";
  reduc_names[RED_MAX] = "RED_MAX";

  YAML_ENUM_FIELD(ast_reduc, ASTReduction, type, reduc_names);
  YAML_SEQ_FIELD(ast_reduc, ASTReduction, elts, ASTExpression);

  structure::register_type("perl/ast_reduc", &typeid(ASTReduction), &ast_reduc.d);
}
*/

/*
static at_init register_ast_node(ASTNode::register_type);

void ASTNode::registerbase_ASTNode(struct_description &sd) {
  static const char *nodetype_names[5];
  nodetype_names[NODE_BLOCK] = "NODE_BLOCK";
  nodetype_names[NODE_IF] = "NODE_IF";
  nodetype_names[NODE_FOR] = "NODE_FOR";
  nodetype_names[NODE_STMT] = "NODE_STMT";

  YAML_ENUM_FIELD(sd, ASTNode, nodetype, nodetype_names);
}

void ASTNode::register_type(){
  static struct_description ast_node_d = { create };

  registerbase_ASTNode(ast_node_d);

  structure::register_type("perl/ast_node", &typeid(ASTNode), &ast_node_d.d);
}

serialize *ASTNode::create(void *user) {
  return new ASTNode_Block();
}

static at_init register_ast_block(ASTNode_Block::register_type);

void ASTNode_Block::register_type(){
  static struct_description ast_block_d = { create };
  YAML_SEQ_FIELD(ast_block_d, ASTNode_Block, stmts, ASTNode);

  registerbase_ASTNode(ast_block_d);

  structure::register_type("perl/ast_block", &typeid(ASTNode_Block), &ast_block_d.d);
  //structure::register_type("perl/ast_node", &typeid(ASTNode), &ast_block_d.d);
}


static at_init register_ast_if(ASTNode_If::register_type);

void ASTNode_If::register_type(){
  static struct_description ast_if = { create };

  registerbase_ASTNode(ast_if);

  YAML_PTR_FIELD(ast_if, ASTNode_If, LHS, ASTExpression);
  YAML_PTR_FIELD(ast_if, ASTNode_If, RHS, ASTExpression);
  YAML_INT_FIELD(ast_if, ASTNode_If, sign);
  YAML_PTR_FIELD(ast_if, ASTNode_If, then, ASTNode_Block);

  structure::register_type("perl/ast_if", &typeid(ASTNode_If), &ast_if.d);
}


static at_init register_ast_for(ASTNode_For::register_type);

void ASTNode_For::register_type(){
  static struct_description ast_for = { create };

  registerbase_ASTNode(ast_for);

  YAML_PTR_FIELD(ast_for, ASTNode_For, iterator, str);
  YAML_PTR_FIELD(ast_for, ASTNode_For, lb, ASTExpression);
  YAML_PTR_FIELD(ast_for, ASTNode_For, ub, ASTExpression);
  YAML_INT_FIELD(ast_for, ASTNode_For, stride);
  YAML_PTR_FIELD(ast_for, ASTNode_For, body, ASTNode_Block);

  structure::register_type("perl/ast_for", &typeid(ASTNode_For), &ast_for.d);
}


static at_init register_ast_stmt(ASTNode_Stmt::register_type);

void ASTNode_Stmt::register_type(){
  static struct_description ast_stmt = { create };

  registerbase_ASTNode(ast_stmt);
  
  static const char *stmt_names[4];
  stmt_names[unset] = "unset";
  stmt_names[Stmt_IPD] = "Stmt_IPD";
  stmt_names[Stmt_OPD] = "Stmt_OPD";
  stmt_names[Stmt_Function] = "Stmt_Function";

  YAML_PTR_FIELD(ast_stmt, ASTNode_Stmt, name, str);
  YAML_ENUM_FIELD(ast_stmt, ASTNode_Stmt, type, stmt_names);

  structure::register_type("perl/ast_stmt", &typeid(ASTNode_Stmt), &ast_stmt.d);
}
*/

static at_init register_ast_yaml(ASTNode_YAMLProxy::register_type);

void ASTNode_YAMLProxy::register_type(){
  static struct_description sd = { create };

  static const char *nodetype_names[5];
  nodetype_names[NODE_BLOCK] = "NODE_BLOCK";
  nodetype_names[NODE_IF] = "NODE_IF";
  nodetype_names[NODE_FOR] = "NODE_FOR";
  nodetype_names[NODE_STMT] = "NODE_STMT";

  YAML_ENUM_FIELD(sd, ASTNode_YAMLProxy, nodetype, nodetype_names);

  YAML_SEQ_FIELD(sd, ASTNode_YAMLProxy, stmts, ASTNode_YAMLProxy);

  YAML_PTR_FIELD(sd, ASTNode_YAMLProxy, LHS, ASTExpression_YAMLProxy);
  YAML_PTR_FIELD(sd, ASTNode_YAMLProxy, RHS, ASTExpression_YAMLProxy);
  YAML_INT_FIELD(sd, ASTNode_YAMLProxy, sign);
  YAML_PTR_FIELD(sd, ASTNode_YAMLProxy, then, ASTNode_YAMLProxy);
  
  YAML_PTR_FIELD(sd, ASTNode_YAMLProxy, iterator, str);
  YAML_PTR_FIELD(sd, ASTNode_YAMLProxy, lb, ASTExpression_YAMLProxy);
  YAML_PTR_FIELD(sd, ASTNode_YAMLProxy, ub, ASTExpression_YAMLProxy);
  YAML_INT_FIELD(sd, ASTNode_YAMLProxy, stride);
  YAML_PTR_FIELD(sd, ASTNode_YAMLProxy, body, ASTNode_YAMLProxy);

  static const char *stmt_names[4];
  stmt_names[Stmt_unset] = "Stmt_unset";
  stmt_names[Stmt_IPD] = "Stmt_IPD";
  stmt_names[Stmt_OPD] = "Stmt_OPD";
  stmt_names[Stmt_Function] = "Stmt_Function";

  YAML_PTR_FIELD(sd, ASTNode_YAMLProxy, name, str);
  YAML_ENUM_FIELD(sd, ASTNode_YAMLProxy, type, stmt_names);

  structure::register_type("perl/ast_nodeyaml", &typeid(ASTNode_YAMLProxy), &sd.d);
}


static at_init register_ast(AST::register_type);

void
AST::register_type(){
  static struct_description ast_d = { create };
  YAML_PTR_FIELD(ast_d, AST, root, ASTNode_YAMLProxy);

  structure::register_type("perl/AST", &typeid(AST), &ast_d.d);
}

void
AST::dump(emitter& e)
{
  root = realRoot->toYAMLProxy();
  yll_emitter_set_transfer(e.e, "perl/AST");
  structure::dump(e);
}




// End of YAML stuff
//////////////////////////////////////////////////////////
// Begin of actual class implementations

const int c_indent = 2;


///
ASTExpression_YAMLProxy::ASTExpression_YAMLProxy() {
  type = EXPR_UNSET;
  name = NULL;
  
  coeff = 0;
  var = NULL;

  binopType = BINOP_UNSET;
  LHS = NULL;
  RHS = 0;

  redType = RED_UNSET;
}

ASTExpression *ASTExpression_YAMLProxy::fromYAMLProxy() {
  ASTName *pName;
  ASTTerm *pTerm;
  ASTBinop *pBinop;
  ASTReduction *pRed;
  int i;
  switch (this->type) {
    case EXPR_NAME:
      pName = new ASTName;
      pName->setName(this->name);
      return pName;
    case EXPR_TERM:
      pTerm = new ASTTerm;
      pTerm->setVar(this->var ? this->var->fromYAMLProxy() : NULL);
      pTerm->setCoeff(this->coeff);
      return pTerm;
    case EXPR_BINOP:
      pBinop = new ASTBinop;
      pBinop->setType(this->binopType);
      pBinop->setLHS(this->LHS->fromYAMLProxy());
      pBinop->setRHS(this->RHS);
      return pBinop;
    case EXPR_REDUCTION:
      pRed = new ASTReduction;
      pRed->setType(this->redType);
      for (unsigned int i = 0; i < this->elts.size(); ++i) {
        pRed->append(this->elts[i]->fromYAMLProxy());
      }
      return pRed;
    default:
      assert(0); // Unknown type
  }
}



///
// If not overridden, assume not constant
bool ASTExpression::isConstant() {
  return false;
}



///
ASTExpression_YAMLProxy *ASTName::toYAMLProxy() {
  ASTExpression_YAMLProxy *proxy = new ASTExpression_YAMLProxy;
  proxy->type = EXPR_NAME;
  proxy->name = this->getName();
  return proxy;
}

str * ASTName::getName() {
  return this->name;
}
void ASTName::setName(str * newname) {
  this->name = newname;
}

void ASTName::dumpCProgram(FILE *out) {
  fprintf(out, "%s", this->getName()->s.c_str());
}


///
ASTExpression_YAMLProxy *ASTTerm::toYAMLProxy() {
  ASTExpression_YAMLProxy *proxy = new ASTExpression_YAMLProxy;
  proxy->type = EXPR_TERM;
  proxy->coeff = this->getCoeff();
  proxy->var = this->getVar() ? this->getVar()->toYAMLProxy() : NULL;
  return proxy;
}

int  ASTTerm::getCoeff() {
  return this->coeff;
}
void ASTTerm::setCoeff(int  newcoeff) {
  this->coeff = newcoeff;
}


ASTExpression * ASTTerm::getVar() {
  return this->var;
}
void ASTTerm::setVar(ASTExpression * newvar) {
  this->var = newvar;
}
void ASTTerm::dumpCProgram(FILE *out) {
  fprintf(out, "%d", this->getCoeff());
  if (this->getVar()) {
    fprintf(out, "*(");
    this->getVar()->dumpCProgram(out);
    fprintf(out, ")");
  }
}

bool ASTTerm::isConstant() {
  return (this->getVar() == NULL);
}


///
ASTExpression_YAMLProxy *ASTBinop::toYAMLProxy() {
  ASTExpression_YAMLProxy *proxy = new ASTExpression_YAMLProxy;
  proxy->type = EXPR_BINOP;
  proxy->binopType = this->getType();
  proxy->LHS = this->getLHS()->toYAMLProxy();
  proxy->RHS = this->getRHS();
  return proxy;
}

ASTBinop::ASTBinop() {
  this->type = BINOP_UNSET;
  this->LHS = NULL;
  this->RHS = NULL;
}

ASTBinop_type ASTBinop::getType() {
  return this->type;
}
void ASTBinop::setType(ASTBinop_type newtype) {
  this->type = newtype;
}


ASTExpression *ASTBinop::getLHS() {
  return this->LHS;
}
void ASTBinop::setLHS(ASTExpression * newLHS) {
  this->LHS = newLHS;
}


int ASTBinop::getRHS() {
  return this->RHS;
}
void ASTBinop::setRHS(int newRHS) {
  this->RHS = newRHS;
}

void ASTBinop::dumpCProgram(FILE *out) {
  switch (this->type) {
    case BINOP_MODULO:
      fprintf(out, "(");
      this->LHS->dumpCProgram(out);
      fprintf(out, ")%%%d", this->RHS);
      break;
    case BINOP_DIV:
      fprintf(out, "(");
      this->LHS->dumpCProgram(out);
      fprintf(out, ")/%d", this->RHS);
      break;
    case BINOP_FLOORDIV:
      fprintf(out, "floor((");
      this->LHS->dumpCProgram(out);
      fprintf(out, ")/%d)", this->RHS);
      break;
    case BINOP_CEILDIV:
      fprintf(out, "ceil((");
      this->LHS->dumpCProgram(out);
      fprintf(out, ")/%d)", this->RHS);
      break;
    default:
      assert(0);
  }
}


///
ASTExpression_YAMLProxy *ASTReduction::toYAMLProxy() {
  ASTExpression_YAMLProxy *proxy = new ASTExpression_YAMLProxy;
  proxy->type = EXPR_REDUCTION;
  proxy->redType = this->getType();
  for (unsigned int i = 0; i < this->elts.size(); ++i) {
    ASTExpression_YAMLProxy *expr = elts[i]->toYAMLProxy();
    proxy->elts.push_back(expr);
  }
  return proxy;
}

ASTReduction::ASTReduction() {
  this->type = RED_UNSET;
}

ASTReduction_type ASTReduction::getType() {
  return this->type;
}
void ASTReduction::setType(ASTReduction_type newtype) {
  this->type = newtype;
}

void ASTReduction::append(ASTExpression* expr) {
  elts.push_back(expr);
}

void ASTReduction::dumpCProgram(FILE *out) {
  assert(this->type == RED_SUM); // only sums are currently supported
  this->elts[0]->dumpCProgram(out);
  for (unsigned int i = 1; i < this->elts.size(); ++i) {
    fprintf(out, "+");
    this->elts[i]->dumpCProgram(out);
  }
}



///
ASTNode_YAMLProxy::ASTNode_YAMLProxy() {
  nodetype = NODE_UNSET;
  LHS = NULL;
  RHS = NULL;
  sign = 0;
  then = NULL;

  iterator = NULL;
  lb = NULL;
  ub = NULL;
  stride = 0;
  body = NULL;

  name = NULL;
  type = Stmt_unset;
}

ASTNode *ASTNode_YAMLProxy::fromYAMLProxy() {
  ASTNode_Block *pBlock;
  ASTNode_If *pIf;
  ASTNode_For *pFor;
  ASTNode_Stmt *pStmt;
  switch (this->nodetype) {
    case NODE_BLOCK:
      pBlock = new ASTNode_Block;
      for (unsigned int i = 0; i < stmts.size(); i++) {
        pBlock->append(stmts[i]->fromYAMLProxy());
      }
      return pBlock;
    case NODE_IF:
      pIf = new ASTNode_If;
      pIf->setLHS(this->LHS->fromYAMLProxy());
      pIf->setRHS(this->RHS->fromYAMLProxy());
      pIf->setSign(this->sign);
      pIf->setThen((ASTNode_Block*)this->then->fromYAMLProxy());
      return pIf;
    case NODE_FOR:
      pFor = new ASTNode_For;
      pFor->setIterator(this->iterator);
      pFor->setLb(this->lb->fromYAMLProxy());
      pFor->setUb(this->ub->fromYAMLProxy());
      pFor->setStride(this->stride);
      pFor->setBody((ASTNode_Block*)this->body->fromYAMLProxy());
      return pFor;
    case NODE_STMT:
      pStmt = new ASTNode_Stmt;
      pStmt->setName(this->name);
      pStmt->setType(this->type);
      return pStmt;
    default:
      assert(0); // unknown node type
  }
}


///
ASTNode::ASTNode() {
  parent = NULL;
}

ASTNode::~ASTNode() {
}

ASTNodeType  ASTNode::getNodetype() {
  return this->nodetype;
}
void ASTNode::setNodetype(ASTNodeType  newnodetype) {
  this->nodetype = newnodetype;
}

ASTNode *ASTNode::getParent() {
  return parent;
}
void *ASTNode::setParent(ASTNode *newParent) {
  this->parent = newParent;
}


///
ASTNode_Block::ASTNode_Block() {
  this->setNodetype(NODE_BLOCK);
}

void ASTNode_Block::append(ASTNode* node) {
  stmts.push_back(node);
}

void ASTNode_Block::dumpCProgram(FILE *out, int indent) {
  if (indent > 0)
    fprintf(out, "{\n");
  indent+=c_indent;

  if (this->getParent() && this->getParent()->getNodetype() == NODE_FOR && this->stmts[0]->getNodetype() != NODE_FOR) {
    // This is the deepest for
    fprintf(out, "%*sprintf(\"%s\\n\");\n", indent, " ", STR_CPROG_NEXTITER);
  }

  for (int i = 0; i < stmts.size(); i++) {
    stmts[i]->dumpCProgram(out, indent);
  }

  indent-=c_indent;
  if (indent > 0)
    fprintf(out, "%*s}\n", indent, " ");
}

ASTNode_YAMLProxy *ASTNode_Block::toYAMLProxy() {
  ASTNode_YAMLProxy *proxy = new ASTNode_YAMLProxy;
  proxy->nodetype = NODE_BLOCK;
  for (unsigned int i = 0; i < stmts.size(); i++) {
    proxy->stmts.push_back(stmts[i]->toYAMLProxy());
  }
  return proxy;
}


///
ASTNode_If::ASTNode_If() {
  this->LHS = NULL;
  this->RHS = NULL;
  this->then = NULL;
  this->sign = 99;
  setNodetype(NODE_IF);
}

ASTExpression * ASTNode_If::getLHS() {                             
  return this->LHS;
}
void ASTNode_If::setLHS(ASTExpression * newLHS) {
  this->LHS = newLHS;
}


ASTExpression * ASTNode_If::getRHS() {
  return this->RHS;
}
void ASTNode_If::setRHS(ASTExpression * newRHS) {
  this->RHS = newRHS;
}


int  ASTNode_If::getSign() {
  return this->sign;
}
void ASTNode_If::setSign(int  newsign) {
  this->sign = newsign;
}


ASTNode_Block * ASTNode_If::getThen() {
  return this->then;
}
void ASTNode_If::setThen(ASTNode_Block * newthen) {
  this->then = newthen;
  newthen->setParent(this);
}

void ASTNode_If::dumpCProgram(FILE *out, int indent) {
  fprintf(out, "%*sif (", indent, " ");
  this->getLHS()->dumpCProgram(out);
  fprintf(out, " %s ", this->getSign()==0 ? "==" : this->getSign()==1 ? ">=" : "<=");
  this->getRHS()->dumpCProgram(out);
  fprintf(out, ") ");
  //fprintf(out, "/*LHS:%d  RHS:%d*/", this->getLHS()->isConstant(), this->getRHS()->isConstant());
  this->getThen()->dumpCProgram(out, indent);
}

ASTNode_YAMLProxy *ASTNode_If::toYAMLProxy() {
  ASTNode_YAMLProxy *proxy = new ASTNode_YAMLProxy;
  proxy->nodetype = NODE_IF;
  proxy->sign = this->getSign();
  proxy->LHS = this->getLHS()->toYAMLProxy();
  proxy->RHS = this->getRHS()->toYAMLProxy();
  proxy->then = this->getThen()->toYAMLProxy();
  return proxy;
}


///
ASTNode_For::ASTNode_For() {
  iterator = new str("undef");
  stride = -99999;
  lb = NULL;
  ub = NULL;
  body = NULL;
  setNodetype(NODE_FOR);
}

str * ASTNode_For::getIterator() {                                  
  return this->iterator;                                            
}                                                                   
void ASTNode_For::setIterator(str * newiterator) {                  
  this->iterator = newiterator;                                     
}                                                                   


ASTExpression * ASTNode_For::getLb() {
  return this->lb;
}
void ASTNode_For::setLb(ASTExpression * newlb) {
  this->lb = newlb;
}


ASTExpression * ASTNode_For::getUb() {
  return this->ub;
}
void ASTNode_For::setUb(ASTExpression * newub) {
  this->ub = newub;
}


int  ASTNode_For::getStride() {
  return this->stride;
}
void ASTNode_For::setStride(int  newstride) {
  this->stride = newstride;
}


ASTNode_Block * ASTNode_For::getBody() {
  return this->body;
}
void ASTNode_For::setBody(ASTNode_Block * newbody) {
  this->body = newbody;
  newbody->setParent(this);
}

void ASTNode_For::dumpCProgram(FILE *out, int indent) {
  const char *iter = this->getIterator()->s.c_str();
  fprintf(out, "%*sfor (%s = ", indent, " ", iter);
  this->getLb()->dumpCProgram(out);
  fprintf(out, "; %s <= ", iter);
  this->getUb()->dumpCProgram(out);
  fprintf(out, "; %s += %d) ", iter, this->getStride());
  //fprintf(out, "/*LB:%d  UB:%d*/", this->getLb()->isConstant(), this->getUb()->isConstant());
  this->getBody()->dumpCProgram(out, indent);
}

ASTNode_YAMLProxy *ASTNode_For::toYAMLProxy() {
  ASTNode_YAMLProxy *proxy = new ASTNode_YAMLProxy();
  proxy->nodetype = NODE_FOR;
  proxy->iterator = this->getIterator();
  proxy->stride = this->getStride();
  proxy->lb = this->getLb()->toYAMLProxy();
  proxy->ub = this->getUb()->toYAMLProxy();
  proxy->body = this->getBody()->toYAMLProxy();
  return proxy;
}


///
ASTNode_Stmt::ASTNode_Stmt() {
  type = Stmt_unset;
  setNodetype(NODE_STMT);
}

str * ASTNode_Stmt::getName() {
  return this->name;
}
void ASTNode_Stmt::setName(str * newname) {
  this->name = newname;
}

Stmt_type ASTNode_Stmt::getType() {
  return this->type;
}
void ASTNode_Stmt::setType(Stmt_type newtype) {
  this->type = newtype;
}

void ASTNode_Stmt::dumpCProgram(FILE *out, int indent) {
  fprintf(out, "%*sprintf(\"%s\\n\");\n", indent, " ", this->getName()->s.c_str());
}

ASTNode_YAMLProxy *ASTNode_Stmt::toYAMLProxy() {
  ASTNode_YAMLProxy *proxy = new ASTNode_YAMLProxy;
  proxy->nodetype = NODE_STMT;
  proxy->name = this->getName();
  return proxy;
}



///
AST::AST() {
  this->realRoot = NULL;
  this->root = NULL;
}

ASTNode_Block * AST::getRoot() {
  if (this->root) {
    this->realRoot = (ASTNode_Block*)this->root->fromYAMLProxy();
    this->root = NULL;
  }
  return this->realRoot;
}


void AST::setRoot(ASTNode_Block * newroot) {
  this->realRoot = newroot;
  this->root = NULL;
}


void AST::dumpCProgram(FILE *out) {
  fprintf(out, "#include <stdio.h>\n");
  fprintf(out, "\n");
  fprintf(out, "int main() {\n");
  fprintf(out, "  int c0, c1, c2, c3, c4, c5, c6, c7, c8, c9;\n"); // Should be enough...
  fprintf(out, "\n");
  getRoot()->dumpCProgram(out, 0);
  fprintf(out, "\n");
  fprintf(out, "  printf(\"%s\\n\");\n", STR_CPROG_FINISHED);
  fprintf(out, "  return 0;\n");
  fprintf(out, "}\n");
}

