#! /usr/local/bin/python
# -*- coding: utf-8 -*-

import sys
import getopt

reload(sys)


def print_supported_extensions():
    return ''


def convert_to_internal_format(file_path):
    return ''


def convert_to_external_format(file_path):
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
