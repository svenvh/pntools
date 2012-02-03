/* Debug helper class for pdgtrans
 * Author: Wouter de Zwijger
 */

#include <iostream>

#include "barvinok/barvinok.h"

#include <isa/yaml.h>
#include <isa/pdg.h>

using namespace std;

namespace TransDebug{
  void setDebugMode(bool);
  void print(const char*);
  void set_dump(isl_set*);
  void constraint_dump(isl_constraint*);

  class Token {
    public:
      Token& operator<<(double);
      Token& operator<<(const char*);
      Token& operator<<(string);
      Token& operator<<(ostream& (*pf)(ostream&));
  };

  extern Token stream;

}
