/* Debug helper class for pdgtrans
 * Author: Wouter de Zwijger
 */

#include "TransDebug.h"

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

  Token& Token::operator<<(double output){
    if(debugMode)
      cerr << output;
    return *this;
  }

  Token& Token::operator<<(const char* output){
    if(debugMode)
      cerr << output;
    return *this;
  }

  Token& Token::operator<<(string output){
    if(debugMode)
      cerr << output;
    return *this;
  }

  // catching endl (pointer function)
  Token& Token::operator<<(ostream& (*pf)(ostream&)){
    if(debugMode)
      cerr << endl;
    return *this;
  }

  Token stream;
}
