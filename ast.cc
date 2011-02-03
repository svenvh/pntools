/*
 * AST.cc
 *
 *  	Created on: Feb 2, 2011
 *      Author: Teddy Zhai
 *      $Id: ast.cc,v 1.1 2011/02/03 14:36:16 svhaastr Exp $
 */

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

	structure::register_type("perl/ast_block", &typeid(AST), &ast_block_d.d);
}


static at_init register_ast_if(ASTNode_If::register_type);

void ASTNode_If::register_type(){
	static struct_description ast_if = { create };

	YAML_PTR_FIELD(ast_if, ASTNode_If, LHS, ASTExpression);
	YAML_PTR_FIELD(ast_if, ASTNode_If, RHS, ASTExpression);
	YAML_INT_FIELD(ast_if, ASTNode_If, sign);
	YAML_PTR_FIELD(ast_if, ASTNode_If, then, ASTNode_Block);

	structure::register_type("perl/ast_if", &typeid(ASTNode_If), &ast_if.d);
}


static at_init register_ast_for(ASTNode_For::register_type);

void ASTNode_For::register_type(){
	static struct_description ast_for = { create };

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

	static const char *stmt_names[ASTNode_Stmt::Stmt_exec];
	stmt_names[Stmt_IPD] = "Stmt_IPD";
	stmt_names[Stmt_OPD] = "Stmt_OPD";

	YAML_PTR_FIELD(ast_stmt, ASTNode_Stmt, name, str);
	YAML_ENUM_FIELD(ast_stmt, ASTNode_Stmt, type, stmt_names);

	structure::register_type("perl/ast_stmt", &typeid(ASTNode_Stmt), &ast_stmt.d);
}

AST*
AST::Load(char *str, void *user)
{
    return yaml::Load<AST>(str, user);
}

AST*
AST::Load(FILE* fp, void* user){
	 return yaml::Load<AST>(fp, user);
}

static at_init register_ast(AST::register_type);

void
AST::register_type(){
	static struct_description ast_d = { create };
	YAML_PTR_FIELD(ast_d, AST, root, ASTNode_Block);

	structure::register_type("perl/ast", &typeid(AST), &ast_d.d);
}

void
AST::dump(emitter& e)
{
    yll_emitter_set_transfer(e.e, "perl/AST");
    structure::dump(e);
}






