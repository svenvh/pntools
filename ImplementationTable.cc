/*
 * Cost Table implementation
 * Author: Sven van Haastregt
 */

#include "ImplementationTable.h"

#include <iostream>
#include <cstring>
#include <libxml/parser.h>
#include <libxml/tree.h>

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
  std::map<std::string, std::vector<Implementation*>* >::iterator i;
  std::vector<Implementation*>::iterator j;

  for (i = table.begin(); i != table.end(); ++i) {
    for (j = i->second->begin(); j != i->second->end(); ++j) {
      delete *j;
    }
    delete i->second;
  }
}

bool ImplementationTable::load(const string &filename) {
  FILE *file;
  if ((file = fopen(filename.c_str(), "r")) != NULL) {
    fclose(file);
  }
  else {
    return false;
  }

  LIBXML_TEST_VERSION

  xmlDocPtr doc;
  doc = xmlReadFile(filename.c_str(), NULL, 0);
  if (doc == NULL) {
    fprintf(stderr, "Failed to parse file\n");
    return false;
  }

  bool processed = processXmlDoc(doc);
  xmlFreeDoc(doc);

  xmlCleanupParser();
  return processed;
}

// Attempts to load data from default locations
bool ImplementationTable::loadDefaultFile() {
  char *homeDir = getenv("HOME");
  char *fileHome = new char[strlen(homeDir) + 32];
  strcpy(fileHome, homeDir);
  strcat(fileHome, "/.daedalus/impldata.xml");
  bool success = false;

  if (load("impldata.xml") == true) {
    success = true;
  }
  else if (load(fileHome) == true) {
    success = true;
  }
  delete[] fileHome;
  return success;
}

int ImplementationTable::getEntryCount(const string &funcName) {
  vector<Implementation*> *v = getImplementations(funcName);
  return v ? v->size() : 0;
}

Implementation *ImplementationTable::getEntry(const string &funcName, int i) {
  vector<Implementation*> *v = getImplementations(funcName);
  return v ? (*v)[i] : NULL;
}

int ImplementationTable::getMetric(ImplementationMetric m, const string &funcName, int i) {
  vector<Implementation*> *v = getImplementations(funcName);
  if (!v) {
    cerr << "Warning: \"" << funcName << "\" not found in cost table. Using default value 1." << endl;
    return 1;
  }
  return (*v)[i]->getMetric(m);
}




//// Private methods

vector<Implementation*> *ImplementationTable::getImplementations(const string &funcName) {
  // We don't want to directly return table[funcName], because then the stl::map would actually insert funcName into table...
  if (table.find(funcName) != table.end()) {
    return table[funcName];
  }
  else {
    return NULL;
  }
}



// XML Parsing (using libxml2)

static ImplementationType strToImplementationType(const char *str) {
  if      (strcmp(str, "MicroBlaze") == 0)    return IT_MICROBLAZE;
  else if (strcmp(str, "PowerPC") == 0)       return IT_POWERPC;
  else if (strcmp(str, "LAURA") == 0)         return IT_LAURA;
  else if (strcmp(str, "CompaanHWNode") == 0) return IT_LAURA;
  else {
    fprintf(stderr, "[strToImplementationType] Unknown type %s\n", str);
    return IT_NONE;
  }
}



xmlNode *ImplementationTable::findSibling(xmlNode *node, const char *name) {
  xmlNode *i;

  for (i = node; i; i = i->next) {
    if (i->type == XML_ELEMENT_NODE) {
      if (xmlStrcmp(i->name, BAD_CAST name) == 0) {
        return i;
      }
    }
  }

  return NULL;
}

xmlChar *ImplementationTable::getRequiredAttribute(xmlNode *node, const char *attrName) {
  xmlAttrPtr attr = xmlHasProp(node, BAD_CAST attrName);
  if (!attr) {
    fprintf(stderr, "Attribute \"%s\" not found for node \"%s\" in line %d\n", attrName, BAD_CAST node->name, node->line);
    exit(1);
  }
  return attr->children->content;
}

void ImplementationTable::processMetric(xmlNode *node, Implementation *impl, const char *tag, const char *attrName, ImplementationMetric metric) {
  xmlNodePtr t;
  int val = 1;
  if ((t = findSibling(node->children, tag)) != NULL) {
    xmlAttrPtr attr = xmlHasProp(t, BAD_CAST attrName);
    if (attr) {
      if (sscanf((char*)(attr->children->content), "%d", &val) == 1) {
        impl->setMetric(metric, val);
      }
      else {
        fprintf(stderr, "Couldn't parse attribute '%s.%s' value '%s'\n", tag, attrName, attr->children->content);
      }
    }
  }
}

Implementation *ImplementationTable::processImplementation(xmlNode *node, const char *fnName) {
  xmlChar *implName = getRequiredAttribute(node, "componentName");
  xmlChar *implType = getRequiredAttribute(node, "implementationType");
  string nm = (char *) implName;
  Implementation *impl = new Implementation(nm, strToImplementationType((char*)implType));
  xmlNodePtr x;
  if ((x = findSibling(node->children, "performance")) != NULL) {
    processMetric(x, impl, "delay",   "average",    IM_DELAY_AVG);
    processMetric(x, impl, "delay",   "worstcase",  IM_DELAY_WORST);
    processMetric(x, impl, "delay",   "bestcase",   IM_DELAY_BEST);
    processMetric(x, impl, "ii",      "value",      IM_II);
  }
  if ((x = findSibling(node->children, "resources")) != NULL) {
    processMetric(x, impl, "slices",  "value",      IM_SLICES);
    processMetric(x, impl, "memory",  "data",       IM_MEMORY_DATA);
    processMetric(x, impl, "memory",  "program",    IM_MEMORY_CODE);
  }
  if ((x = findSibling(node->children, "power")) != NULL) {
  }
  return impl;
}

bool ImplementationTable::processXmlDoc(xmlDocPtr doc) {
  xmlNode *root = xmlDocGetRootElement(doc);

  xmlNode *x = findSibling(root, "implementationMetrics");
  x = findSibling(x->children, "functions");

  for (xmlNodePtr f = x->children; f; f = f->next) {
    if (f->type == XML_ELEMENT_NODE && xmlStrcmp(f->name, BAD_CAST "function") == 0) {
      xmlChar *fName = getRequiredAttribute(f, "functionName");
      vector<Implementation*> *v = new vector<Implementation*>;
      for (xmlNodePtr i = f->children; i; i = i->next) {
        if (i->type == XML_ELEMENT_NODE && xmlStrcmp(i->name, BAD_CAST "implementation") == 0) {
          v->push_back(processImplementation(i, (char*)fName));
        }
      }
      table[(char*)fName] = v;
    }
  }

  return true;
}

