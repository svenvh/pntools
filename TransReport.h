/* Debug and error reporting helper for pdgtrans
 * Author: Wouter de Zwijger
 */

#include <iostream>
#include <sstream>

#include "barvinok/barvinok.h"

#include <isa/yaml.h>
#include <isa/pdg.h>


// this class provides functions for debug purposes and error output.

using namespace std;

namespace TransDebug{
  void setDebugMode(bool);
  bool isDebugMode();
  void print(const char*);
  void set_dump(isl_set*);
  void constraint_dump(isl_constraint*);

  class DebugStreamObject {
    public:
      DebugStreamObject& operator<<(double);
      DebugStreamObject& operator<<(const char*);
      DebugStreamObject& operator<<(string);
      DebugStreamObject& operator<<(ostream& (*pf)(ostream&));
  };

  extern DebugStreamObject stream;

}



namespace TransError {
  void found();
  bool isFound();
  bool print();
  void set_dump(isl_set*);
  void constraint_dump(isl_constraint*);

class ErrorStreamObject {
    public:
      ErrorStreamObject& operator<<(isl_space*);
      ErrorStreamObject& operator<<(isl_constraint*);
      ErrorStreamObject& operator<<(isl_set*);
      ErrorStreamObject& operator<<(double);
      ErrorStreamObject& operator<<(const char*);
      ErrorStreamObject& operator<<(string);
      ErrorStreamObject& operator<<(ostream& (*pf)(ostream&));
  };

  extern ErrorStreamObject stream;

}
