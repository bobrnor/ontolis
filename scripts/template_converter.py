#! /usr/local/bin/python
# -*- coding: utf-8 -*-

import sys
import getopt

reload(sys)
sys.setdefaultencoding('utf-8')

def printSupportedExtensions():
	return ''

def convertToInternalFormat(path):
	return ''	

def convertToExternalFormat(path):
	return ''

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
