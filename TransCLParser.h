/* pdgtrans Command Line Parser
 * Author: Wouter de Zwijger
 */

#include "SetDivider.h"

struct factorInfo {
  factorInfo(){}
  factorInfo(int dim, int offs, int fac) {dimension = dim; offset = offs; factor = fac;}
  int dimension;
  int offset;
  int factor;
};

// class that parses command line input
class TransCLParser {
  public:
    TransCLParser(pdg::PDG* pdgraph);
    ~TransCLParser();

    void parseAll(int argc, char** argv);
    int parseCommand(int argNr,int argc,char** argv);

  private:
    void domain_split_constraints(SplitCommand* command);
    void domain_split_sets(SplitCommand* command);
    void plane_split_constraints(SplitCommand* command);
    void plane_split_factors(SplitCommand* command);
    void modulo_split_factors(SplitCommand* command);
    void applyNextFactor(SetDivider* setSplitter, vector<factorInfo>* info,SplitCommand* command, int factorIndex);
    void divide_set_by_int_list(int* splitValues,int dim,__isl_take isl_set* set,vector<isl_set*>* setStorage);
    int* find_N_split_points(__isl_take isl_set* baseSet, int parts, int dim);
    int number_of_set_elements(__isl_take isl_set* set);
    static int countCard(isl_set *set, isl_qpolynomial *qp, void *user);
    int find_split_point_by_target(int targetLeft, isl_set* baseSet, int dim);
    bool split_set_on_int(__isl_take isl_set* originalSet, int splitInt, int dim, __isl_give isl_set*& firstPart, __isl_give isl_set*& secondPart);
    int finite_set_max(__isl_keep isl_set* set, int dim = 0);
    int finite_set_min(__isl_keep isl_set* set, int dim = 0);

    pdg::PDG* pdg;

};
