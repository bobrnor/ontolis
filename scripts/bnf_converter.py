#! /usr/local/bin/python
# -*- coding: utf-8 -*-

import sys
import getopt
import random
import re
import json

reload(sys)
sys.setdefaultencoding('utf-8')
random.seed()

tokenRegExp = re.compile('(<.+?>\??)|(\w+\??)|([\{\},\.;\[\]\'\"])|(\+\+)|(\-\-)|(\*\=)|(\/\=)|(\%\=)|(\+\=)|(\-\=)|(\<\<\=)|(\>\>\=)|(\>\>\>\=)|(\&\=)|(\^\=)|(\|\=)|(\+)|(\-)|(\?)')
nontermimalRegExp = re.compile('(<.+?>)\??')
next_object_id = 0
nodes = []
relations = []


def group(concept_definition):
    split_by_def = concept_definition.split('::=')
    split_by_or = split_by_def[1].split('|')
    return split_by_def[0].strip(' \n'), [item.strip(' \n') for item in split_by_or]


def tokenize(definition):
    return [next(token for token in item if token != '') for item in tokenRegExp.findall(definition)]


def find_node(name):
    return next((node for node in nodes if node['name'] == name), None)


def create_node(name):
    global next_object_id
    global nodes
    node = {'id': next_object_id, 'name': name, 'position_x': random.uniform(0, 20000), 'position_y': random.uniform(0, 20000)}
    nodes.append(node)
    next_object_id += 1
    return node


def create_relation(name, source_node, destination_node):
    global next_object_id
    global relations
    relation = {'id': next_object_id, 'name': name, 'source_node_id': source_node['id'], 'destination_node_id': destination_node['id']}
    relations.append(relation)
    next_object_id += 1
    return relation


def get_node(name):
    node = find_node(name)
    if node is None:
        node = create_node(name)
    return node


def process_subconcept(parent_node, subconcept):
    is_optional = len(subconcept) > 1 and subconcept.rfind('?') == len(subconcept) - 1
    subconcept = subconcept.strip('?')

    if nontermimalRegExp.match(subconcept) is not None:
        subconcept_node = get_node(subconcept)
        if is_optional:
            create_relation('optional', subconcept_node, parent_node)
        else:
            create_relation('a_part_of', subconcept_node, parent_node)
    else:
        subconcept_nontermital_node = get_node('Nonterminal of ' + subconcept)
        if is_optional:
            create_relation('optional', subconcept_nontermital_node, parent_node)
        else:
            create_relation('a_part_of', subconcept_nontermital_node, parent_node)
        subconcept_node = get_node(subconcept)
        create_relation('is_instance', subconcept_node, subconcept_nontermital_node)


def process_definition(parent_node, definition):
    subconcepts = tokenize(definition)
    for subconcept in subconcepts:
        process_subconcept(parent_node, subconcept)


def process_concept(concept_definition):
    concept, definitions = group(concept_definition)
    concept_node = get_node(concept)

    if len(definitions) > 1:
        definition_index = 0
        for definition in definitions:
            definition_node = get_node(concept + ' [' + str(definition_index) + ']')
            definition_index += 1
            create_relation('is_a', definition_node, concept_node)
            process_definition(definition_node, definition)
    else:
        process_definition(concept_node, definitions[0])

#========================================================================================


def print_supported_extensions():
    print '.bnf'


def convert_to_internal_format(file_path):
    global next_object_id
    global nodes
    global relations

    bnf_file = open(file_path, 'r')
    for line in bnf_file:
        process_concept(line)
    print json.dumps({'last_id': next_object_id - 1, 'nodes': nodes, 'relations': relations})


def convert_to_external_format(file_path):
    print 'not implemented'

if __name__ == '__main__':
    opts, extraparams = getopt.getopt(sys.argv[1:], '', ['method=', 'source-path='])

    methodName = (item[1] for item in opts if item[0] == '--method').next()
    if methodName == 'supported_extensions':
        print_supported_extensions()
    elif methodName == 'import':
        path = (item[1] for item in opts if item[0] == '--source-path').next()
        convert_to_internal_format(path)
    elif  methodName == 'export':
        path = (item[1] for item in opts if item[0] == '--source-path').next()
        convert_to_external_format(path)
    else:
        print "Unknown method name " + methodName
