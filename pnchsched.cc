// pnchsched
// Takes a *pn.yaml file and changes the local node schedules
// NOTE: edge classification (e.g. reordering & size) is no longer valid!
//
// Sven van Haastregt, 2011
// LIACS, Leiden University

#include <iostream>
#include <vector>
#include "argp.h"

#include "version.h"

#include "yaml.h"
#include "pdg.h"

#include "isl/map.h"
#include "isl/dim.h"
#include "isl/set.h"

// Buffersize for reading
#define BUFSIZE 512

using pdg::PDG;
using namespace std;
//using namespace size;

struct argp_option argp_options[] = {
    { "input",            'i',    "file", 0 },
    { "version",          'V',        0,  0 },
    { "help",             'h',        0,  0 },
    { 0 }
};

//struct arguments : size_options {
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
      printf("%s\n", pdg_version());
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


// Parses .sched file
vector<vector<int> >* parseSchedFile(FILE *in) {
  vector<vector<int> > *ret = new vector<vector<int> >;
  
  // Read #stmt, depth, #params
  int stmts, depth, params;
  fscanf(in, "%d %d %d", &stmts, &depth, &params);

  assert(params==0);  // params not tested yet

  for (int i = 0; i < stmts; i++) {
    int nr;
    fscanf(in, "%d", &nr);
    assert(nr==i+1);  // statement schedules have to be given in increasing order

    bool doread = true;
    char buf[BUFSIZE];
    char *pos;
    // Dirty routine to extract a simple 1-D schedule, without any conditions etc.
    vector<int> nv;
    ret->push_back(nv);
    do {
      if (fgets(buf, BUFSIZE, in)) {
        if (strncmp(buf, "#", 1) == 0) {
          doread = false;
        }
        else if ((pos=strstr(buf, "list")) != NULL) {
          char *coeffs = pos+strlen("list #[ ");
          for (int j = 0; j < depth+1; j++) {
            int c = strtol(coeffs, &coeffs, 10);
            ret->back().push_back(c);
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
void insertSchedules(PDG *pdg, vector<vector<int> >* schedules) {
  for (unsigned int i = 0; i < schedules->size(); i++) {
    pdg::node *node = findNode(pdg, i);
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
    for (int ov = 0; ov < node->scattering->output; ov++) {
      newrow.push_back((*schedules)[i][ov]);    // Copy relevant output vars
    }
    newrow.push_back((*schedules)[i].back());   // Constant
    m->el.insert(m->el.begin(), newrow);
    node->scattering->input++;
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
  pdg = yaml::Load<PDG>(in);
  fclose(in);
  assert (pdg);

  vector<vector<int> > *schedules = parseSchedFile(insched);
  insertSchedules(pdg, schedules);
  pdg->dimension++;
  pdg->statement_dimensions.v.insert(pdg->statement_dimensions.v.begin(), 0);

  delete schedules;

  fclose(insched);
  pdg->add_history_line("pnchsched", argc, argv);
  pdg->Dump(out);
  pdg->free();
  delete pdg;

  return 0;
}
