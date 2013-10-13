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
nextObjectId = 0
nodes = []
relations = []

def group(conceptDefinition):
	splitByDef = conceptDefinition.split('::=');
	splitByOr = splitByDef[1].split('|')
	return splitByDef[0].strip(' \n'), [item.strip(' \n') for item in splitByOr]

def tokenize(definition):
	return [next(token for token in item if token != '') for item in tokenRegExp.findall(definition)]

def findNode(name):
	return next((node for node in nodes if node['name'] == name), None)

def createNode(name):
	global nextObjectId
	global nodes
	node = {'id': nextObjectId, 'name': name, 'position_x': random.uniform(0, 20000), 'position_y': random.uniform(0, 20000)}
	nodes.append(node)
	nextObjectId += 1
	return node

def createRelation(name, sourceNode, destinationNode):
	global nextObjectId
	global relations
	relation = {'id': nextObjectId, 'name': name, 'source_node_id': sourceNode['id'], 'destination_node_id': destinationNode['id']}
	relations.append(relation)
	nextObjectId += 1
	return relation

def getNode(name):
	node = findNode(name)
	if node == None:
		node = createNode(name)
	return node

def processSubconcept(parentNode, subconcept):
	isOptional = len(subconcept) > 1 and subconcept.rfind('?') == len(subconcept) - 1
	subconcept = subconcept.strip('?')

	if nontermimalRegExp.match(subconcept) != None:
		subconceptNode = getNode(subconcept)
		if isOptional:
			createRelation('optional', subconceptNode, parentNode)
		else:
			createRelation('a_part_of', subconceptNode, parentNode)
	else:
		subconceptNontermitalNode = getNode('Nonterminal of ' + subconcept)
		if isOptional:
			createRelation('optional', subconceptNontermitalNode, parentNode)
		else:
			createRelation('a_part_of', subconceptNontermitalNode, parentNode)		
		subconceptNode = getNode(subconcept)
		createRelation('is_instance', subconceptNode, subconceptNontermitalNode)

def processDefinition(parentNode, definition):
	subconcepts = tokenize(definition)
	for subconcept in subconcepts:
		processSubconcept(parentNode, subconcept)

def processConcept(conceptDefinition):
	concept, definitions = group(conceptDefinition)
	conceptNode = getNode(concept)

	if len(definitions) > 1:
		definitionIndex = 0
		for definition in definitions:
			definitionNode = getNode(concept + ' [' + str(definitionIndex) + ']')
			definitionIndex += 1
			createRelation('is_a', definitionNode, conceptNode)
			processDefinition(definitionNode, definition)
	else:
		processDefinition(conceptNode, definitions[0])

#========================================================================================

def printSupportedExtensions():
	print '.bnf'

def convertToInternalFormat(path):
	global nextObjectId
	global nodes
	global relations

	bnfFile = open(path, 'r')
	for line in bnfFile:
		processConcept(line)
	print json.dumps({'last_id': nextObjectId - 1, 'nodes': nodes, 'relations': relations})

def convertToExternalFormat(path):
	print 'not implemented'

if __name__ == '__main__':
	opts, extraparams = getopt.getopt(sys.argv[1:], '', ['method=', 'source-path='])

	methodName = (item[1] for item in opts if item[0] == '--method').next()
	if methodName == 'supported_extensions':
		printSupportedExtensions()
	elif methodName == 'import':
		path = (item[1] for item in opts if item[0] == '--source-path').next()
		convertToInternalFormat(path)
	elif  methodName == 'export':
		path = (item[1] for item in opts if item[0] == '--source-path').next()
		convertToExternalFormat(path)
	else:
		print "Unknown method name " + methodName
