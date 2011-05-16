/*
 * Cost Table implementation
 * Author: Sven van Haastregt
 * $Id: ImplementationTable.h,v 1.1 2011/05/16 09:56:06 svhaastr Exp $
 */

#ifndef _IMPLEMENTATIONTABLE_H_
#define _IMPLEMENTATIONTABLE_H_

#include <string>
#include <vector>
#include <map>

// Implementation/processor type
typedef enum {
  IT_NONE,
  IT_MICROBLAZE,
  IT_POWERPC,
  IT_LAURA
} ImplementationType;


// Implementation Metric
typedef enum {
  IM_DELAY_AVG,       // Average delay
  IM_DELAY_WORST,     // Worst case delay
  IM_DELAY_BEST,      // Best case delay
  IM_II,              // Initiation interval
  IM_SLICES,          // Slice usage
  IM_MEMORY_DATA,     // Data memory (bytes)
  IM_MEMORY_CODE      // Code memory (bytes)
} ImplementationMetric;


// Describes a single implementation of a particular function.
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
    bool load(const std::string &filename);
    //bool loadDefaultFile();
    int getEntryCount(const std::string &funcName);
    Implementation *getEntry(const std::string &funcName, int i);
    int getMetric(ImplementationMetric m, const std::string &funcName, int i = 0);

  private:
    std::map<std::string, std::vector<Implementation*>* > table;
};

#endif
