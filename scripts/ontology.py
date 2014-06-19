import json
import random


class Ontology:
    def __init__(self, json=None):
        self.ontology_triplets = []
        self.nodes = {}
        self.relations = []
        self.next_object_id = 1
        self.logs = []

        if json:
            self.next_object_id = json['last_id']
            for node in json['nodes']:
                self.nodes[node['name']] = node

            for relation in json['relations']:
                source_node = next(x for x in self.nodes.values() if x['id'] == relation['source_node_id'])
                destination_node = next(x for x in self.nodes.values() if x['id'] == relation['destination_node_id'])
                self.relations.append(relation)
                self.ontology_triplets.append((source_node, relation, destination_node))

    def generate_json(self):
        json_data = dict()
        json_data['last_id'] = self.next_object_id
        json_data['nodes'] = self.nodes.values()
        json_data['relations'] = self.relations
        return json.dumps(json_data)

    def create_node_if_not_exists(self, node_name=None):
        if not node_name:
            node_name = 'unnamed_' + str(self.next_object_id)

        if node_name not in self.nodes.keys():
            node = {'id': self.next_object_id, 'name': node_name, 'position_x': random.uniform(0, 20000), 'position_y': random.uniform(0, 20000)}
            self.next_object_id += 1
            self.nodes[node_name] = node
            self.logs.append('Node created: ' + node_name)
            return node
        #self.logs.append('Can\'t create node: ' + node_name)
        return None

    def create_relation_if_not_exists(self, source_node_name, destination_node_name, relation_name):
        source_node = self.nodes[source_node_name]
        destination_node = self.nodes[destination_node_name]
        if not self.find_relation(source_node, destination_node):
            relation = {'id': self.next_object_id, 'name': relation_name, 'source_node_id': source_node['id'], 'destination_node_id':destination_node['id']}
            self.next_object_id += 1
            self.relations.append(relation)
            self.ontology_triplets.append((source_node, relation, destination_node))
            self.logs.append('Relation `' + relation_name + '` between ' + source_node_name + ' and ' + destination_node_name + ' created')
            return relation
        #self.logs.append('Can\'t create relation `' + relation_name + '` between ' + source_node_name + ' and ' + destination_node_name)
        return None

    def get_node_relations(self, node):
        return filter(lambda x: x['source_node_id'] == node['id'] or x['destination_node_id'] == node['id'], self.relations)

    def find_relation(self, source_node, destination_node):
        return next((triplet[1] for triplet in self.ontology_triplets if source_node in triplet and destination_node in triplet), None)

    def calculate_relation_weight(self, source_node, destination_node):
        if next((triplet for triplet in self.ontology_triplets if triplet[0] == source_node and triplet[2] == destination_node), None):
            return 100
        else:
            return 1

    def calculate_path_weight(self, path):
        weight = 0
        for i in range(0, len(path) - 1, 1):
            source_node = path[i]
            destination_node = path[i + 1]
            weight += self.calculate_relation_weight(source_node, destination_node)
        return weight

    def find_path(self, from_node, to_node, visited_nodes=None):
        visited_nodes = visited_nodes if visited_nodes else [from_node]

        if from_node is not to_node:
            related_nodes = filter(lambda x: x not in visited_nodes and self.find_relation(from_node, x), self.nodes.values())
            best_path = None
            for related_node in related_nodes:
                path = self.find_path(related_node, to_node, visited_nodes + [related_node])
                if path:
                    if best_path:
                        best_path = best_path if self.calculate_path_weight(best_path) <= self.calculate_path_weight(path) else path
                    else:
                        best_path = path
            return best_path
        else:
            return visited_nodes

    def is_node_has_outcoming_relations(self, node):
        return next((relation for relation in self.relations if relation['source_node_id'] == node['id']), None)

    def find_top_node(self):
        return next((node for node in self.nodes.values() if not self.is_node_has_outcoming_relations(node)), None)