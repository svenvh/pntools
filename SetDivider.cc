/* pdgtrans set divider
 * Author: Wouter de Zwijger
 */

#include "SetDivider.h"


//SetDivider

SetDivider::SetDivider(isl_set* set){
  baseSet = set;
  storedPdg = NULL;
}

SetDivider::SetDivider(pdg::PDG *pdg, int nodeNr){
  baseSet = pdg->nodes[nodeNr]->source->get_isl_set();
  storedNodeNr = nodeNr;
  storedPdg = pdg;
}



SetDivider::~SetDivider(){
  isl_set_free(baseSet);
  //ToDo: loop over vector and delete sub classes?
  // remove sub classes
  vector<SetSubset*>::iterator it;
  for(it = set_subsets.begin(); it < set_subsets.end(); it++)
    delete *it;
}

void SetDivider::add_set_subset(SetSubset* setS){
  set_subsets.push_back(setS);
}

// sets are stored in splitCommand class
void SetDivider::divide_node_by_sets(SplitCommand* setsContainer){
  if(storedPdg != NULL)
    divide_node_by_sets(storedPdg, storedNodeNr, setsContainer);
  else
    Error::stream << "cant call SetDivider::divide_node_by_sets(); pdg and nodeNr are not set." << endl;
}

// sets are stored in splitCommand class
void SetDivider::divide_node_by_sets(pdg::PDG* pdg, int nodeNr,SplitCommand* setsContainer){
  // create the sub sets
  pdg::node* targetNode = pdg->nodes[nodeNr];
  isl_set* next_set;
  pdg::node* newNode;

  setsContainer->reset_set_it();
  next_set = setsContainer->get_next_set();

  if(next_set == NULL){
    Error::stream << "no sets to split on defined!";
    return;
  }

  // change original node
  isl_set_free(targetNode->source->set);
  // check for matching spaces, if correct, apply set constraints
  if(!compareSetSpaces(baseSet,next_set)){
    Error::stream << "Set "<< next_set <<" does not match the space of node " << setsContainer->get_nodeNr() << endl;
    isl_space* tmpSpace = isl_set_get_space(baseSet);
    Error::stream << "space should be:" << tmpSpace << endl;

    // release resources
    isl_space_free(tmpSpace);
    targetNode->source->set = NULL;
    isl_set_free(next_set);
  } else
    targetNode->source->set = isl_set_intersect(isl_set_copy(baseSet),next_set);

  // add new nodes
  while((next_set = setsContainer->get_next_set())!= NULL){
    newNode = procedure_copy_node(targetNode,false);
    // check for matching spaces, if correct, apply set constraints
    if(!compareSetSpaces(baseSet,next_set)){
      Error::stream << "Set "<< next_set <<" does not match the space of node " << setsContainer->get_nodeNr() << endl;
      isl_space* tmpSpace = isl_set_get_space(baseSet);
      Error::stream << "space should be:" << tmpSpace << endl;

      // release resources
      isl_space_free(tmpSpace);
      newNode->source = NULL;
      isl_set_free(next_set);
    } else
      newNode->source = new pdg::IslSet(isl_set_intersect(isl_set_copy(baseSet),next_set));

    newNode->nr = pdg->nodes[pdg->nodes.size()-1]->nr + 1;
    pdg->nodes.push_back(newNode);
  }

}

void SetDivider::divide_node_by_subsets() {
  if(storedPdg != NULL)
    divide_node_by_subsets(storedPdg, storedNodeNr);
  else
    Error::stream << "Cant call SetDivider::divide_node_by_subsets(); pdg and nodeNr are not set." << endl;
}

// split given node in N individual nodes, where the resulting sets of the nodes is split on set_subsets.
void SetDivider::divide_node_by_subsets(pdg::PDG *pdg, int nodeNr) {
  // create the sub sets
  pdg::node* targetNode = pdg->nodes[nodeNr];
  isl_set** setArray;
  pdg::node* newNode;
  int numberOfSets = set_subsets.size();
  // ToDo: set that it is applied on is set on construction, maybe not? or node at construction?
  setArray = split_set();

  // change original node
  isl_set_free(targetNode->source->set);
  targetNode->source->set = setArray[0];

  // add new nodes
  for(int x = 1;x < numberOfSets; x ++) {
    newNode = procedure_copy_node(targetNode,false);
    newNode->source = new pdg::IslSet(setArray[x]);
    newNode->nr = pdg->nodes[pdg->nodes.size()-1]->nr + 1;
    pdg->nodes.push_back(newNode);
  }

  delete[] setArray;

}

void SetDivider::divide_node_by_constraints(SplitCommand* setsContainer) {
  if(storedPdg != NULL)
    divide_node_by_constraints(storedPdg, storedNodeNr, setsContainer);
  else
    Error::stream << "cant call SetDivider::divide_node_by_constraints(); pdg and nodeNr are not set." << endl;
}

// split given node in N individual nodes, where the resulting sets of the nodes is split on set_subsets.
void SetDivider::divide_node_by_constraints(pdg::PDG* pdg, int nodeNr,SplitCommand* setsContainer) {
  // create the sub sets
  pdg::node* targetNode = pdg->nodes[nodeNr];
  isl_set** setArray;
  pdg::node* newNode;
  int numberOfSets;

  setArray = split_set_by_constraints(setsContainer,numberOfSets);

  // change original node
  isl_set_free(targetNode->source->set);
  targetNode->source->set = setArray[0];

  // add new nodes
  for(int x = 1;x < numberOfSets; x ++) {
    newNode = procedure_copy_node(targetNode,false);
    newNode->source = new pdg::IslSet(setArray[x]);
    newNode->nr = pdg->nodes[pdg->nodes.size()-1]->nr + 1;
    pdg->nodes.push_back(newNode);
  }

  delete[] setArray;
}

void SetDivider::divide_node_by_vector_of_sets(vector<isl_set*>& sets) {
  if(storedPdg != NULL)
    divide_node_by_vector_of_sets(storedPdg, storedNodeNr, sets);
  else
    cerr << "cant call SetDivider::divide_node_by_vector_of_sets(); pdg and nodeNr are not set." << endl;
}

void SetDivider::divide_node_by_vector_of_sets(pdg::PDG* pdg, int nodeNr,vector<isl_set*>& sets){
  assert(!sets.empty());
  pdg::node* targetNode = pdg->nodes[nodeNr];
  pdg::node* newNode;
  vector<isl_set*>::iterator set_it;
  set_it = sets.begin();

  // change original node
  isl_set_free(targetNode->source->set);
  targetNode->source->set = *set_it;

  // add new nodes
  for(set_it++;set_it < sets.end();set_it++) {
    newNode = procedure_copy_node(targetNode,false);
    newNode->source = new pdg::IslSet(*set_it);
    newNode->nr = pdg->nodes[pdg->nodes.size()-1]->nr + 1;
    pdg->nodes.push_back(newNode);
  }



}

// returns list of sets, based on baseSet but with the conditions added in set_conditions.
isl_set** SetDivider::split_set() {
  isl_set** resultingSets = new isl_set*[set_subsets.size()];

  isl_set* tmpSet;
  int arrayLocation,constraintDim;
  int setDim = isl_set_dim(baseSet,isl_dim_out);

  vector<SetSubset*>::iterator condition_it;
  vector<isl_constraint*>::iterator constraint_it;

  arrayLocation = 0;

  // loop over all conditions, each condition will create one set based on the baseSet
  for( condition_it = set_subsets.begin(); condition_it < set_subsets.end(); condition_it++) {
    tmpSet = isl_set_copy(baseSet);

    // loop over all constrains(== SetSubsets) of current condition
    for(constraint_it = (*condition_it)->constraints.begin(); constraint_it < (*condition_it)->constraints.end(); constraint_it++) {

      // if constraint has more dimenstions than set, extend
      constraintDim = isl_constraint_dim(*constraint_it,isl_dim_out);

      if(constraintDim > setDim){
        tmpSet = isl_set_insert_dims(tmpSet, isl_dim_set, setDim, constraintDim - setDim); // -1?
      }

      // add constraint to set
      tmpSet = isl_set_add_constraint(tmpSet, isl_constraint_copy(*constraint_it));

      // if set was extended, shrink here!
      if(constraintDim > setDim){
        tmpSet = isl_set_project_out(tmpSet, isl_dim_set, setDim, constraintDim - setDim);
        tmpSet = isl_set_set_tuple_name(tmpSet, isl_set_get_tuple_name(baseSet));
      }

    } // for

    // store created subset
    resultingSets[arrayLocation++] = tmpSet;

  } // for

  return resultingSets;
}

// returns list of sets, based on baseSet but with the conditions added in set_conditions.
isl_set** SetDivider::split_set_by_constraints(SplitCommand* setsContainer,int& arraySize) {
  isl_set** resultingSets = new isl_set*[setsContainer->constraints_size()];

  isl_set* tmpSet = NULL;
  int arrayLocation, setDim, constraintDim;
  isl_constraint* tmpConstraint;

  arrayLocation = 0;
  setsContainer->reset_constraint_it();

  // loop over all conditions, each condition will create one set based on the baseSet
  for( tmpConstraint = setsContainer->get_next_constraint(); tmpConstraint != NULL; tmpConstraint = setsContainer->get_next_constraint() ) {
    if(tmpSet == NULL) {
       tmpSet = isl_set_copy(baseSet);
    } else if(setsContainer->constraint_is_new_sequence()){
      TransDebug::print(":");
      resultingSets[arrayLocation++] = tmpSet;
      TransDebug::print("\n");
      TransDebug::set_dump(tmpSet);
      tmpSet = isl_set_copy(baseSet);
      TransDebug::print(":");
    }
    // ToDo: in case constraint space > set space, project out tmp_constraint

    // if constraint has more dimenstions than set, extend
    setDim = isl_set_dim(baseSet,isl_dim_out);
    constraintDim = isl_constraint_dim(tmpConstraint,isl_dim_out);

    if(constraintDim > setDim){
      tmpSet = isl_set_insert_dims(tmpSet, isl_dim_set, setDim, constraintDim - setDim); // -1?
    }

    tmpSet = isl_set_add_constraint(tmpSet, tmpConstraint);

    // if set was extended, shrink here!
    if(constraintDim > setDim){
      tmpSet = isl_set_project_out(tmpSet, isl_dim_set, setDim, constraintDim - setDim);
      tmpSet = isl_set_set_tuple_name(tmpSet, isl_set_get_tuple_name(baseSet));
    }

  } // for

  resultingSets[arrayLocation++] = tmpSet;

  arraySize = arrayLocation;
  return resultingSets;
}

// Recursively copies a function call (tree); also produces a list of accesses
pdg::function_call *SetDivider::copy_pdg_function_call(pdg::function_call *orig, yaml::seq<pdg::access> &accesses) {
  pdg::function_call *newCall = new pdg::function_call();
  newCall->name = new yaml::str(orig->name->s);

  for (int a = 0; a < orig->arguments.size(); a++) {
    pdg::call_or_access *newArg = new pdg::call_or_access();
    newArg->type = orig->arguments[a]->type;
    if (newArg->type == pdg::call_or_access::t_access) {
      newArg->access = new pdg::access();
      newArg->access->array = orig->arguments[a]->access->array;
      newArg->access->type = orig->arguments[a]->access->type;
      newArg->access->nr = orig->arguments[a]->access->nr;
      newArg->access->extension = orig->arguments[a]->access->extension;
      newArg->access->extended_map = orig->arguments[a]->access->extended_map;
      newArg->access->nested = orig->arguments[a]->access->nested;
      newArg->access->map = new pdg::IslMap (orig->arguments[a]->access->map->get_isl_map());
      accesses.push_back(newArg->access);
    }
    else if (newArg->type == pdg::call_or_access::t_call) {
      newArg->call = copy_pdg_function_call(orig->arguments[a]->call, accesses);
    }
    else {
      assert(0); // only calls and accesses are implemented
    }
    newCall->arguments.push_back(newArg);
  }
  return newCall;
}

pdg::node* SetDivider::procedure_copy_node(pdg::node* sourceNode, bool inclSet) {
  pdg::node* newNode = new pdg::node();
  pdg::access* tmpAccess;
  pdg::call_or_access* tmpArgument;

  if(inclSet)
    newNode->source = new pdg::IslSet(sourceNode->source->set);

  newNode->prefix = sourceNode->prefix;
  if(sourceNode->statement != NULL) {
    // copy sequences of access
    pdg::statement* newStatement = new pdg::statement();

    // copy top_function
    newStatement->top_function = copy_pdg_function_call(sourceNode->statement->top_function, newStatement->accesses);

    // copy other properties of statement
    newStatement->operation = sourceNode->statement->operation;
    newStatement->line = sourceNode->statement->line;
    newStatement->top_outputs = sourceNode->statement->top_outputs;
    newNode->statement = newStatement;
  } else {
    newNode->statement = sourceNode->statement;
  }

  //newNode->nr = pdg->nodes[pdg->nodes.size()-1]->nr + 1;
  newNode->nr = sourceNode->nr;

  return newNode;
}

// add subset without anny constraints
void SetDivider::add_subset(){
  isl_space* sourceSpace = isl_set_get_space(baseSet);
  isl_local_space *lSpace = isl_local_space_from_space(sourceSpace);
  set_subsets.push_back(new SetSubset(lSpace));
}

// add given constraint to current subset
void SetDivider::add_subset_constraint(isl_constraint* constraint){
  if(set_subsets.size() != 0)
    set_subsets[set_subsets.size()-1]->add_set_constraint(constraint);
}



// add (empty) constraint to current subset
void SetDivider::add_subset_constraint(bool is_inequality){
  if(set_subsets.size() != 0)
    set_subsets[set_subsets.size()-1]->add_constraint(is_inequality);
}

void SetDivider::set_subset_dim_value(int dim,int value){
  if(set_subsets.size() != 0)
    set_subsets[set_subsets.size()-1]->set_dim_value(dim,value);
}

void SetDivider::set_subset_constant(int value){
  if(set_subsets.size() != 0)
   set_subsets[set_subsets.size()-1]->set_constant(value);
}

// compares the space of two sets. If equal returns 1, else 0 ////////////
int SetDivider::compareSetSpaces(isl_set* set1,isl_set* set2){
  isl_space* space1 = isl_set_get_space(set1);
  isl_space* space2 = isl_set_get_space(set2);
  int result = isl_space_is_equal(space1,space2);
  isl_space_free(space1);
  isl_space_free(space2);
  return result;
}

// SetSubset

SetSubset::SetSubset(isl_local_space* __isl_take space){
  lSpace = space;
}

SetSubset::~SetSubset(){
  // ToDo: delete objects constrains<> is pointing at
  // remove sub classes
  vector<isl_constraint*>::iterator it;
  for(it = constraints.begin(); it < constraints.end(); it++)
    isl_constraint_free(*it);

  isl_local_space_free(lSpace);
}

void SetSubset::add_set_constraint(isl_constraint* constraint){
  constraints.push_back(constraint);
}

void SetSubset::add_constraint(bool is_inequality){
  isl_constraint* c;
  if(is_inequality)
    c = isl_inequality_alloc(isl_local_space_copy(lSpace));
  else
    c = isl_equality_alloc(isl_local_space_copy(lSpace));

  constraints.push_back(c);
}

void SetSubset::set_dim_value(int dim,int value){
  if(constraints.size() != 0){
    int constraintDim = isl_constraint_dim(constraints[constraints.size()-1],isl_dim_out);

    // check if requested dimension exists (if not: increase dimension size)
    if(constraintDim < dim + 1){
      //constraints[constraints.size()-1] = isl_constraint_insert_dims(constraints[constraints.size()-1], isl_dim_set, constraintDim, dim - constraintDim); // -1?
      constraints[constraints.size()-1] = constraint_insert_dims(constraints[constraints.size()-1], constraintDim, dim + 1 - constraintDim); // -1?
    }

    // add dimension creteria
    isl_constraint_set_coefficient_si(constraints[constraints.size()-1], isl_dim_set, dim, value);

    // debug info
    TransDebug::stream << "set {" << dim << "/" << constraintDim << " value:  " << value << "}";
    TransDebug::constraint_dump(constraints[constraints.size()-1]);

  }
}

void SetSubset::set_constant(int value){
  if(constraints.size() != 0)
    isl_constraint_set_constant_si(constraints[constraints.size()-1], value);
}


// does what isl_constraint_insert_dims should do if existed
__isl_give isl_constraint* SetSubset::constraint_insert_dims(__isl_take isl_constraint* originalConstraint, int fromDim, int numberOfDimsToAdd){

  isl_constraint* newConstraint;
  isl_int v;
  isl_int_init(v);
  int constraintDim = isl_constraint_dim(originalConstraint,isl_dim_out);

  // create space for bigger constraint
  isl_space *newSpace = isl_constraint_get_space(originalConstraint);
  newSpace = isl_space_insert_dims(newSpace,isl_dim_set, fromDim, numberOfDimsToAdd);

  if(isl_constraint_is_equality(originalConstraint)){
    newConstraint = isl_equality_alloc(isl_local_space_from_space(newSpace));
  } else {
    newConstraint = isl_inequality_alloc(isl_local_space_from_space(newSpace));
  }

  // copy old values
  isl_constraint_get_constant(originalConstraint,&v);
  isl_constraint_set_constant(newConstraint,v);

  for(int d = 0; d < constraintDim; d++){
    isl_constraint_get_coefficient(originalConstraint,isl_dim_set,d,&v);
    isl_constraint_set_coefficient(newConstraint,isl_dim_set,d,v);
  }

  isl_constraint_free(originalConstraint);
  return newConstraint;
}
