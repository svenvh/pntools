/*		Copyright (c) 2012 Leiden University (LERC group at LIACS).
 * 		All rights reserved.
 *
 *		pdgcharact.cc
 *
 *  	Created on: Apr 10, 2012
 *      Author: Teddy Zhai
 *		
 *		History:  Apr 10, 2012 Initial version
 */

#include "isa/pdg.h"
#include "PDG_helper.h"

#define PDG_CHARACTERIZATION_DEUBG

using pdg::PDG;
using namespace pdg_helper;

int main(int argc, char * argv[])
{
	FILE *in = stdin;

	isl_ctx *ctx;
	ctx = isl_ctx_alloc();

	// pdg
	PDG *pdg = PDG::Load(in, ctx);
	assert(pdg);

	pdg_helper::PDG_helper *pdgHelper = new pdg_helper::PDG_helper(ctx, pdg);

	pNodes_t nodes = pdgHelper->getNodes();
	std::cout << "nr. of nodes: " << nodes.size() << std::endl;

	pDeps_t deps = pdgHelper->getDependences();
	std::cout << "nr. of edges: " << deps.size() << std::endl;

	/* get also self dependences */
	for (int i = 0; i < deps.size(); ++i) {
		if (pdgHelper->isSelfDependence(deps[i])) {
			std::cout << "self edge on node: " << pdgHelper->getSourceNode(deps[i])->nr
					<< " through variabe: " << deps[i]->array->name->s << std::endl;
		}
	}

	/* get all SCCs of the pdg */
	PDGgraphSCCs SCCs = pdgHelper->getSCCs();
#ifdef PDG_CHARACTERIZATION_DEUBG
	for (int i = 0; i < SCCs.size(); ++i) {
		pdg_helper::PDGgraphSCC SCC = SCCs[i];
		std::cout << "SCC: " << i << ": ";
		for (int j = 0; j < SCC.size(); ++j) {
			std::cout << SCC[j]->name->s << ", ";
		}
		 std::cout << std::endl;
	}
#endif

	pdg->free();
	delete pdg;
	delete pdgHelper;

	isl_ctx_free(ctx);
	return 0;
}
