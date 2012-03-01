/*
 * Copyright (c) 2012 Leiden University (LERC group at LIACS).
 * All rights reserved.
 *
 * 		utility.cc
 *
 *  	Created on: Jan 4, 2011
 *      Author: Sven van Haastregt, Teddy Zhai
 *
 */

#include <cstring>
#include "barvinok/barvinok.h"

#include "isa/yaml.h"
#include "isa/pdg.h"

#include "defs.h"
#include "ADG_helper.h"
#include "utility.h"

using pdg::PDG;
using namespace std;


/* Helper function (callback) for getCardinality
 * This function is mainly used for non-parameterized case,
 * in which only a single piece and a constant are desired
 * */
int countCard(isl_set *set, isl_qpolynomial *qp, void *user) {
  int *count = (int*)user;
  isl_int n, d;
  isl_int_init(n);
  isl_int_init(d);

  /* The resulting pw_qpolynomial should consist of only a single piece,
   * therefore, it should equal to initial vaule -1
   */
  assert(*count == -1);

  if (isl_qpolynomial_is_cst(qp, &n, &d) == 1) {
    if (isl_int_get_si(d) != 1) {
      fprintf(stderr, "Warning: Set cardinality seems fractional!\n");
    }
    *count = isl_int_get_si(n);
  }
  else {
    fprintf(stderr, "Warning: At least one qpolynomial is not constant!\n");
  }
  isl_int_clear(n);
  isl_int_clear(d);
  isl_qpolynomial_free(qp);
  isl_set_free(set);
  return 0;
}


// Count number of points in UnionSet s
int getCardinality(isl_ctx *ctx, pdg::UnionSet *s) {
  int count = -1;
  isl_pw_qpolynomial *pwqp = isl_set_card(s->get_isl_set(ctx));
  isl_pw_qpolynomial_foreach_piece(pwqp, &countCard, &count);
  isl_pw_qpolynomial_free(pwqp);
  return count;
}

int getCardinality(__isl_keep isl_pw_qpolynomial *pwqp){
	int count = -1;

	isl_pw_qpolynomial_foreach_piece(pwqp, &countCard, &count);

	assert(count >= 0);
	return count;
}


__isl_give isl_set*
getUnwrappedDomain(__isl_take isl_set *set){
	isl_set *dom_set = NULL;
//	std::cout << "set:";
//	isl_set_print(set, stdout, 0, ISL_FORMAT_ISL);
//	std::cout << std::endl;
	if (isl_set_is_wrapping(set) == -1) {
		fprintf(stderr, "ERROR: The unwrapped domain cannot be found.\n");
	} else if (isl_set_is_wrapping(set) == 0) {
		// the set is not nested
		dom_set = isl_set_copy(set);
		isl_set_free(set);
		return dom_set;
	}
    assert(isl_set_is_wrapping(set) == 1);
    
    isl_map *unwrapped_set = isl_set_unwrap(set);
    
//    isl_set *ran_set = isl_map_range(isl_map_copy(unwrapped_set));
    /*std::cout << "the range of unwrapped set: ";
    printer = isl_printer_print_set(printer, ran_set);
    printer = isl_printer_end_line(printer);
    std::cout << "dim set: " << isl_set_dim(ran_set, isl_dim_set) << std::endl;
    std::cout << "dim cst: " << isl_set_dim(ran_set, isl_dim_cst) << std::endl;
    std::cout << "dim param:" << isl_set_dim(ran_set, isl_dim_param) << std::endl;
    std::cout << "dim in:" << isl_set_dim(ran_set, isl_dim_in) << std::endl;*/
//    assert(isl_set_dim(ran_set, isl_dim_set) == 0);
//    if (isl_set_dim(ran_set, isl_dim_set) != 0) {
//		fprintf(stderr, "WARNING: the local space of the set have dimensions. "
//		"Getting unwrapped domain from the set might cause problems\n");
//    }
//    isl_set_free(ran_set);
    
    dom_set = isl_map_domain(unwrapped_set);
    while(isl_set_is_wrapping(dom_set)){
    	unwrapped_set = isl_set_unwrap(dom_set);
    	dom_set = isl_map_domain(unwrapped_set);
    }
//    std::cout << "dom set:";
//    isl_set_print(dom_set, stdout, 0, ISL_FORMAT_ISL);
//	std::cout << std::endl;
    assert(isl_set_is_wrapping(dom_set) == 0);
    assert(isl_set_is_empty(dom_set) == 0);
    
    return dom_set;
}


__isl_give isl_set* getPDGDomain(const adg_domain *adgDomain){
	isl_set *pdgDomain = isl_set_copy(adgDomain->bounds);

	// if the bounds is wrapped, we need to unwrap it first to retrieve the id
	isl_id *name = NULL;
	if (isl_set_is_wrapping(pdgDomain)) {
		isl_set *dom = getUnwrappedDomain(isl_set_copy(adgDomain->bounds));
		name = isl_set_get_tuple_id(dom);
		isl_set_free(dom);
	} else {
		name = isl_set_get_tuple_id(pdgDomain);
	}
	assert(name != NULL);

	// project out control variables
	unsigned int nrDim = isl_set_dim(adgDomain->bounds, isl_dim_set);
	unsigned int nrCtrlVar = adgDomain->controls.size();
//	std::cout << "nr. control: " << nrCtrlVar << std::endl;
	if (nrCtrlVar > 0) {
		pdgDomain = isl_set_project_out(pdgDomain, isl_dim_set, nrDim - nrCtrlVar, nrCtrlVar);
	}

	// eliminate nested spaces (get domain of unwrapped map)
//	std::cout << "pdgDomain: ";
//	isl_set_print(pdgDomain, stdout, 0, ISL_FORMAT_ISL);
//	std::cout << std::endl;
	pdgDomain = getUnwrappedDomain(pdgDomain);
	pdgDomain = isl_set_set_tuple_id(pdgDomain, name);

	return pdgDomain;
}


bool
isDimsEqual(__isl_keep isl_set *set1, __isl_keep isl_set *set2,
		unsigned int first, unsigned int n)
{
	bool isFullyDefined = false;
	isl_set *firstnSet1 = isl_set_copy(set1);
	isl_set *firstnSet2 = isl_set_copy(set2);
	unsigned int nrDimsSet1 = isl_set_dim(set1, isl_dim_set);
	unsigned int nrDimsSet2 = isl_set_dim(set2, isl_dim_set);
//	assert(nrDimsSet1 == nrDimsSet2);

	if (first + n > nrDimsSet1) {
		fprintf(stderr, "ERROR: n dimensions starting from position \"first\" cannot be retrieved. \n");
		assert(first + n <= nrDimsSet1);
	}

	// retrieve "n" dimensions starting from "first" from set1
	// 1. project out dimensions from 0 to first -1
	if (first > 0) {
		firstnSet1 = isl_set_project_out(firstnSet1, isl_dim_set, 0, first);
	}
	// 2. project out dimensions from "first + n" to nrDimsSet1
	if (first + n < nrDimsSet1) {
		firstnSet1 = isl_set_project_out(firstnSet1, isl_dim_set, first + n, nrDimsSet1 - first - n - first);
	}

	// retrieve "n" dimensions starting from "first" from set2

	// 1. project out dimensions from 0 to first -1
	if (first > 0) {
		firstnSet2 = isl_set_project_out(firstnSet2, isl_dim_set, 0, first);
	}
	// 2. project out dimensions from "first + n" to nrDimsSet1
	if (first + n < nrDimsSet1) {
		firstnSet2 = isl_set_project_out(firstnSet2, isl_dim_set, first + n, nrDimsSet2 - first - n - first);
	}

	if (isl_set_is_equal(firstnSet1, firstnSet2)) {
		isFullyDefined = true;
	}

	isl_set_free(firstnSet1);
	isl_set_free(firstnSet2);
	return isFullyDefined;
}

int isl_id_cmp(__isl_keep isl_id *s1, __isl_keep isl_id *s2) {
	return strcmp(isl_id_get_name(s1), isl_id_get_name(s2));
}
