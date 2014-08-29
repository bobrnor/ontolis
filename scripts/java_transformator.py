#! /usr/local/bin/python
# -*- coding: utf-8 -*-

import sys
import getopt
import json
import random

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
        self.id = int(json_data['id'])
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
        if "modifiers" in class_desc:
            for modifier in class_desc["modifiers"]:
                self._add_modifier(modifier, class_node)
        if "super" in class_desc:
            self._add_super(class_desc["super"], class_node)
        if "interfaces" in class_desc:
            for interface in class_desc["interfaces"]:
                self._add_interface(interface, class_node)
        return class_node

    def add_method(self, method_desc, class_node):
        method_node = self._add_method_name(method_desc["name"], class_node)
        if "modifiers" in method_desc:
            for modifier in method_desc["modifiers"]:
                self._add_method_modifier(modifier, method_node)
        if "parameters" in method_desc:
            for parameter in method_desc["parameters"]:
                self._add_method_parameter(parameter, method_node)

    def get_class_nodes(self):
        class_declaration_node = self.find_node("<class declaration>")
        relations = self.get_relations_with_destination_node(class_declaration_node)
        nodes = []
        for relation in relations:
            if relation.name == "is_instance":
                nodes.append(relation.source_node)
        return nodes

    def get_class_modifiers(self, class_node):
        class_modifier_node = self.find_node("<class modifier>")
        return self._find_node_between_nodes(class_node, class_modifier_node)

    def get_class_name(self, class_node):
        identifier_node = self.find_node("<identifier>")
        return self._find_node_between_nodes(class_node, identifier_node)

    def get_class_super(self, class_node):
        class_type_node = self.find_node("<class type>")
        return self._find_node_between_nodes(class_node, class_type_node)

    def get_class_interfaces(self, class_node):
        interface_type_node = self.find_node("<interface type>")
        return self._find_node_between_nodes(class_node, interface_type_node)

    def get_class_methods(self, class_node):
        method_header_node = self.find_node("<method header>")
        return self._find_node_between_nodes(class_node, method_header_node)

    def get_method_modifiers(self, method_node):
        method_modifier_node = self.find_node("<method modifier>")
        return self._find_node_between_nodes(method_node, method_modifier_node)

    def get_method_name(self, method_node):
        identifier_node = self.find_node("<identifier>")
        return self._find_node_between_nodes(method_node, identifier_node)

    def get_method_parameters(self, method_node):
        formal_parameter_node = self.find_node("<formal parameter>")
        return self._find_node_between_nodes(method_node, formal_parameter_node)

    def _add_method_name(self, name, class_node):
        method_header_node = self.find_node("<method header>")
        identifier_node = self.find_node("<identifier>")
        method_name_node = self.create_node_if_needed(name)
        method_node = self.create_node_if_needed()
        self.create_relation("is_instance", method_node, method_header_node)
        self.create_relation("is_instance", method_name_node, identifier_node)
        self.create_relation("a_part_of", method_name_node, method_node)
        self.create_relation("a_part_of", method_node, class_node)
        return method_node

    def _add_method_modifier(self, modifier, method_node):
        method_modifier_node = self.find_node("<method modifier>")
        modifier_node = self.create_node_if_needed(modifier)
        self.create_relation("is_instance", modifier_node, method_modifier_node)
        self.create_relation("a_part_of", modifier_node, method_node)

    def _add_method_parameter(self, parameter, method_node):
        formal_parameter_node = self.find_node("<formal parameter>")
        parameter_node = self.create_node_if_needed(parameter)
        self.create_relation("is_instance", parameter_node, formal_parameter_node)
        self.create_relation("a_part_of", parameter_node, method_node)

    def _add_class_name(self, name):
        class_declaration_node = self.find_node("<class declaration>")
        identifier_node = self.find_node("<identifier>")
        class_node = self.create_node_if_needed()
        class_name_node = self.create_node_if_needed(name)
        self.create_relation("is_instance", class_node, class_declaration_node)
        self.create_relation("is_instance", class_name_node, identifier_node)
        self.create_relation("a_part_of", class_name_node, class_node)
        return class_node

    def _add_modifier(self, modifier, class_node):
        class_modifier_node = self.find_node("<class modifier>")
        modifier_node = self.create_node_if_needed(modifier)
        self.create_relation("is_instance", modifier_node, class_modifier_node)
        self.create_relation("a_part_of", modifier_node, class_node)

    def _add_super(self, super, class_node):
        class_type_node = self.find_node("<class type>")
        super_node = self.create_node_if_needed(super)
        self.create_relation("is_instance", super_node, class_type_node)
        self.create_relation("a_part_of", super_node, class_node)

    def _add_interface(self, interface, class_node):
        interface_type_node = self.find_node("<interface type>")
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


class OntToObjCConverter:
    def __init__(self, ont_file):
        self._ontology = Ontology()
        self._ontology.from_json(json.load(ont_file))
        self._code = ""
        self._datatypes = {
            "boolean": "BOOL",
            "String": "NSString *",
            "Byte": "byte",
            "Double": "double",
            "Float": "float",
            "Integer": "int",
            "Long": "long",
            "Short": "short",
            "void": "void",
            "private": "NSString *",
        }

    def convert(self):
        class_nodes = self._ontology.get_class_nodes()
        self._bypass_by_classes(class_nodes)
        return self._code

    def _bypass_by_classes(self, class_nodes):
        for class_node in class_nodes:
            # print class_node
            # modifiers = self._ontology.get_class_modifiers(class_node)
            # self._bypass_by_modifiers(modifiers)
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
            self._code += "\n\n@end\n@implementation " + name[0].name + "\n"
            methods = self._ontology.get_class_methods(class_node)
            self._bypass_methods(methods)
            self._code += "}"

    def _bypass_by_modifiers(self, modifiers):
        for modifier in modifiers:
            # print modifier
            self._code += modifier.name + " "

    def _bypass_by_interfaces(self, interfaces):
        if interfaces:
            self._code += "<"
            for interface in interfaces:
                # print interface
                self._code += interface.name + " "
            self._code += ">"

    def _bypass_methods(self, methods):
        for method in methods:
            self._code += "\n"
            modifiers = self._ontology.get_method_modifiers(method)
            self._bypass_method_modifiers(modifiers)
            name = self._ontology.get_method_name(method)
            # print name[0]
            self._code += name[0].name + " "
            parameters = self._ontology.get_method_parameters(method)
            self._bypass_method_parameters(parameters)
            self._code += "{\n\n}\n"

    def _get_type_name(self, type):
        type = type.strip()
        if type in self._datatypes:
            return self._datatypes[type]
        elif type[0].isupper():
            return type + " *"
        else:
            return type

    def _bypass_method_modifiers(self, modifiers):
        self._code += "- ("
        self._code += self._get_type_name(modifiers[len(modifiers) - 1].name) + " "
        self._code += ")"

    def _bypass_method_parameters(self, parameters):
        is_first = True
        for parameter in parameters:
            if "#" not in parameter.name:
                type, name = parameter.name.split()
                if not is_first:
                    self._code += name
                self._code += ":(" + self._get_type_name(type) + ")" + name + " "
            is_first = False


#----------------------------------------------------

def print_supported_extensions():
    print '.java'


def transform(file_path):
    converter = OntToObjCConverter(open(file_path))
    print converter.convert()

if __name__ == '__main__':
    opts, extraparams = getopt.getopt(sys.argv[1:], '', ['method=', 'source-path='])

    method_name = (item[1] for item in opts if item[0] == '--method').next()
    if method_name == 'supported_extensions':
        print_supported_extensions()
    elif method_name == 'transform':
        path = (item[1] for item in opts if item[0] == '--source-path').next()
        transform(path)
    else:
        print "Unknown method name " + method_name
