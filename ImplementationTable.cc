/*
 * Cost Table implementation
 * Author: Sven van Haastregt
 * $Id: ImplementationTable.cc,v 1.1 2011/05/16 09:56:06 svhaastr Exp $
 */

#include "ImplementationTable.h"

#include <iostream>
//#include <libxml/parser.h>
//#include <libxml/tree.h>

using namespace std;


Implementation::Implementation(const string &nname, ImplementationType ntype) {
  this->name = nname;
  this->type = ntype;
}

Implementation::~Implementation() {
}


string Implementation::getName() {
  return this->name;
}
void Implementation::setName(const string &newname) {
  this->name = newname;
}


ImplementationType Implementation::getType() {
  return this->type;
}
void Implementation::setType(ImplementationType newtype) {
  this->type = newtype;
}

int Implementation::getMetric(ImplementationMetric m) {
  if (metrics.find(m) != metrics.end()) {
    return metrics[m];
  }
  else {
    cerr << "Warning: metric #" << m << " for \"" << this->name << "\" not found in cost table. Returning 1." << endl;
    return 1;
  }
}

void Implementation::setMetric(ImplementationMetric m, int value) {
  metrics[m] = value;
}



ImplementationTable::ImplementationTable() {
}

ImplementationTable::~ImplementationTable() {
}

bool ImplementationTable::load(const string &filename) {
  cerr << "Warning: Filling the ImplementationTable with some hardcoded junk." << endl;
  // TODO: implement parser
  vector<Implementation*> *v = new vector<Implementation*>;
  Implementation *im;
  im = new Implementation("copy", IT_MICROBLAZE);
  im->setMetric(IM_DELAY_WORST, 9);
  im->setMetric(IM_II, 9);
  im->setMetric(IM_SLICES, 250);
  v->push_back(im);
  table["copy"] = v;
  return true;
}

/*bool ImplementationTable::loadDefaultFile() {
  LIBXML_TEST_VERSION

  xmlDocPtr doc;
  doc = xmlReadFile("test.xml", NULL, 0);
  if (doc == NULL) {
    fprintf(stderr, "Failed to parse file\n");
    return false;
  }
  xmlFreeDoc(doc);

  xmlCleanupParser();
  return true;
}*/

int ImplementationTable::getEntryCount(const string &funcName) {
  vector<Implementation*> *v = table[funcName];
  return v ? v->size() : 0;
}

Implementation *ImplementationTable::getEntry(const string &funcName, int i) {
  vector<Implementation*> *v = table[funcName];
  return v ? (*v)[i] : NULL;
}

int ImplementationTable::getMetric(ImplementationMetric m, const string &funcName, int i) {
  vector<Implementation*> *v = table[funcName];
  if (!v) {
    cerr << "Warning: \"" << funcName << "\" not found in cost table. Using default value 1." << endl;
    return 1;
  }
  return (*v)[i]->getMetric(m);
}

