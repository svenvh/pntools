PNTools
=======

This project contains various experimental tools for use with the [PN/ISA](http://repo.or.cz/w/isa.git) compiler. 
Most likely any tool found in here is in a very premature state, with a strong
dependence on the current phase of the moon.


Compilation Instructions
------------------------
    ./autogen.sh
    ./configure --with-pngen=XXX --with-isa=YYY
    make
Replace XXX with your pngen top level directory (i.e., the directory 
containing GMP, Syck, and libxml2 etc.).
Replace YYY with your isa directory (containing pn, pn2espam etc.).


Adding New Tools
----------------

1. Open up `Makefile.am` and add appropriate `_LDADD`, `_SOURCES` lines. Add the tool
   executable name to `bin_PROGRAMS`.
2. Rerun `autogen.sh`, `configure` and `make`.
3. Add your tool to the list below. :-)


How to use the tools
--------------------

- Most of the tools require `XXX.yaml` and/or `XXX-adg.yaml` as input (XXX stands for the name of your application).
- Run `c2pdg` to extract polyhedral description and it generates `XXX.yaml`
- Run `pn` and `pn2adg` to generate `XXX-adg.yaml` and it generates `XXX-adg.yaml` 


Current Tools
-------------
This is a list of tools currently included in this directory. If you add a new
tool, please add an appropriate description to this list as well.

Upate: 30.Nov.2011: We start to adapt all tools to the latest isa version.
        In particular, adg structure (instead of ppn before) will be used as basis for most of the tools.

### pdg1ana (not updated currently)
* Purpose: Example of how to extract information from a PDG.
* Authors: Sven van Haastregt
* Tested:  ISA 0.10

### pn2csdf
* Purpose: Converts a PPN (in the form of ADG) to a CSDF graph.
* Authors: Sven van Haastregt, Teddy Zhai
* Notes:   Implemented, needs further verification.
* Input:   `XXX-adg.yaml`
* Output:  `YYY.xml` (SDF3 format, currently not supported yet) or  
         `YYY.gph` (StreamIT format)
* Tested:  ISA 0.11

### ppnta (to be changed into adgta)
* Purpose: Do some throughput analysis on a PPN.
* Authors: Teddy Zhai, Sven van Haastregt
* Notes:   Implemented, needs further verification.
* Input:   `XXX.ppn`
* Output:  throughput numbers
* Tested:  ISA 0.10

### plamapgen
* Purpose: Generate platform and mapping files for ESPAM.
* Authors: Eyal Halm, Sven van Haastregt
* Notes:   Standalone program, not dependent on any libraries.
* Input:   Commandline argument list containing name and mapping code.
* Output:  .pla and .map files


Bugs/Questions
--------------
In case of bugs/questions, please contact: [Sven van Haastregt](https://github.com/svenvh) or [Teddy Zhai](https://github.com/tzhai)



