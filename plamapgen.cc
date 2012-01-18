// Generates platform and mapping file from mapping code.
// Original version by Eyal Halm,
// adapted by Sven van Haastregt
// LERC, LIACS, Leiden University 2009-2011
#include <cstdio>
#include <cstdlib>
#include <iostream>
#include <cstring>

// Amounts
#define MAX_PROC_NUM 100
#define MAX_CPU_NUM 20

// Codes
#define NO_NODE_CODE -1
#define HW_ACCELERATOR_CODE 999
FILE* fMapping;
FILE* fPlatform;

int main (int argc, char **argv) {
  int i, j;
  int mapping[MAX_CPU_NUM][MAX_PROC_NUM];
  int mappingCounters[MAX_PROC_NUM];
  int hwAccelerators[MAX_PROC_NUM];
  int numHWNs = 0;

  // Initialize matrices and vectors
  for (i = 0; i < MAX_CPU_NUM; i++) {
    for (j = 0; j < MAX_PROC_NUM; j++) {
      mapping[i][j] = NO_NODE_CODE;
    }
  }
  for (j = 0; j < MAX_PROC_NUM; j++) {
    hwAccelerators[j] = NO_NODE_CODE;
    mappingCounters[j] = 0;
  }

  if (argc <= 2) {
    printf("plamapgen - Generate platform and mapping files for ESPAM\n");
    printf("Usage: %s <name> p1 [p2 [p3 [...]]]\n", argv[0]);
    printf("         after <name> a mapping code is specified; see Eyal's MSc Thesis Sec. 3.1\n");
    printf("Example: plamapgen qr 1 2 %d 2\n", HW_ACCELERATOR_CODE);
    printf("         generates qr.pla and qr.map, with 2 MicroBlazes and 1 HWN\n");
    printf("         with process assignment: mb_1: P_1\n");
    printf("                                  mb_2: P_2, P_4\n");
    printf("                                  HWN1: P_3\n");
    exit(0);
  }
  char *outfilename = argv[1];
  char *mapfilename = new char[strlen(outfilename) + 5];
  strcpy(mapfilename, outfilename);
  strcat(mapfilename, ".map");
  char *plafilename = new char[strlen(outfilename) + 5];
  strcpy(plafilename, outfilename);
  strcat(plafilename, ".pla");

  // Build the permutaion into the matrix
  for (i = 2; i < argc; i++) {
    int CPU = atoi(argv[i]);
    if (CPU != HW_ACCELERATOR_CODE) {
      mapping[CPU][mappingCounters[CPU]] = i-2; 
      mappingCounters[CPU]++;
    }
    else {
      hwAccelerators[i-2] = i-2;
      numHWNs++;
    }
  }

  // Print the matrix (DEBUG)
  for (i = 1; i < MAX_CPU_NUM; i++) {
    if (mappingCounters[i] > 0) {
      printf("mb_%d: ", i);
      for (j = 0; j < MAX_PROC_NUM; j++) {
        if (mapping[i][j] != NO_NODE_CODE) {
          printf("ND_%d ", mapping[i][j]);
        }
      }
      printf("\n");
    }
  }

  for (i = 0; i < MAX_PROC_NUM; i++) {
    if (hwAccelerators[i] != NO_NODE_CODE) {
      printf("HWN_%d: ND_%d\n", i, hwAccelerators[i]);
    }
  }


  //--------------------------------------------------------------------------

  //--------------------------------------------------------------------------
  // Create the MAP (mapping) file
  //--------------------------------------------------------------------------
  fMapping = fopen(mapfilename, "w");

  fprintf(fMapping, "<?xml version=\"1.0\" standalone=\"no\"?>\n");
  fprintf(fMapping, "<!DOCTYPE mapping PUBLIC \"-//LIACS//DTD ESPAM 1//EN\"\n");
  fprintf(fMapping, "\"http://www.liacs.nl/~cserc/dtd/espam_1.dtd\">\n");
  fprintf(fMapping, "\n");
  fprintf(fMapping, "<mapping name=\"myMapping\">\n\n");

  for (i = 0; i <= MAX_CPU_NUM; i++) {
    if (mappingCounters[i] > 0) {
      fprintf(fMapping, "   <processor name=\"mb_%d\">\n", i);
      for (j = 0; j < mappingCounters[i]; j++) {
        fprintf(fMapping, "      <process name=\"ND_%d\" />\n", mapping[i][j]);
      } //for
      fprintf(fMapping, "   </processor>\n\n");
    } //if
  } //for

  if (numHWNs > 0) {
    fprintf(fMapping, "   <processor name=\"HWN\">\n");
    for (i = 0; i < MAX_PROC_NUM; i++) {
      if (hwAccelerators[i] != NO_NODE_CODE) {
        fprintf(fMapping, "      <process name=\"ND_%d\" />\n", i);
      } //if
    } //for
    fprintf(fMapping, "   </processor>\n\n");
  } //if

  fprintf(fMapping, "</mapping>\n");

  //--------------------------------------------------------------------------
  // Create the PLA (platform) file
  //--------------------------------------------------------------------------
  fPlatform = fopen(plafilename, "w");

  fprintf(fPlatform, "<?xml version=\"1.0\" standalone=\"no\"?>\n");
  fprintf(fPlatform, "<!DOCTYPE platform PUBLIC \"-//LIACS//DTD ESPAM 1//EN\"\n");
  fprintf(fPlatform, "\"http://www.liacs.nl/~cserc/dtd/espam_1.dtd\">\n");
  fprintf(fPlatform, "\n");
  fprintf(fPlatform, "<platform name=\"myPlatform\">\n");

  for (i = 0; i <= MAX_CPU_NUM; i++) {
    if (mappingCounters[i] > 0) {
      fprintf(fPlatform, "   <processor name=\"mb_%d\" type=\"MB\" data_memory=\"65536\" program_memory=\"65536\">\n", i);
      fprintf(fPlatform, "   </processor>\n\n");
    } //if
  } //for

  if (numHWNs > 0) {
    fprintf(fPlatform, "   <processor name=\"HWN\" type=\"CompaanHWNode\">\n");
    fprintf(fPlatform, "   </processor>\n\n");
  }

  fprintf(fPlatform, "</platform>\n");

  fclose(fMapping);
  fclose(fPlatform);
  return 0;
}
