#! /usr/local/bin/python
# -*- coding: utf-8 -*-

import sys
import getopt
import re
import json
import random
from rsp import *

reload(sys)


class CustomJsonEncoder(json.JSONEncoder):
    def default(self, o):
        if not isinstance(o, OntologyNode) and not isinstance(o, OntologyRelation):
            return super(CustomJsonEncoder, self).default(o)
        return o.json()


class OntologyNode:
    def __init__(self):
        self.id = 0
        self.name = None
        self.pos_x = random.uniform(0, 4000)
        self.pos_y = random.uniform(0, 4000)
        self.meta = {}

    def json(self):
        json_data = {
            'id': self.id,
            'name': self.name,
            'position_x': self.pos_x,
            'position_y': self.pos_y,
            'meta': self.meta
        }
        return json_data

    def from_json(self, json_data):
        self.id = int(json_data['id'])
        self.name = json_data['name']
        self.pos_x = json_data['position_x']
        self.pos_y = json_data['position_y']
        if 'meta' in json_data:
            self.meta = json_data['meta']

    def __str__(self):
        return "{} [{!s}]".format(self.name, self.id)


class OntologyRelation:
    def __init__(self):
        self.id = 0
        self.name = None
        self.source_node = None
        self.destination_node = None
        self.meta = {}

    def json(self):
        json_data = {
            'id': self.id,
            'name': self.name,
            'source_node_id': self.source_node.id,
            'destination_node_id': self.destination_node.id,
            'meta': self.meta
        }
        return json_data

    def from_json(self, json_data, nodes):
        self.id = json_data['id']
        self.name = json_data['name']
        node_id = int(json_data['source_node_id'])
        self.source_node = next((x for x in nodes if x.id == node_id), None)
        node_id = int(json_data['destination_node_id'])
        self.destination_node = next((x for x in nodes if x.id == node_id), None)
        if 'meta' in json_data:
            self.meta = json_data['meta']

    def __str__(self):
        return "{} - {} - {}".format(self.source_node, self.name, self.destination_node)


class Ontology:
    def __init__(self):
        self._next_id = 1
        self.nodes = []
        self.relations = []
        self.meta = {}

    def get_node(self, node_id):
        return next((x for x in self.nodes if x.id == node_id), None)

    def find_node(self, name):
        for node in self.nodes:
            if node.name == name:
                return node
        return None

    def create_node_if_needed(self, name=None):
        name = name if name else 'unnamed #' + str(self._next_id)
        node = self.find_node(name)
        if not node:
            node = OntologyNode()
            node.id = self._next_id
            self._next_id += 1
            node.name = name
            self.nodes.append(node)
        return node

    def create_relation(self, name, source_node, destination_node):
        relation = next((x for x in self.relations if x.source_node.id == source_node.id
                        and x.destination_node.id == destination_node.id
                        and x.name == name), None)
        if relation is None:
            relation = OntologyRelation()
            relation.id = self._next_id
            self._next_id += 1
            relation.name = name
            relation.source_node = source_node
            relation.destination_node = destination_node
            self.relations.append(relation)
        return relation

    def get_relations_with_source_node(self, source_node):
        return list(x for x in self.relations if x.source_node.id == source_node.id)

    def get_relations_with_destination_node(self, destination_node):
        return list(x for x in self.relations if x.destination_node.id == destination_node.id)

    def _get_nodes_related_to_node(self, node):
        relations = self.get_relations_with_source_node(node)
        nodes = []
        for relation in relations:
            if "<" not in relation.destination_node.name and relation.destination_node not in nodes:
                nodes.append(relation.destination_node)

        relations = self.get_relations_with_destination_node(node)
        for relation in relations:
            if "<" not in relation.source_node.name and relation.source_node not in nodes:
                nodes.append(relation.source_node)

        return nodes

    def add_class(self, class_desc):
        class_node = self._add_class_name(class_desc["name"])
        if "super" in class_desc:
            self._add_super(class_desc["super"], class_node)
        if "protocols" in class_desc:
            for protocol in class_desc["protocols"]:
                self._add_interface(protocol, class_node)
        return class_node

    def add_method(self, method_desc, class_node):
        method_node = self._add_method(class_node)
        self._add_method_type(method_desc["type"], method_node)
        if "name" in method_desc:
            self._add_method_name(method_desc["name"], method_node)
        elif "parameters" in method_desc:
            for parameter in method_desc["parameters"]:
                self._add_method_parameter(parameter, method_node)

    def get_class_nodes(self):
        class_declaration_node = self.find_node("<class-interface>")
        relations = self.get_relations_with_destination_node(class_declaration_node)
        nodes = []
        for relation in relations:
            if relation.name == "is_instance":
                nodes.append(relation.source_node)
        return nodes

    def get_class_name(self, class_node):
        identifier_node = self.find_node("<identifier>")
        return self._find_node_between_nodes(class_node, identifier_node)

    def get_class_super(self, class_node):
        class_type_node = self.find_node("<class-name>")
        return self._find_node_between_nodes(class_node, class_type_node)

    def get_class_interfaces(self, class_node):
        interface_type_node = self.find_node("<protocol-name>")
        return self._find_node_between_nodes(class_node, interface_type_node)

    def get_class_methods(self, class_node):
        method_header_node = self.find_node("<instance-method-declaration>")
        return self._find_node_between_nodes(class_node, method_header_node)

    def get_method_name(self, method_node):
        identifier_node = self.find_node("<identifier>")
        return self._find_node_between_nodes(method_node, identifier_node)

    def get_method_type(self, method_node):
        method_type_node = self.find_node("<method-type>")
        return self._find_node_between_nodes(method_node, method_type_node)

    def get_method_parameters(self, method_node):
        formal_parameter_node = self.find_node("<method-selector>")
        return self._find_node_between_nodes(method_node, formal_parameter_node)

    def _add_method(self, class_node):
        method_header_node = self.find_node("<instance-method-declaration>")
        method_node = self.create_node_if_needed()
        self.create_relation("is_instance", method_node, method_header_node)
        self.create_relation("a_part_of", method_node, class_node)
        return method_node

    def _add_method_name(self, name, method_node):
        identifier_node = self.find_node("<identifier>")
        method_name_node = self.create_node_if_needed(name)
        self.create_relation("is_instance", method_name_node, identifier_node)
        self.create_relation("a_part_of", method_name_node, method_node)
        return method_node

    def _add_method_parameter(self, parameter, method_node):
        formal_parameter_node = self.find_node("<method-selector>")
        parameter_node = self.create_node_if_needed(parameter)
        self.create_relation("is_instance", parameter_node, formal_parameter_node)
        self.create_relation("a_part_of", parameter_node, method_node)

    def _add_method_type(self, type, method_node):
        method_type_node = self.find_node("<method-type>")
        type_node = self.create_node_if_needed(type)
        self.create_relation("is_instance", type_node, method_type_node)
        self.create_relation("a_part_of", type_node, method_node)

    def _add_class_name(self, name):
        class_declaration_node = self.find_node("<class-interface>")
        identifier_node = self.find_node("<identifier>")
        class_node = self.create_node_if_needed()
        class_name_node = self.create_node_if_needed(name)
        self.create_relation("is_instance", class_node, class_declaration_node)
        self.create_relation("is_instance", class_name_node, identifier_node)
        self.create_relation("a_part_of", class_name_node, class_node)
        return class_node

    def _add_super(self, super, class_node):
        class_type_node = self.find_node("<class-name>")
        super_node = self.create_node_if_needed(super)
        self.create_relation("is_instance", super_node, class_type_node)
        self.create_relation("a_part_of", super_node, class_node)

    def _add_interface(self, interface, class_node):
        interface_type_node = self.find_node("<protocol-name>")
        interface_node = self.create_node_if_needed(interface)
        self.create_relation("is_instance", interface_node, interface_type_node)
        self.create_relation("a_part_of", interface_node, class_node)

    def _find_node_between_nodes(self, node1, node2):
        nodes_with_node1 = self._get_nodes_related_to_node(node1)
        nodes_with_node2 = self._get_nodes_related_to_node(node2)
        result = []
        for node in nodes_with_node2:
            if node in nodes_with_node1:
                result.append(node)
        return result

    def json(self):
        json_data = {
            'meta': self.meta,
            'last_id': self._next_id - 1,
            'nodes': self.nodes,
            'relations': self.relations
        }
        return json.dumps(json_data, cls=CustomJsonEncoder, indent=4, sort_keys=True)

    def from_json(self, json_data):
        if 'meta' in json_data:
            self.meta = json_data['meta']
        self._next_id = int(json_data['last_id']) + 1
        self.nodes = []
        self.relations = []

        for n in json_data['nodes']:
            node = OntologyNode()
            node.from_json(n)
            self.nodes.append(node)

        for r in json_data['relations']:
            relation = OntologyRelation()
            relation.from_json(r, self.nodes)
            self.relations.append(relation)

    def __str__(self):
        return str(self.json())


#----------------------------------------------------


class ObjCToOntConverter:
    def __init__(self, java_file):
        self._file = java_file
        self._current_class = None
        self._current_method = None
        self._ontology = Ontology()
        self._ontology.from_json(json.load(file(ocbn_path)))
        self._ontology.create_node_if_needed("##.m")

    def convert(self):
        self._bypass()
        return self._ontology.json()

    def _bypass(self):
        for line in self._file:
            self._process_line(line)

    def _process_line(self, line):
        if re.match(objc_class_regexp, line):
            self._process_class(line)
        elif re.match(objc_simple_method_regexp, line):
            self._process_simple_method(line)
        elif re.match(objc_method_with_parameters_regexp, line):
            self._process_method_with_modifiers(line)

    def _process_class(self, line):
        match = re.match(objc_class_regexp, line)
        desc = {}
        desc["name"] = match.group(1)
        desc["super"] = match.group(2)
        desc["protocols"] = match.group(3).split(",")
        # print desc
        self._current_class = self._ontology.add_class(desc)

    def _process_simple_method(self, line):
        match = re.match(objc_simple_method_regexp, line)
        desc = {}
        desc["type"] = match.group(1)
        desc["name"] = match.group(2)
        # print desc
        self._ontology.add_method(desc, self._current_class)

    def _process_method_with_modifiers(self, line):
        match = re.match(objc_method_with_parameters_regexp, line)
        desc = {}
        desc["type"] = match.group(1)
        desc["parameters"] = []
        searchs = re.findall(objc_parameter, match.group(2))
        desc["parameters"] = searchs
        # print desc
        self._ontology.add_method(desc, self._current_class)


#----------------------------------------------------


class OntToObjCConverter:
    def __init__(self, ont_file):
        self._ontology = Ontology()
        self._ontology.from_json(json.load(ont_file))
        self._code = ""

    def convert(self):
        class_nodes = self._ontology.get_class_nodes()
        self._bypass_by_classes(class_nodes)
        return self._code

    def _bypass_by_classes(self, class_nodes):
        for class_node in class_nodes:
            name = self._ontology.get_class_name(class_node)
            self._code += "@interface "
            # print name[0]
            self._code += name[0].name + " "
            super_class = self._ontology.get_class_super(class_node)
            # print super_class[0]
            if super_class[0]:
                self._code += ": " + super_class[0].name + " "
            interfaces = self._ontology.get_class_interfaces(class_node)
            self._bypass_by_interfaces(interfaces)
            self._code += "\n\n\n@end\n\n@implementation " + name[0].name + "\n"
            methods = self._ontology.get_class_methods(class_node)
            self._bypass_methods(methods)
            self._code += "\n@end"

    def _bypass_by_interfaces(self, interfaces):
        if interfaces:
            self._code += "<"
            for interface in interfaces:
                # print interface
                self._code += interface.name + " "
            self._code += "> "

    def _bypass_methods(self, methods):
        for method in methods:
            self._code += "\n"
            method_type = self._ontology.get_method_type(method)
            self._code += "- (" + method_type[0].name + ")"
            name = self._ontology.get_method_name(method)
            parameters = self._ontology.get_method_parameters(method)
            if name:
                # print name[0]
                self._code += name[0].name + " "
            elif parameters:
                self._bypass_method_parameters(parameters)
            self._code += "{\n\n}\n"

    def _bypass_method_parameters(self, parameters):
        is_first = True
        for parameter in parameters:
            # print parameter
            if "#" not in parameter.name:
                if not is_first:
                    self._code += " "
                self._code += parameter.name
                is_first = False


#----------------------------------------------------

def print_supported_extensions():
    print '.m'


def convert_to_internal_format(file_path, ontology_path):
    converter = ObjCToOntConverter(open(file_path))
    print converter.convert()


def convert_to_external_format(file_path):
    converter = OntToObjCConverter(open(file_path))
    print converter.convert()


if __name__ == '__main__':
    opts, extraparams = getopt.getopt(sys.argv[1:], '', ['method=', 'source-path=', 'ontology-path='])

    method_name = (item[1] for item in opts if item[0] == '--method').next()
    if method_name == 'supported_extensions':
        print_supported_extensions()
    elif method_name == 'import':
        path = (item[1] for item in opts if item[0] == '--source-path').next()
        # onto_path = (item[1] for item in opts if item[0] == '--ontology-path').next()
        convert_to_internal_format(path, "")
    elif method_name == 'export':
        path = (item[1] for item in opts if item[0] == '--source-path').next()
        convert_to_external_format(path)
    else:
        print "Unknown method name " + method_name
