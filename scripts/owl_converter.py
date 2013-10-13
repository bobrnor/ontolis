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

nextObjectId = 0
nodes = []
relations = []

#================from .owl==============================

def creatRelation(name, firstClassName, secondClassName, attributes = {}):
	global nextObjectId
	global relations

	firstClassNode = findNodeByName(firstClassName)
	secondClassNode = findNodeByName(secondClassName)

	relation = {'id': nextObjectId, 'name': name, 'source_node_id': firstClassNode['id'], 'destination_node_id': secondClassNode['id']}
	if attributes != {}:
		relation['attributes'] = attributes
	relations.append(relation)
	nextObjectId = nextObjectId + 1

	return relation

def createNodeIfNeeded(name, attributes = {}):
	global nextObjectId
	global nodes

	node = findNodeByName(name)
	if node == None:
		node = {'id': nextObjectId, 'name': name, 'position_x': random.uniform(0, 2000), 'position_y': random.uniform(0, 2000)}
		if attributes != {}:
			node['attributes'] = attributes
		nextObjectId = nextObjectId + 1
		nodes.append(node)
	elif attributes != {}:
		node['attributes'] = attributes

	return node

def findNodeByName(nodeName):
	global nodes
	return next((node for node in nodes if node['name'] == nodeName), None)

def processSubClassOf(owlClass, subClassOfNode):
	if NSMAP['rdf'] + 'resource' in subClassOfNode.attrib:
		subClassNodeName = subClassOfNode.attrib[NSMAP['rdf'] + 'resource']
		if findNodeByName(subClassNodeName) == None:
			createNodeIfNeeded(subClassNodeName)
		creatRelation(NSMAP['rdfs'] + 'subClassOf', owlClass.attrib[NSMAP['rdf'] + 'ID'], subClassNodeName)
	else:
		restrictionNode = subClassOfNode.find(NSMAP['owl'] + 'Restriction')
		onPropertyNode = restrictionNode.find(NSMAP['owl'] + 'onProperty')
		hasValueNode = restrictionNode.find(NSMAP['owl'] + 'hasValue')
		someValuesFromNode = restrictionNode.find(NSMAP['owl'] + 'someValuesFrom')

		if hasValueNode != None:
			subClassNodeName = hasValueNode.attrib[NSMAP['rdf'] + 'resource'].strip('_#')
			if findNodeByName(subClassNodeName) == None:
				createNodeIfNeeded(subClassNodeName)
			creatRelation(onPropertyNode.attrib[NSMAP['rdf'] + 'resource'], owlClass.attrib[NSMAP['rdf'] + 'ID'], subClassNodeName, {'owl': {'owlRelationType': NSMAP['owl'] + 'hasValue'}})
		elif someValuesFromNode != None:
			subClassNodeName = someValuesFromNode.attrib[NSMAP['rdf'] + 'resource'].strip('_#')
			if findNodeByName(subClassNodeName) == None:
				createNodeIfNeeded(subClassNodeName)
			creatRelation(onPropertyNode.attrib[NSMAP['rdf'] + 'resource'], owlClass.attrib[NSMAP['rdf'] + 'ID'], subClassNodeName, {'owl': {'owlRelationType': NSMAP['owl'] + 'someValuesFrom'}})

def processClassAttributes(owlClass):
	attrs = {}
	for attrName in owlClass.attrib.keys():
		if attrName != NSMAP['rdf'] + 'ID':
			attrs[attrName] = owlClass.attrib[attrName]
	return attrs

def processClassContent(owlClass):
	unknownContent = []
	for childNode in owlClass.getchildren():
		if childNode.tag == NSMAP['rdfs'] + 'subClassOf':
			processSubClassOf(owlClass, childNode)
		else:
			unknownContent.append(etree.tostring(childNode, encoding='UTF-8'))

	return unknownContent

def processClass(owlClass):
	name = owlClass.attrib[NSMAP['rdf'] + 'ID']
	node = createNodeIfNeeded(name)
	classAttributes = processClassAttributes(owlClass)
	owlAttributes = processClassContent(owlClass)

	attributes = {}
	if classAttributes != {}:
		attributes['class-attribytes'] = classAttributes

	if owlAttributes != {}:
		attributes['owl'] = {'unknown-content': owlAttributes}

	if attributes != {}:
		node['attributes'] = attributes

#==================to .owl==============================

defaultRdfRoot = '<rdf:RDF xmlns:owl="http://www.w3.org/2002/07/owl#" xmlns:rdf="http://www.w3.org/1999/02/22-rdf-syntax-ns#" xmlns:rdfs="http://www.w3.org/2000/01/rdf-schema#" xmlns:xsd="http://www.w3.org/2001/XMLSchema#"></rdf:RDF>'
defaultOwlHeader = '<owl:Ontology rdf:about="Data Source=.;MultipleActiveResultSetrees=true;Initial Catalog=NightBase;Integrated Security=true"></owl:Ontology>'

def getNodeById(id):
	global nodes
	return next((node for node in nodes if node['id'] == id), None)

def fillClassAttributes(node, classElement):
	if 'attributes' in node:
		attributes = node['attributes']
		if attributes != None and 'class-attribytes' in attributes:
			classAttributes = attributes['class-attribytes']
		classElement.attrib[NSMAP['rdf'] + 'ID'] = node['name']

def fillOwlClassContent(node, classElement):
	if 'attributes' in node:
		attributes = node['attributes']
		if 'owl' in attributes and 'unknown-content' in attributes['owl']:
			owlContent = attributes['owl']['unknown-content']
			for owlTag in owlContent:
				parsedTag = etree.fromstring(owlTag)
				classElement.append(parsedTag)

def fillSubClassOfContent(classElement, relation):
	node = getNodeById(relation['destination_node_id'])
	element = etree.Element(relation['name'], {NSMAP['rdf'] + 'resource': node['name']})
	classElement.append(element)

def fillHasValueContent(classElement, relation):
	node = getNodeById(relation['destination_node_id'])
	restrictionElement = etree.Element(NSMAP['owl'] + 'Restriction')
	onPropertyElement = etree.Element(NSMAP['owl'] + 'onProperty', {NSMAP['rdf'] + 'resource': relation['name']})
	hasValueElement = etree.Element(NSMAP['owl'] + 'hasValue', {NSMAP['rdf'] + 'resource': '#' + node['name']})
	restrictionElement.append(onPropertyElement)
	restrictionElement.append(hasValueElement)
	classElement.append(restrictionElement)

def fillSomeValuesFromContent(classElement, relation):
	node = getNodeById(relation['destination_node_id'])
	restrictionElement = etree.Element(NSMAP['owl'] + 'Restriction')
	onPropertyElement = etree.Element(NSMAP['owl'] + 'onProperty', {NSMAP['rdf'] + 'resource': relation['name']})
	someValuesFromElement = etree.Element(NSMAP['owl'] + 'someValuesFrom', {NSMAP['rdf'] + 'resource': '#' + node['name']})
	restrictionElement.append(onPropertyElement)
	restrictionElement.append(someValuesFromElement)
	classElement.append(restrictionElement)

def fillClassContent(node, classElement):
	for relation in relations:
		if relation['source_node_id'] == node['id']:
			if relation['name'] == NSMAP['rdfs'] + 'subClassOf':
				fillSubClassOfContent(classElement, relation)
			elif 'attributes' in relation and 'owl' in relation['attributes'] and relation['attributes']['owl']['owlRelationType'] == NSMAP['owl'] + 'hasValue':
				fillHasValueContent(classElement, relation)
			elif 'attributes' in relation and 'owl' in relation['attributes'] and relation['attributes']['owl']['owlRelationType'] == NSMAP['owl'] + 'someValuesFrom':
				fillSomeValuesFromContent(classElement, relation)

def processNode(node):
	classElement = etree.Element(NSMAP['owl'] + 'Class')
	fillClassAttributes(node, classElement)
	fillClassContent(node, classElement)
	fillOwlClassContent(node, classElement)

	return classElement

#=======================================================

def printSupportedExtensions():
	print ".owl"

def convertToInternalFormat(path):
	global nextObjectId
	global nodes
	global relations

	xml = etree.parse(path)
	rdfRoot = xml.getroot()
	owlHeader = rdfRoot.find(NSMAP['owl'] + 'Ontology')

	emptyRdfRoot = deepcopy(rdfRoot)
	for child in emptyRdfRoot.getchildren():
		emptyRdfRoot.remove(child)

	owlClasses = rdfRoot.findall(NSMAP['owl'] + 'Class')
	for owlClass in owlClasses:
		processClass(owlClass)

	print json.dumps({'last_id': nextObjectId - 1, 'nodes': nodes, 'relations': relations, 'rdf': etree.tostring(emptyRdfRoot, encoding='UTF-8'), 'owl': etree.tostring(owlHeader, encoding='UTF-8')})

def convertToExternalFormat(path):
	global nextObjectId
	global nodes
	global relations

	jsonData = open(path)
	jsonObject = json.load(jsonData)
	nextObjectId = jsonObject['last_id']
	nodes = jsonObject['nodes']
	relations = jsonObject['relations']

	rdfRoot = None
	owlHeader = None

	if jsonObject['rdf'] != None:
		rdfRoot = etree.fromstring(jsonObject['rdf'])
	else:
		rdfRoot = etree.fromstring(defaultRdfRoot)

	if jsonObject['owl'] != None:
		owlHeader = etree.fromstring(jsonObject['owl'])
	else:
		owlHeader = etree.fromstring(defaultOwlHeader)

	rdfRoot.append(owlHeader)

	for node in nodes:
		classElement = processNode(node)
		rdfRoot.append(classElement)

	print etree.tostring(rdfRoot, encoding='UTF-8')

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
