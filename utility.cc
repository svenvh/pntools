/*
 * 		utility.cc
 *
 *  	Created on: Jan 4, 2011
 *      Author: Sven van Haastregt, Teddy Zhai
 *
 */


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
    
    isl_set *ran_set = isl_map_range(isl_map_copy(unwrapped_set));
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
    isl_set_free(ran_set);
    
    dom_set = isl_map_domain(unwrapped_set);
    
    return dom_set;
}


__isl_give isl_set* getPDGDomain(adg_domain *adgDomain){
	isl_set *pdgDomain = isl_set_copy(adgDomain->bounds);

	// project out control variables
	unsigned int nrDim = isl_set_dim(adgDomain->bounds, isl_dim_set);
	unsigned int nrCtrlVar = adgDomain->controls.size();
	if (nrCtrlVar > 0) {
		pdgDomain = isl_set_project_out(pdgDomain, isl_dim_set, nrDim - nrCtrlVar, nrCtrlVar);
	}

	// eliminate nested spaces (get domain of unwrapped map)
	pdgDomain = getUnwrappedDomain(pdgDomain);

	return pdgDomain;
}


bool
isDimsEqual(__isl_keep isl_set *set1, __isl_keep isl_set *set2,
		unsigned int first, unsigned int n)
{
	bool isFullyDefined = false;
	assert(isl_set_dim(set1, isl_dim_set) == isl_set_dim(set2, isl_dim_set));

	isl_set *firstnSet1 = isl_set_project_out(set1, isl_dim_set, first, n);

	isl_set *firstnSet2 = isl_set_project_out(set1, isl_dim_set, first, n);

	if (isl_set_is_equal(firstnSet1, firstnSet2)) {
		isFullyDefined = true;
	}

	return isFullyDefined;
}
