#! /usr/local/bin/python
# -*- coding: utf-8 -*-

import sys
import getopt

reload(sys)
sys.setdefaultencoding('utf-8')


def print_supported_extensions():
    return ''


def convert_to_internal_format(file_path):
    return ''


def convert_to_external_format(file_path):
    return ''

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
