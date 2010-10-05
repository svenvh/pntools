#include "transgdal.h" 

FILE *errors;

int minimum(int a, int b)
{
    if(a < b)
        return a;
    else return b;
}

int maximum(int a, int b)
{
    if(a > b)
        return a;
    else return b;
}

void array(int size, char *tmp)
{
    for(int i = 0; i < size; i++)
        tmp[i] = '\0';
}

/**********************************************************
 *
 *                 load program into memory 
 *
 **********************************************************/

prg_line::prg_line()
{
    array(LINE, line);
    next = NULL;
}

prg::prg()
{
    lines = NULL;
    nb_lines = 0;
}

prg::~prg()
{
    while(lines != NULL)
    {
        prg_line *tmp = lines;
        lines = lines->next;
        delete(tmp);
    }
}

void prg::print_program(FILE *output)
{
    fprintf(output, "PROGRAM: \n-------------------------------------------------------\n");
    for(int i = 1; i <= get_nb_lines(); i++)
        fprintf(output, "%d    %s", i, get_line(i));
    fprintf(output, "\n");
}

int prg::get_nb_lines()
{
    return nb_lines;
}

void prg::load_program(string input)
{
    char filename[200];
    strcpy(filename, input.c_str());
    strcat(filename, ".c");
    FILE * file = fopen(filename, "r");

    if(!file)
    {
      strcat(filename, "c");
      file = fopen(filename, "r");
    }
    
    if(file)
    {
        char buffer[LINE];
        prg_line *line;
        while(fgets(buffer, sizeof(buffer), file))
        {
            nb_lines++;
            if(lines == NULL)
            {
                lines = new prg_line;
                line = lines;
            }
            else
            {
                line->next = new prg_line;
                line = line->next;
            }
            strcpy(line->line, buffer);
        }
    }
    else
        fprintf(errors, "ERROR: no input file!\n");
}

char *prg::get_line(int line)
{
    if(line > nb_lines)
        fprintf(errors, "ERROR: requesting to high line number in program\n");
    else
    {
        prg_line *tmp = lines;
        for(int i = 1; i < line; i++)
            tmp = tmp->next;

        return tmp->line;
    }
    return NULL;
}

/**********************************************************
 *
 *                 partition structures 
 *
 **********************************************************/

part_node::part_node()
{
    line = 0;
    next = NULL;
}

part_partition::part_partition()
{
    possible = false;
    nodes = NULL;
    nb_nodes = 0;
    type = 0;
    factor = 0;
    iterator = 0;
    next = NULL;
}

part::part()
{
    partitions = NULL;
    nb_partitions = 0;
}

part::~part()
{
    while(partitions != NULL)
    {   
        while(partitions->nodes != NULL)
        {
            part_node *tmp = partitions->nodes;
            partitions->nodes = partitions->nodes->next;
            delete(tmp);
        }
        part_partition *tmp = partitions;
        partitions = partitions->next;
        delete(tmp);
    }
}

void part::init_partitioning(handle *tmp)
{
    basic = tmp;
}

void part::set_partition_possible(int nb_partition, bool p)
{
    part_partition *a_partition = partitions;
    for(int i = 1; i < nb_partition; i++)
        a_partition = a_partition->next;

    a_partition->possible = p;
}

void part::print_structure(FILE *output)
{
    fprintf(output, "TRANSFORMATION STRUCTURE:\n-------------------------------------------------\n");
    fprintf(output, "   transformations: %d\n", nb_partitions);
    part_partition *a_partition = partitions;
    for(int i = 0; i < get_nb_partitions(); i++)
    {
        fprintf(output, "    - \n");
        fprintf(output, "      type     : ");
        if(a_partition->type == 1)
            fprintf(output, "plane\n");
        else if(a_partition->type == 2)
            fprintf(output, "modulo\n");
        else if(a_partition->type == 3)
            fprintf(output, "merge\n");
        else
            fprintf(output, "none\n");
        if(a_partition->type != 3)
        {
            fprintf(output, "      iterator : %d\n", a_partition->iterator);
            fprintf(output, "      factor   : %d\n", a_partition->factor);
        }
        fprintf(output, "      nodes    : %d\n", a_partition->nb_nodes);

        for(part_node *a_node = a_partition->nodes; a_node != NULL; a_node = a_node->next)
        {
            fprintf(output, "        - %d\n", a_node->line);
        }
        a_partition = a_partition->next;
    }
    fprintf(output, "\n");
}

void part::get_partitioning()
{
    char filename[200];
    strcpy(filename, basic->input.c_str());
    strcat(filename, ".part");
    FILE * file = fopen(filename, "r");


    if(file)
    {
        char buffer[LINE];

        // read partitions
        fgets(buffer, sizeof(buffer), file);
        sscanf(buffer, "%d", &nb_partitions);

        part_partition *a_partition;
        for(int i = 0; i < nb_partitions; i++)
        {
            if(partitions == NULL)
            {
                partitions = new part_partition;
                a_partition = partitions;
            }
            else
            {
                a_partition->next = new part_partition;
                a_partition = a_partition->next;
            }

            // get partition type
            char type[LINE];
            fgets(buffer, sizeof(buffer), file);
            sscanf(buffer, "%s", &type);

            if(strcmp(type, "plane") == 0)
                a_partition->type = 1;
            else if(strcmp(type, "modulo") == 0)
                a_partition->type = 2;
            else if(strcmp(type, "merge") == 0)
                a_partition->type = 3;
            else
            {
                fprintf(errors, "ERROR: partition type not recognized, read aborted\n");
                nb_partitions = i;
                return;
            }


            // get nodes (s)
            part_node *a_node;
            int max = 10;
            fgets(buffer, sizeof(buffer), file);
            do
            {
                max--;

                a_partition->nb_nodes++;
                if(a_partition->nodes == NULL)
                {
                    a_partition->nodes = new part_node;
                    a_node = a_partition->nodes;
                }
                else
                {
                    a_node->next = new part_node;
                    a_node = a_node->next;
                }

                sscanf(buffer, "%d", &a_node->line);

                int pos;
                for(pos = 0; buffer[pos] != ' ' && buffer[pos] != '\n'; pos++);
                for(pos; buffer[pos] != '\0' && (buffer[pos] == ' ' || buffer[pos] =='\n'); pos++);
                for(int relocate = 0; pos < sizeof(buffer) - 1; pos++, relocate++)
                    buffer[relocate] = buffer[pos];
            } while(max > 0 && buffer[0] != '\0');
            
            // get factor
            if(a_partition->type != 3)
            {
                fgets(buffer, sizeof(buffer), file);
                sscanf(buffer, "%d", &a_partition->factor);

                // get iterator
                fgets(buffer, sizeof(buffer), file);
                sscanf(buffer, "%d", &a_partition->iterator);
            }
        }
        fclose(file);
    }
}

int part::get_nb_partitions()
{
    return nb_partitions;
}

int part::get_nb_nodes(int nb_partition)
{
    part_partition *a_partition = partitions;
    for(int i = 1; i < nb_partition; i++)
        a_partition = a_partition->next;

    return a_partition->nb_nodes;
}

bool part::get_partition_possible(int nb_partition)
{
    part_partition *a_partition = partitions;
    for(int i = 1; i < nb_partition; i++)
        a_partition = a_partition->next;

    return a_partition->possible;
}

int part::get_nb_factor(int nb_partition)
{
    part_partition *a_partition = partitions;
    for(int i = 1; i < nb_partition; i++)
        a_partition = a_partition->next;

    return a_partition->factor;
}

int part::get_node_line(int nb_partition, int nb_node)
{
    part_partition *a_partition = partitions;
    for(int i = 1; i < nb_partition; i++)
        a_partition = a_partition->next;

    part_node *a_node = a_partition->nodes;
    for(int i = 1; i < nb_node; i++)
        a_node = a_node->next;

    return a_node->line;
}

int part::get_nb_iterator(int nb_partition)
{
    part_partition *a_partition = partitions;
    for(int i = 1; i < nb_partition; i++)
        a_partition = a_partition->next;

    return a_partition->iterator;
}

int part::get_partition_type(int nb_partition)
{
    part_partition *a_partition = partitions;
    for(int i = 1; i < nb_partition; i++)
        a_partition = a_partition->next;

    return a_partition->type;
}

int part::node_partition_type(int line)
{
    for(int nb_partition = 1; nb_partition <= get_nb_partitions(); nb_partition++)
    {
        if(get_node_line(nb_partition, 1) == line)
        {
            if(get_partition_type(nb_partition) == 1)
                return 1;
            else if(get_partition_type(nb_partition) == 2)
                return 2;
        }
    }
    return 0;
}

int part::node_nb_factor(int line)
{
    for(int nb_partition = 1; nb_partition <= get_nb_partitions(); nb_partition++)
        if(get_node_line(nb_partition, 1) == line)
            return get_nb_factor(nb_partition);
}

int part::line_nb_partition(int line)
{
    for(int nb_partition = 1; nb_partition <= get_nb_partitions(); nb_partition++)
        for(int nb_node = 1; nb_node <= get_nb_nodes(nb_partition); nb_node++)
            if(get_node_line(nb_partition, 1) == line)
                return nb_partition;
}

/**********************************************************
 *
 *             mapping statement to node
 *
 **********************************************************/

mp_node::mp_node()
{
    node = -1;
    next = NULL;
}

mp::mp()
{
    statement = -1;
    nb_nodes = 0;
    nodes = NULL;
    original = true;
    next = NULL;
}

mapping::mapping()
{
    mappings = NULL;
    nb_statements = 0;
}

mapping::~mapping()
{
    remove();
}

void mapping::init_mapping(handle *tmp)
{
    basic = tmp;
}

void mapping::initial_mapping(int nb_nodes)
{
    for(int node = 0; node < nb_nodes; node++)
        add(node + 1, node);
}

void mapping::insert(mp *m)
{
    if(empty() || mappings->statement == m->statement)
    {
        m->next = mappings;
        mappings = m;
    }
    else
    {
        mp *current = mappings,
            *previous = NULL;

        while(current != NULL && current->statement != m->statement)
        {
            previous = current;
            current = current->next;
        }
        m->next = current;
        previous->next = m;
        
        while(current != NULL)
        {
            current->statement++;
            current = current->next;
        }
    }
}

void mapping::add_partition(int node, int factor)
{
    int statement = -1;
    for(int i = 0; i < factor; i++)
    {
        if(i == 0)
        {
            mp *a_mapping = mappings;
            while(a_mapping != NULL)
            {
                mp_node * a_node = a_mapping->nodes;
                while(a_node != NULL)
                {
                    if(a_node->node == node)
                    {
                        statement = a_mapping->statement; 
                        a_mapping->original = false;
                    }
                    a_node = a_node->next;
                }
                a_mapping = a_mapping->next;
            }
        }
        else
        {
            mp *current = mappings;
            mp *previous =  NULL;
            while(current != NULL)
            {
                if(current->nodes->node == node)
                { // insert
                    statement++;
                    mp *m = new mp;
                    m->nodes = new mp_node;
                    m->nodes->node = node;
                    m->nb_nodes = 1;
                    m->original = false;
                    m->next = current;
                    m->statement = statement;
                    nb_statements++; 
                    previous->next = m;

                    while(current != NULL)
                    {
                        current->statement++;
                        current = current->next;
                    }
                    break;
                }
                previous = current;
                current = current->next;
            }
            // increment node numbers above and equal to 'node' value
            current = mappings;
            while(current != NULL)
            {
                if(current != previous->next)
                { // don't increment the mapping we just added
                    mp_node *a_node = current->nodes;
                    while(a_node != NULL)
                    {
                        if(a_node->node >= node)
                            a_node->node++;
                        a_node = a_node->next;

                    }
                }
                current = current->next;
            }
        }
        node++;
    }
}

void mapping::remove()
{
    nb_statements = 0;
    mp *a_map = mappings;
    while(!empty())
    {
        while(mappings->nodes != NULL)
        {
            mp_node *a_node = mappings->nodes;
            mappings->nodes = mappings->nodes->next;
            delete(a_node);
        }
        a_map = mappings;
        mappings = mappings->next;
        delete(a_map);
    }
}

int mapping::get_nb_statements()
{
    return nb_statements;
}

int mapping::get_nb_nodes(int nb_statement)
{
    mp *a_statement = mappings;
    for(int i = 1; i < nb_statement; i++)
        a_statement = a_statement->next;

    return a_statement->nb_nodes;
}

int mapping::get_statement_node(int nb_statement, int nb_node)
{
    mp *a_statement = mappings;
    for(int i = 1; i < nb_statement; i++)
        a_statement = a_statement->next;

    mp_node *a_node = a_statement->nodes;
    for(int i = 1; i < nb_node; i++)
        a_node = a_node->next;

    return a_node->node;
}

bool mapping::empty()
{
    return mappings == NULL;
}

void mapping::add(int stmt, int nd)
{
    mp *tmp = new mp;
    tmp->statement = stmt;
    tmp->original = true;
    tmp->nodes = new mp_node;
    tmp->nodes->node = nd;
    tmp->nb_nodes = 1;
    tmp->next = NULL;
    nb_statements++;
    insert(tmp);
}

void mapping::statement_mapping()
{
    mp *a_statement = mappings;
    for(int i = 1; a_statement != NULL; a_statement = a_statement->next, i++)
        a_statement->statement = i;
}

void mapping::min(int node1, int node2)
{
    mp *a_map_1 = mappings;

    // find first merge node
    bool found = false;
    while(!found)
    {
        mp_node *a_node = a_map_1->nodes;
        while(a_node != NULL)
        {
            if(a_node->node == node1)
                found = true;
            a_node = a_node->next;
        }
        
        if(!found)
            a_map_1 = a_map_1->next;
    }
    a_map_1->original = false;

    // find seconde merge node
    mp *a_map_2 = mappings;
    mp *previous_map_2;
    found = false;
    for(mp_node *a_node = mappings->nodes; a_node != NULL; a_node = a_node->next)
        if(a_node->node == node2)
            found = true;

    if(found)
        mappings = mappings->next;
    else
    {
        while(!found && a_map_2 != NULL)
        {
            for(mp_node *a_node = a_map_2->nodes; a_node != NULL; a_node = a_node->next)
                if(a_node->node == node2)
                    found = true;

            if(!found)
            {
                previous_map_2 = a_map_2;
                a_map_2 = a_map_2->next;
            }
        }
        previous_map_2->next = a_map_2->next;
    }
    
    // transfer nodes
    mp_node *a_node = a_map_1->nodes;

    // add nodes to the end! scattering function depends on the lowest node
    while(a_node->next != NULL)
        a_node = a_node->next;

    a_node->next = a_map_2->nodes;
    a_map_1->nb_nodes += a_map_2->nb_nodes;
    delete(a_map_2);
    nb_statements--;
    statement_mapping();
}

void mapping::add(int nd)
{ // FIXME: assuming there are already values in the mapping table
    mp *m = new mp;
    m->nodes = new mp_node;
    m->nodes->node = nd;
    m->nb_nodes = 1;
    m->original = false;
    m->next = NULL;
    nb_statements++;

    mp *current = mappings;
    if(mappings->nodes->node == m->nodes->node)
    { // FIXME: mapping might have more than one node due to previous merging
        if(mappings->original)
        {
            mp *tmp = mappings;
            m->statement = 1;
            mappings = mappings->next;
            m->next = mappings;
            nb_statements--;
            mappings = m;
            current = NULL;
            delete(tmp);
        }
        else
        {
            m->next = mappings;
            m->statement = mappings->statement;
            mappings = m;
        }
    }
    else
    {
        mp *previous = NULL;

        while(current != NULL && current->nodes->node != m->nodes->node)
        {
            previous = current;
            current = current->next;
        }

        previous->next = m;
        if(current != NULL)
        {
            m->statement = current->statement;
            if(current->original)
            {
                nb_statements--;
                mp *tmp = current;
                current = current->next;
                delete(tmp);
            }
        }
        else 
            m->statement = nb_statements;
        m->next = current;
    }
    statement_mapping();
    /*
    while(current != NULL)
    {
        current->statement++;
        current = current->next;
    }
    */
}

void mapping::print_cloog_mapping(FILE *output)
{
    fprintf(output, "\nCLOOG MAPPING: %d elements\n------------------------------------------------\n", nb_statements);
    print(output);

}

void mapping::print_function_mapping(FILE *output)
{
    fprintf(output, "\nFUNCTION MAPPING: %d elements\n------------------------------------------------\n", nb_statements);
    print(output);
}

void mapping::print(FILE *output)
{
    mp *a_statement = mappings;

    for(int i = 1; a_statement != NULL; a_statement = a_statement->next, i++)
    {
        
        fprintf(output, "\t%d\t", a_statement->statement);

        mp_node *a_node = a_statement->nodes;

        while(a_node != NULL)
        {
            if(a_statement->nodes != a_node)
                fprintf(output, ",");
            fprintf(output, "%d", a_node->node);
            a_node = a_node->next;
        }
       /* 
        for(int k = 1; k <= get_nb_nodes(i); k++)
            fprintf(output, ",%d", get_statement_node(i, k));
        */
        if(a_statement->original)
            fprintf(output, "*");
        fprintf(output, "\n");
    }
    fprintf(output, "\n *original nodes\n");
}

/**********************************************************
 *
 *           store structure of funtions 
 *
 **********************************************************/

array_content::array_content()
{
    next = NULL;
}

param::param  ()
{
    iterator = false;
    output = false;
    constant = false;
    nb_dimensions = 0;
    array = false;
    nb_array = 0;
    next = NULL;
    dimension_content = NULL;
}

func::func()
{
    params = NULL;
    nb_parameters = 0;
    next = NULL;
}

funcs::funcs()
{
    functions = NULL;
    variables = NULL;
    nb_functions = 0;
}

void funcs::pdg_reset(PDG *tmp)
{
    pdg = tmp;
}

void funcs::init_function(handle *tmp)
{
    basic = tmp;
    pdg = basic->pdg;
} 

funcs::~funcs()
{
    clear_functions();
}

void funcs::print_parameter(FILE * output, func * f, int par)
{
    param * tmp = f->params;
    for(int i = f->nb_parameters - par; i > 0; i--)
        tmp = tmp->next;

    if(tmp->output)
        fprintf(output, "&");
    if(tmp->iterator)
        fprintf(output, "iterator");
    else
    fprintf(output, "%s", tmp->name);
}

void funcs::print_function(FILE *output, int statement)
{
    func *tmp = functions;
    for(int i = nb_functions - statement; i > 0; i--)
        tmp = tmp->next;
   
    fprintf(output, "%s", tmp->name);
    fprintf(output, "(");
    for(int i = 0; i < tmp->nb_parameters; i++)
    {
        if(i != 0)
            fprintf(output, ",");

        print_parameter(output, tmp, i + 1);
    } 
    fprintf(output, ");\n");
}

void funcs::get_initial_variable(int &rt_node, int &rt_parameter, int nb_statement, int nb_nodes, int nb_parameters)
{
    int a_nb_function = basic->function_mapping.get_statement_node(nb_statement, nb_nodes) + 1;
    func *a_function = get_function(a_nb_function);
    param *a_parameter = a_function->params;
    for(int i = a_function->nb_parameters - nb_parameters; i > 0; i--)
        a_parameter = a_parameter->next;

    bool found;
    for(int nb_node = 1; nb_node <= nb_nodes; nb_node++)
    {
        int nb_function = basic->function_mapping.get_statement_node(nb_statement, nb_node) + 1;
        func *function = get_function(nb_function);

        for(int nb_parameter = 1; nb_parameter <= function->nb_parameters; nb_parameter++)
        {
            if(nb_nodes == nb_node && nb_parameters == nb_parameter)
            {
                rt_node = nb_node;
                rt_parameter = nb_parameter;
                return;
            }

            param *parameter = function->params;
            for(int i = function->nb_parameters - nb_parameter; i > 0; i--)
                parameter = parameter->next;

            found = true;
            if(strcmp(parameter->name, a_parameter->name) == 0 && parameter->constant == a_parameter->constant)
            {
                if(a_parameter->array && parameter->array)
                {
                    array_content *tmp1 = parameter->dimension_content, *tmp2 = a_parameter->dimension_content;
                    while(tmp1 != NULL && tmp2 != NULL)
                    {
                        if(strcmp(tmp1->content, tmp2->content) != 0)
                           found = false;
                        tmp1 = tmp1->next;
                        tmp2 = tmp2->next;
                    }
                    if(tmp1 != NULL || tmp2 != NULL)
                        found = false;
                }
            }
            else
                found = false;

            if(found)
            {
                rt_node = nb_node;
                rt_parameter = nb_parameter;
                return;
            }
        }
    }
    fprintf(errors, "ERROR: array count of merge parameters failed!\n");
}

void funcs::print_parameter_declaration_call(FILE * output, int nb_statement, int nb_node, int nb_parameter)
{
    int nb_function;
    func *function;
    param * parameter;
   
    bool out;
    nb_function = basic->function_mapping.get_statement_node(nb_statement, nb_node) + 1;
    function = get_function(nb_function);
    parameter = function->params;
    for(int i = function->nb_parameters - nb_parameter; i > 0; i--)
        parameter = parameter->next;
    out = parameter->output;

    get_initial_variable(nb_node, nb_parameter, nb_statement, nb_node, nb_parameter);

    nb_function = basic->function_mapping.get_statement_node(nb_statement, nb_node) + 1;
    function = get_function(nb_function);
    
    parameter = function->params;
    for(int i = function->nb_parameters - nb_parameter; i > 0; i--)
        parameter = parameter->next;

    if(out && !parameter->output)
        fprintf(output, "&");
    else if(!out && parameter->output)
        fprintf(output, "*");

    fprintf(output, "%s", parameter->name);
    if(parameter->array)
        fprintf(output, "_%d", get_nb_variable(nb_statement, nb_node, nb_parameter));
}

void funcs::print_function_declaration_call(FILE *output, int nb_function, int nb_statement, int nb_node)
{
    func *tmp = functions;
    for(int i = nb_functions - nb_function; i > 0; i--)
        tmp = tmp->next;
   
    fprintf(output, "%s", tmp->name);
    fprintf(output, "(");
    for(int i = 0; i < tmp->nb_parameters; i++)
    {
        if(i != 0)
            fprintf(output, ", ");

        print_parameter_declaration_call(output, nb_statement, nb_node, i + 1);
    } 
    fprintf(output, ");\n");
}

bool funcs::iterator(int function, int parameter)
{
    func *foo = functions;
    for(int i = nb_functions - function; i > 0; i--)
        foo = foo->next;
   
    param *bar = foo->params;
    for(int i = foo->nb_parameters - parameter; i > 0; i--)
        bar = bar->next;
    return bar->iterator;
}

func* funcs::get_function(int nb_function)
{
    func *tmp = functions;
    for(int i = nb_functions - nb_function; i > 0; i--)
        tmp = tmp->next;

    return tmp;
}

int funcs::get_nb_merge(int nb_statements)
{
    int nb = 1;
    for(int nb_statement = 1; nb_statement < nb_statements; nb_statement++)
    {
        if(basic->function_mapping.get_nb_nodes(nb_statement) > 1)
            nb++;
    }
    return nb;
}


void funcs::print_structure(FILE *output)
{
    fprintf(output, "FUNCTION CALLS:\n------------------------------------------\n");

    func *function;
    int nb_functions = 0;
    for(function = functions; function != NULL; function = function->next)
        nb_functions++;

    for(int i = 1; i <= nb_functions; i++)
    {
        function = functions;
        for(int f = nb_functions - i; f > 0; f--)
            function = function->next;
        
        fprintf(output, "function %d : %s\n", i, function->name);

        param *parameter;
        int nb_parameters = 0;
        for(parameter = function->params; parameter != NULL; parameter = parameter->next)
            nb_parameters++;
        
        for(int j = 1; j <= nb_parameters; j++)
        {
            parameter = function->params;
            for(int g = nb_parameters - j; g > 0; g--)
                parameter = parameter->next;

            fprintf(output, "   parameter %d: (", j);
            if(parameter->constant)
                fprintf(output, "const ");
            fprintf(output,"%s) ", parameter->type);
            if(parameter->output)
                fprintf(output, "&");
            fprintf(output, "%s", parameter->name);
            if(parameter->iterator)
                fprintf(output, " [iterator]");
            fprintf(output, "\n");
        }
        fprintf(output, "\n");
    }
}

bool funcs::is_loop_bracket(int line)
{
    char *buffer;
    int level = 1;
    for(int i = line - 1; i > 0; i--)
    {
        buffer = basic->program.get_line(i);
        for(int x = 0; x < strlen(buffer); x++)
        {
            if(buffer[x] == '{')
                level--;
            else if(buffer[x] == '}')
                level++;
        }
        if(level == 0)
        {
            if(strlen(buffer) > 4)
            {
                char buff1[LINE];
                sscanf(buffer, "%s" , &buff1);
                buff1[sizeof("for") - 1] = '\0';
                if(strcmp(buff1,"for") == 0)
                    return true;
                else
                    return false;
            }
            else
                return false;
        }
    }
    return false;
}

int funcs::is_iterator(int nb_function, char *it)
{
    char *buffer;
    int level = pdg->nodes[nb_function - 1]->source->dim;
    int line = pdg->nodes[nb_function - 1]->statement->line - 1;
    bool zero = false;
    bool last = false;

    for(line; line > 0; line--)
    {
        buffer = basic->program.get_line(line);
        for(int x = 0; x < strlen(buffer); x++)
        {
            if(buffer[x] == '{')
            { // which does not belong to an if statement
                if(strlen(buffer) > 4)
                {
                    char buff1[LINE];
                    sscanf(buffer, "%s" , &buff1);
                    buff1[sizeof("for") - 1] = '\0';
                    if(strcmp(buff1,"for") == 0)
                        level--;
                }
            }
            else if(buffer[x] == '}')
            {
                if(is_loop_bracket(line))
                    level++;
            }
        }
        if(level == 0)
            zero = true;


        if(strlen(buffer) > 4)
        {
            char buff1[LINE];
            sscanf(buffer, "%s" , &buff1);
            buff1[sizeof("for") - 1] = '\0';
            if(strcmp(buff1,"for") == 0 && level < pdg->nodes[nb_function - 1]->source->dim)
            {
                if(zero)
                    last = true;

                int index = 0;

                int count = 0;
                for(index; count != 2; index++)
                    if(buffer[index] == ';')
                        count++;

                bool restart = true;
                bool stop = false;
                char cmp[NAME];
                int cp;
                for(index; buffer[index] != ')'; index++)
                {
                    if(restart)
                    {
                        cp = 0;
                        array(NAME,cmp);
                        restart = false;
                    }
                    if(!stop)
                    {
                        if((buffer[index] == '+' && buffer[index + 1] == '+') || buffer[index] == '=')
                        {
                            stop = true;
                            // compair iterator with argument value
                            if(strcmp(cmp, it) == 0)
                                return level + 1;
                        }
                        if(buffer[index] != ' ' && buffer[index] != '+' && buffer[index] != '=')
                        {
                            cmp[cp] = buffer[index];
                            cp++;
                        }
                    }
                    else if(buffer[index] == ',')
                    {
                        restart = true;
                        stop = false;
                    }
                }
            }
        }
        if(last)
        {
            if(level != 0)
                fprintf(errors, "ERROR: iterator level calculation incorrect %d\n", level);
            return level; // should always be 0!!
        }
        if(zero)
            last = true;

    }
    return 0;
}

void funcs::add_variable(char* variable)
{

    param *tmp = variables;
    bool contain = false;
    while(tmp != NULL)
    {
        if(strcmp(variable, tmp->name) == 0)
            contain = true;
        tmp = tmp->next;
    }
    
    if(!contain)
    {
        tmp = new param;
        array(NAME,tmp->name);
        tmp->next = variables;
        strcpy(tmp->name, variable);
        tmp->iterator = true;
        variables = tmp;
    }
}

void funcs::add_variable(param* parameter)
{
    param *tmp = variables;
    bool contain = false;
    while(tmp != NULL)
    {
        if(strcmp(tmp->name, parameter->name) == 0)
            contain = true;
        tmp = tmp->next;
    }
    if(!contain)
    {
        tmp = new param;
        array(NAME,tmp->name);

        strcpy(tmp->name, parameter->name);
        strcpy(tmp->type, parameter->type);
        tmp->iterator = parameter->iterator;
        tmp->array = parameter->array;
        tmp->constant = parameter->constant;
        tmp->output = parameter->output;
        tmp->nb_dimensions = parameter->nb_dimensions;

        tmp->next = variables;
        variables = tmp;
    }
}

void funcs::print_variable_declarations(FILE *output)
{
    param *tmp = variables;
    while(tmp != NULL)
    {
        if(tmp->iterator)
        {   // possible FIXME: iterators are always integers
            fprintf(output,"int ");
            fprintf(output, "%s;\n", tmp->name); 
        }
        else
        {
            fprintf(output,"%s ", tmp->type);
            fprintf(output, "%s", tmp->name);
            if(tmp->array)
            {
                for(int i = 0; i < pdg->arrays.size(); i++)
                {
                    if(strcmp(pdg->arrays[i]->name->s.c_str(), tmp->name) == 0)
                        for(int j = 0; j < pdg->arrays[i]->dims.size(); j++)
                            fprintf(output, "[%d]", pdg->arrays[i]->dims[j]);
                }
            }
            fprintf(output, ";\n");
        }
        tmp = tmp->next;
    }
}

bool funcs::parameter_declaration(int nb_statement, int nb_nodes, int nb_parameters, param *a_parameter)
{
//    fprintf(stdout, "parameter_declaration(nb_statement: %d, nb_nodes: %d, nb_parameters: %d, parameter: %s_%d\n", nb_statement, nb_nodes, nb_parameters, a_parameter->name, get_nb_array(nb_statement, a_parameter));
    for(int nb_node = 1; nb_node <= nb_nodes; nb_node++)
    {
        int nb_function = basic->function_mapping.get_statement_node(nb_statement, nb_node) + 1;
        func *function = get_function(nb_function);

        for(int nb_parameter = 1; nb_parameter <= function->nb_parameters; nb_parameter++)
        {
  //          fprintf(stdout, "%d == %d, %d == %d\n", nb_node, nb_nodes, nb_parameter, nb_parameters);
            if(nb_node == nb_nodes && nb_parameter == nb_parameters)
                break;

            param *parameter = function->params;
            for(int i = function->nb_parameters - nb_parameter; i > 0; i--)
                parameter = parameter->next;
            
            bool found = true;
    //        fprintf(stdout, "compair '%s' - '%s'\n", a_parameter->name, parameter->name);
            if(strcmp(parameter->name, a_parameter->name) == 0)
            {
                if(a_parameter->array && parameter->array)
                {
                    array_content *tmp1 = parameter->dimension_content, *tmp2 = a_parameter->dimension_content;
                    while(tmp1 != NULL && tmp2 != NULL)
                    {
      //                  fprintf(stdout, "content '%s' - '%s'\n", tmp1->content, tmp2->content);
                        if(strcmp(tmp1->content, tmp2->content) != 0)
                           found = false;
                        tmp1 = tmp1->next;
                        tmp2 = tmp2->next;
                    }
                    if(tmp1 != NULL || tmp2 != NULL)
                        found = false;
                }
            }
            else
                found = false;

            if(found)
            {
        //        fprintf(stdout, "redeclaration!\n");
                return false;
            }
         }
    }
    return true;
}

bool funcs::parameter_declaration_output(int nb_statement, param *a_parameter)
{
    for(int nb_node = 1; nb_node <= basic->function_mapping.get_nb_nodes(nb_statement); nb_node++)
    {
        int nb_function = basic->function_mapping.get_statement_node(nb_statement, nb_node) + 1;
        func *function = get_function(nb_function);
        param *parameter = function->params;

        while(parameter != NULL)
        {
            if(strcmp(a_parameter->name,parameter->name) == 0 && parameter->nb_array == a_parameter->nb_array && parameter->output)
            {
                return true;
            }
            parameter = parameter->next;
        }
    }
    return false;
}

bool funcs::parameter_declaration_constant(int nb_statement, param *a_parameter)
{
    for(int nb_node = 1; nb_node <= basic->function_mapping.get_nb_nodes(nb_statement); nb_node++)
    {
        int nb_function = basic->function_mapping.get_statement_node(nb_statement, nb_node) + 1;
        func *function = get_function(nb_function);
        param *parameter = function->params;

        while(parameter != NULL)
        {
            bool found = true;
            if(strcmp(parameter->name, a_parameter->name) == 0)
            {
                if(a_parameter->array && parameter->array)
                {
                    array_content *tmp1 = parameter->dimension_content, *tmp2 = a_parameter->dimension_content;
                    while(tmp1 != NULL && tmp2 != NULL)
                    {
                        if(strcmp(tmp1->content, tmp2->content) != 0)
                           found = false;
                        tmp1 = tmp1->next;
                        tmp2 = tmp2->next;
                    }
                    if(tmp1 != NULL || tmp2 != NULL)
                        found = false;
                }
            }
            else
                found = false;

            if(found && !parameter->constant)
                return false;

            parameter = parameter->next;
        }
    }

    if(a_parameter->constant)
        return true;
    else
        return false;
}

int funcs::get_nb_array(int nb_statement, int nb_nodes, int nb_parameters, param *a_parameter)
{
    int count = 0;
    bool found;
    for(int nb_node = 1; nb_node <= nb_nodes; nb_node++)
    {
        int nb_function = basic->function_mapping.get_statement_node(nb_statement, nb_node) + 1;
        func *function = get_function(nb_function);

        for(int nb_parameter = 1; nb_parameter <= function->nb_parameters; nb_parameter++)
        {
            if(nb_nodes == nb_node && nb_parameters == nb_parameter)
                return count + 1;

            param *parameter = function->params;
            for(int i = function->nb_parameters - nb_parameter; i > 0; i--)
                parameter = parameter->next;

            if(parameter->array && a_parameter->array && strcmp(parameter->name, a_parameter->name) == 0)
                count++;
        }
    }
    fprintf(errors, "ERROR: array count of merge parameters failed!\n");
    return 9999;
}

void funcs::print_function_declarations(FILE *output)
{
    bool printed = false;
    for(int nb_statement = 1; nb_statement <= basic->function_mapping.get_nb_statements(); nb_statement++)
    {
        if(basic->function_mapping.get_nb_nodes(nb_statement) > 1)
        {
            fprintf(output, "void ");
            for(int nb_node = 1; nb_node <= basic->function_mapping.get_nb_nodes(nb_statement); nb_node++)
            {
                int nb_function = basic->function_mapping.get_statement_node(nb_statement, nb_node) + 1;
                func *function = get_function(nb_function);
                fprintf(output, "%s", function->name); 
            }
            fprintf(output, "_%d", get_nb_merge(nb_statement));
 
            fprintf(output, "(");

            for(int nb_node = 1; nb_node <= basic->function_mapping.get_nb_nodes(nb_statement); nb_node++)
            {
                int nb_function = basic->function_mapping.get_statement_node(nb_statement, nb_node) + 1;
                func *function = get_function(nb_function);
                
                for(int nb_parameter = 1; nb_parameter <= function->nb_parameters; nb_parameter++)
                {
                    param *parameter = function->params;
                    for(int i = function->nb_parameters - nb_parameter; i > 0; i--)
                        parameter = parameter->next;
                    /*
                    bool print = true;
                    print = parameter_declaration(nb_statement, nb_node, nb_parameter, parameter);

                    if(print)
                    {
                        if(printed)
                            fprintf(output, ", ");
                        if(parameter_declaration_constant(nb_statement, parameter))
                            fprintf(output, "const ");
                        fprintf(output, "%s ", parameter->type);
                        if(parameter_declaration_output(nb_statement, parameter))
                            fprintf(output, "*");
                        fprintf(output, "%s", parameter->name);
                        if(parameter->array)
                        {
                            parameter->nb_array = get_nb_array(nb_statement, parameter);
                            fprintf(output, "_%d", parameter->nb_array);
                        }    
                        printed = true;
                        */
                    if(printed)
                        fprintf(output, ", ");
                    if(parameter_declaration_constant(nb_statement, parameter))
                        fprintf(output, "const ");
                    fprintf(output, "%s ", parameter->type);
                    if(parameter->output)
                        fprintf(output, "*");
                    fprintf(output, "%s", parameter->name);
                    fprintf(output, "_%d", get_nb_variable(nb_statement, nb_node, nb_parameter));

                    printed = true;
                }
            }
            fprintf(output, "){\n");

            // print function calls
            for(int nb_node = 1; nb_node <= basic->function_mapping.get_nb_nodes(nb_statement); nb_node++)
            {
                int nb_function = basic->function_mapping.get_statement_node(nb_statement, nb_node) + 1;
                fprintf(output, "  ");
                print_function_declaration_call(output, nb_function, nb_statement, nb_node);
            }
            print_parameter_declaration_output(output, nb_statement);
            fprintf(output, "}\n\n");
        }
    }
}

void funcs::print_parameter_declaration_output(FILE *output, int nb_statement)
{
    bool found;
    for(int a_nb_node = 1; a_nb_node <= basic->function_mapping.get_nb_nodes(nb_statement); a_nb_node++)
    {
        int a_nb_function = basic->function_mapping.get_statement_node(nb_statement, a_nb_node) + 1;
        func *a_function = get_function(a_nb_function);

        for(int a_nb_parameter = 1; a_nb_parameter <= a_function->nb_parameters; a_nb_parameter++)
        {
            param *a_parameter = a_function->params;
            for(int i = a_function->nb_parameters - a_nb_parameter; i > 0; i--)
                a_parameter = a_parameter->next;
        
            if(a_parameter->output)
            {
                int nb_node = a_nb_node;
                int nb_parameter = a_nb_parameter;

                get_initial_variable(nb_node, nb_parameter, nb_statement, a_nb_node, a_nb_parameter);
                if(nb_node != a_nb_node || nb_parameter != a_nb_parameter)
                {
                    int nb_function = basic->function_mapping.get_statement_node(nb_statement, nb_node) + 1;
                    func *function = get_function(nb_function);

                    param *parameter = function->params;
                    for(int i = function->nb_parameters - nb_parameter; i > 0; i--)
                        parameter = parameter->next;

                    fprintf(output, "  *%s_%d = ", a_parameter->name, get_nb_variable(nb_statement, a_nb_node, a_nb_parameter));
                    int par = get_nb_variable(nb_statement, nb_node, nb_parameter);

                    if(get_parameter(nb_statement, nb_node, nb_parameter)->output)
                        fprintf(output, "*");
                    fprintf(output, "%s_%d;\n", parameter->name, get_nb_variable(nb_statement, nb_node, nb_parameter));
                }
            }
        }
    }
}

param* funcs::get_parameter(int nb_statement, int nb_nodes, int nb_parameters)
{
    for(int nb_node = 1; nb_node <= basic->function_mapping.get_nb_nodes(nb_statement); nb_node++)
    {
        int nb_function = basic->function_mapping.get_statement_node(nb_statement, nb_node) + 1;
        func *function = get_function(nb_function);

        for(int nb_parameter = 1; nb_parameter <= function->nb_parameters; nb_parameter++)
        {
            param *parameter = function->params;
            for(int i = function->nb_parameters - nb_parameter; i > 0; i--)
                parameter = parameter->next;

            if(nb_node == nb_nodes && nb_parameter == nb_parameters)
                return parameter;
        }
    }
}

int funcs::get_nb_variable(int nb_statement, int nb_nodes, int nb_parameters)
{   
    int count = 0;
    bool found;
    for(int nb_node = 1; nb_node <= nb_nodes; nb_node++)
    {
        int nb_function = basic->function_mapping.get_statement_node(nb_statement, nb_node) + 1;
        func *function = get_function(nb_function);

        for(int nb_parameter = 1; nb_parameter <= function->nb_parameters; nb_parameter++)
        {
            if(nb_nodes == nb_node && nb_parameters == nb_parameter)
                return count + 1;

                count++;
        }
    }
    fprintf(errors, "ERROR: variable count of merge parameters incorrect!\n");
    return 9999;
}

void funcs::print_function_declaration_variables(FILE *output, int nb_statement, int nb_node)
{
/*    int nb_function1 = basic->function_mapping.get_statement_node(nb_statement, nb_node) + 1;
        func *function1 = get_function(nb_function1);

    for(int nb_node = 1; nb_node <= basic->function_mapping.get_nb_nodes(nb_statement); nb_node++)
    {
        int nb_function2 = basic->function_mapping.get_statement_node(nb_statement, nb_node) + 1;
        func *function2 = get_function(nb_function2);

        for(int nb_parameter = 1; nb_parameter <= function2->nb_parameters; nb_parameter++)
        {
            param *parameter = function->params;
            for(int i = function->nb_parameters - nb_parameter; i > 0; i--)
                parameter = parameter->next;

        }
    }
*/
}

void funcs::remove_element(char *tmp, int MAX, int element)
{
    for(int i = element; i < MAX - 1; i++)
        tmp[i] = tmp[i + 1];
}


void funcs::clear_functions()
{
    while(functions != NULL)
    {
        while(functions->params != NULL)
        {
            param *tmp = functions->params;
            functions->params = functions->params->next;
            delete(tmp);
        }
        func *tmp = functions;
        functions = functions->next;
        delete(tmp);
    }
    while(variables != NULL)
    {
        param *tmp = variables;
        variables = variables->next;
        delete(tmp);
    }

    variables = NULL;
    functions = NULL;
    nb_functions = 0;
}


void funcs::set_functions(char ** calls, int nb)
{
    clear_functions();
    nb_functions = nb;
    for(int i = 1; i <= nb_functions; i++)
    {
        func *function = new func;
        array(NAME,function->name);
        array(NAME,function->type);
        int index = 0;
        int cp = 0;

        // remove white space
        while(calls[i][index] != '\0' && calls[i][index] == ' ' && calls[i][index] == '\t')
            index++;

        // read function name
        while(calls[i][index] != '\0' && calls[i][index] != '(')
        {
            if(calls[i][index] != ' ' && calls[i][index] != '\t')
            {
                function->name[cp] = calls[i][index];
                cp++;
            }
            index++;
        }
        function->name[cp] = '\0';

        // read parameters
        cp = 0;
        index++;
        param *parameter = NULL;
        int level = 1;

        while(calls[i][index] != '\0')
        {
            if(calls[i][index] == '(')
                level++;

            if(calls[i][index] == ')')
                level--;

            if(calls[i][index] == ')' && level == 0)
            { // end of parameter read
                if(parameter != NULL)
                { // still save the last parameter
                    parameter->next = function->params;
                    if(is_iterator(i, parameter->name) > 0)
                        parameter->iterator = true;
                    else
                        parameter->iterator = false;
                    
                    function->params = parameter;
                    function->nb_parameters++;
               }
               break;
            }

            if(parameter == NULL) // new parameter
            {
                parameter = new param;
                array(NAME,parameter->name);
                array(NAME,parameter->type);
            }
    
            if(calls[i][index] == ',')
            { // store parameter
                if(is_iterator(i, parameter->name) > 0)
                    parameter->iterator = true;
                else
                    parameter->iterator = false;
                parameter->next = function->params;
//              parameter->name[cp] = '\0';
                function->params = parameter;
                function->nb_parameters++;
                parameter = NULL;
                cp = 0;
            }
            else if(calls[i][index] == '&')
            {
                parameter->output = true;
            }
            else if(calls[i][index] != ' ' && !(parameter->output && (calls[i][index] == ')' || calls[i][index] == '(')))
            { // ignore whitespaces
                parameter->name[cp] = calls[i][index];
                cp++;
            }
            index++;
        }

        // set parameter types
        char *buffer;
        for(int line = 1; line <= basic->program.get_nb_lines(); line++)
        {

            buffer = basic->program.get_line(line);

            char buff1[LINE];
            sscanf(buffer, "%*s %s", &buff1);

            buff1[strlen(function->name)] = '\0';
            if(strcmp(buff1,function->name) == 0)
            { // store parameter types
                parameter = function->params;

                int index = 0;
                while(buffer[index] != '(')
                    index++;
                index++;

                for(int x = 1; x <= function->nb_parameters; x++)
                {
                    parameter = function->params;
                    for(int y = function->nb_parameters - x; y > 0; y--)
                        parameter = parameter->next;

                    // remove whitespaces
                    while(buffer[index] == ' ')
                        index++;

                    // read type
                    int cp = 0;
                    while(buffer[index] != ' ' && buffer[index] != ')')
                    {
                        parameter->type[cp] = buffer[index];
                        cp++;
                        index++;
                    }

                    // set constant and type
                    if(strcmp(parameter->type, "const") == 0)
                    {
                        parameter->constant = true;
                        array(NAME,parameter->type);

                        int cp = 0;
                        while(buffer[index] == ' ')
                            index++;

                        while(buffer[index] != ' ' && buffer[index] != ')')
                        {
                            parameter->type[cp] = buffer[index];
                            cp++;
                            index++;
                        }
                    }

                    // move iterator to next type

                    while(buffer[index] != ',' && buffer[index] != '\0')
                        index++;

                    index++;
                }
                break;
            }
        }


        // store functions
        function->next = functions;
        functions = function;
    }
    func *function;

    // detect arrays
    function = functions;
    while(function != NULL)
    {
        param *parameter = function->params;
        while(parameter != NULL)
        {
            int index = 0;
            while(parameter->name[index] != '[' && parameter->name[index] != '\0')
                index++;

            if(parameter->name[index] == '[')
            {
                parameter->array = true;
                int index_dim = index;

                while(parameter->name[index_dim] != '\0')
                { // see of how many dimensions this array consists
                    if(parameter->name[index_dim] == '[')
                        parameter->nb_dimensions++;
                    index_dim++;
                }
                array_content *a_dimension;
                for(int nb_dimension = 1; nb_dimension <= parameter->nb_dimensions; nb_dimension++)
                { // store content of all dimensions
                    if(parameter->dimension_content == NULL)
                    {                    
                        parameter->dimension_content = new array_content;
                        a_dimension = parameter->dimension_content;
                    }
                    else
                    {
                        a_dimension->next = new array_content;
                        a_dimension = a_dimension->next;
                    }
                    
                    array(NAME, a_dimension->content);
                    int cp = 0;
                    remove_element(parameter->name, NAME, index); // remove '['
                    while(parameter->name[index] != ']')
                    { // copy content
                        a_dimension->content[cp] = parameter->name[index];
                        cp++;
                        remove_element(parameter->name, NAME, index);
                    }
                    remove_element(parameter->name, NAME, index); // remove ']'
                }
            }
            parameter = parameter->next;
        }
        function = function->next;
    }

    // add parameters to variable declarations
    function = functions;
    while(function != NULL)
    {
        param *parameter = function->params;
        while(parameter != NULL)
        {
            if(!parameter->iterator)
                add_variable(parameter);
            parameter = parameter->next;
        }
        function = function->next;
    }
}


/**********************************************************
 *
 *                handle functions
 *
 **********************************************************/

void handle::error(int msg)
{
    fprintf(err_output, "\nError: ");
    switch(msg)
    {
        case 1:
            fprintf(err_output, "node not set");
            break;
        default:
            fprintf(err_output, "unknown error");
    }
}

handle::handle(PDG *tmp_pdg, string tmp_input)
{
    dependences_analyzed = false;
    err_output = stderr;
    pdg = tmp_pdg;
//    node = -1;
    input = tmp_input;
    functions.init_function(this);
    partitions.init_partitioning(this);
    function_mapping.init_mapping(this);
    cloog_mapping.init_mapping(this);
    program.load_program(input);
    //program.print_program(stdout);

    char filename[NAME];
    strcpy(filename, input.c_str());
//    strcat(filename, ".errors");
//    errors = fopen(filename, "w");
    errors = stderr;
}

handle::~handle()
{
    fclose(errors);
    //for(int statement = 1; statement <= get_total_nb_statements(); statement++)
      //  free(function_call[statement]);
    //free(function_call);
}

void handle::pdg_partition()
{
    for(int nb_partition = 1; nb_partition <= partitions.get_nb_partitions(); nb_partition++)
    {
        int factors = 0;
        for(int i = 1; i < nb_partition; i++)
            if(partitions.get_node_line(i, 1) < partitions.get_node_line(nb_partition, 1)
                    && partitions.get_partition_type(i) != 3
                    && partitions.get_partition_possible(i))
                factors += partitions.get_nb_factor(i) - 1;                       

        if(partitions.get_partition_type(nb_partition) == 1)
        { // plane cut
            if(pdg_contains(nb_partition))
            {
                int lower_bound = 9999, upper_bound = -9999;
                int node = get_node(partitions.get_node_line(nb_partition, 1));
                for(int x = 0; x < pdg->nodes[node]->source->constraints[0]->el.size(); x++)
                {
                    int iterator_value = pdg->nodes[node]->source->constraints[0]->el[x][partitions.get_nb_iterator(nb_partition)];
                    if(iterator_value != 0)
                    { // found iterator row
                        // FIXME: only works when iterator is enabled by 1 or -1
                        int value = pdg->nodes[node]->source->constraints[0]->el[x][pdg->nodes[node]->source->constraints[0]->el[x].size() - 1];
                        if(iterator_value > 0)
                            lower_bound = minimum(lower_bound, value); 
                        else
                            upper_bound = maximum(upper_bound, value);
                    }
                }
                int parts = partitions.get_nb_factor(nb_partition);
                if(partitions.get_nb_iterator(nb_partition) > pdg->nodes[node - factors]->source->dim)
                    fprintf(errors, "ERROR: plane cutting partition %d has been omitted, iterator is of too high dimension\n", nb_partition);
                else if(((upper_bound - lower_bound) + 1) % partitions.get_nb_factor(nb_partition) != 0)
                    fprintf(errors, "ERROR: plane cutting partition %d has been omitted, due to (upper bound - lower bound) mod factor != 0\n", nb_partition);
                else
                {
                    partitions.set_partition_possible(nb_partition, true);

                    int node = get_node(partitions.get_node_line(nb_partition, 1));

                    // store original polyhedron
                    vector< vector< int > > el;
                    // allocate space
                    el.resize(pdg->nodes[node]->source->constraints[0]->el.size());
                    for(int i = 0; i < pdg->nodes[node]->source->constraints[0]->el.size(); i++)
                        el[i].resize(pdg->nodes[node]->source->constraints[0]->el[i].size());
                    // copy polyhedron
                    for(int row = 0; row < pdg->nodes[node]->source->constraints[0]->el.size(); row++)
                        for(int column = 0; column < pdg->nodes[node]->source->constraints[0]->el[row].size(); column++)
                            el[row][column] = pdg->nodes[node]->source->constraints[0]->el[row][column];

                    // create correct polyhedrons
                    for(int i = 1; i <= partitions.get_nb_factor(nb_partition); i++)
                    {
                        // copy node when it does not exist
                        if(i > 1)
                            pdg_copy_node(node + (i - 2));
                        // transfer partitioned polyhedron to node.
                        print_polyhedron_plane(el, node + (i - 1), node, i);
                    }

                    // create correct prefix
                    pdg_prefix(node, partitions.get_nb_factor(nb_partition), nb_partition);
                    
                    cloog_mapping.add_partition(node, partitions.get_nb_factor(nb_partition));
                    for(int i = 0; i < partitions.get_nb_factor(nb_partition); i++)
                        function_mapping.add(node - factors);
                }
            }
            else
            {
                fprintf(errors, "ERROR: plane cutting partition %d has been omitted, indicated line number doesn't contain a function call\n", nb_partition);
                partitions.set_partition_possible(nb_partition, false);
            }
        }
        else if(partitions.get_partition_type(nb_partition) == 2)
        { // modulo
            if(pdg_contains(nb_partition))
            {
                int lower_bound = 9999, upper_bound = -9999;
                int node = get_node(partitions.get_node_line(nb_partition, 1));
                for(int x = 0; x < pdg->nodes[node]->source->constraints[0]->el.size(); x++)
                {
                    int iterator_value = pdg->nodes[node]->source->constraints[0]->el[x][partitions.get_nb_iterator(nb_partition)];
                    if(iterator_value != 0)
                    { // found iterator row
                        // FIXME: only works when iterator is enabled by 1 or -1
                        int value = pdg->nodes[node]->source->constraints[0]->el[x][pdg->nodes[node]->source->constraints[0]->el[x].size() - 1];
                        if(iterator_value > 0)
                            lower_bound = minimum(lower_bound, value); 
                        else
                            upper_bound = maximum(upper_bound, value);
                    }
                }
                int parts = partitions.get_nb_factor(nb_partition);
                if(partitions.get_nb_iterator(nb_partition) > pdg->nodes[node - factors]->source->dim)
                    fprintf(errors, "ERROR: modulo cutting partition %d has been omitted, iterator is of too high dimension\n", nb_partition);
                else if(((upper_bound - lower_bound) + 1) % partitions.get_nb_factor(nb_partition) != 0)
                    fprintf(errors, "ERROR: modulo cutting partition %d has been omitted, due to (upper bound - lower bound) mod factor != 0\n", nb_partition);
                else
                {
                    partitions.set_partition_possible(nb_partition, true);

                    int node = get_node(partitions.get_node_line(nb_partition, 1));
                    // store original polyhedron
                    vector< vector< int > > el;
                    // allocate space
                    el.resize(pdg->nodes[node]->source->constraints[0]->el.size());
                    for(int i = 0; i < pdg->nodes[node]->source->constraints[0]->el.size(); i++)
                        el[i].resize(pdg->nodes[node]->source->constraints[0]->el[i].size());
                    // copy polyhedron
                    for(int row = 0; row < pdg->nodes[node]->source->constraints[0]->el.size(); row++)
                        for(int column = 0; column < pdg->nodes[node]->source->constraints[0]->el[row].size(); column++)
                            el[row][column] = pdg->nodes[node]->source->constraints[0]->el[row][column];

                    // create correct polyhedrons
                    for(int i = 1; i <= partitions.get_nb_factor(nb_partition); i++)
                    {
                        // copy node when it does not exist
                        if(i > 1)
                            pdg_copy_node(node + (i - 2));
                        // transfer partitioned polyhedron to node.
                        print_polyhedron_modulo(el, node + (i - 1), node, i);
                    }

                    // create correct prefix
                    pdg_prefix(node, partitions.get_nb_factor(nb_partition), nb_partition);

                    cloog_mapping.add_partition(node, partitions.get_nb_factor(nb_partition));
                    for(int i = 0; i < partitions.get_nb_factor(nb_partition); i++)
                        function_mapping.add(node - factors);
                }
            }
            else
            {
                fprintf(errors, "ERROR: modulo cutting partition %d has been omitted, indicated line number doesn't contain a function call\n", nb_partition);
                partitions.set_partition_possible(nb_partition, false);

            }
        }
    }
}

void handle::pdg_prefix(int node, int factor, int nb_partition)
{
    if(pdg->nodes[node]->prefix[pdg->nodes[node]->prefix.size() - 2] < 0 && partitions.get_partition_type(nb_partition) != 2)
    { // insert 1
        int for_insert;
        int dim = pdg->nodes[node]->source->dim;
        int tmp = 0, it = 0;
        while(tmp != dim)
        {
            if(pdg->nodes[node]->prefix[it] == -1)
                tmp++;

            it++;
        }
        for_insert = it;
        
        for(int i = node - 1; i >= 0; i--)
        { // insert one above 'node'
            bool insert = true;
            if(pdg->nodes[node]->prefix.size() <= pdg->nodes[i]->prefix.size())
            {
                int x = 0, tmp_dim = 0;
                while(tmp_dim < dim)
                {   
                    if(pdg->nodes[node]->prefix[x] != pdg->nodes[i]->prefix[x])
                    {
                        insert = false;
                        break;
                    }
                    if(pdg->nodes[node]->prefix[x] == -1)
                        tmp_dim++;

                    x++;
                }
            }
            else
                insert = false;

            if(insert)
            {
                pdg->nodes[i]->prefix.resize(pdg->nodes[i]->prefix.size() + 1);
                for(int x = pdg->nodes[i]->prefix.size() - 1; x - 1 >= for_insert; x--)
                    pdg->nodes[i]->prefix[x] = pdg->nodes[i]->prefix[x - 1];
                pdg->nodes[i]->prefix[for_insert] = 1;
            }
            else
                break;
        }
        for(int i = node + factor; i < pdg->nodes.size(); i++)
        { // insert one below 'node'
            bool insert = true;
            if(pdg->nodes[node]->prefix.size() <= pdg->nodes[i]->prefix.size())
            {
                int x = 0, tmp_dim = 0;
                while(tmp_dim < dim)
                {   
                    if(pdg->nodes[node]->prefix[x] != pdg->nodes[i]->prefix[x])
                    {
                        insert = false;
                        break;
                    }
                    if(pdg->nodes[node]->prefix[x] == -1)
                        tmp_dim++;

                    x++;
                }
            }
            else
                insert = false;

            if(insert)
            {
                pdg->nodes[i]->prefix.resize(pdg->nodes[i]->prefix.size() + 1);
                for(int x = pdg->nodes[i]->prefix.size() - 1; x - 1 >= for_insert; x--)
                    pdg->nodes[i]->prefix[x] = pdg->nodes[i]->prefix[x - 1];
                pdg->nodes[i]->prefix[for_insert] = 1;
             }
            else
                break;

        }
        // insert one in 'node'
        pdg->nodes[node]->prefix.resize(pdg->nodes[node]->prefix.size() + 1);
        for(int x = pdg->nodes[node]->prefix.size() - 1; x - 1 >= for_insert; x--)
            pdg->nodes[node]->prefix[x] = pdg->nodes[node]->prefix[x - 1];
        pdg->nodes[node]->prefix[for_insert] = 1;
    }

    // copy prefix of node to other partition nodes
    for(int i = node + 1; i < node + factor; i++)
    {
        pdg->nodes[i]->prefix.resize(pdg->nodes[node]->prefix.size());
        for(int j = 0; j < pdg->nodes[node]->prefix.size(); j++)
        {
            pdg->nodes[i]->prefix[j] = pdg->nodes[node]->prefix[j];   
        }
    }

    // increment following function call prefixes
    for(int i = node + 1; i < pdg->nodes.size(); i++)
    {
        bool increment = true;
        if(pdg->nodes[node]->prefix.size() <= pdg->nodes[i]->prefix.size())
        {
            for(int x = 0; x < pdg->nodes[node]->prefix.size() - 1; x++)
                if(pdg->nodes[i]->prefix[x] != pdg->nodes[node]->prefix[x])
                    increment = false;
        }
        else
            increment = false;

        if(increment)
            pdg->nodes[i]->prefix[pdg->nodes[node]->prefix.size() - 1] = pdg->nodes[i - 1]->prefix[pdg->nodes[node]->prefix.size() - 1] + 2;
        else
            break;
    }

    // add 1 to end of all partitioned polyhedrons
    for(int i = node; i < node + factor; i++)
    {
        pdg->nodes[i]->prefix.resize(pdg->nodes[i]->prefix.size() + 1);
        pdg->nodes[i]->prefix[pdg->nodes[i]->prefix.size() - 1] = 1;
    }
}

void handle::pdg_copy_node(int copy)
{
    for(int i = 0; i < pdg->nodes[copy]->statement->accesses.size(); i++)
    {
        if(pdg->nodes[copy]->statement->accesses[i]->backward.size() != 0)
            fprintf(errors, "WARNING: pdg->nodes[%d]->statement->accesses[%d]->backward.size() != 0; this will not be copied into partion node %d!\n", copy, i, copy + 1);
        if(pdg->nodes[copy]->statement->accesses[i]->extended_map != NULL)
            fprintf(errors, "WARNING: pdg->nodes[%d]->statement->accesses[%d]->extended_map != NULL; this will not be copied into partion node %d!\n", copy, i, copy + 1);
        if(pdg->nodes[copy]->statement->accesses[i]->nested.size() != 0)
            fprintf(errors, "WARNING: pdg->nodes[%d]->statement->accesses[%d]->nested != NULL; this will not be copied into partion node %d!\n", copy, i, copy + 1);
        if(pdg->nodes[copy]->statement->accesses[i]->extension != NULL)
            fprintf(errors, "WARNING: pdg->nodes[%d]->statement->accesses[%d]->extension != NULL; this will not be copied into partion node %d!\n", copy, i, copy + 1);
    }

    pdg->nodes.resize(pdg->nodes.size() + 1);
    for(int i = pdg->nodes.size() - 1; i - 1 > copy; i--)
    {
        pdg->nodes[i] = pdg->nodes[i - 1];
        pdg->nodes[i]->nr++;
    }

    pdg->nodes[copy + 1] = new pdg::node();
    pdg->nodes[copy + 1]->nr = pdg->nodes[copy]->nr + 1;
    pdg->nodes[copy + 1]->scattering = NULL; 
    pdg->nodes[copy + 1]->source = new pdg::UnionSet;
    pdg->nodes[copy + 1]->source->constraints.resize(1);
    pdg->nodes[copy + 1]->source->constraints[0] = new pdg::Matrix;
    pdg->nodes[copy + 1]->source->dim = pdg->nodes[copy]->source->dim;

//  pdg->nodes[copy + 1]->statement = pdg->nodes[copy]->statement;
    // copy statement
    pdg->nodes[copy + 1]->statement = new pdg::statement;

    // copy statement top_function
    pdg::function_call *call = new pdg::function_call;
//    call->name = pdg->nodes[copy]->statement->top_function->name;
    call->name = new str(pdg->nodes[copy]->statement->top_function->name->s);
    call->arguments.resize(pdg->nodes[copy]->statement->accesses.size());
   
    // copy statement accesses
    for(int i = 0; i < pdg->nodes[copy]->statement->accesses.size(); i++)
    {
        pdg->nodes[copy + 1]->statement->accesses.resize(pdg->nodes[copy + 1]->statement->accesses.size() + 1);
        pdg::access *access = new pdg::access;
        access->array = pdg->nodes[copy]->statement->accesses[i]->array;

        // copy access unionmap

        int input = pdg->nodes[copy]->statement->accesses[i]->map->input;
  
//      access->map = new pdg::UnionMap(pdg->nodes[copy]->statement->accesses[i]->map->get_isl_map(pdg->get_isl_ctx()),
//              pdg->nodes[copy]->statement->accesses[i]->map->params);
        access->map = new pdg::UnionMap(
                pdg->nodes[copy]->statement->accesses[i]->map->input,
                pdg->nodes[copy]->statement->accesses[i]->map->output,
                constraints2polyhedron(pdg->nodes[copy]->statement->accesses[i]->map->constraints),
                pdg->nodes[copy]->statement->accesses[i]->map->params);

        // FIXME: fugly hack.. constraint2polyhedron or constructor of unionmap makes polyhedron with values times -1
        for(int j = 0; j < access->map->constraints[0]->el.size(); j++)
            for(int k = 0; k < access->map->constraints[0]->el[0].size(); k++)
                access->map->constraints[0]->el[j][k] *= -1;
        access->type = pdg->nodes[copy]->statement->accesses[i]->type;

        access->nr = i;
        call->arguments[i] = new pdg::call_or_access;
        call->arguments[i]->access = access;
        call->arguments[i]->type = pdg->nodes[copy]->statement->top_function->arguments[i]->type;
        pdg->nodes[copy + 1]->statement->accesses[pdg->nodes[copy + 1]->statement->accesses.size() - 1] = access;
    }
    pdg->nodes[copy + 1]->statement->top_function = call;
    pdg->nodes[copy + 1]->statement->line = pdg->nodes[copy]->statement->line;
    pdg->nodes[copy + 1]->statement->operation = pdg->nodes[copy]->statement->operation;

   
    for(int i = 0 ; i < pdg->nodes[copy]->source->params.size(); i++)
    {
        pdg->nodes[copy + 1]->source->params.resize(pdg->nodes[copy + 1]->source->params.size() + 1);
        pdg->nodes[copy + 1]->source->params[pdg->nodes[copy + 1]->source->params.size() - 1] = pdg->nodes[copy]->source->params[i];
    }

    for(int i = 0; i < pdg->nodes.size(); i++)
    {
        pdg->nodes[i]->statement->operation = i + 1;
        pdg->root = i + 2;
    }
}

bool handle::pdg_contains(int nb_partition)
{
    bool contains = false;

    for(int nb_node = 1; nb_node <= partitions.get_nb_nodes(nb_partition); nb_node++)
    {
        for(int i = 0; i < pdg->nodes.size(); i++)
        {
            if(pdg->nodes[i]->statement->line == partitions.get_node_line(nb_partition, nb_node))
                contains = true;
        }

    }
    return contains;
}

void handle::main_handle()
{
    //if(!dependences_analyzed)
    //{
        set_function_call();
        functions.set_functions(function_call, get_total_nb_statements());
        //functions.print_structure(stdout);

        // get partition information
        partitions.get_partitioning();
        //partitions.print_structure(stdout);

        // create mapping and adjust pdg
        cloog_mapping.initial_mapping(pdg->nodes.size());
        function_mapping.initial_mapping(pdg->nodes.size());
        if(partitions.get_nb_partitions() > 0)
            pdg_partition();

        dependences_analyzed = true;
    //}
/*    else
    {
        // add merging partitions
        for(int nb_partition = 1; nb_partition <= partitions.get_nb_partitions(); nb_partition++)
        {
            if(partitions.get_partition_type(nb_partition) == 3)
            { // merge
                if(pdg_contains(nb_partition))
                {
                    // for every node to be merged, adjust the mapping
                    int node1, node2;
                    bool merge = true;
                    for(int a = 1; a <= partitions.get_nb_nodes(nb_partition); a++)
                    {
                        node1 = get_node(partitions.get_node_line(nb_partition, a));
                        for(int b = a + 1; b <= partitions.get_nb_nodes(nb_partition); b++)
                        {
                            node2 = get_node(partitions.get_node_line(nb_partition, b));
                            analyzed.resize(0);
                            analyzed.resize(pdg->dependences.size(), false);
                            if(!mergable(nb_partition, node1, node2,-1, true) || !mergable(nb_partition, node2, node1,-1, true))
                                merge = false;
                        }
                    }

                    if(merge)
                    {
                        partitions.set_partition_possible(nb_partition, true);
                        node1 = get_node(partitions.get_node_line(nb_partition, 1));
                        for(int b = 2; b <= partitions.get_nb_nodes(nb_partition); b++)
                        {
                            node2 = get_node(partitions.get_node_line(nb_partition, b));
                            cloog_mapping.min(node1, node2);
                                                       int factors = 0;
                            for(int i = 1; i < nb_partition; i++)
                            {
                                if(partitions.get_node_line(i, 1) < partitions.get_node_line(nb_partition, 1)
                                        && partitions.get_partition_type(i) != 3
                                        && partitions.get_partition_possible(i))
                                    factors += partitions.get_nb_factor(i) - 1;                       
                            }
                            function_mapping.min(node1 - factors, node2 - factors);
                        }
                    }
                    else
                    {
                        fprintf(errors, "ERROR: merging partition %d has been omitted, due to dependencies!\n", nb_partition);
                        partitions.set_partition_possible(nb_partition, false);
                    }
                }
                else
                {
                        fprintf(errors, "ERROR: merging partition %d has been omitted, indicated line number doesn't containt a function call\n", nb_partition);
                        partitions.set_partition_possible(nb_partition, false);

                }
            }
        }
        cloog_mapping.print_cloog_mapping(stdout);
        function_mapping.print_function_mapping(stdout);
    }*/
}

void handle::print_errors(FILE *output)
{  
    fclose(errors);
    char filename[NAME];
    strcpy(filename, input.c_str());
    strcat(filename, ".errors");
    errors = fopen(filename, "r");

    bool e = true;
    if(errors)
    {
        char buffer[LINE];
        while(fgets(buffer, sizeof(buffer), errors))
        {
            if(e)
                fprintf(output, "\nERRORS:\n-------------------------------------------------\n");
            fprintf(output, "%s", buffer);
            e = false;
        }
        fprintf(output, "\n");
    }
}

void handle::print_polyhedron(FILE * output, int node)
{  
    for(int i = 0; i < pdg->nodes[node]->source->constraints[0]->el.size(); i++)
    {
        for(int j = 0; j < pdg->nodes[node]->source->constraints[0]->el[i].size(); j++)
            fprintf(output, "\t%d", pdg->nodes[node]->source->constraints[0]->el[i][j]);
        fprintf(output, "\n");
    }
}

void handle::print_scattering(FILE * output, int node)
{
    for(int i = 0; i < pdg->nodes[node]->scattering->constraints[0]->el.size(); i++)
    {
        for(int j = 0; j < pdg->nodes[node]->scattering->constraints[0]->el[i].size(); j++)
            fprintf(output, "\t%d", pdg->nodes[node]->scattering->constraints[0]->el[i][j]);
        fprintf(output, "\n");
    }
}

void handle::print_context_parameters(FILE * output)
{
    for(int i = 0; i < pdg->context->params.size(); i++)
        fprintf(output, "\t%s", pdg->context->params[i]->name->s.c_str());
    fprintf(output, "\n");
}

void handle::print_polyhedron_parameters(FILE * output, int node)
{
    for(int i = 0; i < pdg->nodes[node]->source->params.size(); i++)
        fprintf(output, "\t%s", pdg->nodes[node]->source->params[i]->name->s.c_str());
    fprintf(output, "\n");
}

void handle::print_context(FILE * output)
{
    for(int i = 0; i < pdg->context->constraints[0]->el.size(); i++)
    {
        for(int j = 0; j < pdg->context->constraints[0]->el[i].size(); j++)
            fprintf(output,"\t%d", pdg->context->constraints[0]->el[i][j]);
        fprintf(output, "\n");
    }
}

/*
void handle::set_node(int line)
{
    node = -1;
    for(int i = 0; i < pdg->nodes.size(); i++)
        if(pdg->nodes[i]->statement->line == line)
           node = i;
}
*/

int handle::get_node(int line)
{
    for(int i = 0; i < pdg->nodes.size(); i++)
        if(pdg->nodes[i]->statement->line == line)
           return i;
    return -1;

}

int handle::get_node_line(int node)
{
    return pdg->nodes[node]->statement->line;
}

int handle::get_nb_polyhedrons(int node)
{
    int nb_polyhedrons = 0;
    for(int polyhedron = 0; polyhedron < pdg->nodes[node]->source->constraints.size(); polyhedron++)
        if(pdg->nodes[node]->source->constraints[polyhedron]->el.size() != 0)
            nb_polyhedrons++;

    return nb_polyhedrons;
}

int handle::get_total_nb_statements()
{
    int total = 0;
    for(int node = 0; node < pdg->nodes.size(); node++)
        if(get_nb_polyhedrons(node) > 0)
            total++;

    return total;
}

int handle::get_nb_scattering(int node)
{
    int nb_scattering = 0;
    for(int polyhedron = 0; polyhedron < pdg->nodes[node]->scattering->constraints.size(); polyhedron++)
        if(pdg->nodes[node]->scattering->constraints[polyhedron]->el.size() != 0)
            nb_scattering++;

    return nb_scattering;
}

int handle::get_total_nb_scattering()
{
    int total = 0;
    for(int node = 0; node < pdg->nodes.size(); node++)
        total += get_nb_scattering(node);

    return total;
}

char *handle::get_function_call(int statement)
{
    char *buffer;
    buffer = program.get_line(pdg->nodes[statement - 1]->statement->line);

    char * call = (char*) malloc (strlen(buffer)+1);
    strcpy(call, buffer);
    return call;
}

void handle::set_function_call()
{
    function_call = (char**) malloc((get_total_nb_statements() + 1)*sizeof(char*));
    for(int statement = 1; statement <= get_total_nb_statements(); statement++)
        function_call[statement] = get_function_call(statement);
}

void handle::print_polyhedron_plane(const vector< vector< int > > &el, int w, int nb_node, int nb)
{
    int nb_partition = partitions.line_nb_partition(get_node_line(nb_node));
    int nb_factor = partitions.get_nb_factor(nb_partition);
    int lower_bound = 9999, upper_bound = -9999, lower_bound_line, upper_bound_line;

    // resize polyhedron of node
    pdg->nodes[w]->source->constraints[0]->el.resize(el.size());
    for(int i = 0; i < pdg->nodes[w]->source->constraints[0]->el.size(); i++)
        pdg->nodes[w]->source->constraints[0]->el[i].resize(el[0].size());

    for(int x = 0; x < el.size(); x++)
    {
        int iterator_value = el[x][partitions.get_nb_iterator(nb_partition)];
        if(iterator_value != 0)
        { // found iterator row
            // FIXME: only works when iterator is enabled by 1 or -1
            int value = el[x][el[x].size() - 1];
            if(iterator_value > 0) {
                if(lower_bound > minimum(lower_bound, value))
                {               
                    lower_bound = minimum(lower_bound, value);
                    lower_bound_line = x;
                }
            }
            else
            {
                if(upper_bound < maximum(upper_bound, value))
                {
                    upper_bound = maximum(upper_bound, value);
                    upper_bound_line = x;
                }
            }
        }
    }

    int upper_value;
    int lower_value;
    int total = (upper_bound - lower_bound) + 1;
    int step = total/nb_factor;

    lower_value = lower_bound;
    for(int i = 1; i < nb; i++)
        lower_value += step;
    upper_value = (lower_value + step) - 1;

    // the rest partition
    if(upper_value > upper_bound)
        upper_value = upper_bound;

    for(int i = 0; i < el.size(); i++)
    {
        for(int j = 0; j < el[i].size(); j++)
        {
            if((i == lower_bound_line) && ( j == el[i].size() - 1))
                pdg->nodes[w]->source->constraints[0]->el[i][j] = -1 * lower_value;
            else if((i == upper_bound_line) && ( j == el[i].size() - 1))
                 pdg->nodes[w]->source->constraints[0]->el[i][j] = upper_value;
            else
                 pdg->nodes[w]->source->constraints[0]->el[i][j] = el[i][j];
        }
    }
}

void handle::print_polyhedron_modulo(const vector< vector< int > > &el, int w, int nb_node, int nb)
{
    fprintf(stdout, "print modulo nb = %d\n", nb);
    int nb_partition = partitions.line_nb_partition(get_node_line(nb_node));
    int nb_factor = partitions.get_nb_factor(nb_partition);
    int nb_iterator = partitions.get_nb_iterator(nb_partition);

    // resize polyhedron of node
    pdg->nodes[w]->source->constraints[0]->el.resize(el.size() + 1);
    for(int i = 0; i < pdg->nodes[w]->source->constraints[0]->el.size(); i++)
        pdg->nodes[w]->source->constraints[0]->el[i].resize(el[0].size() + 1);

    int insert = el[0].size() - 1;
    if(pdg->context != NULL && pdg->context->params.size() > 0)
        insert -= pdg->context->params.size();

    fprintf(stdout, "printing modulo row\n");
    fprintf(stdout, "node: %d\n---------------------------\n", nb_node);
    print_polyhedron(stdout, nb_node);
    for(int x = 0; x < el[0].size() + 1; x++)
    {
        if(x == nb_iterator)
            pdg->nodes[w]->source->constraints[0]->el[0][x] = -1;
        else if(x == insert)
            pdg->nodes[w]->source->constraints[0]->el[0][x] = nb_factor;
        else if(x == el[0].size())
            pdg->nodes[w]->source->constraints[0]->el[0][x] = nb - 1; 
        else
            pdg->nodes[w]->source->constraints[0]->el[0][x] = 0;
    }
    fprintf(stdout, "printing rest\n");
    print_polyhedron(stdout, nb_node);

    // lower and upper bound calculation
    int lower_bound = 9999, upper_bound = -9999, lower_bound_line, upper_bound_line;
    for(int x = 0; x < el.size(); x++)
    {
        int iterator_value = el[x][partitions.get_nb_iterator(nb_partition)];
        if(iterator_value != 0)
        { // found iterator row
            // FIXME: only works when iterator is enabled by 1 or -1
            int value = el[x][el[x].size() - 1];
            if(iterator_value > 0) {
                if(lower_bound > minimum(lower_bound, value))
                {               
                    lower_bound = minimum(lower_bound, value);
                    lower_bound_line = x;
                }
            }
            else
            {
                if(upper_bound < maximum(upper_bound, value))
                {
                    upper_bound = maximum(upper_bound, value);
                    upper_bound_line = x;
                }
            }
        }
    }

    int upper = nb - 1;
    int lower = nb_factor - nb;
    for(int i = 0; i < el.size(); i++)
    {
        for(int j = 0; j <= insert; j++)
        {
            if(j == insert)
                pdg->nodes[w]->source->constraints[0]->el[i+1][j] = 0;
            else
                pdg->nodes[w]->source->constraints[0]->el[i+1][j] = el[i][j];
        }

        for(int j = insert; j < el[i].size(); j++)
        {
            if(i == upper_bound_line && j == el[i].size() - 1)
                pdg->nodes[w]->source->constraints[0]->el[i+1][j + 1] = el[i][j] - upper;
            else if(i == lower_bound_line && j == el[i].size() - 1)
                pdg->nodes[w]->source->constraints[0]->el[i+1][j + 1] = el[i][j] - lower;
            else
                pdg->nodes[w]->source->constraints[0]->el[i+1][j + 1] = el[i][j];
        }
    }
    fprintf(stdout, "printing done\n");
    print_polyhedron(stdout, nb_node);
    fprintf(stdout, "modulo print done\n");
}

void handle::print_scattering_modulo(FILE *output, int nb_node)
{
    fprintf(output," # scattering modulo:\n    # node: %d\n", nb_node);
    for(int i = 0; i < pdg->nodes[nb_node]->scattering->constraints[0]->el.size(); i++)
    {
        for(int j = 0; j < pdg->nodes[nb_node]->scattering->constraints[0]->el[i].size(); j++)
        {
            if(pdg->context != NULL)
            { 
                if(j == pdg->nodes[nb_node]->scattering->constraints[0]->el[i].size() - 
                    (pdg->context->params.size() + 1))
                fprintf(output, "\t0");
            }
            else if(j == pdg->nodes[nb_node]->scattering->constraints[0]->el[i].size() - 1)
                fprintf(output, "\t0");
            fprintf(output, "\t%d", pdg->nodes[nb_node]->scattering->constraints[0]->el[i][j]);
        }
        fprintf(output, "\n");
    }
}

bool handle::mergable(int nb_partition, int node1, int node2, int tmp, bool connected)
{
    bool merge = true;
    if(tmp != -1 && connected)
    {
        connected = false;
        for(int nb_node = 1; nb_node <= partitions.get_nb_nodes(nb_partition); nb_node++)
        {
            int node_line = partitions.get_node_line(nb_partition, nb_node);
            int node = get_node(node_line);
            if(tmp == node)
                connected = true;
        }
    }

    for(int dependency = 0; dependency < pdg->dependences.size(); dependency++)
    {
        if(pdg->dependences[dependency]->from != NULL && pdg->dependences[dependency]->to != NULL)
        { // PN makes one of these pointers NULL when dealing with a selfdependency 
            if(!analyzed[dependency])
            {
                if(tmp != -1)
                {
                    if(pdg->dependences[dependency]->from->nr == tmp && pdg->dependences[dependency]->to->nr != node2)
                    {
                        analyzed[dependency] = true;
                        if(!mergable(nb_partition, node1, node2, pdg->dependences[dependency]->to->nr, connected))
                            merge = false;
                    }
                    else if(!connected && pdg->dependences[dependency]->from->nr == tmp && pdg->dependences[dependency]->to->nr == node2)
                        return false;              
                }
                else if(pdg->dependences[dependency]->from->nr == node1 && pdg->dependences[dependency]->to->nr != node2)
                {
                    analyzed[dependency] = true;
                    if(!mergable(nb_partition, node1, node2, pdg->dependences[dependency]->to->nr, connected))
                        merge = false;
                }
                analyzed[dependency] = false;
            }
        }
    }
    return merge;
}

void handle::pdg_reset(PDG *tmp)
{
    pdg = tmp;
    functions.pdg_reset(tmp);
}

void handle::write_cloog()
{

    // print cloog file:
    int nb_statements = cloog_mapping.get_nb_statements();

    char filename[NAME];
    strcpy(filename, input.c_str());
    strcat(filename, ".cloog");
    FILE * file = fopen(filename, "w");
    fprintf(file, "# --------------- CONTEXT --------------- \n");
    fprintf(file, "c # program language\n\n");
    fprintf(file, "# context (on paramenters)\n");

    if(pdg->context != NULL && pdg->context->params.size() > 0)
    {
        fprintf(file, "\t%d\t%d # %d rows %d colums\n",
            pdg->context->constraints[0]->el.size(),
            pdg->context->constraints[0]->el[0].size(),
            pdg->context->constraints[0]->el.size(),
            pdg->context->constraints[0]->el[0].size());
        print_context(file);
        fprintf(file, "\n1 # set parameter names manually\n");
        print_context_parameters(file);
    }
    else
    {
        fprintf(file, "\t1\t2 \n\t1\t0 # 0 >= 0 always true\n");
        fprintf(file, "\n0 # do not set parameter names manually\n");
    }
    fprintf(file, "\n# -------------- STATEMENTS ------------- \n");
    fprintf(file, "%d # number of statements\n\n", nb_statements); 
    
    for(int nb_statement = 1; nb_statement <= nb_statements; nb_statement++)
    {

        fprintf(file, "%d # statement %d: %d domain(s)\n", cloog_mapping.get_nb_nodes(nb_statement), nb_statement, cloog_mapping.get_nb_nodes(nb_statement));
        for(int nb_node = 1; nb_node <= cloog_mapping.get_nb_nodes(nb_statement); nb_node++)
        {
            fprintf(file, "# domain %d\n", nb_node);
            int node = cloog_mapping.get_statement_node(nb_statement, nb_node);
            fprintf(file, "# %s\n", pdg->nodes[node]->statement->top_function->name->s.c_str());

            fprintf(file, "\t%d\t%d # %d row(s) %d column(s)\n",
                    pdg->nodes[node]->source->constraints[0]->el.size(),
                    pdg->nodes[node]->source->constraints[0]->el[0].size(),
                    pdg->nodes[node]->source->constraints[0]->el.size(),
                    pdg->nodes[node]->source->constraints[0]->el[0].size());
            print_polyhedron(file, node);

        }
        fprintf(file, "0\t0\t0\t# future options\n\n");
    }
    fprintf(file, "0\t# don't set iterator names manually\n");
    fprintf(file, "\n# -------------- SCATTERING ------------- \n");
    fprintf(file, "%d # number of functions\n\n", nb_statements); 

    for(int nb_statement = 1; nb_statement <= nb_statements; nb_statement++)
    {
        // print scattering function from first node
        int node = cloog_mapping.get_statement_node(nb_statement, 1);
        fprintf(file, "# statement %d\n", nb_statement);
        if(partitions.node_partition_type(get_node_line(node)) == 2)
        {
             fprintf(file, "\t%d\t%d # %d row(s) %d column(s)\n",
                    pdg->nodes[node]->scattering->constraints[0]->el.size(),
                    pdg->nodes[node]->scattering->constraints[0]->el[0].size() + 1,
                    pdg->nodes[node]->scattering->constraints[0]->el.size(),
                    pdg->nodes[node]->scattering->constraints[0]->el[0].size() + 1);
             print_scattering_modulo(file, node);
        }
        else
        {
            fprintf(file, "\t%d\t%d # %d row(s) %d column(s)\n",
                    pdg->nodes[node]->scattering->constraints[0]->el.size(),
                    pdg->nodes[node]->scattering->constraints[0]->el[0].size(),
                    pdg->nodes[node]->scattering->constraints[0]->el.size(),
                    pdg->nodes[node]->scattering->constraints[0]->el[0].size());
            print_scattering(file, node);
        }
    }
    fprintf(file, "\n0\t# don't set scattering polyhedron names manually\n");

    fclose(file);
}

void handle::print_preamble(FILE *output)
{
    for(int line = 1; line <= program.get_nb_lines(); line++)
    {
        char *buffer = program.get_line(line);
        char buff1[LINE];
        sscanf(buffer, "%*s %s", &buff1);
        buff1[sizeof("main") - 1] = '\0';
        if(strcmp(buff1,"main") == 0)
        { // print function call
            functions.print_function_declarations(output);
        }
        fprintf(output, "%s", buffer);
        if(strcmp(buff1,"main") == 0)
            break;
    }        
    functions.print_variable_declarations(output);
}

void handle::print_postemble(FILE *output)
{
    fprintf(output, "  return 0;\n}\n");
}

