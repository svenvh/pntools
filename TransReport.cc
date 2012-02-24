/* Debug and error reporting helper for pdgtrans
 * Author: Wouter de Zwijger
 */

#include "TransReport.h"

namespace TransDebug {
  namespace {
    bool debugMode = false;
  }

  void setDebugMode(bool newValue){
    debugMode = newValue;
  }

  bool isDebugMode(){
    return debugMode;
  }

  void print(const char* output){
    if(debugMode)
      cerr << output;
  }

  void set_dump(isl_set* set){
    if(debugMode)
      isl_set_dump(set);
  }

  void constraint_dump(isl_constraint* constraint){
    if(debugMode)
      isl_constraint_dump(constraint);
  }

/*  out& operator<<(out& token, const char* output){
    if(debugMode)
      cerr << output;
    return out&;
  }

  out& operator<<(out& token, double output){
    if(debugMode)
      cerr << output;
    return out&;
  }*/

  DebugStreamObject& DebugStreamObject::operator<<(double output){
    if(debugMode)
      cerr << output;
    return *this;
  }

  DebugStreamObject& DebugStreamObject::operator<<(const char* output){
    if(debugMode)
      cerr << output;
    return *this;
  }

  DebugStreamObject& DebugStreamObject::operator<<(string output){
    if(debugMode)
      cerr << output;
    return *this;
  }

  // catching endl (pointer function)
  DebugStreamObject& DebugStreamObject::operator<<(ostream& (*pf)(ostream&)){
    if(debugMode)
      cerr << endl;
    return *this;
  }

  DebugStreamObject stream;
}

namespace TransError {
  void found();
  bool isFound();


  namespace {
    bool foundError = false;
    stringstream errorMessage;
    vector<isl_set*> errorSets;
    vector<isl_constraint*> errorConstraints;
    vector<isl_space*> errorSpaces;
  }

  void found(){
    foundError = true;
  }

  bool isFound(){
    return foundError;
  }

  bool print(){
    if(isFound()){
      cerr << endl;
      cerr << "-----------------------------------------------------------" << endl;
      cerr << "An error has been detected during execution of this program." << endl;
      cerr << "The program is prematurely terminated as a result." << endl;
      cerr << "Please adjust your input to prevent errors. " << endl;
      cerr << "The error that has occured:" << endl;
      cerr << errorMessage.str();

      // sets
      vector<isl_set*>::iterator sit;
      int i = 0;
      for ( sit=errorSets.begin() ; sit < errorSets.end(); sit++ ){
        i++;
        cerr << endl << "<set " << i << ">:";
        isl_set_dump(*sit);
        isl_set_free(*sit);
      }
      i = 0;

      // constraints
      vector<isl_constraint*>::iterator cit;
      for ( cit=errorConstraints.begin() ; cit < errorConstraints.end(); cit++ ){
        i++;
        cerr << endl << "<constraint " << i << ">:";
        isl_constraint_dump(*cit);
        isl_constraint_free(*cit);
      }
      i = 0;

      // constraints
      vector<isl_space*>::iterator spit;
      for ( spit=errorSpaces.begin() ; spit < errorSpaces.end(); spit++ ){
        i++;
        cerr << endl << "<space " << i << ">:";
        isl_space_dump(*spit);
        isl_space_free(*spit);
      }

      cerr << endl;
    }
    return isFound();
  }

  ErrorStreamObject& ErrorStreamObject::operator<<(isl_space* space){
    errorMessage << "<space " << errorSpaces.size()+1 << ">";
    errorSpaces.push_back(isl_space_copy(space));
    found();
    return *this;
  }

  ErrorStreamObject& ErrorStreamObject::operator<<(isl_set* set){
    errorMessage << "<set " << errorSets.size()+1 << ">";
    errorSets.push_back(isl_set_copy(set));
    found();
    return *this;
  }

  ErrorStreamObject& ErrorStreamObject::operator<<(isl_constraint* constraint){
    errorMessage << "<constraint " << errorConstraints.size()+1 << ">";
    errorConstraints.push_back(isl_constraint_copy(constraint));
    found();
    return *this;
  }

  ErrorStreamObject& ErrorStreamObject::operator<<(double message){
    errorMessage << message;
    found();
    return *this;
  }

  ErrorStreamObject& ErrorStreamObject::operator<<(const char* message){
    errorMessage << message;
    found();
    return *this;
  }

  ErrorStreamObject& ErrorStreamObject::operator<<(string message){
    errorMessage << message;
    found();
    return *this;
  }

  ErrorStreamObject& ErrorStreamObject::operator<<(ostream& (*pf)(ostream&)){
    errorMessage << endl;
    found();
    return *this;
  }

  ErrorStreamObject stream;

}
