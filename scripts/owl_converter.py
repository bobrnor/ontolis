#! /usr/local/bin/python
# -*- coding: utf-8 -*-

import sys
import random
import getopt
import json
from lxml import etree
from copy import deepcopy

reload(sys)
sys.setdefaultencoding('utf-8')
random.seed()

NSMAP = {'owl' : '{http://www.w3.org/2002/07/owl#}', 'rdf' : '{http://www.w3.org/1999/02/22-rdf-syntax-ns#}', 'rdfs' : '{http://www.w3.org/2000/01/rdf-schema#}'}

next_object_id = 0
nodes = []
relations = []

#================from .owl==============================


def create_relation(name, first_class_name, second_class_name, attributes=None):
    if not attributes:
        attributes = {}

    global next_object_id
    global relations

    first_class_node = find_node_by_name(first_class_name)
    second_class_node = find_node_by_name(second_class_name)

    relation = {'id': next_object_id, 'name': name, 'source_node_id': first_class_node['id'], 'destination_node_id': second_class_node['id']}
    if attributes != {}:
        relation['attributes'] = attributes
    relations.append(relation)
    next_object_id += 1

    return relation


def create_node_if_needed(name, attributes=None):
    if not attributes:
        attributes = {}

    global next_object_id
    global nodes

    node = find_node_by_name(name)
    if node is None:
        node = {'id': next_object_id, 'name': name, 'position_x': random.uniform(0, 2000), 'position_y': random.uniform(0, 2000)}
        if attributes != {}:
            node['attributes'] = attributes
        next_object_id += 1
        nodes.append(node)
    elif attributes != {}:
        node['attributes'] = attributes

    return node


def find_node_by_name(node_name):
    global nodes
    return next((node for node in nodes if node['name'] == node_name), None)


def process_sub_class_of(owl_class, sub_class_of_node):
    if NSMAP['rdf'] + 'resource' in sub_class_of_node.attrib:
        sub_class_node_name = sub_class_of_node.attrib[NSMAP['rdf'] + 'resource']
        if find_node_by_name(sub_class_node_name) is None:
            create_node_if_needed(sub_class_node_name)
        create_relation(NSMAP['rdfs'] + 'subClassOf', owl_class.attrib[NSMAP['rdf'] + 'ID'], sub_class_node_name)
    else:
        restriction_node = sub_class_of_node.find(NSMAP['owl'] + 'Restriction')
        on_property_node = restriction_node.find(NSMAP['owl'] + 'onProperty')
        has_value_node = restriction_node.find(NSMAP['owl'] + 'hasValue')
        some_values_from_node = restriction_node.find(NSMAP['owl'] + 'someValuesFrom')

        if has_value_node is not None:
            sub_class_node_name = has_value_node.attrib[NSMAP['rdf'] + 'resource'].strip('_#')
            if find_node_by_name(sub_class_node_name) is None:
                create_node_if_needed(sub_class_node_name)
            create_relation(on_property_node.attrib[NSMAP['rdf'] + 'resource'], owl_class.attrib[NSMAP['rdf'] + 'ID'], sub_class_node_name, {'owl': {'owlRelationType': NSMAP['owl'] + 'hasValue'}})
        elif some_values_from_node is not None:
            sub_class_node_name = some_values_from_node.attrib[NSMAP['rdf'] + 'resource'].strip('_#')
            if find_node_by_name(sub_class_node_name) is None:
                create_node_if_needed(sub_class_node_name)
            create_relation(on_property_node.attrib[NSMAP['rdf'] + 'resource'], owl_class.attrib[NSMAP['rdf'] + 'ID'], sub_class_node_name, {'owl': {'owlRelationType': NSMAP['owl'] + 'someValuesFrom'}})


def process_class_attributes(owl_class):
    attrs = {}
    for attr_name in owl_class.attrib.keys():
        if attr_name != NSMAP['rdf'] + 'ID':
            attrs[attr_name] = owl_class.attrib[attr_name]
    return attrs


def process_class_content(owl_class):
    unknown_content = []
    for child_node in owl_class.getchildren():
        if child_node.tag == NSMAP['rdfs'] + 'subClassOf':
            process_sub_class_of(owl_class, child_node)
        else:
            unknown_content.append(etree.tostring(child_node, encoding='UTF-8'))

    return unknown_content


def process_class(owl_class):
    name = owl_class.attrib[NSMAP['rdf'] + 'ID']
    node = create_node_if_needed(name)
    class_attributes = process_class_attributes(owl_class)
    owl_attributes = process_class_content(owl_class)

    attributes = {}
    if class_attributes != {}:
        attributes['class-attributes'] = class_attributes

    if owl_attributes != {}:
        attributes['owl'] = {'unknown-content': owl_attributes}

    if attributes != {}:
        node['attributes'] = attributes

#==================to .owl==============================


DEFAULT_RDF_ROOT = '<rdf:RDF xmlns:owl="http://www.w3.org/2002/07/owl#" xmlns:rdf="http://www.w3.org/1999/02/22-rdf-syntax-ns#" xmlns:rdfs="http://www.w3.org/2000/01/rdf-schema#" xmlns:xsd="http://www.w3.org/2001/XMLSchema#"></rdf:RDF>'
DEFAULT_OWL_HEADER = '<owl:Ontology rdf:about="Data Source=.;MultipleActiveResultSetrees=true;Initial Catalog=NightBase;Integrated Security=true"></owl:Ontology>'


def get_node_by_id(node_id):
    global nodes
    return next((node for node in nodes if node['id'] == node_id), None)


def fill_class_attributes(node, class_element):
    if 'attributes' in node:
        attributes = node['attributes']
        if attributes is not None and 'class-attributes' in attributes:
            class_attributes = attributes['class-attributes']
        class_element.attrib[NSMAP['rdf'] + 'ID'] = node['name']


def fill_owl_class_content(node, class_element):
    if 'attributes' in node:
        attributes = node['attributes']
        if 'owl' in attributes and 'unknown-content' in attributes['owl']:
            owl_content = attributes['owl']['unknown-content']
            for owl_tag in owl_content:
                parsed_tag = etree.fromstring(owl_tag)
                class_element.append(parsed_tag)


def fill_sub_class_of_content(class_element, relation):
    node = get_node_by_id(relation['destination_node_id'])
    element = etree.Element(relation['name'], {NSMAP['rdf'] + 'resource': node['name']})
    class_element.append(element)


def fill_has_value_content(class_element, relation):
    node = get_node_by_id(relation['destination_node_id'])
    restriction_element = etree.Element(NSMAP['owl'] + 'Restriction')
    on_property_element = etree.Element(NSMAP['owl'] + 'onProperty', {NSMAP['rdf'] + 'resource': relation['name']})
    has_value_element = etree.Element(NSMAP['owl'] + 'hasValue', {NSMAP['rdf'] + 'resource': '#' + node['name']})
    restriction_element.append(on_property_element)
    restriction_element.append(has_value_element)
    class_element.append(restriction_element)


def fill_some_values_from_content(class_element, relation):
    node = get_node_by_id(relation['destination_node_id'])
    restriction_element = etree.Element(NSMAP['owl'] + 'Restriction')
    on_property_element = etree.Element(NSMAP['owl'] + 'onProperty', {NSMAP['rdf'] + 'resource': relation['name']})
    some_values_from_element = etree.Element(NSMAP['owl'] + 'someValuesFrom', {NSMAP['rdf'] + 'resource': '#' + node['name']})
    restriction_element.append(on_property_element)
    restriction_element.append(some_values_from_element)
    class_element.append(restriction_element)


def fill_class_content(node, class_element):
    for relation in relations:
        if relation['source_node_id'] == node['id']:
            if relation['name'] == NSMAP['rdfs'] + 'subClassOf':
                fill_sub_class_of_content(class_element, relation)
            elif 'attributes' in relation and 'owl' in relation['attributes'] and relation['attributes']['owl']['owlRelationType'] == NSMAP['owl'] + 'hasValue':
                fill_has_value_content(class_element, relation)
            elif 'attributes' in relation and 'owl' in relation['attributes'] and relation['attributes']['owl']['owlRelationType'] == NSMAP['owl'] + 'someValuesFrom':
                fill_some_values_from_content(class_element, relation)


def process_node(node):
    class_element = etree.Element(NSMAP['owl'] + 'Class')
    fill_class_attributes(node, class_element)
    fill_class_content(node, class_element)
    fill_owl_class_content(node, class_element)

    return class_element

#=======================================================


def print_supported_extensions():
    print ".owl"


def convert_to_internal_format(file_path):
    global next_object_id
    global nodes
    global relations

    xml = etree.parse(file_path)
    rdf_root = xml.getroot()
    owl_header = rdf_root.find(NSMAP['owl'] + 'Ontology')

    empty_rdf_root = deepcopy(rdf_root)
    for child in empty_rdf_root.getchildren():
        empty_rdf_root.remove(child)

    owl_classes = rdf_root.findall(NSMAP['owl'] + 'Class')
    for owlClass in owl_classes:
        process_class(owlClass)

    print json.dumps({'last_id': next_object_id - 1, 'nodes': nodes, 'relations': relations, 'rdf': etree.tostring(empty_rdf_root, encoding='UTF-8'), 'owl': etree.tostring(owl_header, encoding='UTF-8')})


def convert_to_external_format(file_path):
    global next_object_id
    global nodes
    global relations

    json_data = open(file_path)
    json_object = json.load(json_data)
    next_object_id = json_object['last_id']
    nodes = json_object['nodes']
    relations = json_object['relations']

    rdf_root = None
    owl_header = None

    if json_object['rdf'] is not None:
        rdf_root = etree.fromstring(json_object['rdf'])
    else:
        rdf_root = etree.fromstring(DEFAULT_RDF_ROOT)

    if json_object['owl'] is not None:
        owl_header = etree.fromstring(json_object['owl'])
    else:
        owl_header = etree.fromstring(DEFAULT_OWL_HEADER)

    rdf_root.append(owl_header)

    for node in nodes:
        class_element = process_node(node)
        rdf_root.append(class_element)

    print etree.tostring(rdf_root, encoding='UTF-8')

if __name__ == '__main__':
    opts, extraparams = getopt.getopt(sys.argv[1:], '', ['method=', 'source-path='])

    method_name = (item[1] for item in opts if item[0] == '--method').next()
    if method_name == 'supported_extensions':
        print_supported_extensions()
    elif method_name == 'import':
        path = (item[1] for item in opts if item[0] == '--source-path').next()
        convert_to_internal_format(path)
    elif  method_name == 'export':
        path = (item[1] for item in opts if item[0] == '--source-path').next()
        convert_to_external_format(path)
    else:
        print "Unknown method name " + method_name
