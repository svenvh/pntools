/*
 * pn2ppn_util.h
 * pn2ppn utilities are kept in this file to reduce the dependence of the PPN class on external libs.
 *
 *    Created on: Sep 30, 2010
 *      Author: Teddy Zhai, Sven van Haastregt
 *      $Id: pn2ppn_util.h,v 1.2 2011/02/07 09:55:34 svhaastr Exp $
 *
 */

ppn::AST *cloog_clast_to_AST(CloogInput *input, int dim, CloogOptions *options);
