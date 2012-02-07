/*
 * Copyright (c) 2012 Leiden University (LERC group at LIACS).
 * All rights reserved.
 *
 * adg2csdf.cc
 *
 *  Created on: Dec 19, 2011
 *      Author: Teddy Zhai
 */

#include <sstream>
#include <iostream>
#include <vector>

#include "barvinok/barvinok.h"

#include "isl/ctx.h"
#include "isl/set.h"

#include "isa/adg.h"
#include "adg_parse.h"

#include "defs.h"
#include "ADG_helper.h"
#include "ImplementationTable.h"
#include "suffix_tree.h"
#include "utility.h"

using namespace adg_helper;

#define TABS(i) std::string((i), '\t')

isl_printer *printer;

struct ind_dims_t {
	unsigned int dims;
	int nr_ind_dim;

	std::map<unsigned int, bool> dim_dep_map;


};

typedef std::map<std::string, std::vector<short>* > phaseMap_t;

typedef std::map<isl_id*, isl_set*> var_domain_t;

typedef std::map<isl_id*, std::vector<short>* > phases_t;

/// Csdf Dumper class
class CsdfDumper {
	ADG_helper *ppn;
	phases_t phases;
	var_domain_t var_domains;
	ImplementationTable *implTable;
	isl_ctx *ctx;

public:
    CsdfDumper(ADG_helper *adg_helper, ImplementationTable *t, isl_ctx *ctx);
    ~CsdfDumper();
    void DumpCsdf(std::ostream &strm);
//    void dumpCsdf3(std::ostream &strm);


private:
//    void writePortCsdf3(std::string name, std::string type, std::ostream &strm);
    void writePhase(isl_id *portName, std::ostream &strm, char sep);

    int getCommonPortsDims(__isl_keep isl_set *process_domain, Ports &ports, ind_dims_t &ind_dims);
    void findVariantDomain2(const Process *process);
	unsigned int getPhaseLength(__isl_keep isl_set *var_domain);
    void computePhases(const Process *process);
    unsigned getWCET(Process *process);


    bool checkSimplePattern(const Process *process);
    bool checkPhaseValidity(const Process *process);

    void dumpChannels(std::ostream &strm);

};


/////// Implementation
CsdfDumper::CsdfDumper(ADG_helper *adg_helper, ImplementationTable *t, isl_ctx *ctx) {
  this->ppn = adg_helper;
  this->implTable = t;
  this->ctx = ctx;

  // Allocate phases for each port
  for (unsigned int i = 0; i < this->ppn->getChannels().size(); ++i) {
    Channel *ch = this->ppn->getChannels()[i];
    phases[(ch->from_port_name)] = new std::vector<short>;
    phases[(ch->to_port_name)] = new std::vector<short>;
  }
}



//// Destructor
CsdfDumper::~CsdfDumper() {
	for (var_domain_t::iterator mit = var_domains.begin();
			mit != var_domains.end();
			++mit)
	{
		isl_set_free(mit->second);
	}

	// deallocate allocated sets during computing phases for each port
	for (phases_t::iterator it = phases.begin(); it != phases.end(); it++) {
		delete it->second;
	}
}

/* project out control variables in bounds of adg_domain to obtain
 * the representation with existential variables
 * */
__isl_give isl_set*
projectCtrlVars(const adg_domain *adg_dom){
	isl_set *set = isl_set_copy(adg_dom->bounds);

	if (adg_dom->controls.size() == 0) {
		return set;
	}

	unsigned nr_ctrl_vars = adg_dom->controls.size();
	unsigned nr_dim = isl_set_dim(set, isl_dim_set);

	// project out control variables from bounds
	set = isl_set_project_out(set, isl_dim_set, nr_dim- nr_ctrl_vars, nr_ctrl_vars);
	return set;
}

/* check all dependent dimensions
 * if the corresponding coefficients are non-zero in this constraint
 * */
int
check_ind_constratint(__isl_take isl_constraint *constrnt, void *user){
	struct ind_dims_t *ind_dims = (ind_dims_t *)user;
	//	std::cout << "nr. dim:" << nr_dims << std::endl;


	std::map<unsigned int, bool> dim_deps;

	isl_int iter_coffi, zero_int;
	isl_int_init(iter_coffi);
	isl_int_init(zero_int);
	isl_int_set_si(zero_int, 0);
	unsigned int nr_dep = 0;
	unsigned int nr_dims = ind_dims->dims;
	// 2 dimensions are dependent if the corresponding coefficient is non-zero
	for (int i = 0; i < nr_dims; ++i) {
		isl_constraint_get_coefficient(constrnt, isl_dim_set, i, &iter_coffi);



		/* if the coefficients of set and dimensions in port domain is not equal to those in process domain,
		 * this means, port domain on the set domain is a subset of the set dimension in the process domain.
		 * This dimension cannot be projected later on.
		 */


		if (isl_int_eq(iter_coffi, zero_int)) {
			dim_deps[i] = false;
		}else{
			// the coefficient is non-zero
			dim_deps[i] = true;
			nr_dep ++;
		}
	}
	isl_int_clear(zero_int);
	isl_int_clear(iter_coffi);


	// at least two dimensions are dependent
	// store all dependent dimensions
	if (nr_dep > 1){
		for (int i = 0; i < nr_dims; ++i) {
			if (dim_deps[i] == false) continue;

			ind_dims->dim_dep_map[i] = true;
		}
	}


	isl_constraint_free(constrnt);
	return 0;
}

int
check_ind_dims(__isl_take isl_basic_set *domain_bset, void *user){

	ind_dims_t *ind_dims = (ind_dims_t *)user;

	/*std::cout << "nr. dim:" << nr_dims << std::endl;
	std::cout << "the domain: ";
	printer = isl_printer_print_basic_set(printer, domain_bset);
	printer = isl_printer_end_line(printer);*/

	int rt = isl_basic_set_foreach_constraint(domain_bset, &check_ind_constratint, ind_dims);
	assert(rt == 0);

	/* first n dimensions must be projected out.
	 * If the domain is already dependent on the first dimension, no variant domain can be found.*/
	int first_dim = -1;
	int n_dim = 0;
	if (ind_dims->dim_dep_map[0] == false) {
		// The first dimension is independent.
		first_dim = 0;
//		std::cout << "first dim. to project out is:" << first_dim << std::endl;
	}

	// port domain shoud be live in the same space with node domain
	unsigned int nr_dims = ind_dims->dims;
	for (int i = 0; i < nr_dims; ++i) {
		if (ind_dims->dim_dep_map[i] == true) {
//			std::cout << i << " dimension is dependent" << std::endl;
		}else{
			n_dim++;
		}
	}

	// all dimensions are independent, we need to keep at one,
	// therefore, we only project out the outer-most "nr_dims-1" dimensions
	if (nr_dims == n_dim) {
		n_dim--;
	}
	//std::cout << "project out dims: " << n_dim << std::endl;


	// project outer-most "n_dim" independent dimensions
	if (first_dim != -1) {
		ind_dims->nr_ind_dim = n_dim;
	}

	isl_basic_set_free(domain_bset);

	return 0;
}

void printTest(adg_domain *adgdomain){
	std::cout << "controls: " << adgdomain->controls.size() << std::endl;
	std::cout << "filters: " << adgdomain->filters.size() << std::endl;
}

int
CsdfDumper::getCommonPortsDims(__isl_keep isl_set *process_domain, Ports &ports, ind_dims_t &ind_dims){
	int nr_common_dims = INT_MAX;

	for (int i = 0; i < ports.size(); ++i) {
		// ignore self-edges
		CDNode cdNode;
		Node *node = ppn->getNode(ports[i]->node_name);
		cdNode.push_back(node);
		if (ppn->isSelfEdge(ppn->getEdge(ports[i]->edge_name), &cdNode)) {
			continue;
		}

		isl_set *port_domain = getPDGDomain(ports[i]->domain);
//		std::cout << "in port domain: ";
//		printer = isl_printer_print_set(printer, port_domain);
//		printer = isl_printer_end_line(printer);

		ind_dims.nr_ind_dim = -1;
		// find the variant domain of this port domain and store in "ind_dims->var_domain"
		int rt = isl_set_foreach_basic_set(port_domain, check_ind_dims, &ind_dims);
		assert(rt == 0);

		/* find the common variant domain:
		 * If the common variant domain is a superset of the currently found variant domain,
		 * i.e. dimension of the common variant domain is larger than the currently found variant domain,
		 * then assign it as the new common variant domain
		 */
		if ( ind_dims.nr_ind_dim <= 0) {
			nr_common_dims = 0;
			isl_set_free(port_domain);

			return nr_common_dims;
		}
		if (ind_dims.nr_ind_dim < nr_common_dims) {
			nr_common_dims = ind_dims.nr_ind_dim;
		}

		/* step 2: iterate over all found common and outer-dimensions.
		 * All port domain must be fully defined in the node domain at that dimension,
		 * if a pattern can be possibly found.
		 * */
		int nr_equal_dims = 0;
		for (int j = 0; j < nr_common_dims; ++j) {
			if (!isDimsEqual(process_domain, port_domain, 0, j+1)) {
				nr_equal_dims = j;
				break;
			} else {
				nr_equal_dims++;
			}
		}

		if (nr_equal_dims < nr_common_dims){
			nr_common_dims = nr_equal_dims;
		}
		assert(nr_common_dims != INT_MAX && nr_common_dims >=0);
		isl_set_free(port_domain);
	} // end ports

	return nr_common_dims;
}


void
CsdfDumper::findVariantDomain2(const Process *process){
	isl_set *process_domain = getPDGDomain(process->domain);
//	std::cout << "process domain: ";
//	printer = isl_printer_print_set(printer, process_domain);
//	printer = isl_printer_end_line(printer);

	/* step 1: check the dependences between dimensions in each port domain.
	 *  if a outer-dimension depends on a certain inner-dimension,
	 *  no pattern can be found at the outer-dimension.
	 *  NOTE: while(1) is naturally handled.
	 *  */
	int nr_common_dims = INT_MAX;
	struct ind_dims_t ind_dims;
	unsigned int nr_dims = isl_set_dim(process_domain, isl_dim_set);
	ind_dims.dims = nr_dims;

	// iterate over all input port domains
	Ports input_ports = ppn->getInPorts(process);
	nr_common_dims = getCommonPortsDims(process_domain, input_ports, ind_dims);

//	// iterate over all output port domains
	Ports output_ports = ppn->getOutPorts(process);
	int nr_common_out_dims = getCommonPortsDims(process_domain, output_ports, ind_dims);
	if (nr_common_dims > nr_common_out_dims) {
		nr_common_dims = nr_common_out_dims;
	}
	// number should be between 0 and number of dimensions of the process domain
//	std::cout << "nr common dimension in variant domains: " << nr_common_dims << std::endl;


	// process the process domain and store its processed variant domain
//	std::cout << "process domain :";
//	printer = isl_printer_print_set(printer, process_domain);
//	printer = isl_printer_end_line(printer);
	if (nr_common_dims > 0) {
		// NOTE: currently the tuple is removed if certain dimensions of a set is projected out.
		// Therefore, we need id to reset it in the new set
		isl_id *process_name = isl_set_get_tuple_id(process_domain);
		isl_set *var_domain_process = isl_set_project_out((process_domain), isl_dim_set,
					0, nr_common_dims);
		var_domain_process = isl_set_set_tuple_id(var_domain_process, process_name);
		var_domains[process->name] = var_domain_process;
	} else {
		// no common variant domain is found
		var_domains[process->name] = process_domain;
	}
//	std::cout << "process variant domain :";
//	printer = isl_printer_print_set(printer, var_domains[process->name]);
//	printer = isl_printer_end_line(printer);


	// for each input port, project out independent dimensions from the port domain
	for (int i = 0; i < input_ports.size(); ++i) {
		isl_set *port_domain = getPDGDomain(input_ports[i]->domain);

		// project out independent dimensions
		if (nr_common_dims > 0) {
			isl_id *port_name = isl_set_get_tuple_id(port_domain);
			isl_set *var_domain_port = isl_set_project_out((port_domain), isl_dim_out,
								0, nr_common_dims);
			var_domain_port = isl_set_set_tuple_id(var_domain_port, port_name);
			var_domains[input_ports[i]->name] = var_domain_port;
		} else {
			var_domains[input_ports[i]->name] = port_domain;
		}
//		std::cout << "in port variant domain :";
//		printer = isl_printer_print_set(printer, var_domains[input_ports[i]->name]);
//		printer = isl_printer_end_line(printer);
	}

	// for each output port, project out independent dimensions from the port domain
	for (int i = 0; i < output_ports.size(); ++i) {
		isl_set *port_domain = getPDGDomain(output_ports[i]->domain);

		// first project out independent dimensions
		if (nr_common_dims > 0) {
			isl_id *port_name = isl_set_get_tuple_id(port_domain);
			isl_set *var_domain_port = isl_set_project_out(port_domain, isl_dim_out,
								0, nr_common_dims);
			var_domain_port = isl_set_set_tuple_id(var_domain_port, port_name);
			var_domains[output_ports[i]->name] = var_domain_port;
		} else {
			var_domains[output_ports[i]->name] = port_domain;
		}
//		std::cout << "-->port variant domain :";
//		printer = isl_printer_print_set(printer, var_domains[output_ports[i]->name]);
//		printer = isl_printer_end_line(printer);
	}

	assert(var_domains.empty() == false);
}


/* the length of the phases is equal to the number of integer points in the variant domain
 * Note that the variant domain is non-parameterized, therefore, this function also return a constant
 */
unsigned int
CsdfDumper::getPhaseLength(__isl_keep isl_set *var_domain){
	isl_pw_qpolynomial *var_domain_qwpq =  isl_set_card(isl_set_copy(var_domain));
	unsigned int var_domain_card = getCardinality(var_domain_qwpq);
	isl_pw_qpolynomial_free(var_domain_qwpq);

	return var_domain_card;
}


/* compute phases for each port of the process according to the max variant domain
 * */
void
CsdfDumper::computePhases(const Process *process){

	// variant domains should be generated before hand
	assert(var_domains.size() > 0);
	assert(var_domains.count(process->name) > 0);
	isl_set *process_var_domain = var_domains[process->name];
	/*std::cout << "process variant domain: ";
	printer = isl_printer_print_set(printer, process_var_domain);
	printer = isl_printer_end_line(printer);*/

	// scan the process domain according to lexicographical order
	isl_set *process_dom_new = isl_set_copy(process_var_domain);
	while(isl_set_is_empty(process_dom_new) != 1){
		// get the lexicographical point
		isl_set *lexmin_process_dom = isl_set_lexmin(isl_set_copy(process_dom_new));

		// iterate over all input port domains
		Ports input_ports = ppn->getInPorts(process);
		for (int i = 0; i < input_ports.size(); ++i) {
			isl_id *port_id = input_ports[i]->name;
			assert(var_domains.count(port_id) > 0);
			isl_set *port_var_domain = var_domains[port_id];

			if (isl_set_is_subset(lexmin_process_dom, port_var_domain)) {
				//std::cout << "1" << std::endl;
				phases[port_id]->push_back(1);
			} else {
				//std::cout << "0" << std::endl;
				phases[port_id]->push_back(0);
			}
		} // end input ports

		// iterate over all output port domains
		Ports output_ports = ppn->getOutPorts(process);
		for (int i = 0; i < output_ports.size(); ++i) {
			isl_id *port_id = output_ports[i]->name;
			assert(var_domains.count(port_id) > 0);
			isl_set *port_var_domain = var_domains[port_id];

			if (isl_set_is_subset(lexmin_process_dom, port_var_domain)) {
				//std::cout << "1" << std::endl;
				phases[port_id]->push_back(1);
			} else {
				//std::cout << "0" << std::endl;
				phases[port_id]->push_back(0);
			}
		} // end output ports


		// remove lexmin point from the set
		process_dom_new = isl_set_subtract(process_dom_new, lexmin_process_dom);
	}
	isl_set_free(process_dom_new);
}

// Write phases belonging to given port to ostream.
void
CsdfDumper::writePhase(isl_id *portName, std::ostream &strm, char sep){
	std::vector<short> *phases_port = phases[portName];
	int len_phases = phases_port->size();
	assert(len_phases > 0);

	strm << (*phases_port)[0];
	for (int i = 1; i < len_phases; i++) {
		strm << sep << (*phases_port)[i];
	}
}

// Returns the WCET of a process
unsigned
CsdfDumper::getWCET(Process *process) {
  return implTable->getMetric( IM_DELAY_WORST, isl_id_get_name(process->function->name) );
}

/* check if the process has a simple consumption/production pattern, such as [1].
 * Basic idea is that, if all input/output port domains are equal to process domain,
 * all consumption/production patterns have only one phase with rate 1.
 *
 * If the process has a simple consumption/production pattern, store the pattern [1] for all input/output ports */
bool
CsdfDumper::checkSimplePattern(const Process *process){
	bool isSimplePattern = true;

//	std::cout << "process domain: ";
//	printer = isl_printer_print_set(printer, process->domain->bounds);
//	printer = isl_printer_end_line(printer);

	isl_set *unwrapped_procs_domain = getPDGDomain(process->domain);

//	std::cout << "projected process domain: ";
//	printer = isl_printer_print_set(printer, unwrapped_procs_domain);
//	printer = isl_printer_end_line(printer);

	// check all input ports
	Ports input_ports = process->input_ports;
	for (int i = 0; i < input_ports.size(); ++i) {
		isl_set *unwrapped_port_domain = getPDGDomain(input_ports[i]->domain);
//		std::cout << "in port domain: ";
//		printer = isl_printer_print_set(printer, input_ports[i]->domain->bounds);
//		printer = isl_printer_end_line(printer);

		if (isl_set_is_equal(unwrapped_port_domain, unwrapped_procs_domain) != 1) {
			isSimplePattern = false;
		}

		isl_set_free(unwrapped_port_domain);
	} // end input ports

	if (!isSimplePattern) {
		isl_set_free(unwrapped_procs_domain);
		return isSimplePattern;
	}


	// check all output ports
	Ports output_ports = process->output_ports;
	for (int i = 0; i < output_ports.size(); ++i) {
		isl_set *unwrapped_port_domain = getPDGDomain(output_ports[i]->domain);
//		std::cout << "out port domain: ";
//		printer = isl_printer_print_set(printer, output_ports[i]->domain->bounds);
//		printer = isl_printer_end_line(printer);

		if (isl_set_is_equal(unwrapped_port_domain, unwrapped_procs_domain) != 1){
			isSimplePattern = false;
		}

		isl_set_free(unwrapped_port_domain);
	} // end output ports

//	std::cout<< "it is a simple pattern." << std::endl;
	isl_set_free(unwrapped_procs_domain);
	if (!isSimplePattern) {
			return isSimplePattern;
	}

	// simple pattern, store the pattern [1]
	for (int i = 0; i < input_ports.size(); ++i) {
		isl_id *port_id = input_ports[i]->name;
		phases[port_id]->push_back(1);
	}
	for (int i = 0; i < output_ports.size(); ++i) {
		isl_id *port_id = output_ports[i]->name;
		phases[port_id]->push_back(1);
	}

	return isSimplePattern;
}


/* valid phases of a port must at least have one 1*/
bool
CsdfDumper::checkPhaseValidity(const Process *process){

	Ports in_ports = process->input_ports;
	for (int i = 0; i < in_ports.size(); ++i) {
		isl_id *port_id = in_ports[i]->name;
		std::vector<short> *port_phases = phases[port_id];

		if (find(port_phases->begin(), port_phases->end(), 1) == port_phases->end()) {
			return false;
		}
	}
	Ports out_ports = process->output_ports;
	for (int i = 0; i < out_ports.size(); ++i) {
		isl_id *port_id = out_ports[i]->name;
		std::vector<short> *port_phases = phases[port_id];

		if (find(port_phases->begin(), port_phases->end(), 1) == port_phases->end()) {
			return false;
		}
	}

	return true;
}


void
CsdfDumper::dumpChannels(std::ostream& strm) {
	int indent = 0;
	Channels ppn_channels = this->ppn->getChannels();
	strm << TABS(indent) << "edge_number:" << ppn_channels.size() << "\n";
	// iterate over all channels
	unsigned int edge_ed = 0;
	for (PPNchIter eit = ppn_channels.begin();
			eit != ppn_channels.end();
			++eit)
	{
		Channel* ch = *eit;

		strm << TABS(indent) << "edge:" << "\n";
		indent++;
		strm << TABS(indent) << "id:" << edge_ed++ << "\n";
		strm << TABS(indent) << "name:" << isl_id_get_name(ch->name) << "\n";
		strm << TABS(indent) << "src:" << ppn->getId(ch->from_node_name) <<
				" " << ppn->getId(ch->from_port_name) << "\n";
		strm << TABS(indent) << "dst:" << ppn->getId(ch->to_node_name) <<
				" " << ppn->getId(ch->to_port_name) << "\n";
		indent = 0;
	}
}

// print the csdf in the StreamIT-compatible format
void CsdfDumper::DumpCsdf(std::ostream& strm) {
	const Processes procs = this->ppn->getProcesses();

	unsigned int indent = 0;
	const unsigned int nd_nr = procs.size();

	strm<< TABS(indent) << "node_number:" << nd_nr << "\n";

	// iterate over all processes
	for (unsigned int i = 0; i < procs.size(); i++){
		Process* process = procs[i];

		strm<< TABS(indent) << "node:\n";
		indent++;

		strm << TABS(indent) << "id:" << ppn->getId(process) << "\n";
		strm << TABS(indent) << "name:" << isl_id_get_name(process->name) << "\n";

		// there are special cases in which simple patterns, such as [1],  can be derived without further processing
		bool isSimplePattern = checkSimplePattern(process);
////		bool isSimplePattern = false;
		if (isSimplePattern) {
			strm << TABS(indent) << "length:1"  << "\n";

			strm << TABS(indent) << "wcet:" << getWCET(process);
		} else { // simple pattern cannot be found
			findVariantDomain2(process);

			computePhases(process);

			strm << TABS(indent) << "length:" << getPhaseLength(var_domains[process->name]) << "\n";
			strm << TABS(indent) << "wcet:" ;
			int wcet = getWCET(process);
			for (unsigned int wc = 1; wc <= getPhaseLength(var_domains[process->name]); wc++) {
				strm << wcet;
				if (wc < getPhaseLength(var_domains[process->name])) strm << " ";
			}
		} // isSImplePattern

		strm << "\n";


		strm << TABS(indent) << "port_number:" << process->input_ports.size() + process->output_ports.size() <<"\n";
		// iterator over input ports of the process
		for (PPNportIter pit = process->input_ports.begin();
				pit != process->input_ports.end();
				++pit)
		{
			Port *port = *pit;

			strm << TABS(indent) << "port:\n";
			indent++;

			strm << TABS(indent) << "type:in" << "\n";
			strm << TABS(indent) << "id:" << ppn->getId(port) << "\n";
			strm << TABS(indent) << "rate:";
			writePhase(port->name, strm, ' ');
			strm << "\n";
			indent--;
		}

		// iterate over all output ports of the process
		for (PPNportIter pit = process->output_ports.begin();
				pit != process->output_ports.end();
				++pit)
		{
			Port *port = *pit;

			strm << TABS(indent) << "port:\n";
			indent++;

			strm << TABS(indent) << "type:out" << "\n";
			strm << TABS(indent) << "id:" << ppn->getId(port) << "\n";
			strm << TABS(indent) << "rate:";
			writePhase(port->name, strm, ' ');
			strm << "\n";
			indent--;
		}

		indent = 0;
	} // end processes

	// write channels
	dumpChannels(strm);
}

static int gOutputFormat; // 1 = StreamIT, 3 = SDF3

void printUsage() {
  fprintf(stderr, "Usage: ppn2csdf [options] < [adg].yaml\n");
  fprintf(stderr, "Supported options:\n");
  fprintf(stderr, "  -3     Dump in SDF3 format \n");
  fprintf(stderr, "  -s     Dump in StreamIT format [default]\n");
}


int parseCommandline(int argc, char ** argv)
{
  gOutputFormat = 1;
  for (int i = 1; i < argc; i++) {
    if (strcmp(argv[i], "-3") == 0) {
    	// currently not supported
    	fprintf(stderr, "Currently generating csdf in the sdf3 format is not supported yet!\n");
    	return 1;

      gOutputFormat = 3;
      return 0;
    }
    else if (strcmp(argv[i], "-s") == 0) {
      gOutputFormat = 1;
      return 0;
    }
    else {
      fprintf(stderr, "Error: unrecognized command line option '%s'\n", argv[i]);
      return 1;
    }
  }

  return 1;
}


int main(int argc, char * argv[])
{
	FILE *in = stdin;

//	if (parseCommandline(argc, argv) == 1) {
//		printUsage();
//		exit(1);
//	}

	isl_ctx *ctx;
	ctx = isl_ctx_alloc();

	adg *csdf_adg;
	csdf_adg = adg_parse(ctx, in);
	assert(csdf_adg);

	// initialize the global variable: printer
	printer = isl_printer_to_file(ctx, stdout);
	printer = isl_printer_set_output_format(printer, ISL_FORMAT_ISL);


	ADG_helper adg_helper(csdf_adg, ctx);

	Parameters params = adg_helper.getParameters();
	if (params.size() > 0) {
		fprintf(stderr, "PPN with parameters has no equivalent CSDF\n");
		delete csdf_adg;
		isl_ctx_free(ctx);
		exit(1);
	}

	ImplementationTable *implTable = new ImplementationTable();
	if (!implTable->loadDefaultFile()) {
		fprintf(stderr, "Warning: Could not load implementation data from default files;\n"
				"         please put impldata.xml in the current directory or in ~/.daedalus\n");
	}

	CsdfDumper *dumper = new CsdfDumper(&adg_helper, implTable, ctx);
//	if (gOutputFormat == 3) {
//		dumper->dumpCsdf3(cout);
//	}else{
//		assert(gOutputFormat == 1);
		dumper->DumpCsdf(cout);
//	}

	delete dumper;
	delete implTable;

	isl_printer_free(printer);
	delete csdf_adg;
	isl_ctx_free(ctx);

	return 0;
}

