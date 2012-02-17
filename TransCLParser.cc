/* pdgtrans Command Line Parser
 * Author: Wouter de Zwijger
 */

#include "TransCLParser.h"

using pdg::PDG;

TransCLParser::TransCLParser(pdg::PDG* pdgraph){
  pdg = pdgraph;
}

TransCLParser::~TransCLParser(){

}

void TransCLParser::parseAll(int argc, char** argv){
  // execute all given arguments
  int argumentNr = 1;
  while(argumentNr < argc && !TransError::isFound())
    argumentNr = parseCommand(argumentNr,argc, argv);
}

int TransCLParser::parseCommand(int argNr,int argc,char** argv){
  SplitCommand* commandReader = new SplitCommand(pdg);
  argNr = commandReader->parseCommand(argNr,argc,argv);

  switch(commandReader->get_type()){
    case SplitCommand::domain_split:
      if(commandReader->get_c_type() == SplitCommand::constraint_conditions)
        domain_split_constraints(commandReader);
      else if(commandReader->get_c_type() == SplitCommand::constraint_sets)
        domain_split_sets(commandReader);
      else
        TransError::stream << "unknown constraint type given";
      break;
    case SplitCommand::plane_split:
      if(commandReader->get_c_type() == SplitCommand::constraint_conditions)
        plane_split_constraints(commandReader);
      else if(commandReader->get_c_type() == SplitCommand::constraint_factors)
        plane_split_factors(commandReader);
      else
        TransError::stream << "unknown constraint type given, plane-split requires --conditions or --factors";
      break;
    case SplitCommand::modulo_split:
      if(commandReader->get_c_type() == SplitCommand::constraint_factors)
        modulo_split_factors(commandReader);
      else
        TransError::stream << "The --modulo-split method currently does not support this constraint type. Currently supported: --factors";
      break;
    case SplitCommand::debug_mode:
      TransDebug::setDebugMode(true);
      TransDebug::print("debug mode is on!\n");
      break;
    default:
      TransError::stream << endl << "Undefined command!";
      break;
  }

  delete commandReader;

  // return new arg number
  return argNr;
}

void TransCLParser::domain_split_constraints(SplitCommand* command){
  // create SetDivider
  SetDivider setSplitter(pdg,command->get_nodeNr());

  // execute
  setSplitter.divide_node_by_constraints(command);
}

void TransCLParser::domain_split_sets(SplitCommand* command){
  // create SetDivider
  SetDivider setSplitter(pdg,command->get_nodeNr());

  // apply split by data inside SplitCommand class
  setSplitter.divide_node_by_sets(command);
}

void TransCLParser::plane_split_constraints(SplitCommand* command){
  TransDebug::print("\nplane split constraints entered\n");
  // create SetDivider
  SetDivider setSplitter(pdg,command->get_nodeNr());

  // tmp, as long as test is called before this function...
  command->reset_constraint_it();

  // add constrains
  isl_constraint* biggerC = NULL;
  isl_constraint* smallerC, *tmpC;
  isl_space* tmpSpace;
  isl_int v;
  isl_int_init(v);
  while( (smallerC = command->get_next_constraint()) != NULL){
    //if(command->constraint_is_new_sequence()) //Always new subset, , and ; are equal here
      setSplitter.add_subset();


    // add maximal condition
    // create space, inequality
    tmpSpace = isl_constraint_get_space(smallerC);
    tmpC = isl_inequality_alloc(isl_local_space_from_space(tmpSpace));

    // copy values
    isl_constraint_get_constant(smallerC,&v);
    // ToDo: multiply by -1 and add -1
    isl_int_neg(v,v);
    isl_int_sub_ui(v,v,1);
    isl_constraint_set_constant(tmpC,v);

    for(int dim =0; dim < isl_constraint_dim(smallerC,isl_dim_out); dim++){
      isl_constraint_get_coefficient(smallerC,isl_dim_set,dim,&v);
      isl_int_neg(v,v);
      // ToDo: multiply by -1
      isl_constraint_set_coefficient(tmpC,isl_dim_set,dim,v);
    }

    // add to splitter class
    TransDebug::print("maximal constraint");
    TransDebug::constraint_dump(tmpC);
    setSplitter.add_subset_constraint(tmpC);


    // add minimal constraint. (not for first part)
    if(biggerC != NULL){
      // create space, inequality
      tmpSpace = isl_constraint_get_space(biggerC);
      tmpC = isl_inequality_alloc(isl_local_space_from_space(tmpSpace));

      // copy values
      isl_constraint_get_constant(biggerC,&v);
      isl_constraint_set_constant(tmpC,v);

      for(int dim =0; dim < isl_constraint_dim(biggerC,isl_dim_out); dim++){
        isl_constraint_get_coefficient(biggerC,isl_dim_set,dim,&v);
        isl_constraint_set_coefficient(tmpC,isl_dim_set,dim,v);
      }

      // add to splitter class
      setSplitter.add_subset_constraint(tmpC);
      TransDebug::print("minimal constraint");
      TransDebug::constraint_dump(tmpC);
    }

    if(biggerC != NULL)
      isl_constraint_free(biggerC);

    biggerC = smallerC;
  }  // while
  // Only bigger than (last part)

  // add subset
  setSplitter.add_subset();

  // create space, inequality
  tmpSpace = isl_constraint_get_space(biggerC);
  tmpC = isl_inequality_alloc(isl_local_space_from_space(tmpSpace));

  // copy values
  isl_constraint_get_constant(biggerC,&v);
  isl_constraint_set_constant(tmpC,v);

  for(int dim =0; dim < isl_constraint_dim(biggerC,isl_dim_out); dim++){
    isl_constraint_get_coefficient(biggerC,isl_dim_set,dim,&v);
    isl_constraint_set_coefficient(tmpC,isl_dim_set,dim,v);
  }

  // add to splitter class
  setSplitter.add_subset_constraint(tmpC);
  TransDebug::stream << "minimal constraint";
  TransDebug::constraint_dump(tmpC);

  // free resources
  isl_constraint_free(biggerC);
  isl_int_clear(v);

  // execute splitting
  setSplitter.divide_node_by_subsets();
}

void TransCLParser::plane_split_factors(SplitCommand* command){
TransDebug::print("\nplane split constraints entered\n");
  int* splitValues;
  vector<isl_set*> *sets,*newSets;
  vector<isl_set*>::iterator set_it;
  sets = new vector<isl_set*>();
  newSets = new vector<isl_set*>();

  // store original set
  sets->push_back(pdg->nodes[command->get_nodeNr()]->source->get_isl_set());

  // call some split method

  // iterate over each factor
  for(int fI = 0; fI < command->get_factor_size();fI++){
    TransDebug::print("\n1st loop");
    // skip dimensions that dont require to split over
    if(command->get_factor(fI) < 2)
      continue;

    TransDebug::print("\ncont loop 1");
    // apply split method on all available sets
    for(set_it = sets->begin();set_it < sets->end(); set_it++){
      TransDebug::print("\n2nd loop");
      TransDebug::stream << "\nsize newSets" << newSets->size() << "\n";
      // create int array with values where current dimension should be split
      splitValues = find_N_split_points(isl_set_copy(*set_it), command->get_factor(fI),fI);

      // ToDo: apply on vector sets (* get_factor(fI) sets)

      TransDebug::print("\nset split lijnen print\n");
      for(int tmp = 1; tmp < splitValues[0];tmp++){
        TransDebug::stream << splitValues[tmp] << "\n";
      }
      TransDebug::print("einde print \n");


      // devide given set by split points of 'splitValues', and stores them in 'newSets'
      divide_set_by_int_list(splitValues,fI,*set_it,newSets);

      TransDebug::stream << "\nsize newSets" << newSets->size() << "\n";
      TransDebug::stream << "size first set:" << number_of_set_elements(isl_set_copy(newSets->front())) << "\n";
    } // for

    delete sets;
    sets = newSets;
    newSets = new vector<isl_set*>();
  } // for

  TransDebug::stream << endl << "# of sets: " << sets->size() << "}";
  // create set divider
  SetDivider setSplitter(pdg,command->get_nodeNr());

  // execute splitting
  setSplitter.divide_node_by_vector_of_sets(*sets);

  delete sets,newSets;
}

void TransCLParser::divide_set_by_int_list(int* splitValues,int dim,__isl_take isl_set* originalSet,vector<isl_set*>* setStorage){
  int splitAmount = splitValues[0];
  isl_space *sourceSpace = isl_set_get_space(originalSet);
  isl_local_space *lSpace = isl_local_space_from_space(sourceSpace);

  isl_int v;
  isl_constraint *c;
  isl_int_init(v);


  isl_set* tmpSet;

  for(int x=1; x <= splitValues[0]; x++) {
    tmpSet = isl_set_copy(originalSet);

    // greater than condition
    if(x != 1){
      c = isl_inequality_alloc(isl_local_space_copy(lSpace));
      isl_int_set_si(v, -splitValues[x-1]-1);
      isl_constraint_set_constant(c, v);
      isl_int_set_si(v, 1);
      isl_constraint_set_coefficient(c, isl_dim_set, dim, v);
      tmpSet = isl_set_add_constraint(tmpSet, c);
    }

    // smaller than condition
    if(x != splitValues[0]){
      c = isl_inequality_alloc(isl_local_space_copy(lSpace));
      isl_int_set_si(v, splitValues[x]);
      isl_constraint_set_constant(c, v);
      isl_int_set_si(v, -1);
      isl_constraint_set_coefficient(c, isl_dim_set, dim, v);
      tmpSet = isl_set_add_constraint(tmpSet, c);
    }
    // push set into vector
    setStorage->push_back(tmpSet);
  }// for

  isl_local_space_free(lSpace);
  isl_set_free(originalSet);
  isl_int_clear(v);
  return;
}


void TransCLParser::modulo_split_factors(SplitCommand* command){
  // create SetDivider
  SetDivider setSplitter(pdg,command->get_nodeNr());

  // apply split by data inside SplitCommand class

  command->reset_factor_it();
  int nextFactor, dimNumber(0);
  int nodeDim = pdg->nodes[command->get_nodeNr()]->source->dim();


  // add constraints... based on factors
  vector<factorInfo>* infoVector = new vector<factorInfo>();
  applyNextFactor(&setSplitter,infoVector,command,0);
  delete infoVector;

  // apply constraints
  setSplitter.divide_node_by_subsets();
}

void TransCLParser::applyNextFactor(SetDivider* setSplitter, vector<factorInfo>* info,SplitCommand* command, int factorIndex){
  TransDebug::stream << "Entering applyNextFactor with factor index on " << factorIndex;
  int factorFound;
  int factorFoundIndex = -1;

  // find next relevant factor
  for(int z = factorIndex; z < command->get_factor_size();z++){
    factorFound = command->get_factor(z);
    if(factorFound != 0 and factorFound != 1){
      factorFoundIndex = z;
      break;
    }
  }
  TransDebug::stream << ", factorFoundIdex: " << factorFoundIndex << endl;
  // if found: branch for each offset of this factor
  if(factorFoundIndex != -1){
    for(int offset = 0; offset < factorFound; offset++){
      TransDebug::stream << "initiating offset " << offset << " for factor " << factorFound << " at index " << factorFoundIndex << endl;
      info->push_back(factorInfo(factorFoundIndex,offset,factorFound));
      applyNextFactor(setSplitter,info,command,factorFoundIndex+1);
    }
    TransDebug::stream << "pop";
    info->pop_back();
  }else{
    // if no important factor found and info stack is empty: stop
    if(info->size() == 0)
      return;

    TransDebug::stream << "end of factors, push subset!" << endl;
    // if no relevent factor found: apply all founded factors with respective offsets. Then pop last factor.
    setSplitter->add_subset();
    int dimOutOfBounds = pdg->nodes[command->get_nodeNr()]->source->dim();

    vector<factorInfo>::iterator info_it;
    for(info_it = info->begin(); info_it < info->end(); info_it++){
      setSplitter->add_subset_constraint(false);
       setSplitter->set_subset_constant(-(*info_it).offset);
       setSplitter->set_subset_dim_value((*info_it).dimension,-1);
       setSplitter->set_subset_dim_value(dimOutOfBounds++,-(*info_it).factor);
    }
    TransDebug::stream << "pop(end)";
    info->pop_back();
  }
}

// split set into 'parts' sets.
// every split point is recursevily searched by selecting a canidate split point, and
// examining in which potential set the actual split point must be
int* TransCLParser::find_N_split_points(__isl_take isl_set* baseSet, int parts,int dim) {
  isl_set *setL,*setR;

  assert(parts >= 2);
  int* splitPoints = new int[parts];
  splitPoints[0] = parts;
  int cardSet = number_of_set_elements(isl_set_copy(baseSet));
  int targetElements;

  // older methods
  TransDebug::stream << endl << "op zoek naar " << parts << " onderdelen. Met " << targetElements << " elementen per set"<< endl;

  for(int x = 1; x < parts; x++) {
    targetElements = cardSet/(parts - x + 1.0) + .5; // (re)calculate target elements
    splitPoints[x] = find_split_point_by_target(targetElements,isl_set_copy(baseSet),dim);
    split_set_on_int(baseSet,splitPoints[x],dim,setL,setR); // ToDo: replace with more efficient splitter
    baseSet = setR; // could be done in one step
    isl_set_free(setL); // tmp overhead
    TransDebug::set_dump(baseSet);
    cardSet = number_of_set_elements(isl_set_copy(baseSet));
  }
  isl_set_free(baseSet);
  return splitPoints;
}

int TransCLParser::number_of_set_elements(__isl_take isl_set* set) {
  int count = 0;
  /// determines number of elements in set
  isl_pw_qpolynomial *pwqp = isl_set_card(set);
  isl_pw_qpolynomial_foreach_piece(pwqp, & TransCLParser::countCard, &count);
  isl_pw_qpolynomial_free(pwqp);
  return count;
}

// Helper function (callback) for getCardinality
int TransCLParser::countCard(isl_set *set, isl_qpolynomial *qp, void *user) {
  int *count = (int*)user;
  isl_int n, d;
  isl_int_init(n);
  isl_int_init(d);

  // The resulting pw_qpolynomial should consist of only a single piece:
  assert(*count == 0);

  if (isl_qpolynomial_is_cst(qp, &n, &d) == 1) {
    if (isl_int_get_si(d) != 1) {
      fprintf(stderr, "Warning: Set cardinality seems fractional!\n");
    }
    *count = isl_int_get_si(n);
  }
  else {
    fprintf(stderr, "Warning: At least one qpolynomial is not constant!\n");
  }
  isl_int_clear(n);
  isl_int_clear(d);
  isl_qpolynomial_free(qp);
  isl_set_free(set);
  return 0;
}

// split set by average, then determines if split settisfies target
// recursively examines this set to find better split point
int TransCLParser::find_split_point_by_target(int targetLeft, isl_set* baseSet, int dim) {
  // define how precise sets should be split (min 1)
  static int offset = 1;

  // split set; old method: split in half
  //int evSplitPoint = 0.5 * finite_set_max(baseSet) + 0.5 * finite_set_min(baseSet); // ToDo: maybe based on (max-min)/N + min

  // split based on where splitpoint should be on an evenly distributed set
  int baseSetSize = number_of_set_elements(isl_set_copy(baseSet));
  int evSplitPoint = (targetLeft * finite_set_max(baseSet,dim) + (baseSetSize-targetLeft) * finite_set_min(baseSet,dim)) / baseSetSize;

  isl_set* setL,*setR;

  // split set. If resulting sets make no change: return
  if(!split_set_on_int(baseSet,evSplitPoint,dim,setL,setR)){
    TransDebug::stream << "TransDebug: no set change, thus best found split value" << evSplitPoint << endl;
    // satisfying split point found

    isl_set_free(setR);
    isl_set_free(setL);
    return evSplitPoint;
  };

  // calculate elements distribution if this would be final split
  int elementsSetL = number_of_set_elements(isl_set_copy(setL));

  // --debug info--
  TransDebug::stream << " elements_left " << elementsSetL << "," << "target:" << targetLeft << " SplitPoint:" << evSplitPoint << endl;

  // check conditions
  // final condition in case of offset = 0
  if ( elementsSetL == 0) {
    TransDebug::stream << "TransDebug: (elementsSetL == 0) not exact split point possible" << endl;
    // satisfying split point found

    isl_set_free(setR);
    isl_set_free(setL);
    return evSplitPoint;
  }

  if (elementsSetL < targetLeft) {
    isl_set_free(setL);
    return find_split_point_by_target(targetLeft - elementsSetL,setR,dim);
  }

  if (elementsSetL > targetLeft ) {
    isl_set_free(setR);
    return find_split_point_by_target(targetLeft,setL,dim);
  }

  // satisfying split point found
  isl_set_free(setR);
  isl_set_free(setL);
  return evSplitPoint;

}


// splits a set on given value at given dim.
// Returns false if split gives one empty set and one set equal to original set.
bool TransCLParser::split_set_on_int(__isl_take isl_set* originalSet, int splitInt, int dim, __isl_give isl_set*& firstPart, __isl_give isl_set*& secondPart) {

  TransDebug::stream << "Original set:" << splitInt << endl;
  TransDebug::set_dump(originalSet);


  // split set by adding constrains
  isl_space* sourceSpace = isl_set_get_space(originalSet);
  isl_local_space *lSpace = isl_local_space_from_space(sourceSpace);
  isl_int v;
  isl_constraint *c;
  isl_int_init(v);

  firstPart = isl_set_copy(originalSet);
  secondPart = isl_set_copy(originalSet);
  isl_set_free(originalSet);

  // constrain first set
  c = isl_inequality_alloc(isl_local_space_copy(lSpace));
  isl_int_set_si(v, splitInt);
  isl_constraint_set_constant(c, v);
  isl_int_set_si(v, -1);
  isl_constraint_set_coefficient(c, isl_dim_set, dim, v);
  firstPart = isl_set_add_constraint(firstPart, c);

  // constrain second set
  c = isl_inequality_alloc(lSpace);
  isl_int_set_si(v, -splitInt-1);
  isl_constraint_set_constant(c, v);
  isl_int_set_si(v, 1);
  isl_constraint_set_coefficient(c, isl_dim_set, dim, v);
  secondPart = isl_set_add_constraint(secondPart, c);
  isl_int_clear(v);


  // check if a set is empty, meaning the split did not have an effective change
  if( isl_set_is_empty(firstPart) || isl_set_is_empty(secondPart) ){
    TransDebug::stream << endl << "a splitted set is empty";
    return false;
  }

  TransDebug::stream << "split Int:" << splitInt << endl;
  TransDebug::set_dump(firstPart);
  TransDebug::set_dump(secondPart);

  return true;
}


// returns max value as intiger for given demension
int TransCLParser::finite_set_max(__isl_keep isl_set* set, int dim) {
  isl_int value;
  int intValue;
  isl_set* maxSet = isl_set_copy(set);
  int dimSize = isl_set_dim(set,isl_dim_out);

  // remove all other dimensions (so lexmin/max find correct result)
  if(dim < dimSize - 1 ){
    maxSet = isl_set_project_out(maxSet, isl_dim_set, dim+1, dimSize - dim - 1);
  }

  if(dim > 0){
    maxSet = isl_set_project_out(maxSet, isl_dim_set, 0, dim);
  }

  maxSet = isl_set_lexmax(maxSet);
  isl_point * maxPoint = isl_set_sample_point(maxSet);

  // extract integer from point
  isl_int_init(value);
  isl_point_get_coordinate(maxPoint, isl_dim_out, 0, &value);
  isl_point_free(maxPoint);
  intValue = isl_int_get_si(value);
  isl_int_clear(value);

  return intValue;
}

// returns lexmin value as intiger (of demension 0)
int TransCLParser::finite_set_min(__isl_keep isl_set* set, int dim) {
  isl_int value;
  int intValue;
  isl_set* maxSet = isl_set_copy(set);
  int dimSize = isl_set_dim(set,isl_dim_out);

  // remove all other dimensions (so lexmin/max find correct result)
  if(dim < dimSize - 1 ){
    maxSet = isl_set_project_out(maxSet, isl_dim_set, dim+1, dimSize - dim - 1);
  }

  if(dim > 0){
    maxSet = isl_set_project_out(maxSet, isl_dim_set, 0, dim);
  }

  maxSet = isl_set_lexmin(maxSet);
  isl_point * maxPoint = isl_set_sample_point(maxSet);

  isl_int_init(value);
  isl_point_get_coordinate(maxPoint, isl_dim_out, 0, &value);
  isl_point_free(maxPoint);
  intValue = isl_int_get_si(value);
  isl_int_clear(value);
  return intValue;
}
