// pnchsched
// Takes a *pn.yaml file and changes the local node schedules
// NOTE: edge classification (e.g. reordering & size) is no longer valid!
//
// Sven van Haastregt, 2011
// LIACS, Leiden University

#include <iostream>
#include <vector>
#include <map>
#include "argp.h"

#include "version.h"

#include "isl/map.h"
#include "isl/set.h"
#include "isa/yaml.h"
#include "isa/pdg.h"

// Buffersize for reading
#define BUFSIZE 512

using pdg::PDG;
using namespace std;

struct argp_option argp_options[] = {
    { "input",            'i',    "file", 0 },
    { "version",          'V',        0,  0 },
    { "help",             'h',        0,  0 },
    { 0 }
};

struct arguments  {
    char *input;
};

struct arguments options;

void print_usage() {
  fprintf(stderr, "pnchsched -- change pn schedule\n");
  fprintf(stderr, "Usage: pnchsched -i file_pn.yaml > new_pn.yaml\n");
  fprintf(stderr, "This tool inserts the schedule in file_pn.sched into a PDG.\n");
  fprintf(stderr, "The updated PDG is written to stdout.\n");
  fprintf(stderr, "The .sched file follows the syntax of LooPo .sched files.\n");
}

error_t parse_opt(int key, char *arg, struct argp_state *state)
{
  struct arguments *arguments = (struct arguments *)(state->input);

  switch (key) {
    case ARGP_KEY_INIT:
      arguments->input = NULL;
      break;
    case 'V':
      //printf("%s\n", pdg_version());
      exit(0);
    case 'i':
      arguments->input = arg;
      break;
    case 'h':
      print_usage();
      exit(0);
    default:
      return ARGP_ERR_UNKNOWN;
  }
  return 0;
}


/* For any two pairs of corresponding writes and reads,
 * do the writes occur in a different order than the reads ?
 * If so, then the dependence is "reordering".
 */
int isl_map_is_reordering(struct isl_map *dep)
{
  isl_map *read_before;
  isl_map *write_after;
  int r;

  read_before = isl_map_lex_lt(isl_dim_range(isl_map_get_dim(dep)));
  write_after = isl_map_lex_gt(isl_dim_domain(isl_map_get_dim(dep)));
  write_after = isl_map_apply_domain(write_after, isl_map_copy(dep));
  write_after = isl_map_apply_range(write_after, isl_map_copy(dep));
  write_after = isl_map_intersect(write_after, read_before);

  r = !isl_map_is_empty(write_after);

  isl_map_free(write_after);

  return r;
}


// Parses .sched file
map<int, vector<int> >* parseSchedFile(FILE *in) {
  map<int, vector<int> > *ret = new map<int, vector<int> >;
  
  // Read #stmt, depth, #params
  int stmts, depth, params;
  fscanf(in, "%d %d %d", &stmts, &depth, &params);

  assert(params==0);  // params not tested yet

  for (int i = 0; i < stmts; i++) {
    int nr;
    fscanf(in, "%d", &nr);
//    assert(nr==i+1);  // statement schedules have to be given in increasing order

    bool doread = true;
    char buf[BUFSIZE];
    char *pos;
    // Dirty routine to extract a simple 1-D schedule, without any conditions etc.
    vector<int> nv;
    (*ret)[nr] = nv;
    do {
      if (fgets(buf, BUFSIZE, in)) {
        if (strncmp(buf, "#", 1) == 0) {
          doread = false;
        }
        else if ((pos=strstr(buf, "list")) != NULL) {
          char *coeffs = pos+strlen("list #[ ");
          for (int j = 0; j < depth+1; j++) {
            int c = strtol(coeffs, &coeffs, 10);
            (*ret)[nr].push_back(c);
          }
        }
        else {
          // ignore
        }
      }
    } while (doread);

  }

  return ret;
}


pdg::node* findNode(PDG *pdg, int nr) {
  for (unsigned int i = 0; i < pdg->nodes.size(); i++) {
    pdg::node *node = pdg->nodes[i];
    if (node->nr == nr)
      return node;
  }
  return NULL;
}


// Extends each node's scattering matrix with schedules
void insertSchedules(PDG *pdg, map<int, vector<int> >* schedules) {
  for (unsigned int i = 0; i < pdg->nodes.size(); i++) {
    pdg::node *node = pdg->nodes[i];
    assert(node->scattering->constraints.size() == 1);   // >1 constraint sets not implemented

    // Insert new input dim
    pdg::Matrix *m = node->scattering->constraints[0];
    for (unsigned int si = 0; si < m->el.size(); si++) {
      m->el[si].insert(m->el[si].begin()+1, 0);
    }

    vector<int> newrow;
    newrow.push_back(0);                        // Sign (equality)
    newrow.push_back(-1);                       // First dimension
    for (int iv = 0; iv < node->scattering->input; iv++) {
      newrow.push_back(0);                      // Zero out other input vars
    }
    node->scattering->input++;

    if (schedules->find(i) != schedules->end()) {
      // There is a schedule
      for (int ov = 0; ov < node->scattering->output; ov++) {
        newrow.push_back((*schedules)[i][ov]);    // Copy relevant output vars
      }
      newrow.push_back((*schedules)[i].back());   // Constant
    }
    else {
      // No schedule, insert 0's
      for (int ov = 0; ov < node->scattering->output+1; ov++) {
        newrow.push_back(0);
      }
    }
    m->el.insert(m->el.begin(), newrow);
  }
}


void analyzeChannels(PDG *pdg) {
  isl_ctx *ctx = pdg->get_isl_ctx();

  for (unsigned int i = 0; i < pdg->dependences.size(); i++) {
    pdg::dependence *dep = pdg->dependences[i];

    isl_map *dep_map = dep->relation->get_isl_map(pdg->get_isl_ctx());
    int was_reordering = isl_map_is_reordering(dep_map);
//    fprintf(stderr, "Edge %d: isreord? %d\n", i, isl_map_is_reordering(dep_map));
    isl_map *srcSched = dep->from->scattering->get_isl_map(pdg->get_isl_ctx());
    isl_map *dstSched = dep->to->scattering->get_isl_map(pdg->get_isl_ctx());
//    fprintf(stderr, "Src sched: ");
//    isl_map_dump(srcSched);
//    fprintf(stderr, "Dst sched: ");
//    isl_map_dump(dstSched);

    isl_map *sched = dep_map;
//    fprintf(stderr, "Before:    ");
//    isl_map_dump(sched);

    sched = isl_map_apply_range(sched, isl_map_reverse(dstSched));
    sched = isl_map_apply_domain(sched, isl_map_reverse(srcSched));
//    fprintf(stderr, "After:     ");
//    isl_map_dump(sched);

    fprintf(stderr, "Edge %2d: was / is reordering?  %d  %d\n", i, was_reordering, isl_map_is_reordering(sched));
    dep->reordering = isl_map_is_reordering(sched);

    isl_map_free(sched);
  }
}


void addDimensionToAccesses(pdg::node *node) {
  for (unsigned int j = 0; j < node->statement->accesses.size(); j++) {
    assert(node->statement->accesses[j]->map->constraints.size() == 1);
    pdg::Matrix *m = node->statement->accesses[j]->map->constraints[0];
    for (unsigned int si = 0; si < m->el.size(); si++) {
      m->el[si].insert(m->el[si].begin()+1, 0);
    }
    node->statement->accesses[j]->map->input++;
  }
  node->prefix.v.insert(node->prefix.v.begin(), -1);
  node->prefix.v.insert(node->prefix.v.begin(), 1);
}


void applyScheduleToNode(pdg::node *node, vector<int> schedule) {
  assert(node->source->constraints.size() == 1);   // >1 constraint sets not implemented

  // Insert new input dim
  pdg::Matrix *m = node->source->constraints[0];
  for (unsigned int si = 0; si < m->el.size(); si++) {
    m->el[si].insert(m->el[si].begin()+1, 0);
  }

  vector<int> newrow;
  newrow.push_back(0);                        // Sign (equality)
  newrow.push_back(-1);                       // First dimension
  for (int ov = 0; ov < node->source->dim; ov++) {
    newrow.push_back(schedule[ov]);    // Copy relevant output vars
  }
  newrow.push_back(schedule.back());   // Constant
  m->el.insert(m->el.begin(), newrow);
  node->source->dim++;
  addDimensionToAccesses(node);
}


void applySchedulesToDomains(PDG *pdg, map<int, vector<int> >* schedules) {
  for (unsigned int i = 0; i < pdg->nodes.size(); i++) {
//    pdg::node *node = findNode(pdg, i);
    pdg::node *node = pdg->nodes[i];
    applyScheduleToNode(node, (*schedules)[i]);
  }
}


void dumpDomains(PDG *pdg) {
  for (unsigned int i = 0; i < pdg->nodes.size(); i++) {
    pdg::node *node = pdg->nodes[i];
    pdg::Matrix *m = node->source->constraints[0];
    printf("# %d (%s)\n", node->nr, node->statement->top_function->name->s.c_str());
    printf("%d %d\n", m->el.size(), m->el[0].size());
    for (unsigned int si = 0; si < m->el.size(); si++) {
      for (unsigned int sj = 0; sj < m->el[si].size(); sj++) {
        printf("%3d ", m->el[si][sj]);
      }
      printf("\n");
    }
    printf("\n");

  }
}


int main(int argc, char * argv[])
{
  FILE *in = NULL, *out = stdout, *insched = NULL;
  char *fn_sched;

  PDG *pdg;
  static struct argp argps = { argp_options, parse_opt, 0, 0, NULL };

  argp_parse(&argps, argc, argv, 0, 0, &options);

  if (options.input && strcmp(options.input, "-")) {
    in = fopen(options.input, "r");
    assert(in);
    int len = strlen(options.input);
    if (len > 5 && !strcmp(options.input+len-5, ".yaml"))
      len -= 5;
    fn_sched = new char[len+6+1];
    strncpy(fn_sched, options.input, len);
    strcpy(fn_sched+len, ".sched");
    insched = fopen(fn_sched, "r");
    if (!insched) {
      fprintf(stderr, "Error: could not open schedule file \"%s\"\n", fn_sched);
      exit(1);
    }
  }
  else {
    fprintf(stderr, "Error: please specify an input file name using -i\n");
    exit(1);
  }
  isl_ctx *ctx = isl_ctx_alloc();
//  pdg = PDG::Load(in, ctx);
  pdg = yaml::Load<PDG>(in);
  
  fclose(in);
  assert (pdg);

  map<int, vector<int> > *schedules = parseSchedFile(insched);
  if (pdg->dependences.size() > 0) {
    fprintf(stderr, "Changing the scattering matrices...\n");
    insertSchedules(pdg, schedules);
    pdg->dimension++;
    pdg->statement_dimensions.v.insert(pdg->statement_dimensions.v.begin(), 0);
    fprintf(stderr, "Reclassifying communication order...\n");
    analyzeChannels(pdg);
    fprintf(stderr, "Warning: other edge properties (size, multiplicity) are no longer valid!\n");
  }
  else {
    // Put schedules into .yaml file
    fprintf(stderr, "Applying schedules to node domains...\n");
    applySchedulesToDomains(pdg, schedules);
    //dumpDomains(pdg);
  }

  delete schedules;

  fclose(insched);
  pdg->add_history_line("pnchsched", argc, argv);
  pdg->Dump(out);

  pdg->free();
  delete pdg;
  isl_ctx_free(ctx);

  return 0;
}
