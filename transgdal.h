#ifndef HANDLE_H
#define HANDLE_H

#include "pdg.h"
#include "yaml.h"
#include <gmp.h>
#include <polylib/polylibgmp.h>
#include <iostream>

# include <sys/types.h>
# include <sys/time.h>
# include <stdarg.h>
# include <stdlib.h>
# include <stdio.h>
# include <string.h>
# include <assert.h>
# include <ctype.h>
# include <unistd.h>

#ifdef CLOOG_RUSAGE
# include <sys/resource.h>
#endif

using pdg::PDG;
using namespace std;
using namespace yaml;

const int 
    NAME = 1000,
    LINE = 1000;

class handle;

struct part_node
{
    part_node();
    int line;
    part_node *next;
};

struct part_partition
{
    bool possible;
    part_partition();
    int type;
    part_node *nodes;
    int nb_nodes;
    int factor;
    int iterator;
    part_partition *next;
};

class part
{
    public:
        part();
        ~part();
        void init_partitioning(handle*);
        void get_partitioning();
        void print_structure(FILE*);
        void set_partition_possible(int, bool);
        int get_nb_partitions();
        bool get_partition_possible(int);
        int get_nb_factor(int);
        int get_nb_iterator(int);
        int get_nb_nodes(int);
        int get_node_line(int, int);
        int get_partition_type(int);
        int line_nb_partition(int);
        int node_partition_type(int);
        int node_nb_factor(int);
    private:
        int nb_partitions;
        part_partition *partitions;
        handle *basic;
};

struct mp_node
{
    mp_node();
    int node;
    mp_node *next;
};

struct mp
{
    mp();
    int statement;
    int nb_nodes;
    mp_node *nodes;
    bool original;
    mp *next;
};

class mapping
{
    public:
        mapping();
        ~mapping();
        void init_mapping(handle*);
        void initial_mapping(int);
        int get_nb_nodes(int);
        int get_statement_node(int,int);
        void insert(mp*);
        void remove();
        void statement_mapping();
        bool empty();
        void add(int, int);
        void add(int);
        void add_partition(int, int);
        void min(int,int);
        void print(FILE*);
        void print_cloog_mapping(FILE*);
        void print_function_mapping(FILE*);
        int get_nb_statements();
    private:
        int nb_statements;
        mp* mappings;
        handle *basic; 
};

struct array_content
{
    array_content();
    char content[NAME];
    array_content *next;
};

struct param
{
    param();
    char name[NAME];
    char type[NAME];
    array_content *dimension_content;
    int nb_dimensions;
    bool iterator;
    bool constant;
    bool output;
    bool array;
    int nb_array;
    param *next;
};

struct func
{
    func();
    char name[NAME];
    char type[NAME];
    param *params;
    int nb_parameters;
    func *next;
};

class funcs
{
    public:
        funcs();
        ~funcs();
        void init_function(handle *);
        void print_function(FILE *, int);
        void print_function(FILE *, int, struct cloogoptions *options, struct clast_user_stmt *u);

        void print_functions(FILE *);
        void print_parameter(FILE *, func *, int);
        void print_parameter_declaration_call(FILE *, int, int, int);
        void print_parameter_declaration_output(FILE*, int);
        void print_function_declaration_call(FILE *, int, int, int);
        void print_function_declaration_variables(FILE *, int, int);
        void print_structure(FILE *);
        void print_variable_declarations(FILE *);
        void print_function_declarations(FILE *);
        void add_variable(char*);
        void add_variable(param *);
        void remove_element(char*, int , int);
        int is_iterator(int, char*);
        void print_iterator(FILE *, int, char*, struct cloogoptions *options, struct clast_user_stmt *u);
        bool parameter_declaration_output(int, param *);
        bool parameter_declaration_constant(int, param *);
        bool parameter_declaration(int, int, int, param*);
        bool is_loop_bracket(int);      
        void pdg_reset(PDG *);
        bool iterator(int, int);
        void set_functions(char**, int);
        func* get_function(int);
        param* get_parameter(int, int, int);
        void get_initial_variable(int&, int&, int, int, int);
        int get_nb_merge(int);
        int get_nb_variable(int, int, int);
        int get_nb_array(int, int, int, param*);
        void clear_functions();
    private:
        handle* basic;
        func *functions;
        param *variables;
        int nb_functions;
        PDG *pdg;
};

struct prg_line
{
    prg_line();
    char line[LINE];
    prg_line *next;
};

class prg
{
    public:
        prg();
        ~prg();
        void print_program(FILE*);
        void load_program(string);
        char *get_line(int);
        int get_nb_lines();
    private:
        int nb_lines;
        prg_line *lines;
};

class handle
{
    public:
        handle(PDG *, string);
        ~handle();
        void main_handle();
        void print_polyhedron(FILE *, int);
        void print_scattering(FILE *,int);
        void print_context_parameters(FILE *);
        void print_polyhedron_parameters(FILE *,int);
        void print_context(FILE *);
        int get_node(int);
        int get_node_line(int);
        int get_nb_polyhedrons(int);
        int get_total_nb_statements();
        int get_nb_scattering(int);
        int get_total_nb_scattering();
        char * get_function_call(int);
        void set_function_call();
        bool mergable(int, int, int, int, bool);

        bool pdg_contains(int);
        void pdg_reset(PDG *);        
        void pdg_copy_node(int);
        void pdg_partition();
        void pdg_prefix(int, int, int);
        void nodes2c();
        void set_node(int);
        void write_cloog();
        void print_preamble(FILE*);
        void print_postemble(FILE*);
        void print_polyhedron_plane(const vector< vector< int > > &el, int, int, int);
        void print_polyhedron_modulo(const vector< vector< int > > &el, int, int, int);
        void print_scattering_modulo(FILE*, int);
        void error(int);
        void print_errors(FILE *);
        bool dependences_analyzed;
        PDG * pdg;
        FILE *err_output;
        // int node;
        string input;
        char **function_call;
        funcs functions;
        mapping cloog_mapping;
        mapping function_mapping;
        part partitions;
        vector<bool> analyzed;
        prg program;
};

#endif // HANDLE_H
