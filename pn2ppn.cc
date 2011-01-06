#include <getopt.h>
#include <algorithm>
#include <set>
#include <sstream>

#include "yaml.h"
#include "pdg.h"
#include "pdgutil.h"
extern "C" {
#include "isl_util.h"
}
#include "isl_div.h"
#include "isl_set_polylib.h"
#include "ppn.h"

struct option options[] = {
    { "input",	required_argument,  0,  'i' },
    { "output",	required_argument,  0,  'o' },
    { 0, 0, 0, 0 }
};

using namespace std;
using pdg::PDG;

/*struct espam_edge {
    char	  *name;
    char	  *from_port;
    char	  *to_port;
    isl_set	 *from_domain;
    isl_set	 *to_domain;
    pdg::node    *from_node;
    pdg::node    *to_node;
    vector<pdg::access *>    from_access;
    vector<pdg::access *>    to_access;
    isl_mat	 *map;
    pdg::array *array;
    int		reordering;
    int		multiplicity;
    integer *	size;
    int		nr;
    bool	sticky;
    bool	shift_register;

    espam_edge() : from_domain(NULL), to_domain(NULL), from_node(NULL), to_node(NULL),
	     map(NULL), array(NULL), reordering(-1), multiplicity(-1), size(NULL) {}
};*/


struct add_edge_data {
	vector<espam_edge *> &split_edges;
	pdg::dependence *dep;
	const vector<pdg::access *>& to_accesses;
	bool sticky;
	int i;
	char *name;
	isl_set *from_domain;

	add_edge_data(vector<espam_edge *> &split_edges, pdg::dependence *dep,
		const vector<pdg::access *>& to_accesses, bool sticky,
		char *name)
		: split_edges(split_edges), dep(dep), to_accesses(to_accesses),
		  sticky(sticky), name(name), i(0), from_domain(NULL) {}
};

static int add_basic_lexmin_edge(isl_basic_set *dom, isl_mat *map, void *user)
{
	add_edge_data *data = (add_edge_data *)user;

	espam_edge *edge = new espam_edge;
	edge->reordering = data->dep->reordering;
	edge->multiplicity = data->dep->multiplicity;
	edge->size = data->dep->value_size;
	edge->array = data->dep->array;
	edge->from_node = data->dep->from;
	edge->to_node = data->dep->to;
	edge->from_access.push_back(data->dep->from_access);
	edge->to_access = data->to_accesses;
	edge->from_domain = data->from_domain;
	edge->to_domain = isl_basic_set_compute_divs(dom);
	edge->map = map;
	edge->nr = data->i++;
	edge->name = strdup(data->name);
	edge->sticky = data->sticky;
	edge->shift_register = data->dep->type == pdg::dependence::pn_shift;
	data->split_edges.push_back(edge);

	return 0;
}

/* Even though we know each output (read) iteration has a unique
 * input (write) iteration associated to it, reading off the mapping
 * from the constraints may be non-trivial.  We therefore compute
 * the lexicographically smallest write corresponding to any read.
 * Since there is exactly one write associated to every read,
 * we simply get this write back, but in a more manageable form.
 */
static int add_basic_edge(isl_basic_map *bmap, void *user)
{
	int r;
	add_edge_data *data = (add_edge_data *)user;

	data->from_domain = isl_basic_set_compute_divs(
		isl_basic_map_domain(isl_basic_map_copy(bmap)));

	isl_basic_map *inv = isl_basic_map_reverse(bmap);
	r = isl_basic_map_foreach_lexmin(inv, &add_basic_lexmin_edge, user);
	isl_basic_map_free(inv);

	return 0;
}

/* ESPAM expects the input port of a BroadcastInOrder channel
 * to only consist of the iterations that first read a value
 * because that is what Compaan gives.
 */
static int add_min_edge(isl_basic_map *bmap, void *user)
{
	add_edge_data *data = (add_edge_data *)user;

	isl_map *map = isl_basic_map_lexmin(bmap);
	isl_map_foreach_basic_map(map, &add_basic_edge, data);
	isl_map_free(map);

	return 0;
}

/* The ESPAM people want all "control" variables of the node domain
 * to appear in the port domains.
 * Moreover, the variables in the (reverse) "mapping"s should correspond
 * to the variables in the port domains.  We therefore need to perform
 * the required transformation on the "to" domain before extracting the mapping.
 * In particular, we compute a lifting on the node domain that adds
 * set variables that correspond to the existentially quantified variables.
 * This lifting is then applied to the range of the dependence relation.
 * Before generating code, these extra dimensions need to be projected
 * out again.
 * (Recall that a dependence is a relation from the source of the data
 * to the user of the data, while the mapping in the ESPAM data structures
 * maps iterations of an input port to iterations of an output port.)
 * The "from" domain is handled later in normalize_domain.
 */
static void add_edge(PDG *pdg, vector<espam_edge*>& split_edges,
		     pdg::dependence *dep, pdg::UnionMap *rel,
		     __isl_take isl_set *to_node_domain,
		     const vector<pdg::access *>& to_accesses, bool sticky,
		     int i, const std::vector<Global_Var_Decl *> &params)
{
	int nparam = pdg->params.size();
	char buf[10];
	snprintf(buf, 10, "ED_%d", i);

	add_edge_data add_edge_data(split_edges, dep, to_accesses, sticky, buf);

	isl_ctx *ctx = pdg->get_isl_ctx();
	isl_map *map = rel->get_isl_map(ctx);
	isl_map *lift = isl_set_lifting(to_node_domain);
	map = isl_map_apply_range(map, lift);
	map = isl_map_detect_equalities(map);
	if (dep->multiplicity && !dep->reordering)
		isl_map_foreach_basic_map(map, &add_min_edge, &add_edge_data);
	else
		isl_map_foreach_basic_map(map, &add_basic_edge, &add_edge_data);
	isl_map_free(map);
}


struct div_info {
    string s;
    vector<int> coeff;
    int denom;
};

struct div_vector {
    int		      index;
    vector<div_info*> divs;
};

/* Return the index of the div_info in divs that corresponds to div "pos"
 * in "bset", creating a new div_info if needed.
 */
int div_index(vector<div_info *> &divs, isl_basic_set *bset, unsigned pos,
	      const seq<pdg::parameter> &params)
{
	int j;
	isl_int v;
	div_info *di = new div_info;
	std::ostringstream strm;
	unsigned dim = isl_basic_set_dim(bset, isl_dim_set);
	unsigned nparam = isl_basic_set_dim(bset, isl_dim_param);

	isl_div *div = isl_basic_set_div(isl_basic_set_copy(bset), pos);
	assert(div);

	isl_int_init(v);
	for (j = 0; j < isl_basic_set_dim(bset, isl_dim_div); ++j) {
		isl_div_get_coefficient(div, isl_dim_div, j, &v);
		assert(isl_int_is_zero(v));
	}

	isl_div_get_denominator(div, &v);
	di->denom = isl_int_get_si(v);
	for (j = 0; j < dim; ++j) {
		isl_div_get_coefficient(div, isl_dim_set, j, &v);
		di->coeff.push_back(isl_int_get_si(v));
	}
	for (j = 0; j < nparam; ++j) {
		isl_div_get_coefficient(div, isl_dim_param, j, &v);
		di->coeff.push_back(isl_int_get_si(v));
	}
	isl_div_get_constant(div, &v);
	di->coeff.push_back(isl_int_get_si(v));

	isl_int_clear(v);
	isl_div_free(div);

	for (j = 0; j < divs.size(); ++j) {
		if (divs[j]->denom != di->denom || divs[j]->coeff != di->coeff)
			continue;
		delete di;
		return j;
	}

	divs.push_back(di);
	strm << "div(";
	bool first = true;
	for (j = 0; j < dim; ++j) {
		if (di->coeff[j] == 0)
			continue;
		if (!first && di->coeff[j] >= 0)
			strm << "+";
		first = false;
		strm << di->coeff[j] << "*" << "c" << j;
	}
	for (j = 0; j < nparam; ++j) {
		if (di->coeff[dim+j] == 0)
			continue;
		if (!first && di->coeff[dim+j] >= 0)
			strm << "+";
		first = false;
		strm << di->coeff[dim+j] << "*" << params[j]->name->s;
	}
	if (di->coeff[dim+nparam] != 0) {
		if (!first && di->coeff[dim+nparam] >= 0)
			strm << "+";
		strm << di->coeff[dim+nparam];
	}
	strm << "," << di->denom << ")";
	di->s = strm.str();
	return divs.size()-1;
}

int only_set(std::vector<int> &constraint, unsigned pos, unsigned n,
		unsigned only)
{
    for (int j = 0; j < n; ++j) {
	if (only == pos+j)
	    continue;
	if (constraint[1+pos+j] != 0)
	    return 0;
    }
    return 1;
}

static unsigned n_statement_dims(pdg::PDG *pdg)
{
    unsigned nstat = 0;
    for (int i = 0; i < pdg->statement_dimensions.size(); ++i)
	if (pdg->statement_dimensions[i])
	    ++nstat;
    return nstat;
}

/* Remove the statement level dimensions from domain.
 */
static __isl_keep isl_basic_set *strip_statement_dims(pdg::PDG *pdg,
	__isl_take isl_basic_set *domain)
{
	unsigned dim = pdg->statement_dimensions.size();

	for (int j = dim - 1; j >= 0; --j) {
		if (!pdg->statement_dimensions[j])
			continue;
		domain = isl_basic_set_remove(domain, isl_dim_set, j, 1);
	}

	return domain;
}

/* Remove the rows and columns from M that refer to statement level dimensions.
 */
static pdg::Matrix *strip_statement_dims_from_map(pdg::PDG *pdg, isl_mat *map)
{
    pdg::Matrix *stripped = new pdg::Matrix;
    unsigned dim = pdg->statement_dimensions.size();
    unsigned nstat = n_statement_dims(pdg);

    for (int i = 0; i < map->n_row - 1; ++i) {
	int k;

	if (pdg->statement_dimensions[i])
	    continue;

	vector<int> row(map->n_col - nstat);
	k = 0;
	for (int j = 0; j < map->n_col - 1; ++j) {
	    if (j < dim && pdg->statement_dimensions[j])
		continue;
	    row[k++] = isl_int_get_si(map->row[1 + i][1 + j]);
	}
	row[k++] = isl_int_get_si(map->row[1 + i][0]);
	stripped->el.push_back(row);
    }
    isl_mat_free(map);
    return stripped;
}

static char *writePort(bool out, int index, char *node_name,
		    const vector<pdg::access *>& accesses, espam_edge* edge,
		    isl_set *domain, isl_set *parent_domain,
		    pdg::PDG *pdg, div_vector& dv)
{
    char buf[30];
    std::ostringstream strm;
    char *port_name;

    strm << node_name << (out ? 'O' : 'I') << "P_" << edge->name << '_' << index;
    strm << "_V";
    for (int i = 0; i < accesses.size(); ++i)
	strm << '_' << accesses[i]->nr;
    port_name = strdup(strm.str().c_str());

    return port_name;
}

struct disjoint_edge {
    Relation		    domain;
    vector<pdg::access *>   accesses;
    bool		    sticky;

    disjoint_edge(Relation d, vector<pdg::access *> a, bool s) : 
	domain(d), accesses(a), sticky(s) {}
};

static void disjoin_dependences(vector<disjoint_edge>& parts, 
				const std::vector<Global_Var_Decl *>& globals,
				Relation& remainder, pdg::access *access,
				bool sticky)
{
    int parts_size = parts.size();
    for (int k = 0; k < parts_size; ++k) {
	Relation inter = Intersection(copy(remainder), copy(parts[k].domain));
	if (!inter.is_satisfiable())
	    continue;
	parts[k].domain = Difference(parts[k].domain, copy(inter));
	remainder = Difference(remainder, copy(inter));
	vector<pdg::access *> accesses = parts[k].accesses;
	if (access)
	    accesses.push_back(access);
	parts.push_back(disjoint_edge(inter, accesses, parts[k].sticky || sticky));
    }
    if (remainder.is_satisfiable()) {
	vector<pdg::access *> accesses;
	if (access)
	    accesses.push_back(access);
	parts.push_back(disjoint_edge(remainder, accesses, sticky));
    }
}

/* Scatter dependence and convert to Omega Relation
 */
static Relation scat_dep_relation(PDG *pdg, pdg::dependence *dep,
				  const std::vector<Global_Var_Decl *>& params)
{
    isl_map *map = scatter_dependence(pdg, dep);
    pdg::UnionMap *rel = new pdg::UnionMap(map, pdg->params);
    isl_map_free(map);
    Relation R = pdg::map2relation(rel, params);
    rel->params.resize(0);
    rel->free();
    delete rel;
    return R;
}

static __isl_give isl_set *scatter_node(PDG *pdg, pdg::node *node)
{
	isl_ctx *ctx = pdg->get_isl_ctx();
	isl_set *set = node->source->get_isl_set(ctx);
	isl_map *sr = node->scattering->get_isl_map(ctx);
	sr = isl_map_reverse(sr);
	set = isl_set_apply(set, sr);
	return set;
}

/*static void add_scat(cloog_program_builder& pb, isl_dim *domain_dim, int offset)
{
    isl_map *scat = isl_map_identity(domain_dim);
    scat = isl_map_extend(scat, isl_map_n_param(scat),
				isl_map_n_in(scat), isl_map_n_out(scat) + 1);
    scat = isl_map_fix_si(scat, isl_dim_out,
			  isl_map_dim(scat, isl_dim_out) - 1, offset);
    
    pb.add_scattering(scat);
}*/

/* The ESPAM people want all "control" variables of the node domain
 * to appear in the port domains.
 * We therefore temporarily treat the control variables of the node domain
 * as iterators by applying a lifting of the existential variables in
 * the node domain and then simplify the control variables of the port domain
 * in terms of these extra iterators.
 */
static isl_set *normalize_domain(isl_set *port_domain, isl_set *node_domain)
{
	isl_map *lift = isl_set_lifting(isl_set_copy(node_domain));
	port_domain = isl_set_apply(port_domain, lift);
	port_domain = isl_set_detect_equalities(port_domain);
	return port_domain;
}


/* Comparison function for sorting parts of the same edge such that
 * a part with an input port corresponding to an earlier argument position
 * comes before a part with an input port corresponding to a later argument
 * position.
 * This is needed for pn_union channels as their detection allows
 * the case where two (or more) tokens are consumed in the same iteration,
 * provided they don't cause reordering when consumed in the order of
 * the arguments.
 * Presumably, these edges should only be contected to one argument,
 * so the order for edges with more than one to_access is arbitrary.
 * Perhaps we should add an explicit test for that.
 */
struct less_output_port :
		public std::binary_function<espam_edge*, espam_edge*, bool> {
	bool operator()(espam_edge *x, espam_edge *y) {
		if (x->nr != y->nr)
			return x->nr < y->nr;
		if (x->to_access.size() != y->to_access.size())
			return x->to_access.size() < y->to_access.size();
		for (int i = 0; i < x->to_access.size(); ++i) {
			if (x->to_access[i]->nr == y->to_access[i]->nr)
				continue;
			return x->to_access[i]->nr < y->to_access[i]->nr;
		}
		return 0;
	}
};

static void writeADG(pdg::PDG *pdg)
{
    int rc;
    isl_set **scattered_domains = NULL;
    isl_ctx *ctx = pdg->get_isl_ctx();

    int nparam = pdg->params.size();


    for (int i = 0; i < pdg->params.size(); ++i) {

	pdg::Matrix *M = NULL;
	if (pdg->context) {
	    assert(pdg->context->constraints.size() == 1);
	    M = pdg->context->constraints[0];
	}
	for (int j = 0; M && j < M->el.size(); ++j) {
	    char buf[10];
	    if (!M->el[j][1+i])
		continue;
	    int k;
	    for (k = 0; k < pdg->params.size(); ++k)
		if (i != k && M->el[j][1+k])
		    break;
	    if (k < pdg->params.size())
		continue;
	    if (M->el[j][0] == 0) {
		int v = -M->el[j][1+pdg->params.size()]/M->el[j][1+i];
		snprintf(buf, sizeof(buf), "%d", v);
		continue;
	    }
	    if (M->el[j][1+i] == 1) {
		snprintf(buf, sizeof(buf), "%d", -M->el[j][1+pdg->params.size()]);
	    }
	    if (M->el[j][1+i] == -1) {
		snprintf(buf, sizeof(buf), "%d", M->el[j][1+pdg->params.size()]);
	    }
	}

	if (pdg->params[i]->value) {
	    char buf[10];
	    snprintf(buf, 10, "%d", pdg->params[i]->value->v);
	}

    }

    scattered_domains = new isl_set *[pdg->nodes.size()];
    for (int i = 0; i < pdg->nodes.size(); ++i)
	scattered_domains[i] = scatter_node(pdg, pdg->nodes[i]);

    vector<espam_edge*> split_edges;

    std::vector<Global_Var_Decl *> globals = pdg::extract_globals(pdg);
    for (int i = 0; i < pdg->dependences.size(); ++i) {
	pdg::dependence *dep = pdg->dependences[i];
	if (dep->type == pdg::dependence::uninitialized)
	    continue;
	if (dep->type == pdg::dependence::pn_part)
	    continue;
	if (dep->type == pdg::dependence::pn_wire)
	    continue;
	if (dep->type == pdg::dependence::pn_hold)
	    continue;
	vector<disjoint_edge> parts;

	if (dep->type == pdg::dependence::pn_union) {
	    for (int j = 0; j < pdg->dependences.size(); ++j) {
		pdg::dependence *part_dep = pdg->dependences[j];
		if (part_dep->type != pdg::dependence::pn_part)
		    continue;
		if (part_dep->container != dep)
		    continue;
		Relation remainder = scat_dep_relation(pdg, part_dep, globals);
		disjoin_dependences(parts, globals, remainder, part_dep->to_access,
				    false);
	    }
	} else {
	    Relation R = scat_dep_relation(pdg, dep, globals);
	    vector<pdg::access *> accesses;
	    accesses.push_back(dep->to_access);
	    parts.push_back(disjoint_edge(R, accesses, false));
	}

	for (int j = 0; j < pdg->dependences.size(); ++j) {
	    pdg::dependence *wire_dep = pdg->dependences[j];
	    if (wire_dep->type != pdg::dependence::pn_wire)
		continue;
	    if (dep->to != wire_dep->from)
		continue;
	    if (dep->array != wire_dep->array)
		continue;
	    if (dep->to_access != wire_dep->from_access)
		continue;
	    Relation dep_rel = scat_dep_relation(pdg, dep, globals);
	    Relation wire_rel = scat_dep_relation(pdg, wire_dep, globals);
	    Relation join = Join(dep_rel, wire_rel);
	    join.simplify();
	    if (join.is_satisfiable())
		disjoin_dependences(parts, globals, 
				    join, wire_dep->to_access, false);
	}

	for (int j = 0; j < pdg->dependences.size(); ++j) {
	    pdg::dependence *hold_dep = pdg->dependences[j];
	    if (hold_dep->type != pdg::dependence::pn_hold)
		continue;
	    if (dep->to != hold_dep->from)
		continue;
	    if (dep->array != hold_dep->array)
		continue;
	    if (dep->to_access != hold_dep->from_access)
		continue;
	    Relation dep_rel = scat_dep_relation(pdg, dep, globals);
	    Relation hold_rel = scat_dep_relation(pdg, hold_dep, globals);
	    Relation domain = Domain(hold_rel);
	    Relation sticky_part = Restrict_Range(dep_rel, domain);
	    if (sticky_part.is_satisfiable())
		disjoin_dependences(parts, globals, sticky_part, NULL, true);
	}

	for (int k = 0; k < parts.size(); ++k) {
	    if (!parts[k].domain.is_satisfiable())
		continue;
	    pdg::UnionMap *rel = new pdg::OUnionMap(&parts[k].domain, globals);
	    rel->params = pdg->params;
	    add_edge(pdg, split_edges, dep, rel, 
		     isl_set_copy(scattered_domains[dep->to->nr]),
		     parts[k].accesses, parts[k].sticky, i, globals);
	}
    }

    std::sort(split_edges.begin(), split_edges.end(), less_output_port());

    div_vector dv;
    dv.index = 0;

    int max_access = 1;

    for (int i = 0; i < pdg->nodes.size(); ++i) {
	pdg::node *node = pdg->nodes[i];
	if (node->statement->accesses.size() > max_access)
	    max_access = node->statement->accesses.size();
    }


    for (int i = 0; i < pdg->nodes.size(); ++i) {
	char buf[10];
	char *name;
	pdg::node *node = pdg->nodes[i];

	snprintf(buf, 10, "ND_%d", node->nr);
	name = strdup(buf);

	vector<bool> is_write;
	vector<pdg::access*> accesses;
	for (int a = 0; a < node->statement->accesses.size(); ++a) {
	    int nr = node->statement->accesses[a]->nr;
	    if (nr >= is_write.size()) {
		is_write.resize(nr+1);
		accesses.resize(nr+1);
	    }
	    assert(nr < is_write.size());
	    if (node->statement->accesses[a]->type == pdg::access::write)
		is_write[nr] = true;
	    accesses[nr] = node->statement->accesses[a];
	}

	isl_set *domain = isl_set_copy(scattered_domains[i]);
	for (int i = 0, j = 0; i < split_edges.size(); ++i) {
	    if (split_edges[i]->to_node != node)
		continue;
	    char *port_name;
	    isl_set *to_domain;
	    to_domain = split_edges[i]->to_domain;
	    port_name = writePort(false, j, name, split_edges[i]->to_access,
				  split_edges[i],
				  to_domain, domain, pdg, dv);
	    split_edges[i]->to_port = port_name;
	    ++j;

	    unsigned to_dim = isl_set_dim(to_domain, isl_dim_set);
	    unsigned dim = isl_set_dim(domain, isl_dim_set);
	    to_domain = isl_set_project_out(to_domain, isl_dim_set,
					    dim, to_dim - dim);
	    isl_dim *dims = isl_dim_copy(to_domain->dim);
	}

	for (int i = 0, j = 0; i < split_edges.size(); ++i) {
	    if (split_edges[i]->from_node != node)
		continue;
	    char *port_name;
	    isl_set *from_domain;
	    from_domain = normalize_domain(split_edges[i]->from_domain, domain);
      split_edges[i]->from_domain = from_domain;
	    port_name = writePort(true, j, name, split_edges[i]->from_access,
				  split_edges[i],
				  from_domain, domain, pdg, dv);
	    split_edges[i]->from_port = port_name;
	    ++j;

	    unsigned from_dim = isl_set_dim(from_domain, isl_dim_set);
	    unsigned dim = isl_set_dim(domain, isl_dim_set);
	    from_domain = isl_set_project_out(from_domain, isl_dim_set,
					      dim, from_dim - dim);
	    isl_dim *dims = isl_dim_copy(from_domain->dim);
	}


	for (int i = 0; i < is_write.size(); ++i) {
	    snprintf(buf, 10, "%s_%d", is_write[i] ? "out" : "in", i);
	}

	assert(pdg->dimension == node->scattering->input);
	isl_set *normalized_domain = normalize_domain(isl_set_copy(domain),
						      domain);
	isl_set_free(normalized_domain);

	dv.index += dv.divs.size();
	dv.divs.clear();

	//isl_dim *dims = isl_dim_copy(domain->dim);
	//pb.add_domain(domain, node->statement->operation, name);
	//add_scat(pb, dims, (3*node->nr+1)*max_access);
    }

    for (int i = 0; i < split_edges.size(); ++i) {
      pdg::Matrix *stripped;
      stripped = strip_statement_dims_from_map(pdg, split_edges[i]->map);
      split_edges[i]->map_stripped = stripped;
      //delete stripped; // TODO
    }

    ppn::PPN *ppn = new ppn::PPN;
    ppn->import_pn(pdg, split_edges);
    ppn->Dump(stdout);

    for (int i = 0; i < pdg->nodes.size(); ++i)
	isl_set_free(scattered_domains[i]);
    delete [] scattered_domains;

}

int main(int argc, char * argv[])
{
    char *input = NULL, *output = NULL;
    FILE *in = stdin, *out = stdout;
    int c, ind = 0;
    int rc;

    //LIBXML_TEST_VERSION

    while ((c = getopt_long(argc, argv, "i:o:", options, &ind)) != -1) {
	switch (c) {
	case 'i':
	    input = optarg;
	    break;
	case 'o':
	    output = optarg;
	    break;
	}
    }

    if (input && strcmp(input, "-")) {
	in = fopen(input, "r");
	assert(in);
	if (!output) {
	    int len = strlen(input);
	    if (len > 5 && !strcmp(input+len-5, ".yaml"))
		len -= 5;
	    output = new char[len+4+1];
	    strncpy(output, input, len);
	    strcpy(output+len, ".xml");
	}
    }

    PDG *pdg;
    pdg = yaml::Load<PDG>(in);

    if (pdg->dependences.size() == 0) {
      fprintf(stderr, "Error: PDG does not contain dependence information.\n");
      exit(1);
    }
    writeADG(pdg);

    if (output && strcmp(output, "-")) {
	out = fopen(output, "w");
	assert(out);
    }

    pdg->free();
    delete pdg;

    return 0;
}
