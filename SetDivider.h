/* pdgtrans set divider
 * Author: Wouter de Zwijger
 */

#include "SplitCommand.h"

class SetSubset;

class SetDivider {
  public:
    SetDivider(isl_set* __isl_take set);
    SetDivider(pdg::PDG* pdg, int nodeNr);
    ~SetDivider();

    void add_set_subset(SetSubset* setC);

    void add_subset();
    // following functions apply on last added condition
    void add_subset_constraint(isl_constraint* constraint);
    void add_subset_constraint(bool is_inequality = true);
    void set_subset_dim_value(int dim,int value);
    void set_subset_constant(int value);

    // using the intern stored subset constraints
    void divide_node_by_subsets(pdg::PDG* pdg, int nodeNr);
    void divide_node_by_subsets();
    // using the information stored in a SplitCommand class
    void divide_node_by_sets(pdg::PDG* pdg, int nodeNr,SplitCommand* setsContainer);
    void divide_node_by_sets(SplitCommand* setsContainer);
    void divide_node_by_constraints(pdg::PDG* pdg, int nodeNr,SplitCommand* setsContainer);
    void divide_node_by_constraints(SplitCommand* setsContainer);
    void divide_node_by_vector_of_sets(vector<isl_set*>& sets);
    void divide_node_by_vector_of_sets(pdg::PDG* pdg, int nodeNr,vector<isl_set*>& sets);

    isl_set** split_set();

    private:
      pdg::function_call *copy_pdg_function_call(pdg::function_call *orig, yaml::seq<pdg::access> &accesses);
      pdg::node* procedure_copy_node(pdg::node* sourceNode, bool inclSet = false);
      isl_set** split_set_by_constraints(SplitCommand* setsContainer,int& arraySize);
      int compareSetSpaces(isl_set* set1,isl_set* set2);

      vector<SetSubset*> set_subsets;
      isl_set* baseSet;

      int storedNodeNr;
      pdg::PDG* storedPdg;


};

class SetSubset{
  public:
    SetSubset(isl_local_space* __isl_take space);
    ~SetSubset();
    void add_set_constraint(isl_constraint* constraint);

    void add_constraint(bool is_inequality = true);
    // following functions apply on last added constraint
    void set_dim_value(int dim,int value);
    void set_constant(int value);

  private:
    friend isl_set** SetDivider::split_set();
    isl_constraint* constraint_insert_dims(__isl_take isl_constraint* originalConstraint, int fromDim, int numberOfDimsToAdd);
    vector<isl_constraint*> constraints;
    isl_local_space* lSpace;
};
