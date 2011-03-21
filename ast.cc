/*
 * AST.cc
 *
 *  	Created on: Feb 2, 2011
 *      Author: Teddy Zhai
 *      $Id: ast.cc,v 1.4 2011/03/21 15:48:33 svhaastr Exp $
 */

#include <limits>
#include "global.h"

#include "ast.h"
#include "yaml.h"

using namespace ppn;


static at_init register_ast_name(ASTName::register_type);

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

	static const char *op_names[ASTBinop::OP_CEILDIV];
	op_names[OP_MODULO] = "OP_MODULO";
	op_names[OP_DIV] = "OP_DIV";
	op_names[OP_FLOORDIV] = "OP_FLOORDIV";

	YAML_ENUM_FIELD(ast_binop, ASTBinop, type, op_names);
	YAML_PTR_FIELD(ast_binop, ASTBinop, LHS, ASTTerm);
	YAML_PTR_FIELD(ast_binop, ASTBinop, RHS, ASTTerm);

	structure::register_type("perl/ast_binop", &typeid(ASTBinop), &ast_binop.d);
}


static at_init register_ast_reduc(ASTReduction::register_type);

void ASTReduction::register_type(){
	static struct_description ast_reduc = { create };

	static const char *reduc_names[ASTReduction::RED_MAX];
	reduc_names[RED_SUM] = "RED_SUM";
	reduc_names[RED_MIN] = "RED_MIN";

	YAML_ENUM_FIELD(ast_reduc, ASTReduction, type, reduc_names);
	YAML_INT_FIELD(ast_reduc, ASTReduction, n);
	YAML_SEQ_FIELD(ast_reduc, ASTReduction, elts, ASTExpression);

	structure::register_type("perl/ast_reduc", &typeid(ASTReduction), &ast_reduc.d);
}


static at_init register_ast_block(ASTNode_Block::register_type);

void ASTNode_Block::register_type(){
	static struct_description ast_block_d = { create };
	YAML_SEQ_FIELD(ast_block_d, ASTNode_Block, stmts, ASTNode);

	structure::register_type("perl/ast_block", &typeid(ASTNode_Block), &ast_block_d.d);
}


static at_init register_ast_if(ASTNode_If::register_type);

void ASTNode_If::register_type(){
	static struct_description ast_if = { create };

  //TODO: reenable all fields
	//YAML_PTR_FIELD(ast_if, ASTNode_If, LHS, ASTExpression);
	//YAML_PTR_FIELD(ast_if, ASTNode_If, RHS, ASTExpression);
	YAML_INT_FIELD(ast_if, ASTNode_If, sign);
	YAML_PTR_FIELD(ast_if, ASTNode_If, then, ASTNode_Block);

	structure::register_type("perl/ast_if", &typeid(ASTNode_If), &ast_if.d);
}


static at_init register_ast_for(ASTNode_For::register_type);

void ASTNode_For::register_type(){
	static struct_description ast_for = { create };

  //TODO: reenable all fields
	YAML_PTR_FIELD(ast_for, ASTNode_For, iterator, str);
	//YAML_PTR_FIELD(ast_for, ASTNode_For, lb, ASTExpression);
	//YAML_PTR_FIELD(ast_for, ASTNode_For, ub, ASTExpression);
	YAML_INT_FIELD(ast_for, ASTNode_For, stride);
	YAML_PTR_FIELD(ast_for, ASTNode_For, body, ASTNode_Block);

	structure::register_type("perl/ast_for", &typeid(ASTNode_For), &ast_for.d);
}


static at_init register_ast_stmt(ASTNode_Stmt::register_type);

void ASTNode_Stmt::register_type(){
	static struct_description ast_stmt = { create };

	static const char *stmt_names[4];
	stmt_names[unset] = "unset";
	stmt_names[Stmt_IPD] = "Stmt_IPD";
	stmt_names[Stmt_OPD] = "Stmt_OPD";
	stmt_names[Stmt_Function] = "Stmt_Function";

	YAML_PTR_FIELD(ast_stmt, ASTNode_Stmt, name, str);
	YAML_ENUM_FIELD(ast_stmt, ASTNode_Stmt, type, stmt_names);

	structure::register_type("perl/ast_stmt", &typeid(ASTNode_Stmt), &ast_stmt.d);
}
/*
AST*
AST::Load(char *str, void *user)
{
    return yaml::Load<AST>(str, user);
}

AST*
AST::Load(FILE* fp, void* user){
	 return yaml::Load<AST>(fp, user);
}
*/
static at_init register_ast(AST::register_type);

void
AST::register_type(){
	static struct_description ast_d = { create };
	YAML_PTR_FIELD(ast_d, AST, root, ASTNode_Block);

	structure::register_type("perl/AST", &typeid(AST), &ast_d.d);
}

void
AST::dump(emitter& e)
{
    yll_emitter_set_transfer(e.e, "perl/AST");
    structure::dump(e);
}




// End of YAML stuff
//////////////////////////////////////////////////////////
// Begin of actual class implementations

ASTNode::ASTNode() {
  parent = NULL;
}

ASTNode::~ASTNode() {
}

void ASTNode_Block::append(ASTNode* node) {
  stmts.v.push_back(node);
}


///
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
}


///
ASTNode_For::ASTNode_For() {
  iterator = new str("undef");
  stride = -99999;
  lb = NULL;
  ub = NULL;
  body = NULL;
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
}


///
ASTNode_Stmt::ASTNode_Stmt() {
  type = unset;
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




///
ASTNode_Block * AST::getRoot() {
  return this->root;
}


void AST::setRoot(ASTNode_Block * newroot) {
  this->root = newroot;
}


