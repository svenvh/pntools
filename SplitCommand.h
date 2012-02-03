/* Used by pdgtrans to describe a single transformation.
 * Author: Wouter de Zwijger
 */

#include <vector>
#include "TransDebug.h"

class SplitCommand {
  public:
    enum splitType {invalid, plane_split, modulo_split, domain_split, debug_mode};
    enum constraintType {unknown, constraint_conditions, constraint_hyperplanes, constraint_sets, constraint_factors};

    SplitCommand(pdg::PDG* pdgraph);
    ~SplitCommand();
    int parseCommand(int argNr,int argc,char** argv);

    splitType get_type();
    constraintType get_c_type();
    int get_nodeNr();
    isl_constraint* get_next_constraint();
    bool constraint_is_new_sequence();
    void reset_constraint_it();
    int constraints_size();
    isl_set* get_next_set();
    void reset_set_it();
    int factors_size();
    int get_next_factor();
    int get_factor(int index);
    int get_factor_size();
    void reset_factor_it();


  private:
    int nodeNr;
    splitType method;
    constraintType conditionType;
    vector<isl_constraint*> constraints;
    vector<isl_constraint*>::iterator constraint_it;
    bool newSeq;
    vector<isl_set*> sets;
    vector<isl_set*>::iterator set_it;
    vector<int> factors;
    vector<int>::iterator factor_it;

    pdg::PDG* pdg;

    bool parseNodeNr(int &argNr,int argc,char **argv);
    bool parseConstraintType(int &argNr,int argc,char **argv);
    bool parseConstraintList(int &argNr,int argc,char **argv);
    bool parseConstraintSets(int &argNr,int argc,char **argv);
    bool parseConstraintFactors(int &argNr,int argc,char **argv);
    int modify_local_space(isl_local_space*& lSpace,char* command,int fromHere);
};
