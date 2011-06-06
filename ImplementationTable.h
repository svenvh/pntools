/*
 * Cost Table implementation
 * Author: Sven van Haastregt
 * $Id: ImplementationTable.h,v 1.4 2011/06/06 13:56:09 svhaastr Exp $
 */

#ifndef _IMPLEMENTATIONTABLE_H_
#define _IMPLEMENTATIONTABLE_H_

#include <string>
#include <vector>
#include <map>
#include <libxml/parser.h>
#include <libxml/tree.h>

// Implementation/processor type
typedef enum {
  IT_NONE,
  IT_MICROBLAZE,
  IT_POWERPC,
  IT_LAURA
} ImplementationType;


// Implementation Metric
typedef enum {
  IM_DELAY_AVG,               // Average delay (cycles)
  IM_DELAY_WORST,             // Worst case delay (cycles)
  IM_DELAY_BEST,              // Best case delay (cycles)
  IM_II,                      // Initiation interval (cycles)
  IM_SLICES,                  // Slice usage
  IM_MEMORY_DATA,             // Data memory (bytes)
  IM_MEMORY_CODE              // Code memory (bytes)
} ImplementationMetric;


// Holds metrics for a single implementation of a function.
class Implementation {
  public:
    Implementation(const std::string &name, ImplementationType type);
    ~Implementation();
    std::string getName();
    void setName(const std::string &newname);
    ImplementationType getType();
    void setType(ImplementationType newtype);
    int getMetric(ImplementationMetric m);
    void setMetric(ImplementationMetric m, int value);

  private:
    std::string name;
    ImplementationType type;
    std::map<ImplementationMetric, int> metrics;
};


// Stores a set of functions, and for each function one or more implementations.
class ImplementationTable {
  public:
    ImplementationTable();
    ~ImplementationTable();

    // Loads implementation data from given XML file.
    bool load(const std::string &filename);

    // Loads implementation data from default files (current directory or home directory).
    bool loadDefaultFile();

    // Returns number of implementations for given function.
    int getEntryCount(const std::string &funcName);

    // Returns i-th implementation of given function.
    Implementation *getEntry(const std::string &funcName, int i);

    // Returns metric m for (i-th implementation of) given function.
    int getMetric(ImplementationMetric m, const std::string &funcName, int i = 0);

  private:
    std::vector<Implementation*> *getImplementations(const std::string &funcName);
    xmlNode *findSibling(xmlNode *node, const char *name);
    xmlChar *getRequiredAttribute(xmlNode *node, const char *attrName);
    void processMetric(xmlNode *node, Implementation *impl, const char *tag, const char *attrName, ImplementationMetric metric);
    Implementation *processImplementation(xmlNode *node, const char *fnName);
    bool processXmlDoc(xmlDocPtr doc);

    std::map<std::string, std::vector<Implementation*>* > table;
};

#endif
