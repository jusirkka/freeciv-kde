#!/usr/bin/env python
# -*- coding: utf-8 -*-
"""
Created on Tue Feb 26 06:06:00 2019

@author: jusirkka
"""

import re

interface_fname = '../freeciv/client/gui_interface.c'

def create_blacklist():
    f = open(interface_fname)
    names = []
    func_re = re.compile(r'.*\s+\**?(\w+)\([^)]*[),]\s*')
    for line in f.readlines():
        mo = func_re.match(line)
        if not mo:
            continue
        name = mo.group(1)
        names.append("'{}'".format(name))
    f.close()
    mod = \
"""
names = [
{}
]

def in_blacklist(name):
    return name in names


"""
    f = open('stub_blacklist.py', 'w')
    f.write(mod.format(',\n'.join(names)))
    f.close()
        

try:
    from stub_blacklist import in_blacklist
except ModuleNotFoundError:
    create_blacklist()
    from stub_blacklist import in_blacklist
    


class Parser(object):
    start_re = re.compile(r'\s*GUI_FUNC_PROTO\(([^)]+)')
    full_re = re.compile(r'\s*GUI_FUNC_PROTO\(([^)]+)\)\s*')
    end_re = re.compile(r'([^)]+)\)\s*')
    def __init__(self, fname):
        f = open(fname)
        self.funcs = {}
        self.key = fname.rpartition('/')[2].rpartition('_')[0]
        declaring = False
        decl = ''
        for line in f.readlines():
            if declaring:
                mo = Parser.end_re.match(line)
                if mo:
                    declaring = False
                    decl += mo.group(1)
                    self.add_to_funcs(decl)
                    decl = ''
                else:
                    decl += line
            else:
                mo = Parser.full_re.match(line)
                if mo:
                    self.add_to_funcs(mo.group(1))
                else:
                    mo = Parser.start_re.match(line)
                    if mo:
                        declaring = True
                        decl = mo.group(1)
                        
        f.close()

    def add_to_funcs(self, decl):
        parts = list(map(lambda x: x.strip(), decl.split(',')))
        func_name = parts[1]
        if in_blacklist(func_name):
            #print('{} already defined, skipping'.format(func_name))
            return
        self.funcs[func_name] = {'return_type': parts[0], 'parameters': parts[2:]}
            
    
    def print_stubs(self):
        print('extern "C" {')
        print('#include "{}_g.h"'.format(self.key))
        print('}')
        print('#include "logging.h"\n')
                  
        for func_name, data in self.funcs.items():
            params = ', '.join(data['parameters'])
            r_type = data['return_type']
            if params == 'void':
                print('{} {}() {{'.format(r_type, func_name))
            else:
                print('{} {}({}) {{'.format(r_type, func_name, params))
            print('  qCDebug(FC) << "{}";'.format(func_name))
            if r_type == 'bool':
                print('  return false;')
            elif r_type == 'int':
                print('  return 0;')
            elif r_type != 'void':
                print('  return X;')
            print('}\n')



from sys import argv
from argparse import ArgumentParser as AP


if __name__ == '__main__':
    parser = AP()
    parser.add_argument('-f', type=str)
    options = parser.parse_args(argv[1:])

    p = Parser(options.f)
    p.print_stubs()

