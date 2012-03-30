PNTools
=======

This project contains various experimental tools for use with the [PN/ISA](http://repo.or.cz/w/isa.git) compiler. 


Compilation Instructions
------------------------
    ./autogen.sh
    ./configure --with-pngen=XXX --with-isa=YYY
    make
Replace XXX with your pngen top level directory (i.e., the directory where GMP, Syck, and libxml2 are installed).
In case that standard libaries are installed on Linux distribution, XXX does not play a roll anymore.

Replace YYY with your isa build directory (containing c2pdg, pn, pn2adg, libpdg.la etc.).


Adding New Tools
----------------
We recommend the coding guide written by [Applied Informatics](http://www.appinf.com/download/CppCodingStyleGuide.pdf)
throughout the development process in pntools.

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

### pdg1ana (not updated yet)
* Purpose: Example of how to extract information from a PDG.
* Authors: Sven van Haastregt
* Tested:  ISA 0.10

### adg2csdf
* Purpose: Converts a PPN (in the form of ADG) to a CSDF graph.
* Authors: Sven van Haastregt, Teddy Zhai
* Notes:   Implemented, needs further verification.
* Input:   `XXX-adg.yaml`
* Output:  `YYY.xml` (SDF3 format) or
           `YYY.gph` (Extended [StreamIt](http://groups.csail.mit.edu/cag/streamit/) format). See "Graph Formats" below.
* Tested:  ISA 0.11

### adgromgen
* Purpose: Generate ROM tables from an ADG.
* Author:  Sven van Haastregt
* Input:   `XXX.adg`
* Output:  VHDL fragments (to be consumed by the ESPAM ISE visitor).
* Tested:  `isa-0.11-229-g049a7f9`

### adgstat
* Purpose: Dump some statistics about an ADG, such as channel implementation cost estimates.
* Author:  Sven van Haastregt
* Input:   `XXX.adg`
* Output:  plaintext
* Tested:  `isa-0.11-196-g2af2525`

### mcmmodel
* Purpose: Derive an MCM modeling graph from a PDG (and later perhaps also ADG).
* Authors: Hristo Nikolov, Sven van Haastregt
* Input:   `XXXpn.yaml`
* Output:  `YYY.sdf` (SDF3 format)
* Tested:  `isa-0.11-229-g049a7f9`

### pdgtrans
* Purpose: Transform a PDG.
* Authors: Wouter de Zwijger, Sven van Haastregt
* Input:   `XXX.yaml`
* Output:  `YYY.yaml`
* Notes:   See "Specifying Transformations" below.

### ppnta (to be changed into adgta)
* Purpose: Do some throughput analysis on a PPN.
* Authors: Teddy Zhai, Sven van Haastregt
* Notes:   Implemented, needs further verification.
* Input:   `XXX.ppn`
* Output:  throughput numbers
* Tested:  ISA 0.10

### visset
* Purpose: Visualize 2D and 3D isl sets.
* Author:  Sven van Haastregt
* Input:   Reads an isl set from stdin.
* Output:  Writes a gnuplot script to stdout.
* Tested:  isl-0.09-33-gc7dc962

### plamapgen
* Purpose: Generate platform and mapping files for ESPAM.
* Authors: Eyal Halm, Sven van Haastregt
* Notes:   Standalone program, not dependent on any libraries.
* Input:   Commandline argument list containing name and mapping code.
* Output:  .pla and .map files


Graph Formats
-------------

### adg2csdf

`adg2csdf` is capable of exporting the ADG graph into either the [SDF For Free](http://www.es.ele.tue.nl/sdf3/) XML format or the Extended [StreamIt](http://groups.csail.mit.edu/cag/streamit/) format. The Extended StreamIt format is based on the original StreamIt format and adds support for CSDF graphs. Extended StreamIt graphs have an extension `.gph` as the original StreamIt graphs. A simple producer/consumer graph looks like this:

    node_number:2
    node:
	    id:0
	    name:ND_0
	    function:producer
	    length:1
	    wcet:5
	    port_number:1
	    port:
		    type:out
		    id:1
		    rate:1
    node:
	    id:1
	    name:ND_1
	    function:consumer
	    length:1
	    wcet:2
	    port_number:1
	    port:
		    type:in
		    id:1
		    rate:1
    edge_number:1
    edge:
        id:0
        name:ED_0
        src:0 1
        dst:1 1	

A graph can have multiple nodes and multiple edges. In turn, a node can have multiple ports. The `length` field in the node specifies the length of the function and production/consumption rates sequences in the CSDF graph (cf. `P_j` in the original CSDF article). The `wcet` field specifies the Worst-Case Execution Time (WCET) of the actor. The `src` and `dst` field in the edge are interpreted as follows: `src: src_actor_id port_id` and `dst: dst_actor_id port_id`.

`CSDFConverter.py` in [csdf-rtschedtools](https://github.com/mohamed/csdf-rtschedtools) is capable of accepting the Extended StreamIt graphs and producing the SDF For Free XML format and Compact StreamIt format. 


Specifying Transformations
--------------------------

### Example invocations

Transformations are applied on the PDG output by c2pdg.

Split node 0 in two nodes. One intersected with { S_0[j,i] : i >= 5 } and the other with { S_0[j,i] : j > 22}. The space of each specified set should match the space of the corresponding node domain.

    ./pdgtrans --domain-split --node 0 --sets "{ S_0[j,i] : i >= 5 }; { S_0[j,i] : j > 22}" < splitjoin.yaml

Split a 6-dimensional node 0 in 10*12*5 parts, where each of these new nodes is a unique offset combination of modulo 10 12 and 5 of dim 3, 4 and 5 respectively:

    ./pdgtrans --modulo-split --node 0 --factors 1,1,1,10,12,5 < splitjoin2.yaml

Plane cut by specifying cuts using PolyLib notation. One specifies the hyperplanes that divide the node domain as equalities.
In the following example we split node 1 into four parts:


* One with dim 0 lower or equal than 5 - 1.
* One with dim 0 between 5 and 10 - 1.
* One with dim 0 between 10 and 15 - 1.
* The last partition has as extra condition bigger than 15 in dim 0.

    ./pdgtrans --plane-split --node 1 --conditions "0 1 0 -5,0 1 0 -10,0 1 0 -15" < splitjoin.yaml

Plane cut by specifying the number of partitions per dimension.
Split node 3, by dividing the set in 2 parts over dimension 0. Then splitting each of these parts by dividing them in three parts over dimension 1.
While the algorithm tries to split each set in equal parts, giving more than one dimension might result in unequal parts.
A dimension can be skipped for splitting, by giving it a factor 1, meaning one part, therefor no split on this dimension.

    ./pdgtrans --plane-split --node 3 --factors 2,3 < splitjoin.yaml


Bugs/Questions
--------------
In case of bugs/questions, please contact: [Sven van Haastregt](https://github.com/svenvh) or [Teddy Zhai](https://github.com/tzhai)
