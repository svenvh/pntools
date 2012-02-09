/* Used by pdgtrans to describe a single transformation.
 * Author: Wouter de Zwijger
 */

#include "SplitCommand.h"
// SplitCommand

SplitCommand::SplitCommand(pdg::PDG* pdgraph){
  pdg = pdgraph;
  constraint_it = constraints.end();
  set_it = sets.end();
  factor_it = factors.end();
  method = invalid;
  conditionType = unknown;
}

int SplitCommand::parseCommand(int argNr,int argc,char** argv){

  // detect type of command
  if(!strcmp(argv[argNr] ,"--domain-split"))
    method = domain_split;
  else if(!strcmp(argv[argNr],"--plane-split"))
    method = plane_split;
  else if(!strcmp(argv[argNr],"--modulo-split"))
    method = modulo_split;
  else if (!strcmp(argv[argNr],"--debug")) {
    method = debug_mode;
    return argNr + 1;
  }else {
    method = invalid;
    cerr << "command '" << argv[argNr] << "' is not a valid command, and is ignored!" << endl;
    return argNr + 1;
  }

  // set argNr for next command to be red
  argNr++;

  // get node nr
  // (argNr is called by reference by parseNOdeNr, so argNr will be set correct)
  if (!parseNodeNr(argNr,argc,argv))
    return argNr;

  // parse constraint type
  // (argNr is called by reference by parseConstraintType, so argNr will be set correct)
  if (!parseConstraintType(argNr,argc,argv))
    return argNr;

  // parse constraint (based on given type)
  // (argNr is called by reference, so argNr will be set correct for next use)
  switch(conditionType){
    case constraint_conditions:
      if (!parseConstraintList(argNr,argc,argv))
        return argNr;

      constraint_it = constraints.begin();
      newSeq = true;
      break;
    case constraint_hyperplanes:
      cerr << "Hyperplanes as constraints is not yet implemented, please use conditions or sets instead" << endl;
      return argNr;

      break;
    case constraint_sets:
      if (!parseConstraintSets(argNr,argc,argv))
        return argNr;

      set_it = sets.begin();
      break;
    case constraint_factors:
      if (!parseConstraintFactors(argNr,argc,argv))
        return argNr;

      factor_it = factors.begin();
      break;
    default:
      cerr << "Unsupported condition type, please use one that is defined" << endl;
      break;
  }

  return argNr;
}

SplitCommand::~SplitCommand(){

  // delete constraints
  vector<isl_constraint*>::iterator it;
  for(it = constraints.begin(); it < constraints.end(); it++)
    if(*it != NULL)
      isl_constraint_free(*it);

  // delete sets
  vector<isl_set*>::iterator it2;
  for(it2 = sets.begin(); it2 < sets.end(); it2++)
    if(*it2 != NULL)
      isl_set_free(*it2);

}

bool SplitCommand::parseNodeNr(int &argNr,int argc,char **argv){
  if(argNr >= argc-1){
    cerr << "invallid number of arguments to specify node nr. use '--node 0' to specify node nr 0" << endl;
    return false;
  }

  if(strcmp(argv[argNr],"--node")){
    cerr << "please specify node nr after split method. For instance '--domain-split --node 0 ect..'" << endl;
    return false;
  }
  // save nodeNr
  nodeNr = atoi(argv[argNr+1]);

  // set argNr to first not parsed command.
  argNr += 2;

  return true;
}

bool SplitCommand::parseConstraintType(int &argNr,int argc,char **argv){
  if(argNr >= argc){
    cerr << "Please specify constraint type. Example: --constraints, --hyperplanes, --sets, --factors" << endl;
    return false;
  }

  if(!strcmp(argv[argNr],"--hyperplanes")){
    conditionType = constraint_hyperplanes;
  }else if(!strcmp(argv[argNr],"--conditions")){
    conditionType = constraint_conditions;
  }else if(!strcmp(argv[argNr],"--sets")){
    conditionType = constraint_sets;
  }else if(!strcmp(argv[argNr],"--factors")){
    conditionType = constraint_factors;
  } else {
    cerr << "Please specify constraint type. Example: \n --conditions \n --hyperplanes \n --sets \n --factors" << endl;
    return false;
  }

  // set argNr to first not parsed command.
  argNr++;

  return true;
}

bool SplitCommand::parseConstraintList(int &argNr,int argc,char **argv){
 if(argNr >= argc){
    cerr << "Please specify constraint(s) like \"1 3 4 2, 1 4 5 2; 1 3 4 2\" " << endl;
    return false;
 }

  isl_constraint* c;
  isl_local_space* lSpace;

  char* command = argv[argNr];
  int currentValue = 0;
  int dimNumber = -1;
  int maxDimNumber = 0;
  bool negative = false;
  bool emptySpace = false;

  //while(command[x] != '\0') {
  for(int x = 0;command[x] != '\0'; x++) {
    TransDebug::stream << "{" << command[x] << "} ";
    switch(command[x]){
      case ' ':
        // do nothing if no data was found since last ',', ';' or ' '
        if(emptySpace)
          break;

        if(negative)
          currentValue *= -1;
        // add to constraint with dimNumber
        // or eq/uneq in case of dimNumber -1
        if(dimNumber == -1){
          TransDebug::stream << " create space, and (in)equality";
          // create local space for constraint
          lSpace = isl_local_space_from_space(pdg->nodes[nodeNr]->source->get_dim());
          maxDimNumber = modify_local_space(lSpace,command,x);

          if(currentValue == 1)
            c = isl_inequality_alloc(lSpace);
          else
            c = isl_equality_alloc(lSpace);
        } else if(dimNumber < maxDimNumber){
          // add constrain value for this dimension
          TransDebug::stream << "added to constrain: dim " << dimNumber << " value " << currentValue;
          isl_constraint_set_coefficient_si(c, isl_dim_set, dimNumber, currentValue);
        } else {
          TransDebug::stream << "add parameter instead of constraint";
          //ToDo: add parameter
        }
        // set counters corect
        currentValue = 0;
        negative = false;
        dimNumber++;
        emptySpace = true;
        break;
      case ',':
          if(negative)
            currentValue *= -1;
          TransDebug::stream << "added constant value " << currentValue;
          isl_constraint_set_constant_si(c, currentValue);
          TransDebug::stream << "constrain pushed";
          constraints.push_back(c);
          c = NULL;

          dimNumber = -1;
          currentValue = 0;
          negative = false;
          emptySpace = true;
        break;
      case ';':
          if(negative)
            currentValue *= -1;
          TransDebug::stream << "added constant value " << currentValue;
          isl_constraint_set_constant_si(c, currentValue);
          TransDebug::stream << "constrain pushed";
          constraints.push_back(c);
          // push extra NULL, indicating new set of constrains
          cerr << "Extra NULL pushed";
          constraints.push_back(NULL);

          c = NULL;
          dimNumber = -1;
          currentValue = 0;
          negative = false;
          emptySpace = true;
        break;
      default:
        // parse numbers
        if(command[x] == '-'){
          negative = true;
          emptySpace = false;
        } else if(command[x] >= '0' && command[x] <= '9'){
          currentValue *= 10;
          currentValue += command[x] - '0'; //atoi(command[x]);
          TransDebug::stream << "cVal:" << currentValue << "; ";
          emptySpace = false;
        }

        break;
    } // switch
  } // for

  // add last constraint
  if(negative)
    currentValue *= -1;
  TransDebug::stream << "added constant value " << currentValue;
  isl_constraint_set_constant_si(c, currentValue);
  TransDebug::stream << "constrain pushed";
  constraints.push_back(c);

  // set argNr correct
  argNr++;
  return true;
}

// modifies local space, based on given dimensions (dim out of bound need to be transferred).
int SplitCommand::modify_local_space(isl_local_space*& lSpace,char* command,int fromHere){

  // Count number of dimensions given (including constant and parameters)
  bool last_was_space = true;
  int dimAndParams = 0;
  for(int y = fromHere;command[y] != '\0' && command[y] != ',' && command[y] != ';'; y++){
    if(command[y] != ' ' && last_was_space){
      last_was_space = false;
      dimAndParams++;
    } else if(command[y] == ' '){
      last_was_space = true;
    } // else
  } // for

  // number of parameters
  int parameters = isl_local_space_dim(lSpace,isl_dim_param);

  // number of dimensions that need to be added
  int undefinedDims = dimAndParams - isl_local_space_dim(lSpace,isl_dim_out) - parameters - 1;

  // add demensions required
  if(undefinedDims > 0)
    lSpace = isl_local_space_add_dims(lSpace,isl_dim_out, undefinedDims);

  TransDebug::stream << endl << "DimAndParams " << dimAndParams <<  " Parameters " << parameters <<  " UndefinedDims " << undefinedDims << "  isl_local_space_dim(lSpace,isl_dim_out): " <<  isl_local_space_dim(lSpace,isl_dim_out) << endl;

  // return number of dimensions (thus excluding given parameters and constant)
  return dimAndParams - parameters - 1;
}

bool SplitCommand::parseConstraintSets(int &argNr,int argc,char **argv){
 if(argNr >= argc){
    cerr << "Please specify sets like \"{ S_3[i,j] : i >= 1 and i <= 16 and j >= 1 and j <= 20 };{ next set }\" " << endl;
    return false;
 }

 string string_sets = argv[argNr];
 string singleSet;
 size_t current_pos(0), split_pos;

 while( (split_pos = string_sets.find_first_of(';',current_pos)) != string::npos ){
   singleSet = string_sets.substr(current_pos, split_pos - current_pos);
   sets.push_back(isl_set_read_from_str(pdg->ctx  , singleSet.c_str()));

   // debug info
   TransDebug::stream << split_pos << ": singleSet string:" << singleSet << endl;
   if(TransDebug::isDebugMode()){
     isl_set* tmpSet;
     tmpSet = isl_set_read_from_str(pdg->ctx  , singleSet.c_str());
     TransDebug::set_dump(tmpSet);
     isl_set_free(tmpSet);
   }

   current_pos = split_pos + 1;
 } // while

 singleSet = string_sets.substr(current_pos);
 sets.push_back(isl_set_read_from_str(pdg->ctx  , singleSet.c_str()));

 // debug info
 TransDebug::stream << "singleSet string:" << singleSet << endl;

 argNr++;
 return false;
}

bool SplitCommand::parseConstraintFactors(int &argNr,int argc,char **argv){
 if(argNr >= argc){
    cerr << "Please specify factors like \"1,3,4" << endl;
    return false;
 }

  char* command = argv[argNr];
  int currentValue = 0;
  bool negative = false;

  //while(command[x] != '\0') {
  for(int x = 0;command[x] != '\0'; x++) {
    switch(command[x]){
      case ',':
          if(negative)
            currentValue *= -1;
          factors.push_back(currentValue);

          currentValue = 0;
          negative = false;
        break;
      default:
        // parse numbers
        if(command[x] == '-'){
          negative = true;
        } else if(command[x] >= '0' && command[x] <= '9'){
          currentValue *= 10;
          currentValue += command[x] - '0'; //atoi(command[x]);
        }
        break;
    } // switch
  } // for

  // add last constraint
  if(negative)
    currentValue *= -1;

  factors.push_back(currentValue);

  // set argNr correct
  argNr++;
  return true;
}

int SplitCommand::constraints_size(){
  return constraints.size();
}

void SplitCommand::reset_constraint_it(){
  constraint_it = constraints.begin();
  newSeq = true;

}

void SplitCommand::reset_set_it(){
  set_it = sets.begin();
}

int SplitCommand::factors_size(){
  return factors.size();
}

void SplitCommand::reset_factor_it(){
  factor_it = factors.begin();
}

isl_constraint* SplitCommand::get_next_constraint(){
  if(constraint_it != constraints.end()){
    // null pointers: indicator of split between tuple sets
    while(constraint_it != constraints.end() && *(constraint_it) == NULL){
      newSeq = true;
      constraint_it++;
    } // while
    if(constraint_it != constraints.end())
      return isl_constraint_copy(*(constraint_it++));

  } // if
  return NULL;
}

isl_set* SplitCommand::get_next_set(){
  if(set_it != sets.end())
    return isl_set_copy(*(set_it++));

  return NULL;
}

// 0 represents end of factor sequence
// a original 0 will be returned as 1, representing modulo 1.
int SplitCommand::get_next_factor(){
  if(factor_it != factors.end()){
    if(*factor_it == 0)
      return *(factor_it++)+1;
    else
      return *(factor_it++);
  }

  return 0;
}

int SplitCommand::get_factor(int index){
  return factors[index];
}

int SplitCommand::get_factor_size(){
  return factors.size();
}

bool SplitCommand::constraint_is_new_sequence(){
  if(newSeq){
    newSeq = false;
    return true;
  }

  return false;
}

int SplitCommand::get_nodeNr(){
  return nodeNr;
}

SplitCommand::splitType SplitCommand::get_type(){
  return method;
}

SplitCommand::constraintType SplitCommand::get_c_type(){
  return conditionType;
}
