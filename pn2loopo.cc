// pn2loopo
// Takes a *pn.yaml file and generates .ispc and .dep files, meant for input to the LooPo scheduler.
// TODO: this tool needs to be completely rewritten to make use of the necessary ISL functions.
// TODO: currently, this tool is NOT FUNCTIONAL
//
// Sven van Haastregt, 2009-2011
// LIACS, Leiden University

#include <iostream>
#include <set>
#include "argp.h"

#include "version.h"

#include "yaml.h"
#include "pdg.h"

#include "isl/map.h"
#include "isl/dim.h"
#include "isl/set.h"

using pdg::PDG;
using namespace std;
//using namespace size;

struct argp_option argp_options[] = {
    { "input",		    		'i',		"file",	0 },
    { "output",		    		'o',		"file",	0 },
    { "version",	    		'V',	    	0,	0 },
    { "human-readable",		'H',	    	0,	0 },
    { 0 }
};

//struct arguments : size_options {
struct arguments  {
    char *input;
    char *output;
};

static struct arguments arguments;

struct state {
  FILE *out_ispc;
  FILE *out_dep;
  bool verbose;
  int n_vars;
  int n_scontrol;
  int n_params;
};

static struct state state;

/*
static Matrix* Matrix2Matrix(pdg::Matrix *mat, int dim, int nparam)
{
    return mat->el.size() ? (Matrix*)*mat : Matrix_Alloc(0, 2+dim+nparam);
}

static pdg::UnionSet *scatter_node(pdg::node *node)
{
    int nparam = node->source->params.size();
    Polyhedron *domain = NULL;
    assert(node->source->dim == node->scattering->output);
    assert(node->source->params.size() == node->scattering->params.size());

    Matrix *scattering[node->scattering->constraints.size()];
    for (int j = 0; j < node->scattering->constraints.size(); ++j) {
        scattering[j] = *node->scattering->constraints[j];
    }

    for (int i = 0; i < node->source->constraints.size(); ++i) {
        Matrix *source = Matrix2Matrix(node->source->constraints[i], 
                                       node->source->dim, nparam);
        int extra = source->NbColumns-2 - node->source->dim - nparam;
        for (int j = 0; j < node->scattering->constraints.size(); ++j) {
            int scatdim = node->scattering->input + node->scattering->output;
            assert(scattering[j]->NbColumns-2 == scatdim + nparam);
            int cols = 2 + scatdim + extra + nparam;
            Matrix *M = Matrix_Alloc(source->NbRows + scattering[j]->NbRows, cols);
            for (int k = 0; k < scattering[j]->NbRows; ++k) {
                Vector_Copy(scattering[j]->p[k], M->p[k], 1+scatdim);
                Vector_Copy(scattering[j]->p[k]+1+scatdim, M->p[k]+1+scatdim+extra,
                            nparam+1);
            }
            int offset = scattering[j]->NbRows;
            for (int k = 0; k < source->NbRows; ++k) {
                value_assign(M->p[offset+k][0], source->p[k][0]);
                Vector_Copy(source->p[k]+1, M->p[offset+k]+1+node->scattering->input,
                            source->NbColumns - 1);
            }
            Polyhedron *P = Constraints2Polyhedron(M, POL_NO_DUAL);
            P->next = domain;
            domain = P;
            Matrix_Free(M);
        }
        Matrix_Free(source);
    }

    for (int j = 0; j < node->scattering->constraints.size(); ++j) {
        Matrix_Free(scattering[j]);
    }
    Polyhedron *S = Domain_projectout(domain, node->scattering->input,
                                      node->scattering->output, nparam);
    Domain_Free(domain);
    pdg::UnionSet *scat_domain = new pdg::UnionSet(node->scattering->input,
                                                   S, node->source->params);
    Domain_Free(S);
    return scat_domain;
}

static unsigned n_statement_dims(pdg::PDG *pdg)
{
    unsigned nstat = 0;
    for (int i = 0; i < pdg->statement_dimensions.size(); ++i)
        if (pdg->statement_dimensions[i])
            ++nstat;
    return nstat;
}
*/
/* Remove the columns from M that refer to statement level dimensions
 * and remove the rows that have become zero by this removal.
 * As an added bonus, we also remove the "positivity constraint"
 * that PolyLib may have added.
 *//*
static pdg::Matrix *strip_statement_dims(pdg::PDG *pdg, pdg::Matrix *M)
{
    M->dump();
    pdg::Matrix *stripped = new pdg::Matrix;
    int dim = pdg->statement_dimensions.size();
    unsigned nstat = n_statement_dims(pdg);
    for (int i = 0; i < M->el.size(); ++i) {
        vector<int> row(M->el[i].size() - nstat);
        bool nonzero = false;
        bool stat = false;
        for (int j = 0, k = 0; j < M->el[i].size(); ++j) {
            if (j >= 1 && j <= M->el[i].size()-2 && M->el[i][j] != 0) {
                if (j <= dim && pdg->statement_dimensions[j-1])
                    stat = true;
                else
                    nonzero = true;
            }
            if (1 <= j && j <= dim && pdg->statement_dimensions[j-1])
                continue;
            row[k++] = M->el[i][j];
        }
        assert(!(nonzero && stat));
        if (nonzero)
            stripped->el.push_back(row);
    }
    return stripped;
}

int
loopo_matrix_get_nb_eq (pdg::Matrix * matrix)
{
		int x,y;
		int res = 0;

		for (x=0;x<matrix->el.size(); x++) {
			if (matrix->el[x][0] == 0) {
				res++;
			}
		}
	return res;
}
*/
void
loopo_initialize(pdg::PDG * pdg)
{
	int nb_nodes  = pdg->nodes.size();
	int nb_vars   = 0;
	int nb_params = 0;
  int nb_scontrol = 0;
  int i, j;

  // Find maximum values for var & param count
  // TODO: rewrite to use ISL functions (isl_get_dim etc)
	for (int i = 0; i < nb_nodes; ++i) {
		pdg::node * n = pdg->nodes[i];
		pdg::UnionSet * domain = n->source;

		if (domain->dim > nb_vars)
      nb_vars = domain->dim;

    if (domain->params.size() > nb_params)
      nb_params = domain->params.size();

    for (j = 0; j < domain->constraints.size(); j++) {
      int scontrol = 0;
      if (domain->constraints[j]->el.size()) {
        scontrol = domain->constraints[j]->el[0].size() - (domain->dim + domain->params.size() + 2);
      }
      else {
        fprintf(stderr, "Warning: found empty constraints matrix for node %d\n", i);
      }
      if (scontrol > 0) {
        printf("Found %d additional column(s) for node %d, output will not be correct.\n", nb_scontrol, n->nr);
      }
      if (scontrol > nb_scontrol) {
        nb_scontrol = scontrol;
      }
    }
  }

	for (int i = 0; i < pdg->dependences.size(); ++i) {
	  pdg::dependence *dep = pdg->dependences[i];
		pdg::node * to = dep->to, * from = dep->from;
		if (!(from && to)) {
			continue;
		}

		pdg::UnionMap * map = dep->relation;

    // Loop over all constraint matrices from this relation and search for static control vars:
    for (int c = 0; c < map->constraints.size(); c++) {
      pdg::Matrix* matrix = map->constraints[c];
      int n_src_vars = from->source->dim;
      int n_dst_vars = to->source->dim;
      int n_sc_vars = map->constraints[0]->el[0].size() - (n_src_vars + n_dst_vars + state.n_params + 2);
      if (n_sc_vars > 0) {
        printf("Warning: %d static control variable for dep %d[%d]; output will not be correct\n", n_sc_vars, i, c);
        if (n_sc_vars > nb_scontrol) {
          nb_scontrol = n_sc_vars;
        }
      }
    }
  }

  printf("Global: %d index vars, %d static control vars, %d params\n", nb_vars, nb_scontrol, nb_params);
  state.n_vars = nb_vars;
  state.n_scontrol = nb_scontrol;
  state.n_params = nb_params;
}
/*
void
loopo_print_matrix (pdg::Matrix * matrix, FILE *fout)
{
		int x,y;
		for (x=0;x<matrix->el.size(); x++) {
			for (y=0;y<matrix->el[x].size(); y++) {
				fprintf(fout, " %3i", matrix->el[x][y]);
			}
			fprintf(fout, "\n");
		}
}

void
loopo_print_matrix_ineq (pdg::Matrix * matrix, FILE *fout, int colskip)
{
		int x,y;
		for (x=0;x<matrix->el.size(); x++) {
			if (matrix->el[x][0] == 0) continue;
			for (y=colskip+1;y<matrix->el[x].size(); y++) {
				fprintf(fout, " %i", matrix->el[x][y]);
			}
			fprintf(fout, "\n");
		}
}

void
loopo_print_matrix_eq (pdg::Matrix * matrix, FILE *fout)
{
		int x,y;
		for (x=0;x<matrix->el.size(); x++) {
			if (matrix->el[x][0] == 1) continue;
			for (y=1;y<matrix->el[x].size(); y++) {
				fprintf(fout, " %i", matrix->el[x][y]);
			}
			fprintf(fout, "\n");
		}
}


// Prints a matrix row for the ispc file.
// Optionally negates all fields (useful when printing equalities).
static void loopo_print_ispc_row(std::vector<int> *line, FILE *fout, int nb_vars, int nb_scontrol, int nb_params, int coeff) {
  int y,k;
  for (y=1; y<=nb_vars; y++) {              // Var coeffs
    fprintf(fout, " %2i", coeff*(*line)[y]);
  }
  for (k=nb_vars; k<state.n_vars; k++) {    // Pad coeffs to cover all vars
    fprintf(fout, "  0");
  }
  for (y=nb_vars+1; y<=nb_vars+nb_scontrol; y++) {  // Static control coeffs
    fprintf(fout, " %2i", coeff*(*line)[y]);
  }
  for (k=nb_scontrol; k<state.n_scontrol; k++) {  // Pad coeffs to cover all static control vars
    fprintf(fout, "  0");
  }
  for (k=0; k<nb_params; k++) {             // Param coeffs
    fprintf(fout, " %2i", coeff*(*line)[y+k]);
  }
  fprintf(fout, " %2i", coeff*(*line)[line->size()-1]);  // constant
  if (state.verbose) {
    fprintf(fout, "            ### ");
    for (y=1;y<line->size(); y++) {
      fprintf(fout, " %i", (*line)[y]);
    }
  }
  fprintf(fout, "\n");
}



// Prints a matrix with var coefficient columns padded to cover all vars
static void loopo_print_ispc_matrix_padded(pdg::Matrix *m, FILE *fout, int nb_vars, int nb_scontrol, int nb_params)
{
  int k;
  int x,y;
  for (x=0;x<m->el.size(); x++) {
    if (m->el[x][0] == 0) {
      loopo_print_ispc_row(&(m->el[x]), fout, nb_vars, nb_scontrol, nb_params, -1);
    }
    loopo_print_ispc_row(&(m->el[x]), fout, nb_vars, nb_scontrol, nb_params, 1);
  }
}


// Counts the number of rows that will be printed for an ispc matrix.
static int loopo_count_ispc_matrix_rows(pdg::Matrix *m)
{
  int x;
  int count = 0;
  for (x=0;x<m->el.size(); x++) {
    if (m->el[x][0] == 0) {
      count++;
    }
    count++;
  }
  return count;
}

// Returns true if given matrix row contains one or more vars
static bool loopo_matrix_row_has_vars(std::vector<int> row, int n_src_vars, int n_dst_vars)
{
  int j;
  for (j = 0; j < n_src_vars; j++) {
    if (row[1+j] != 0) {
      return true;
    }
  }
  return false;
}

// Returns true if columns from start to (& including) end in matrix line all contain zeroes.
static bool is_range_zero(std::vector<int> *line, int start, int end) {
  int i;

  for (i = start; i <= end; i++) {
    if ( (*line)[i] != 0)
      return false;
  }

  return true;
}

// Returns true if the given row describes an equality and contains non-zero coeffs for both the source & destination vars,
// thus, it describes the transformation from source to destination
static bool is_src_dst_trans_row(std::vector<int> *line, int n_src_vars, int n_dst_vars)
{
  return (*line)[0] == 0 &&
         !is_range_zero(line, 1, n_src_vars) &&
         !is_range_zero(line, n_src_vars+1, n_src_vars+n_dst_vars);
}

// Prints a single line describing an inequality FOR SOURCE INEQS, with additionally needed variables padded to zero
// Optionally negates all fields (useful when printing equalities).
static void loopo_print_src_ineq_padded(std::vector<int> *line, FILE *fout, int n_src_vars, int n_dst_vars, int n_scontrol, int coeff)
{
  int i,j,k;
  for (j = 1; j <= n_src_vars; j++) {                 // Vars
    fprintf(fout, " %2i", coeff * (*line)[j]);
  }
  for (k = 0; k < state.n_vars-n_src_vars; k++) {     // Padding to cover all vars
    fprintf(fout, "  0");
  }
  for (k = n_src_vars+n_dst_vars+1; k <= n_src_vars+n_dst_vars+n_scontrol; k++) {  // Static control vars
    fprintf(fout, " %2i", coeff * (*line)[k]);
  }
  for (k = 0; k < state.n_scontrol-n_scontrol; k++) { // Padding to cover static control vars
    fprintf(fout, "  0");
  }
  for (k = 0; k < state.n_params; k++) {              // Param coeffs
    fprintf(fout, " %2i", coeff * (*line)[j+n_dst_vars+k]);
  }
  fprintf(fout, " %2i", coeff * (*line)[line->size()-1]);  // constant
  fprintf(fout, "\n");
}

// Prints a single line describing an inequality, with additionally needed variables padded to zero
// Optionally negates all fields (useful when printing equalities).
static void loopo_print_dst_ineq_padded(std::vector<int> *line, FILE *fout, int n_src_vars, int n_dst_vars, int n_scontrol, int coeff)
{
  int i,j,k;
  for (j = 1; j <= n_dst_vars; j++) {                 // Vars
    fprintf(fout, " %2i", coeff * (*line)[j+n_src_vars]);
  }
  for (k = 0; k < state.n_vars-n_dst_vars; k++) {     // Padding to cover all vars
    fprintf(fout, "  0");
  }
  for (k = n_src_vars+n_dst_vars+1; k <= n_src_vars+n_dst_vars+n_scontrol; k++) {  // Static control vars
    fprintf(fout, " %2i", coeff * (*line)[k]);
  }
  for (k = 0; k < state.n_scontrol-n_scontrol; k++) { // Padding to cover static control vars
    fprintf(fout, "  0");
  }
  for (k = 0; k < state.n_params; k++) {              // Param coeffs
    fprintf(fout, " %2i", coeff * (*line)[j+n_src_vars+k]);
  }
  fprintf(fout, " %2i", coeff * (*line)[line->size()-1]);  // constant
  fprintf(fout, "\n");
}

// Returns true if the given row does not describe a src->dst transform, and contains source variables
static bool is_src_row(std::vector<int> *line, int n_src_vars, int n_dst_vars)
{
  return !is_src_dst_trans_row(line, n_src_vars, n_dst_vars) &&  // Not a transformation row...          
         !is_range_zero(line, 1, n_src_vars);                    // Contains vars we're interested in...

}

// Same as is_src_row, but now for destination vars
static bool is_dst_row(std::vector<int> *line, int n_src_vars, int n_dst_vars)
{
  return !is_src_dst_trans_row(line, n_src_vars, n_dst_vars) &&  // Not a transformation row...          
         !is_range_zero(line, n_src_vars+1, n_src_vars+n_dst_vars);
}

// Prints the non-context ineqs, padded where necessary to cover all variables
// TODO new version..., untested, still under construction
// Note: just prints all static control vars, does not analyze which of these are actually used in the source (in)equalities.
void loopo_print_src_matrix_padded(pdg::Matrix *m, FILE *fout, int n_src_vars, int n_dst_vars, int n_scontrol)
{
  int i;
  for (i = 0; i < m->el.size(); i++) {
    if (is_src_row(&(m->el[i]), n_src_vars, n_dst_vars) ||          // describes a source (in)equality
        is_range_zero(&(m->el[i]), 1, n_src_vars+n_dst_vars)) {     // or describes a static control var
      if (m->el[i][0] == 0) {
        // This is an equality, print an additional inequality with its coeffs negated
        loopo_print_src_ineq_padded(&(m->el[i]), fout, n_src_vars, n_dst_vars, n_scontrol, -1);
      }
      loopo_print_src_ineq_padded(&(m->el[i]), fout, n_src_vars, n_dst_vars, n_scontrol, 1);
    }
  }
}

// Same as loopo_print_src_matrix_padded, but now for destination vars
void loopo_print_dst_matrix_padded(pdg::Matrix *m, FILE *fout, int n_src_vars, int n_dst_vars, int n_scontrol)
{
  int i;
  for (i = 0; i < m->el.size(); i++) {
    if (is_dst_row(&(m->el[i]), n_src_vars, n_dst_vars) ||        // describes a dest (in)equality
        is_range_zero(&(m->el[i]), 1, n_src_vars+n_dst_vars)) {   // or describes a static control var
      if (m->el[i][0] == 0) {
        // This is an equality, print an additional inequality with its coeffs negated
        loopo_print_dst_ineq_padded(&(m->el[i]), fout, n_src_vars, n_dst_vars, n_scontrol, -1);
      }
      loopo_print_dst_ineq_padded(&(m->el[i]), fout, n_src_vars, n_dst_vars, n_scontrol, 1);
    }
  }
}

// Returns #rows that will be printed for the source matrix
int loopo_count_src_matrix_rows(pdg::Matrix *m, int n_src_vars, int n_dst_vars)
{
  int i, count = 0;
  for (i = 0; i < m->el.size(); i++)
    if (is_src_row(&(m->el[i]), n_src_vars, n_dst_vars) ||
        is_range_zero(&(m->el[i]), 1, n_src_vars+n_dst_vars)) {     // or describes a static control var
      if (m->el[i][0] == 0)                               // Equalities will be split into 2 rows
        count++;
      count++;
    }
  return count;
}

// Returns #rows that will be printed for the destination matrix
int loopo_count_dst_matrix_rows(pdg::Matrix *m, int n_src_vars, int n_dst_vars)
{
  int i, count = 0;
  for (i = 0; i < m->el.size(); i++)
    if (is_dst_row(&(m->el[i]), n_src_vars, n_dst_vars) ||
        is_range_zero(&(m->el[i]), 1, n_src_vars+n_dst_vars)) {     // or describes a static control var
      if (m->el[i][0] == 0)                               // Equalities will be split into 2 rows
        count++;
      count++;
    }
  return count;
}
*/
// Prints the non-context ineqs, padded where necessary to cover all variables
/*void loopo_print_matrix_nc_ineqs_padded(pdg::Matrix *m, FILE *fout, int n_src_vars, int n_dst_vars)
{
  int i, j, k;

  for (i = 0; i < m->el.size(); i++) {
    if (m->el[i][0] == 0)
      continue;
    if (loopo_matrix_row_has_vars(m->el[i], n_src_vars, n_dst_vars)) {
      for (j = 1; j <= n_dst_vars; j++) {                 // Vars
        fprintf(fout, " %i", m->el[i][j+n_src_vars]);
      }
      for (k = 0; k < state.n_vars-n_dst_vars; k++) {       // Padding to cover all vars
        fprintf(fout, " p");
      }
      for (k = 0; k < state.n_params; k++) {              // Param coeffs
        fprintf(fout, " %i", m->el[i][j+n_src_vars+k]);
      }
      fprintf(fout, " %i", m->el[i][m->el[i].size()-1]);
      fprintf(fout, "\n");
    }
  }
}*/

/*void loopo_print_matrix_nc(pdg::Matrix *m, FILE *fout)
{
  int i, j, k;

  for (i = 0; i < m->el.size(); i++) {
    //if (m->el[i][0] == 0)
    //  continue;
    for (j = 0; j < m->el[i].size(); j++) {                 // Vars
      fprintf(fout, " %i", m->el[i][j]);
    }
    fprintf(fout, "\n");
  }
}*/

// Counts number of rows that will be printed for the h-transformation matrix
int loopo_count_htrans_matrix_rows(pdg::Matrix *m, int n_src_vars)
{
  int i, v, k;
  int cnt = 0;
  for (i = 0; i < m->el.size(); i++) {
    if (m->el[i][0] != 0)
      continue;
    for (v = 0; v < n_src_vars; v++) {
      if (m->el[i][v+1] == 1) {
        cnt++;
      }
      else if (m->el[i][v+1] != 0) {
        printf("Warning: found candidate h-trans row with coeff != 1: ");
        for (k = 0; k < m->el[i].size(); k++) {
          printf(" %2d", m->el[i][k]);
        }
        printf("\n");
      }
    }
  }
  return cnt;
}

// Prints the h-transformation matrix
void loopo_print_htrans_matrix(pdg::Matrix *m, FILE *fout, int n_src_vars, int n_dst_vars, int n_scontrol)
{
  int i, j, v, k;

  for (i = 0; i < m->el.size(); i++) {
    if (m->el[i][0] != 0)
      continue;
    for (v = 0; v < n_src_vars; v++) {
      if (m->el[i][v+1] == 1) {
        for (j = 1; j <= n_dst_vars; j++) {                   // Dst vars
          fprintf(fout, " %2i", -(m->el[i][n_src_vars+j]));
        }
        for (k = 0; k < state.n_vars-n_dst_vars; k++) {         // Padding to cover all vars
          fprintf(fout, "  0");
        }
        for (k = n_src_vars+n_dst_vars+1; k <= n_src_vars+n_dst_vars+n_scontrol; k++) {  // Static control vars
          fprintf(fout, " %2i", -(m->el[i][k]));
        }
        for (k = 0; k < state.n_scontrol-n_scontrol; k++) { // Padding to cover static control vars
          fprintf(fout, "  0");
        }
        for (k = 1; k <= state.n_params; k++) {               // Param coeffs
          fprintf(fout, " %2i", -(m->el[i][n_dst_vars+n_src_vars+k]));
        }
        fprintf(fout, " %2i", -(m->el[i][m->el[i].size()-1]));  // Constant
        fprintf(fout, "\n");
      }
    }
  }
}

// Prints the .ispc file
void print_loopo_ispc(pdg::PDG * pdg, FILE *fout)
{
  bool verbose = state.verbose;
	int nb_nodes = pdg->nodes.size();
	int nb_vars;
	int nb_params;
  int nb_scontrol;
  int i, k;

  if (verbose) fprintf(fout, "This file was annotated to improve human readability, but it will not parse anymore\n");

	if (verbose) fprintf(fout, "Total number of statements: ");
	fprintf(fout, "%i\n", nb_nodes);

	for (int i = 0; i < nb_nodes; ++i) {
		pdg::node * n = pdg->nodes[i];
		pdg::UnionSet * domain = n->source;

		assert (domain->constraints.size()==1);
		pdg::Matrix *m = domain->constraints[0]; 
		nb_vars = domain->dim;
		nb_params = domain->params.size(); 
    nb_scontrol = m->el.size() ? m->el[0].size() - (nb_vars + nb_params + 2) : 0;
		
    fprintf(fout, "\n");

		if (verbose) fprintf(fout, "Statement number:     ");
		fprintf(fout, "%i\n", i+1);
		
		if (verbose) fprintf(fout, "Number of rows:       ");
		//fprintf(fout, "%i\n", loopo_count_ispc_matrix_rows(m));
		
		if (verbose) fprintf(fout, "Number of variables (iters+scvars): ");
		fprintf(fout, "%i\n", state.n_vars + state.n_scontrol);
		
		if (verbose) fprintf(fout, "Number of parameters: ");
		fprintf(fout, "%i\n", state.n_params);

		if (verbose) fprintf(fout, "Denominator:          ");
		fprintf(fout, "1\n");
	
    //loopo_print_ispc_matrix_padded(m, fout, nb_vars, nb_scontrol, nb_params);
	}
}

void print_loopo_dep(pdg::PDG * pdg, FILE *fout)
{
  bool verbose = state.verbose;
	int nb_deps = 0;
  int total_deps = pdg->dependences.size();
	int nb_nodes = pdg->nodes.size();
  int nb_params = state.n_params;
	int i,c;

	int nb_vars, n_src_vars, n_dst_vars;

  // Count # deps
	for (i = 0; i < total_deps; i++){
		if (pdg->dependences[i]->from && pdg->dependences[i]->to)
      nb_deps += pdg->dependences[i]->relation->constraints.size();
  }

  // Scatter nodes
  pdg::UnionSet **scattered_domains = NULL;
  scattered_domains = new pdg::UnionSet *[pdg->nodes.size()];
  //for (i = 0; i < pdg->nodes.size(); ++i)
  //    scattered_domains[i] = scatter_node(pdg->nodes[i]);

  if (verbose) fprintf(fout, "This file was annotated to improve human readability, but it will not parse anymore\n");
/*
	for (int i = 0; i < nb_nodes; ++i) {
		pdg::node * n = pdg->nodes[i];
		pdg::UnionSet * domain = n->source;

		assert (domain->constraints.size()==1);
		pdg::Matrix *m = domain->constraints[0]; 
		
		/ Number of variables /
		nb_vars = domain->dim;
		
		/ Number of parameters /
		nb_params = domain->params.size(); 
	}
*/
	if (verbose) fprintf(fout, "number of dependences: ");
	fprintf(fout, "%i\n", nb_deps);
	if (verbose) fprintf(fout, "Number of variables:   ");
	fprintf(fout, "%i\n", state.n_vars+state.n_scontrol);
	if (verbose) fprintf(fout, "Number of parameters:  ");
	fprintf(fout, "%i\n", state.n_params);

	if (verbose) fprintf(fout, "Number of pseudo-parameters: ");
	fprintf(fout, "%i\n", 0);

	fprintf(fout, "\n");
	
	int dep_nr=0;	

	for (int i = 0; i < total_deps; ++i) {
		
		pdg::dependence *dep = pdg->dependences[i];
		pdg::node * to = dep->to, * from = dep->from;
		if (!(from && to)) {
			continue;
		}

		pdg::UnionMap * map = dep->relation;

    // Loop over all constraint matrices from this relation; for each matrix, generate a separate dependence for LooPo
    for (c = 0; c < map->constraints.size(); c++) {
      pdg::Matrix* matrix = map->constraints[c];

      nb_vars = from->source->dim;
      n_src_vars = from->source->dim;
      n_dst_vars = to->source->dim;
      int n_sc_vars = map->constraints[0]->el[0].size() - (n_src_vars + n_dst_vars + state.n_params + 2);

      printf("Dep %2d  from %2d to %2d (dep %d[%d]), srcvars=%d dstvars=%d scvars=%d params=%d array %s\n", dep_nr, from->nr, to->nr, i, c, n_src_vars, n_dst_vars, n_sc_vars, state.n_params, dep->array->name->s.c_str());

      if (verbose) fprintf(fout, "-- Dep %d  from %d to %d (dep %d[%d]), array '%s'\n", dep_nr, from->nr, to->nr, i, c, dep->array->name->s.c_str());
      if (verbose) fprintf(fout, "number of dependence: ");
      fprintf(fout, "%i\n", ++dep_nr);
    
      /* dependence type */	
      if (dep->type == pdg::dependence::flow) {
        fprintf(fout, "%i\n", 8);                 // NON_UNIFORM_FLOW
      } else if (dep->type == pdg::dependence::anti){
        fprintf(fout, "%i\n", 9);                 // NON_UNIFORM_ANTI
      } else if (dep->type == pdg::dependence::output){
        fprintf(fout, "%i\n", 10);                // NON_UNIFORM_OUTPUT
      } else {
        printf("Warning: unhandled dependence type `%i'\n", dep->type);
        fprintf(fout, "8\n");                     // Assume NON_UNIFORM_FLOW
      }

      /* Source description */
      pdg::UnionSet * domain = from->source;
      assert (domain->constraints.size()==1);
      //pdg::Matrix *m = domain->constraints[0]; 

      if (verbose) fprintf(fout, "Source:\n");

      /* number of rows */
      //fprintf(fout, "%i\n", loopo_count_src_matrix_rows(matrix, n_src_vars, n_dst_vars));

      /* denominator */
      fprintf(fout, "1\n");

      /* matrix */
      //loopo_print_matrix_nc_ineqs_padded(matrix, fout, n_src_vars, n_dst_vars);
      printf("Relation:\n");
      //loopo_print_matrix(matrix, stdout);
      //loopo_print_src_matrix_padded(matrix, fout, n_src_vars, n_dst_vars, n_sc_vars);


  /*
      matrix = scattered_domains[from->nr]->constraints[0];
      assert(scattered_domains[from->nr]->constraints.size() == 1);
      pdg::Matrix *stripped = strip_statement_dims(pdg, matrix);
      loopo_print_matrix_nc(matrix, fout);
      delete stripped;
      fprintf(fout, "org eq:\n");
      loopo_print_matrix_eq (matrix, fout);
      fprintf(fout, "org ineq:\n");
      loopo_print_matrix_ineq (matrix, fout, 0);
      */


      if (verbose) fprintf(fout, "Dest:\n");
  /*
      if (to->nr == from->nr) {
        fprintf(fout, "0\n");
        fprintf(fout, "1\n");
        goto CONT;
      }*/

      domain = to->source;
      assert (domain->constraints.size()==1);
      //m = domain->constraints[0]; 
      //map = dep->relation;
      //matrix = map->constraints[0];

      /* number of rows */
      //fprintf(fout, "%i\n", loopo_count_dst_matrix_rows(matrix, n_src_vars, n_dst_vars));

      /* denominator */
      fprintf(fout, "1\n");

      /* matrix */
      //loopo_print_dst_matrix_padded(matrix, fout, n_src_vars, n_dst_vars, n_sc_vars);

  //CONT:
      if (verbose) fprintf(fout, "Source statement: ");
      fprintf(fout, "%i\n", from->nr + 1);
      if (verbose) fprintf(fout, "Target statement: ");
      fprintf(fout, "%i\n", to->nr + 1);
      
      if (verbose) fprintf(fout, "h-transformation:\n");

      //fprintf(fout, "%i\n", loopo_count_htrans_matrix_rows(matrix, n_src_vars));
      fprintf(fout, "1\n");
      //loopo_print_htrans_matrix(matrix, fout, n_src_vars, n_dst_vars, n_sc_vars);
      //fprintf(fout, "org:\n");
      //loopo_print_matrix_eq (matrix, fout);
      
      fprintf(fout, "\n");
    }
	}

  delete [] scattered_domains;
}

error_t parse_opt(int key, char *arg, struct argp_state *state)
{
	struct arguments *arguments = (struct arguments *)(state->input);

	switch (key) {
		case ARGP_KEY_INIT:
			arguments->input = NULL;
			arguments->output = NULL;
			break;
		case 'V':
			printf("%s\n", pdg_version());
			exit(0);
		case 'i':
			arguments->input = arg;
			break;
		case 'o':
			arguments->output = arg;
			break;
    case 'H':
      ::state.verbose = true;
      break;
		default:
			return ARGP_ERR_UNKNOWN;
	}
	return 0;
}


int main(int argc, char * argv[])
{
	FILE *in = stdin, *out = stdout;
  char *fn_ispc;
  char *fn_dep;

	PDG *pdg;
	static struct argp argps = { argp_options, parse_opt, 0, 0, NULL };
	//struct barvinok_options *options = barvinok_options_new_with_defaults();

	//arguments.barvinok = options;
	argp_parse(&argps, argc, argv, 0, 0, &arguments);


	if (arguments.input && strcmp(arguments.input, "-")) {
		in = fopen(arguments.input, "r");
		assert(in);
    int len = strlen(arguments.input);
    if (len > 5 && !strcmp(arguments.input+len-5, ".yaml"))
      len -= 5;
    arguments.output = new char[len+9+1];
    strncpy(arguments.output, arguments.input, len);
    fn_ispc = new char[len+5+1];
    fn_dep  = new char[len+4+1];
    strncpy(fn_ispc, arguments.output, len);
    strncpy(fn_dep, arguments.output, len);
    strcpy(fn_ispc+len, ".ispc");
    strcpy(fn_dep+len, ".dep");
    printf("Output will be written to \"%s\" and \"%s\"\n", fn_ispc, fn_dep);
    state.out_ispc = fopen(fn_ispc, "w");
    state.out_dep  = fopen(fn_dep, "w");
    assert(state.out_ispc && state.out_dep);
	}
  else {
    fprintf(stderr, "Error: please specify an input file name using -i\n");
    exit(1);
  }
	pdg = yaml::Load<PDG>(in);
	assert (pdg);

  //state.verbose = true;
  loopo_initialize(pdg);
  print_loopo_ispc(pdg, state.out_ispc);
  print_loopo_dep(pdg, state.out_dep);

  fclose(in);
  fclose(state.out_ispc);
  fclose(state.out_dep);
  fprintf(stderr, "Error: this program is not yet functional; it needs to be implemented further before the output can be used.\n");
  return 0;
}
