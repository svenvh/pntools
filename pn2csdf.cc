/*
 * pn2csdf.cc
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

#include "ADG_helper.h"
#include "defs.h"
#include "ImplementationTable.h"
#include "utility.h"

//using pdg::PDG;
//using ppn::PPN;
//using namespace std;
using namespace adg_helper;

#define TABS(i) std::string((i), '\t')

isl_printer *printer;

struct ind_dims_t {
	unsigned int dims;
	int nr_ind_dim;

	isl_set *var_domain;
	std::map<unsigned int, bool> dim_dep_map;

	ind_dims_t() : var_domain(NULL) {}
};

typedef std::map<std::string, std::vector<short>* > phaseMap_t;

typedef std::map<isl_id*, isl_set*> var_domain_t;

typedef std::map<isl_id*, std::vector<short>* > phases_t;

/// Csdf Dumper class
// Currently only used by ppn2csdf, so declaration is inside this file.
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
    __isl_give isl_set* findVariantDomain(const Process *process);
    void findVariantDomain2(const Process *process);
	unsigned int getPhaseLength(__isl_keep isl_set *var_domain);
	void scanIterationPoints_lexmin(const Process *process, const Port *port);
    void computePhases(const Process *process);
    unsigned getWCET(Process *process);


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



int
check_ind_constratint(__isl_take isl_constraint *constrnt, void *user){
	struct ind_dims_t *printer_dims = (ind_dims_t *)user;
	unsigned int nr_dims = printer_dims->dims;
//	std::cout << "nr. dim:" << nr_dims << std::endl;


	std::map<unsigned int, bool> dim_deps;

	isl_int iter_coffi, zero_int;
	isl_int_init(iter_coffi);
	isl_int_init(zero_int);
	isl_int_set_si(zero_int, 0);
	unsigned int nr_dep = 0;
	for (int i = 0; i < nr_dims; ++i) {
		isl_constraint_get_coefficient(constrnt, isl_dim_set, i, &iter_coffi);

		if (isl_int_eq(iter_coffi, zero_int)) {
			dim_deps[i] = false;
		}else{
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

			printer_dims->dim_dep_map[i] = true;
		}
	}


	isl_constraint_free(constrnt);
	return 0;
}

int
check_ind_dims(__isl_take isl_basic_set *domain_bset, void *user){

	ind_dims_t *ind_dims = (ind_dims_t *)user;
	unsigned int nr_dims = ind_dims->dims;

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
		isl_basic_set *d_var = isl_basic_set_project_out(domain_bset, isl_dim_out, first_dim, n_dim);
		ind_dims->var_domain = isl_set_from_basic_set(d_var);

		ind_dims->nr_ind_dim = n_dim;
	}else{
		// no compact variant domain is found.
		// the variant domain is equal to the original port domain.
		ind_dims->var_domain = isl_set_from_basic_set(domain_bset);
	}

	/*std::cout << "the variant domain: ";
	printer = isl_printer_print_set(printer, ind_dims->var_domain);
	printer = isl_printer_end_line(printer);
	isl_basic_set_free(d_var);*/

	return 0;
}

void printTest(adg_domain *adgdomain){
	std::cout << "constrols: " << adgdomain->controls.size() << std::endl;
	std::cout << "filters: " << adgdomain->filters.size() << std::endl;
}

void
CsdfDumper::findVariantDomain2(const Process *process){
	isl_set *process_domain = ppn->getProcessDomainBound(process);
	/*std::cout << "process domain: ";
	printer = isl_printer_print_set(printer, process->domain->bounds);
	printer = isl_printer_end_line(printer);*/

	//isl_set *common_var_domain = isl_set_empty_like(process_domain);
	int nr_common_vardomain_dims = INT_MAX;

	// iterate over all input port domains
	Ports input_ports = ppn->getInPorts(process);
	for (int i = 0; i < input_ports.size(); ++i) {
		isl_set *port_domain = ppn->getPortDomainBound(input_ports[i]);

		struct ind_dims_t *ind_dims = new ind_dims_t;
		unsigned int nr_dims = isl_set_dim(port_domain, isl_dim_out);
		ind_dims->dims = nr_dims;
		ind_dims->nr_ind_dim = -1;

		// find the variant domain of this port domain and store in "ind_dims->var_domain"
		int rt = isl_set_foreach_basic_set(port_domain, check_ind_dims, ind_dims);
		assert(rt == 0);

		/* find the common variant domain:
		 * If the common variant domain is a subset of the currently found variant domain,
		 * then assign it as the new common variant domain
		if (isl_set_is_subset(common_var_domain, ind_dims->var_domain) != 1){
			isl_set_free(common_var_domain);
			common_var_domain = isl_set_copy(ind_dims->var_domain);
		}*/
		if ( ind_dims->nr_ind_dim != -1 && ind_dims->nr_ind_dim < nr_common_vardomain_dims) {
			nr_common_vardomain_dims = ind_dims->nr_ind_dim;
		}


		isl_set_free(ind_dims->var_domain);
		free(ind_dims);
		isl_set_free(port_domain);
	}

	// iterate over all output port domains
	Ports output_ports = ppn->getOutPorts(process);
	for (int i = 0; i < output_ports.size(); ++i) {
		isl_set *port_domain = ppn->getPortDomainBound(output_ports[i]);
		/*std::cout << "port domain :";
		printer = isl_printer_print_set(printer, port_domain);
		printer = isl_printer_end_line(printer);*/
//		printTest(output_ports[i]->domain);
//		if (isl_set_is_subset(port_domain, process_domain) == 0) {
//			std::cout << "how come, port domain is not a subset of the process domain." << std::endl;
//		}

		struct ind_dims_t *ind_dims = new ind_dims_t;
		ind_dims->dims  = isl_set_dim(port_domain, isl_dim_out);
		ind_dims->nr_ind_dim = -1;

		// find variant domains and store in "ind_dims->var_domain"
		int rt = isl_set_foreach_basic_set(port_domain, check_ind_dims, ind_dims);
		assert(rt == 0);
		assert(ind_dims->var_domain != NULL);

		/* find the max common variant domain:
		 * If the common variant domain is a subset of the currently found variant domain,
		 * then assign it as the new common variant domain
		if (isl_set_is_subset(common_var_domain, ind_dims->var_domain) != 1) {
			isl_set_free(common_var_domain);
			common_var_domain = isl_set_copy(ind_dims->var_domain);
		}*/
		if ( ind_dims->nr_ind_dim != -1 && ind_dims->nr_ind_dim < nr_common_vardomain_dims) {
			nr_common_vardomain_dims = ind_dims->nr_ind_dim;
		}

		isl_set_free(port_domain);
		isl_set_free(ind_dims->var_domain);
		free(ind_dims);
	}

	// number should be between 0 and number of dimensions of the process domain
	assert(nr_common_vardomain_dims != INT_MAX);

	// process the process domain and store its processed variant domain
	isl_set *var_domain_process = isl_set_project_out(process_domain, isl_dim_out,
			0, nr_common_vardomain_dims);
	// TODO: possibly controls in the process domain needs to be projected out as well.
	var_domains[process->name] = var_domain_process;
	/*std::cout << "process variant domain :";
	printer = isl_printer_print_set(printer, var_domain_process);
	printer = isl_printer_end_line(printer);*/


	// for each input port, project out independent dimensions from the port domain
	for (int i = 0; i < input_ports.size(); ++i) {
		isl_set *port_domain = ppn->getPortDomainBound(input_ports[i]);

		// project out independent dimensions
		isl_set *var_domain_port = isl_set_project_out(port_domain, isl_dim_out,
					0, nr_common_vardomain_dims);

		// project out control variables if any
		unsigned nr_controls_port_domain = input_ports[i]->domain->controls.size();
		if ( nr_controls_port_domain > 0) {
			int nr_dim_var_port_dom = isl_set_dim(var_domain_port, isl_dim_set);
			var_domain_port = isl_set_project_out(var_domain_port, isl_dim_set,
					nr_dim_var_port_dom - nr_controls_port_domain, nr_controls_port_domain);
		}

		var_domains[input_ports[i]->name] = var_domain_port;
	}

	// for each output port, project out independent dimensions from the port domain
	for (int i = 0; i < output_ports.size(); ++i) {
		isl_set *port_domain = ppn->getPortDomainBound(output_ports[i]);

		// first project out independent dimensions
		isl_set *var_domain_port = isl_set_project_out(port_domain, isl_dim_out,
					0, nr_common_vardomain_dims);

		// project out control variables if any
		unsigned nr_controls_port_domain = output_ports[i]->domain->controls.size();
		if ( nr_controls_port_domain > 0) {
			int nr_dim_var_port_dom = isl_set_dim(var_domain_port, isl_dim_set);
			var_domain_port = isl_set_project_out(var_domain_port, isl_dim_set,
					nr_dim_var_port_dom - nr_controls_port_domain, nr_controls_port_domain);
		}
		var_domains[output_ports[i]->name] = var_domain_port;
		/*std::cout << "port variant domain :";
		printer = isl_printer_print_set(printer, var_domain_port);
		printer = isl_printer_end_line(printer);*/
	}

	assert(var_domains.empty() == false);
}


__isl_give isl_set*
CsdfDumper::findVariantDomain(const Process *process){

	isl_set *process_domain = ppn->getProcessDomainBound(process);
	std::cout << "process domain: ";
	printer = isl_printer_print_set(printer, process->domain->bounds);
	printer = isl_printer_end_line(printer);

	isl_set *common_var_domain = isl_set_empty_like(process_domain);
	int nr_common_vardomain_dims = INT_MAX;

	// iterate over all input port domains
	Ports input_ports = ppn->getInPorts(process);
	for (int i = 0; i < input_ports.size(); ++i) {
		isl_set *port_domain = ppn->getPortDomainBound(input_ports[i]);

		struct ind_dims_t *ind_dims = new ind_dims_t;
		unsigned int nr_dims = isl_set_dim(port_domain, isl_dim_out);
		ind_dims->dims = nr_dims;
		ind_dims->nr_ind_dim = -1;

		// find the variant domain of this port domain and store in "ind_dims->var_domain"
		int rt = isl_set_foreach_basic_set(port_domain, check_ind_dims, ind_dims);
		assert(rt == 0);

		/* find the common variant domain:
		 * If the common variant domain is a subset of the currently found variant domain,
		 * then assign it as the new common variant domain */
		if (isl_set_is_subset(common_var_domain, ind_dims->var_domain) != 1){
			isl_set_free(common_var_domain);
			common_var_domain = isl_set_copy(ind_dims->var_domain);
		}

		isl_set_free(ind_dims->var_domain);
		free(ind_dims);
		isl_set_free(port_domain);
	}

	// iterate over all output port domains
	Ports output_ports = ppn->getOutPorts(process);
	for (int i = 0; i < output_ports.size(); ++i) {
		isl_set *port_domain = ppn->getPortDomainBound(output_ports[i]);
		std::cout << "port domain :";
		printer = isl_printer_print_set(printer, port_domain);
		printer = isl_printer_end_line(printer);

		struct ind_dims_t *ind_dims = new ind_dims_t;
		ind_dims->dims  = isl_set_dim(port_domain, isl_dim_out);
		ind_dims->nr_ind_dim = -1;

		// find variant domains and store in "ind_dims->var_domain"
		int rt = isl_set_foreach_basic_set(port_domain, check_ind_dims, ind_dims);
		assert(rt == 0);
		assert(ind_dims->var_domain != NULL);

		/* find the max common variant domain:
		 * If the common variant domain is a subset of the currently found variant domain,
		 * then assign it as the new common variant domain
		if (isl_set_is_subset(common_var_domain, ind_dims->var_domain) != 1) {
			isl_set_free(common_var_domain);
			common_var_domain = isl_set_copy(ind_dims->var_domain);
		}*/
		if ( ind_dims->nr_ind_dim != -1 && ind_dims->nr_ind_dim < nr_common_vardomain_dims) {
			nr_common_vardomain_dims = ind_dims->nr_ind_dim;
		}

		isl_set_free(port_domain);
		isl_set_free(ind_dims->var_domain);
		free(ind_dims);
	}


	/*std::cout << "the common variant domain: ";
	printer = isl_printer_print_set(printer, common_var_domain);
	printer = isl_printer_end_line(printer);*/

	// for each input port, process the port domain to variant domain (dimensions)

	// for each output port, process the port domain to variant domain (dimensions)
	for (int i = 0; i < output_ports.size(); ++i) {
		isl_set *port_domain = ppn->getPortDomainBound(output_ports[i]);

	}

	isl_set_free(process_domain);

	assert(isl_set_is_empty(common_var_domain) != 1);
	return common_var_domain;
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
//	std::cout << "process variant domain: ";
//	printer = isl_printer_print_set(printer, process_var_domain);
//	printer = isl_printer_end_line(printer);


	// iterate over all input port domains
	Ports input_ports = ppn->getInPorts(process);
	for (int i = 0; i < input_ports.size(); ++i) {
		isl_id *port_id = input_ports[i]->name;

		assert(var_domains.count(port_id) > 0);
		isl_set *port_var_domain = var_domains[port_id];

		// scan the process domain according to lexicographical order
		isl_set *process_dom_new = isl_set_copy(process_var_domain);
		while(isl_set_is_empty(process_dom_new) != 1){
			// get the lexicographical point
			isl_set *lexmin_process_dom = isl_set_lexmin(isl_set_copy(process_dom_new));

			if (isl_set_is_subset(lexmin_process_dom, port_var_domain)) {
				//std::cout << "1" << std::endl;
				phases[port_id]->push_back(1);
			} else {
				//std::cout << "0" << std::endl;
				phases[port_id]->push_back(0);
			}

			// remove lexmin point from the set
			process_dom_new = isl_set_subtract(process_dom_new, lexmin_process_dom);
		}
		isl_set_free(process_dom_new);

		assert(phases[port_id]->size() > 0);
	} // end input_ports

	// iterate over all output port domains
	Ports output_ports = ppn->getOutPorts(process);
	for (int i = 0; i < output_ports.size(); ++i) {
		isl_id *port_id = output_ports[i]->name;

		assert(var_domains.count(port_id) > 0);
		isl_set *port_var_domain = var_domains[port_id];

		// scan the process domain according to lexicographical order
		isl_set *process_dom_new = isl_set_copy(process_var_domain);
		while(isl_set_is_empty(process_dom_new) != 1){
			// get the lexicographical point
			isl_set *lexmin_process_dom = isl_set_lexmin(isl_set_copy(process_dom_new));

			if (isl_set_is_subset(lexmin_process_dom, port_var_domain)) {
				//std::cout << "1" << std::endl;
				phases[port_id]->push_back(1);
			} else {
				//std::cout << "0" << std::endl;
				phases[port_id]->push_back(0);
			}

			// remove lexmin point from the set
			process_dom_new = isl_set_subtract(process_dom_new, lexmin_process_dom);
		}
		isl_set_free(process_dom_new);

		assert(phases[port_id]->size() > 0);
//		printer = isl_printer_print_set(printer, domain_port);
//		printer = isl_printer_end_line(printer);
	} // end output ports
}

// print the csdf in the StreamIT-compatible format
void CsdfDumper::DumpCsdf(std::ostream& strm) {
	const Processes procs = this->ppn->getProcesses();

	unsigned int indent = 0;
	const unsigned int nd_nr = procs.size();

	printer = isl_printer_to_file(this->ctx, stdout);
	printer = isl_printer_set_output_format(printer, ISL_FORMAT_ISL);


	strm<< TABS(indent) << "node_number:" << nd_nr << "\n";

	// iterate over all processes
	for (unsigned int i = 0; i < procs.size(); i++){
		Process* process = procs[i];

		strm<< TABS(indent) << "node:\n";
		indent++;

		strm << TABS(indent) << "id:" << i << "\n";
		strm << TABS(indent) << "name:" << isl_id_get_name(process->name) << "\n";

		findVariantDomain2(process);
//		isl_set *var_domain = findVariantDomain(process);

		computePhases(process);

    strm << TABS(indent) << "length:" << getPhaseLength(var_domains[process->name]) << "\n";
    strm << TABS(indent) << "wcet:" ;
    int wcet = getWCET(process);
    for (unsigned int wc = 1; wc <= getPhaseLength(var_domains[process->name]); wc++) {
      	strm << wcet;
		if (wc < getPhaseLength(var_domains[process->name])) strm << " ";
    }
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
    	strm << TABS(indent) << "id:" << isl_id_get_name(port->name) << "\n";
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
    	strm << TABS(indent) << "id:" << isl_id_get_name(port->name) << "\n";
    	strm << TABS(indent) << "rate:";
    	writePhase(port->name, strm, ' ');
    	strm << "\n";
    	indent--;
	}

    indent = 0;
  } // end processes


  // write channels
  indent = 0;
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
    strm << TABS(indent) << "src:" << isl_id_get_name(ch->from_node_name) <<
    		" " << isl_id_get_name(ch->from_port_name) << "\n";
    strm << TABS(indent) << "dst:" << isl_id_get_name(ch->to_node_name) <<
        		" " << isl_id_get_name(ch->to_port_name) << "\n";
    indent = 0;
  }


  isl_printer_free(printer);
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


static int gOutputFormat; // 1 = StreamIT, 3 = SDF3

void printUsage() {
  fprintf(stderr, "Usage: ppn2csdf [options] < [adg].yaml\n");
  fprintf(stderr, "Supported options:\n");
  fprintf(stderr, "  -3     Dump in SDF3 format [default]\n");
  fprintf(stderr, "  -s     Dump in StreamIT format\n");
}


int parseCommandline(int argc, char ** argv)
{
  gOutputFormat = 3;
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

  if (parseCommandline(argc, argv) == 1) {
    printUsage();
    exit(1);
  }

  isl_ctx *ctx;
  ctx = isl_ctx_alloc();

  adg *csdf_adg;
  csdf_adg = adg_parse(ctx, in);
  assert(csdf_adg);

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
  if (gOutputFormat == 3) {
//    dumper->dumpCsdf3(cout);
  }else{
	  assert(gOutputFormat == 1);
	  dumper->DumpCsdf(cout);
  }

  delete dumper;
  delete implTable;


  delete csdf_adg;
  isl_ctx_free(ctx);

  return 0;
}

