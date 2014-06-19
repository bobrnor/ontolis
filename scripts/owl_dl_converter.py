#! /usr/local/bin/python
# -*- coding: utf-8 -*-

import sys
import getopt
import copy
import json
import random
import xml.dom.minidom
from lxml import etree

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
        self.id = json_data['id']
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
        node_id = json_data['source_node_id']
        self.source_node = next((x for x in nodes if x.id == node_id), None)
        node_id = json_data['destination_node_id']
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
        self._next_id = json_data['last_id'] + 1
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


#-----------------------------------------------------------------


class OWLParser:
    def __init__(self, file):
        self._etree = etree.parse(file)
        self._ontology = Ontology()
        self._handlers = {
            'Ontology': self._process_ontology,
            'versionInfo': self._process_version_info,
            'Class': self._process_class,
            'subClassOf': self._process_subclassof,
            'label': self._process_label,
            'Thing': self._pass,
            'type': self._process_type,
            'ObjectProperty': self._process_object_property,
            'Property': self._process_property,
            'domain': self._process_domain,
            'range': self._process_range,
            'subPropertyOf': self._process_subproperty,
            'Restriction': self._process_restriction,
            'onProperty': self._process_on_property,
            'minCardinality': self._process_min_cardinality,
            'maxCardinality': self._process_max_cardinality,
            'inverseOf': self._process_inverse_of,
            'allValuesFrom': self._pass,
            'someValuesFrom': self._process_some_values_from,
            'cardinality': self._process_cardinality,
            'hasValue': self._process_has_value,
            'equivalentClass': self._pass,
            'sameAs': self._pass,
            'differentFrom': self._pass,
            'AllDifferent': self._pass,
            'distinctMembers': self._pass,
            'intersectionOf': self._pass,
            'unionOf': self._pass,
            'complementOf': self._pass,
            'oneOf': self._pass,
            'disjointWith': self._process_disjoint_with,
            'comment': self._process_comment
        }

    def parse_to_ont(self):
        self._bypass(self._etree.getroot())
        print(self._ontology.json())

    def _bypass(self, root, context=None):
        context = {'n0': None, 'r': None, 'n1': None} if not context else copy.copy(context)
        if context['n1']:
            context = {'n0': context['n1'], 'r': None, 'n1': None}
        for child in root.getchildren():
            self._process_element(child, context)
            self._bypass(child, context)

    def _pass(self, element, context):
        print(element.tag)

    @staticmethod
    def _get_tag_name(tag):
        brace_pos = tag.find('}')
        if brace_pos > -1:
            return tag[brace_pos + 1:]
        else:
            return tag

    def _process_element(self, element, context):
        tag_name = self._get_tag_name(element.tag)
        if tag_name in self._handlers:
            self._handlers[tag_name](element, context)
        else:
            print(element.tag)

    def _process_class(self, element, context):
        class_id = element.attrib.get('{http://www.w3.org/1999/02/22-rdf-syntax-ns#}ID')
        if not class_id:
            class_id = element.attrib.get('{http://www.w3.org/1999/02/22-rdf-syntax-ns#}about')
        class_node = self._ontology.create_node_if_needed(element.tag)
        class_instance_node = self._ontology.create_node_if_needed(class_id)
        self._ontology.create_relation('is_instance', class_instance_node, class_node)
        if context.get('r'):
            source_node = context['n0']
            relation = context['r']
            ontology_relation = self._ontology.create_relation(relation.name, source_node, class_instance_node)
            ontology_relation.meta = relation.meta
        else:
            context['n0'] = class_instance_node

    def _process_subclassof(self, element, context):
        relation = OntologyRelation()
        relation.name = 'subClassOf'
        context['r'] = relation
        superclass_id = element.attrib.get('{http://www.w3.org/1999/02/22-rdf-syntax-ns#}resource')
        if superclass_id:
            superclass_id = superclass_id.replace('#', '')
            superclass_node = self._ontology.create_node_if_needed(superclass_id)
            self._ontology.create_relation(relation.name, context['n0'], superclass_node)

    def _process_comment(self, element, context):
        comment_text = element.text
        if context['n1']:
            context['n1'].meta['comment'] = comment_text
        elif context['n0']:
            context['n0'].meta['comment'] = comment_text
        else:
            self._ontology.meta['comment'] = element.text

    def _process_property(self, element, context):
        property_id = element.attrib.get('{http://www.w3.org/1999/02/22-rdf-syntax-ns#}ID')
        property_node = self._ontology.create_node_if_needed(element.tag)
        property_instance_node = self._ontology.create_node_if_needed(property_id)
        self._ontology.create_relation('is_instance', property_instance_node, property_node)
        if context.get('r'):
            relation = context['r']
            source_node = context['n0']
            ontology_relation = self._ontology.create_relation(relation.name, source_node, property_instance_node)
            ontology_relation.meta = relation.meta
        else:
            context['n0'] = property_instance_node

    def _process_subproperty(self, element, context):
        relation = OntologyRelation()
        relation.name = 'subPropertyOf'
        context['r'] = relation
        super_property_id = element.attrib.get('{http://www.w3.org/1999/02/22-rdf-syntax-ns#}resource')
        if super_property_id:
            super_property_id = super_property_id.replace('#', '')
            super_property_node = self._ontology.create_node_if_needed(super_property_id)
            self._ontology.create_relation(relation.name, context['n0'], super_property_node)

    def _process_domain(self, element, context):
        domain_resource = element.attrib.get('{http://www.w3.org/1999/02/22-rdf-syntax-ns#}resource')
        domain_resource = domain_resource.replace('#', '')
        # domain_node = self._ontology.create_node_if_needed(element.tag)
        domain_instance_node = self._ontology.create_node_if_needed(domain_resource)
        # self._ontology.create_relation('is_instance', domain_instance_node, domain_node)
        self._ontology.create_relation('has_domain', context['n0'], domain_instance_node)

    def _process_range(self, element, context):
        range_resource = element.attrib.get('{http://www.w3.org/1999/02/22-rdf-syntax-ns#}resource')
        range_resource = range_resource.replace('#', '')
        # range_node = self._ontology.create_node_if_needed(element.tag)
        range_instance_node = self._ontology.create_node_if_needed(range_resource)
        # self._ontology.create_relation('is_instance', range_instance_node, range_node)
        self._ontology.create_relation('has_range', context['n0'], range_instance_node)

    def _process_ontology(self, element, context):
        self._ontology.meta['Ontology'] = etree.tostring(element)

    def _process_version_info(self, element, context):
        pass
    #     self._ontology.meta[element.tag] = {}
    #     self._ontology.meta[element.tag]['attrib'] = element.attrib
    #     self._ontology.meta[element.tag]['text'] = element.text
    #
    def _process_label(self, element, context):
        pass
    #     if not context['n0']:
    #         self._ontology.meta[element.tag] = {}
    #         self._ontology.meta[element.tag]['attrib'] = element.attrib
    #         self._ontology.meta[element.tag]['text'] = element.text

    def _process_restriction(self, element, context):
        restriction_node = self._ontology.create_node_if_needed()
        context['n1'] = restriction_node
        self._ontology.create_relation("has_restriction", context['n0'], restriction_node)

    def _process_cardinality(self, element, context):
        datatype = element.attrib.get('{http://www.w3.org/1999/02/22-rdf-syntax-ns#}datatype')
        value = element.text
        cardinality_instance_node = self._ontology.create_node_if_needed()
        datatype_instance_node = self._ontology.create_node_if_needed(datatype)
        value_instance_node = self._ontology.create_node_if_needed(value)
        self._ontology.create_relation('has_cardinality', context['n0'], cardinality_instance_node)
        self._ontology.create_relation('has_datatype', cardinality_instance_node, datatype_instance_node)
        self._ontology.create_relation('has_value', cardinality_instance_node, value_instance_node)

    def _process_min_cardinality(self, element, context):
        datatype = element.attrib.get('{http://www.w3.org/1999/02/22-rdf-syntax-ns#}datatype')
        value = element.text
        cardinality_instance_node = self._ontology.create_node_if_needed()
        datatype_instance_node = self._ontology.create_node_if_needed(datatype)
        value_instance_node = self._ontology.create_node_if_needed(value)
        self._ontology.create_relation('has_min_cardinality', context['n0'], cardinality_instance_node)
        self._ontology.create_relation('has_datatype', cardinality_instance_node, datatype_instance_node)
        self._ontology.create_relation('has_value', cardinality_instance_node, value_instance_node)

    def _process_max_cardinality(self, element, context):
        datatype = element.attrib.get('{http://www.w3.org/1999/02/22-rdf-syntax-ns#}datatype')
        value = element.text
        cardinality_instance_node = self._ontology.create_node_if_needed()
        datatype_instance_node = self._ontology.create_node_if_needed(datatype)
        value_instance_node = self._ontology.create_node_if_needed(value)
        self._ontology.create_relation('has_max_cardinality', context['n0'], cardinality_instance_node)
        self._ontology.create_relation('has_datatype', cardinality_instance_node, datatype_instance_node)
        self._ontology.create_relation('has_value', cardinality_instance_node, value_instance_node)

    def _process_on_property(self, element, context):
        relation = OntologyRelation()
        relation.name = 'onProperty'
        context['r'] = relation
        property_id = element.attrib.get('{http://www.w3.org/1999/02/22-rdf-syntax-ns#}resource')
        if property_id:
            property_node = self._ontology.create_node_if_needed(property_id)
            self._ontology.create_relation(relation.name, context['n0'], property_node)

    def _process_object_property(self, element, context):
        property_id = element.attrib.get('{http://www.w3.org/1999/02/22-rdf-syntax-ns#}ID')
        if not property_id:
            property_id = element.attrib.get('{http://www.w3.org/1999/02/22-rdf-syntax-ns#}about')
        property_node = self._ontology.create_node_if_needed(element.tag)
        property_instance_node = self._ontology.create_node_if_needed(property_id)
        self._ontology.create_relation('is_instance', property_instance_node, property_node)
        if context.get('r'):
            relation = context['r']
            source_node = context['n0']
            ontology_relation = self._ontology.create_relation(relation.name, source_node, property_instance_node)
            ontology_relation.meta = relation.meta
        else:
            context['n0'] = property_instance_node

    def _process_some_values_from(self, element, context):
        relation = OntologyRelation()
        relation.name = 'someValuesFrom'
        context['r'] = relation
        from_id = element.attrib.get('{http://www.w3.org/1999/02/22-rdf-syntax-ns#}resource')
        if from_id:
            from_node = self._ontology.create_node_if_needed(from_id)
            self._ontology.create_relation(relation.name, context['n0'], from_node)

    def _process_disjoint_with(self, element, context):
        relation = OntologyRelation()
        relation.name = 'disjointWith'
        context['r'] = relation
        from_id = element.attrib.get('{http://www.w3.org/1999/02/22-rdf-syntax-ns#}resource')
        if from_id:
            from_node = self._ontology.create_node_if_needed(from_id)
            self._ontology.create_relation(relation.name, context['n0'], from_node)

    def _process_inverse_of(self, element, context):
        relation = OntologyRelation()
        relation.name = 'inverseOf'
        context['r'] = relation
        from_id = element.attrib.get('{http://www.w3.org/1999/02/22-rdf-syntax-ns#}resource')
        if from_id:
            from_node = self._ontology.create_node_if_needed(from_id)
            self._ontology.create_relation(relation.name, context['n0'], from_node)

    def _process_type(self, element, context):
        type_id = element.attrib.get('{http://www.w3.org/1999/02/22-rdf-syntax-ns#}resource')
        type_node = self._ontology.create_node_if_needed(type_id)
        self._ontology.create_relation('type', context['n0'], type_node)

    def _process_has_value(self, element, context):
        value_id = element.attrib.get("{http://www.w3.org/1999/02/22-rdf-syntax-ns#}resource")
        value_node = self._ontology.create_node_if_needed(value_id)
        context['n1'] = value_node
        self._ontology.create_relation('has_value', context['n0'], value_node)


#-----------------------------------------------------------------


class ONTParser:
    def __init__(self, file):
        json_data = json.load(file)
        self._ontology = Ontology()
        self._ontology.from_json(json_data)
        self._bypass_sequence = self._node_sequence()
        self._xml_root = etree.fromstring(
            '<rdf:RDF xmlns:rdf="http://www.w3.org/1999/02/22-rdf-syntax-ns#" xmlns:rdfs="http://www.w3.org/2000/01/rdf-schema#" xmlns:owl="http://www.w3.org/2002/07/owl#" xml:base="http://cidoc.ics.forth.gr/rdfs/cidoc_crm_v3.4.9.rdfs" xml:lang="en"/>')
        self._top_element = self._xml_root
        self._put_owl()
        self._sudden_root = None
        self._stop_flag = False
        self._handlers = {
            'is_instance': self._process_is_instance,
            'has_domain': self._process_has_domain,
            'has_range': self._process_has_range,
            'subClassOf': self._process_sub_class_of,
            'subPropertyOf': self._process_sub_property_of,
            'has_max_cardinality': self._process_has_max_cardinality,
            'type': self._process_type,
            'has_datatype': self._process_has_datatype,
            'has_value': self._process_has_value,
            'has_cardinality': self._process_has_cardinality,
            'has_min_cardinality': self._process_has_min_cardinality,
            'has_restriction': self._process_has_restriction,
            'onProperty': self._process_on_property,
            'minCardinality': self._pass,
            'maxCardinality': self._pass,
            'inverseOf': self._process_inverse_of,
            'allValuesFrom': self._pass,
            'someValuesFrom': self._process_some_values_from,
            'cardinality': self._pass,
            'hasValue': self._pass,
            'equivalentClass': self._pass,
            'sameAs': self._pass,
            'differentFrom': self._pass,
            'AllDifferent': self._pass,
            'distinctMembers': self._pass,
            'intersectionOf': self._pass,
            'unionOf': self._pass,
            'complementOf': self._pass,
            'oneOf': self._pass,
            'disjointWith': self._process_disjoint_with,
            'comment': self._pass
        }
        print(self._bypass_sequence)

    def _put_owl(self):
        if "Ontology" in self._ontology.meta:
            element = etree.fromstring(self._ontology.meta["Ontology"])
            self._xml_root.append(element)
            self._xml_root = element

    def parse_to_owl(self):
        while len(self._bypass_sequence):
            from_node = next(x for x in self._ontology.nodes if x.id == self._bypass_sequence[0])
            self._bypass(from_node, [], self._xml_root)
        self._print_pretty_xml()

    def _print_pretty_xml(self):
        xml_string = etree.tostring(self._top_element, encoding='UTF-8')
        reparsed_xml = xml.dom.minidom.parseString(xml_string)
        print(reparsed_xml.toprettyxml(indent='\t'))

    def _node_sequence(self):
        relation_counts = {}
        for relation in self._ontology.relations:
            if relation.source_node.id in relation_counts:
                relation_counts[relation.source_node.id] += 1
            else:
                relation_counts[relation.source_node.id] = 0

        sorted_tuples = sorted(relation_counts.items(), key=lambda x: x[1], reverse=True)
        sorted_ids = list(x[0] for x in sorted_tuples)
        return sorted_ids

    @staticmethod
    def _is_node_in_context(context, node):
        return next((x for x in context if x.source_node.id == node.id or x.destination_node.id == node.id),
                    None) is not None

    @staticmethod
    def _is_relation_in_context(context, relation):
        return next((x for x in context if x.id == relation.id), None) is not None

    @staticmethod
    def _arrange_relations(relations):
        is_instance_index = -1
        for relation in relations:
            is_instance_index += 1
            if relation.name == "is_instance":
                if is_instance_index > 0:
                    relations[0], relations[is_instance_index] = relations[is_instance_index], relations[0]
                break
        return relations

    def _bypass(self, from_node, context, last_xml_element):
        # print(etree.tostring(last_xml_element, encoding='UTF-8'))
        if context:
            last_xml_element = self._process_relation(context, last_xml_element)

        if from_node.id in self._bypass_sequence:
            self._bypass_sequence.remove(from_node.id)

        relations = self._ontology.get_relations_with_source_node(from_node)
        relations = self._arrange_relations(relations)

        for relation in relations:
            next_context = copy.copy(context)
            if not self._is_node_in_context(context, relation.destination_node):
                next_context.append(relation)
                self._bypass(relation.destination_node, next_context, last_xml_element)

            if self._sudden_root is not None:
                # print("<sudden>")
                last_xml_element = self._sudden_root
                self._sudden_root = None

            if self._stop_flag:
                # print("<stop>")
                self._stop_flag = False
                break

    def _find_xml_element_by_id(self, tag, id):
        return next((x for x in self._xml_root if x.tag == tag and x.attrib["{http://www.w3.org/1999/02/22-rdf-syntax-ns#}ID"] == id), None)

    def _find_xml_element_by_about(self, tag, id):
        return next((x for x in self._xml_root if x.tag == tag and x.attrib["{http://www.w3.org/1999/02/22-rdf-syntax-ns#}about"] == id), None)

    def _process_relation(self, context, last_xml_element):
        last_relation = context[len(context) - 1]

        # print(last_relation)

        if last_relation.name in self._handlers:
            element = self._handlers[last_relation.name](context, last_xml_element)
            if element is not None:
                last_xml_element = element
        else:
            self._pass(context, last_xml_element)
        return last_xml_element

    @staticmethod
    def _add_comment(context, last_xml_element):
        last_relation = context[len(context) - 1]
        if "comment" in last_relation.source_node.meta:
            element = etree.Element("{http://www.w3.org/2000/01/rdf-schema#}comment",
                                {"{http://www.w3.org/1999/02/22-rdf-syntax-ns#}datatype": "http://www.w3.org/2001/XMLSchema#string"})
            element.text = last_relation.source_node.meta["comment"]
            last_xml_element.append(element)

    def _process_is_instance(self, context, last_xml_element):
        last_relation = context[len(context) - 1]
        if last_relation.destination_node.name == "{http://www.w3.org/1999/02/22-rdf-syntax-ns#}Property":
            element = self._find_xml_element_by_id(last_relation.destination_node.name, last_relation.source_node.name)
            if element is None:
                element = etree.Element(last_relation.destination_node.name,
                                        {"{http://www.w3.org/1999/02/22-rdf-syntax-ns#}ID": last_relation.source_node.name})
                self._add_comment(context, element)
                self._xml_root.append(element)
                self._sudden_root = element
            else:
                self._stop_flag = True
            return element
        elif last_relation.destination_node.name == "{http://www.w3.org/2000/01/rdf-schema#}Class":
            element = self._find_xml_element_by_id(last_relation.destination_node.name, last_relation.source_node.name)
            if element is None:
                element = etree.Element(last_relation.destination_node.name,
                                        {"{http://www.w3.org/1999/02/22-rdf-syntax-ns#}ID": last_relation.source_node.name})
                self._add_comment(context, element)
                self._xml_root.append(element)
                self._sudden_root = element
            else:
                self._stop_flag = True
            return element
        elif last_relation.destination_node.name == "{http://www.w3.org/2002/07/owl#}Class":
            element = self._find_xml_element_by_about(last_relation.destination_node.name, last_relation.source_node.name)
            if element is None:
                element = etree.Element(last_relation.destination_node.name,
                                        {"{http://www.w3.org/1999/02/22-rdf-syntax-ns#}about": last_relation.source_node.name})
                self._add_comment(context, element)
                self._xml_root.append(element)
                self._sudden_root = element
            else:
                self._stop_flag = True
            return element
        elif last_relation.destination_node.name == "{http://www.w3.org/2002/07/owl#}ObjectProperty":
            element = self._find_xml_element_by_about(last_relation.destination_node.name, last_relation.source_node.name)
            if element is None:
                element = etree.Element(last_relation.destination_node.name,
                                        {"{http://www.w3.org/1999/02/22-rdf-syntax-ns#}about": last_relation.source_node.name})
                self._add_comment(context, element)
                self._xml_root.append(element)
                self._sudden_root = element
            else:
                self._stop_flag = True
            return element
        return None

    @staticmethod
    def _process_has_domain(context, last_xml_element):
        last_relation = context[len(context) - 1]
        element = etree.Element("{http://www.w3.org/2000/01/rdf-schema#}domain",
                                {"{http://www.w3.org/1999/02/22-rdf-syntax-ns#}resource": '#' + last_relation.destination_node.name})
        last_xml_element.append(element)
        return element

    @staticmethod
    def _process_has_range(context, last_xml_element):
        last_relation = context[len(context) - 1]
        element = etree.Element("{http://www.w3.org/2000/01/rdf-schema#}range",
                                {"{http://www.w3.org/1999/02/22-rdf-syntax-ns#}resource": '#' + last_relation.destination_node.name})
        last_xml_element.append(element)
        return element

    def _process_sub_class_of(self, context, last_xml_element):
        last_relation = context[len(context) - 1]
        element = etree.Element("{http://www.w3.org/2000/01/rdf-schema#}subClassOf",
                                {"{http://www.w3.org/1999/02/22-rdf-syntax-ns#}resource": '#' + last_relation.destination_node.name})
        last_xml_element.append(element)
        return element

    def _process_has_restriction(self, context, last_xml_element):
        last_relation = context[len(context) - 1]
        parent_element = etree.Element("{http://www.w3.org/2000/01/rdf-schema#}subClassOf")
        element = etree.Element("{http://www.w3.org/2002/07/owl#}Restriction")
        last_xml_element.append(parent_element)
        parent_element.append(element)
        return element

    @staticmethod
    def _process_sub_property_of(context, last_xml_element):
        last_relation = context[len(context) - 1]
        element = etree.Element("{http://www.w3.org/2000/01/rdf-schema#}subPropertyOf",
                                {"{http://www.w3.org/1999/02/22-rdf-syntax-ns#}resource": '#' + last_relation.destination_node.name})
        last_xml_element.append(element)
        return element

    @staticmethod
    def _process_disjoint_with(context, last_xml_element):
        last_relation = context[len(context) - 1]
        element = etree.Element("{http://www.w3.org/2002/07/owl#}disjointWith",
                                {"{http://www.w3.org/1999/02/22-rdf-syntax-ns#}resource": '#' + last_relation.destination_node.name})
        last_xml_element.append(element)
        return element

    @staticmethod
    def _process_inverse_of(context, last_xml_element):
        last_relation = context[len(context) - 1]
        element = etree.Element("{http://www.w3.org/2002/07/owl#}inverseOf",
                                {"{http://www.w3.org/1999/02/22-rdf-syntax-ns#}resource": '#' + last_relation.destination_node.name})
        last_xml_element.append(element)
        return element

    @staticmethod
    def _process_type(context, last_xml_element):
        last_relation = context[len(context) - 1]
        element = etree.Element("{http://www.w3.org/1999/02/22-rdf-syntax-ns#}type",
                                {"{http://www.w3.org/1999/02/22-rdf-syntax-ns#}resource": '#' + last_relation.destination_node.name})
        last_xml_element.append(element)
        return element

    @staticmethod
    def _process_on_property(context, last_xml_element):
        last_relation = context[len(context) - 1]
        element = etree.Element("{http://www.w3.org/2002/07/owl#}onProperty",
                                {"{http://www.w3.org/1999/02/22-rdf-syntax-ns#}resource": '#' + last_relation.destination_node.name})
        last_xml_element.append(element)
        return element

    @staticmethod
    def _process_some_values_from(context, last_xml_element):
        last_relation = context[len(context) - 1]
        element = etree.Element("{http://www.w3.org/2002/07/owl#}someValuesFrom",
                                {"{http://www.w3.org/1999/02/22-rdf-syntax-ns#}resource": '#' + last_relation.destination_node.name})
        last_xml_element.append(element)
        return element

    @staticmethod
    def _process_has_cardinality(context, last_xml_element):
        return last_xml_element

    @staticmethod
    def _process_has_max_cardinality(context, last_xml_element):
        return last_xml_element

    @staticmethod
    def _process_has_min_cardinality(context, last_xml_element):
        return last_xml_element

    def _process_has_datatype(self, context, last_xml_element):
        last_relation = context[len(context) - 1]
        pre_last_relation = context[len(context) - 2]
        tag = "cardinality"
        if pre_last_relation.name == "has_max_cardinality":
            tag = "maxCardinality"
        elif pre_last_relation.name == "has_min_cardinality":
            tag = "minCardinality"

        element = etree.Element("{http://www.w3.org/2002/07/owl#}" + tag,
                                {"{http://www.w3.org/1999/02/22-rdf-syntax-ns#}datatype": '#' + last_relation.destination_node.name})
        last_xml_element.append(element)
        self._sudden_root = element
        return element

    @staticmethod
    def _process_has_value(context, last_xml_element):
        last_relation = context[len(context) - 1]
        last_xml_element.text = last_relation.destination_node.name
        return last_xml_element

    def _pass(self, context, last_xml_element):
        last_relation = context[len(context) - 1]
        source_element = self._find_xml_element_by_id("{http://www.w3.org/2002/07/owl#}Class", last_relation.source_node.name)
        if source_element is None:
            source_element = etree.Element("{http://www.w3.org/2002/07/owl#}Class",
                                           {"{http://www.w3.org/1999/02/22-rdf-syntax-ns#}ID": last_relation.source_node.name})

        destination_element = self._find_xml_element_by_id("{http://www.w3.org/2002/07/owl#}Class", last_relation.destination_node.name)
        if destination_element is None:
            destination_element = etree.Element("{http://www.w3.org/2002/07/owl#}Class",
                                                {"{http://www.w3.org/1999/02/22-rdf-syntax-ns#}ID": last_relation.destination_node.name})

        property_element = self._find_xml_element_by_id("{http://www.w3.org/2002/07/owl#}ObjectProperty", last_relation.name)
        if property_element is None:
            property_element = etree.Element("{http://www.w3.org/2002/07/owl#}ObjectProperty",
                                     {"{http://www.w3.org/1999/02/22-rdf-syntax-ns#}ID": last_relation.name})

        sub_class_element = etree.Element("{http://www.w3.org/2000/01/rdf-schema#}subClassOf")
        restriction_element = etree.Element("{http://www.w3.org/2002/07/owl#}Restriction")
        on_property_elemnt = etree.Element("{http://www.w3.org/2002/07/owl#}onProperty",
                                           {"{http://www.w3.org/1999/02/22-rdf-syntax-ns#}resource": last_relation.name})
        has_value_element = etree.Element("{http://www.w3.org/2002/07/owl#}hasValue",
                                          {"{http://www.w3.org/1999/02/22-rdf-syntax-ns#}resource": "#" + last_relation.destination_node.name})
        restriction_element.append(on_property_elemnt)
        restriction_element.append(has_value_element)
        sub_class_element.append(restriction_element)
        source_element.append(sub_class_element)
        last_xml_element.append(source_element)
        last_xml_element.append(destination_element)
        last_xml_element.append(property_element)
        return last_xml_element


#-----------------------------------------------------------------


def print_supported_extensions():
    return '.owl'


def convert_to_internal_format(file_path):
    parser = OWLParser(file(file_path))
    parser.parse_to_ont()
    return ''


def convert_to_external_format(file_path):
    parser = ONTParser(file(file_path))
    parser.parse_to_owl()
    return ''


if __name__ == '__main__':
    opts, extraparams = getopt.getopt(sys.argv[1:], '', ['method=', 'source-path='])

    method_name = (item[1] for item in opts if item[0] == '--method').next()
    if method_name == 'supported_extensions':
        print_supported_extensions()
    elif method_name == 'import':
        path = (item[1] for item in opts if item[0] == '--source-path').next()
        convert_to_internal_format(path)
    elif method_name == 'export':
        path = (item[1] for item in opts if item[0] == '--source-path').next()
        convert_to_external_format(path)
    else:
        print "Unknown method name " + method_name
