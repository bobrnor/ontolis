#! /usr/local/bin/python
# -*- coding: utf-8 -*-

import sys
import getopt
import itertools
import json
from codeline import *
from ontology import *

reload(sys)

lang_ontology = None
lang_ontology_cursor = None
code_ontology = None
code_ontology_cursor = None
code_symbols = []
viewed_code_symbols = []
logs = []


def print_supported_extensions():
    print 'code_conv'


def find_node(symbol):
    if symbol in lang_ontology.nodes.keys():
        node = lang_ontology.nodes[symbol]
        lang_ontology_node_relations = lang_ontology.get_node_relations(node)
        is_instance_relation = next((x for x in lang_ontology_node_relations if x['name'] == 'is_instance'), None)
        node = next(x for x in lang_ontology.nodes.values() if x['id'] == is_instance_relation['destination_node_id'])
    else:
        node = lang_ontology.nodes['id']

    return node


def put_symbol(symbol):
    global lang_ontology_cursor

    lang_ontology_node = find_node(symbol)
    assert lang_ontology_node
    path_to_insert = [lang_ontology_node] if not lang_ontology_cursor else lang_ontology.find_path(lang_ontology_cursor, lang_ontology_node)
    put_path(path_to_insert)
    lang_ontology_cursor = lang_ontology_node

    code_ontology_node = code_ontology.nodes[lang_ontology_node['name']]
    code_ontology_node_relations = code_ontology.get_node_relations(code_ontology_node)

    is_instance_triplets = filter(lambda triplet: triplet[1]['name'] == 'is_instance', code_ontology.ontology_triplets)
    used_instance_node_ids = map(lambda triplet: triplet[2]['id'], is_instance_triplets)
    is_instance_relation = next((x for x in code_ontology_node_relations if x['name'] == 'is_instance' and x['source_node_id'] not in used_instance_node_ids), None)

    if is_instance_relation:
        code_node = next(x for x in code_ontology.nodes.values() if x['id'] == is_instance_relation['source_node_id'])
        code_ontology.create_node_if_not_exists(symbol)
        code_ontology.create_relation_if_not_exists(symbol, code_node['name'], 'is_instance')
    else:
        print 'No is_instance relation for ' + symbol


def put_path(path):
    print ' -> '.join((list(x['name'] for x in path)))

    global code_ontology

    lang_ontology_related_triplets = []
    if len(path) > 1:
        for i in range(0, len(path) - 1):
            lang_ontology_related_triplets += filter(lambda x: path[i] in x and path[i + 1] in x and x[1]['name'] != 'is_instance', lang_ontology.ontology_triplets)
    else:
        code_ontology.create_node_if_not_exists(path[0]['name'])
        instance_node = code_ontology.create_node_if_not_exists(path[0]['name'] + ' #' + str(random.uniform(0, 20000)))
        code_ontology.create_relation_if_not_exists(instance_node['name'], path[0]['name'], 'is_instance')

    branch_generators = ('Interfaces designation begin keyword', 'Super class designation begin keyword', 'Class declaration begin keyword')
    new_branch_needed = next((node for node in path if node['name'] in branch_generators), None)
    already_create_for = []
    shared_path_part = False

    for triplet in lang_ontology_related_triplets:
        code_ontology.create_node_if_not_exists(triplet[0]['name'])
        code_ontology.create_node_if_not_exists(triplet[2]['name'])
        code_ontology.create_relation_if_not_exists(triplet[0]['name'], triplet[2]['name'], triplet[1]['name'])

        instance_node0 = next((tr[0] for tr in code_ontology.ontology_triplets if tr[2]['name'] == triplet[0]['name'] and tr[1]['name'] == 'is_instance'), None)
        instance_node1 = next((tr[0] for tr in code_ontology.ontology_triplets if tr[2]['name'] == triplet[2]['name'] and tr[1]['name'] == 'is_instance'), None)
        is_already_created0 = triplet[0]['name'] in already_create_for
        is_already_created1 = triplet[2]['name'] in already_create_for

        node0_lang_relations = lang_ontology.get_node_relations(triplet[0])
        a_part_of_count = reduce(lambda x, y: x + 1 if y['name'] == 'a_part_of' else 0, node0_lang_relations, 0)
        if a_part_of_count > 1:
            shared_path_part = True

        if not instance_node0 or (triplet[0]['name'] == 'id' and path[len(path) - 1]['name'] == 'id') or (new_branch_needed and shared_path_part and not is_already_created0):
            instance_node0 = code_ontology.create_node_if_not_exists(triplet[0]['name'] + ' #' + str(random.uniform(0, 20000)))
            code_ontology.create_relation_if_not_exists(instance_node0['name'], triplet[0]['name'], 'is_instance')
            already_create_for.append(triplet[0]['name'])
            print '__Create ' + instance_node0['name']

        node0_lang_relations = lang_ontology.get_node_relations(triplet[2])
        a_part_of_count = reduce(lambda x, y: x + 1 if y['name'] == 'a_part_of' else 0, node0_lang_relations, 0)
        if a_part_of_count > 1:
            shared_path_part = True

        if not instance_node1 or (new_branch_needed and shared_path_part and not is_already_created1):
            instance_node1 = code_ontology.create_node_if_not_exists(triplet[2]['name'] + ' #' + str(random.uniform(0, 20000)))
            code_ontology.create_relation_if_not_exists(instance_node1['name'], triplet[2]['name'], 'is_instance')
            already_create_for.append(triplet[2]['name'])
            print '__Create ' + instance_node1['name']

        code_ontology.create_relation_if_not_exists(instance_node0['name'], instance_node1['name'], triplet[1]['name'])


def convert_to_internal_format(file_path):
    if not lang_ontology:
        print 'No related ontology'
        return

    code_file = open(file_path)
    if not code_file:
        print 'Can\'t open file'
    else:
        lines = code_file.readlines()
        for line in lines:
            code_line = CodeLine(line.strip())
            symbol = code_line.read_symbol()
            while symbol:
                if ' ' != symbol:
                    put_symbol(symbol)
                symbol = code_line.read_symbol()

    print code_ontology.generate_json()


def ontology_bypass(node, visited_nodes=None):
    global code_ontology

    visited_nodes = visited_nodes + [node] if visited_nodes else [node]
    bypass_triplets = filter(lambda triplet: triplet[2] == node and triplet[1]['name'] != 'is_instance', code_ontology.ontology_triplets)
    if bypass_triplets:
        map(lambda triplet: ontology_bypass(triplet[0], visited_nodes), bypass_triplets)
    else:
        instance_triplets = filter(lambda triplet: triplet[2] == node and triplet[1]['name'] == 'is_instance', code_ontology.ontology_triplets)
        if instance_triplets and instance_triplets[0][0] not in viewed_code_symbols:
            code_symbols.append(instance_triplets[0][0]['name'])
            viewed_code_symbols.append(instance_triplets[0][0])
            logs.append(' -> '.join((list(x['name'] for x in visited_nodes))))


def convert_to_external_format(file_path):
    if not code_ontology:
        print 'No related ontology'
        return

    top_node = code_ontology.find_top_node()
    top_node_instances = filter(lambda triplet: triplet[2] == top_node and triplet[1]['name'] == 'is_instance', code_ontology.ontology_triplets)
    map(lambda triplet: ontology_bypass(triplet[0]), top_node_instances)
    print ' '.join(code_symbols)

if __name__ == '__main__':
    opts, extraparams = getopt.getopt(sys.argv[1:], '', ['method=', 'source-path=', 'ontology-path='])

    method_name = (item[1] for item in opts if item[0] == '--method').next()
    related_ontology_path = (item[1] for item in opts if item[0] == '--ontology-path').next()

    if method_name == 'supported_extensions':
        print_supported_extensions()
    elif method_name == 'import':
        if related_ontology_path:
            json_data = open(related_ontology_path)
            lang_ontology_json = json.load(json_data)
            lang_ontology = Ontology(lang_ontology_json)
            json_data.close()
        code_ontology = Ontology()

        path = (item[1] for item in opts if item[0] == '--source-path').next()
        convert_to_internal_format(path)

        log_file = open(path + '.log', 'w')
        for log_line in code_ontology.logs:
            log_file.write(log_line + '\n')
        log_file.close()

    elif method_name == 'export':
        if related_ontology_path:
            json_data = open(related_ontology_path)
            code_ontology_json = json.load(json_data)
            code_ontology = Ontology(code_ontology_json)
            json_data.close()

        path = (item[1] for item in opts if item[0] == '--source-path').next()
        convert_to_external_format(path)

        log_file = open(path + '.log', 'w')
        for log_line in logs:
            log_file.write(log_line + '\n')
        log_file.close()
    else:
        print "Unknown method name " + method_name
