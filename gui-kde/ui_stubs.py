#!/usr/bin/env python
# -*- coding: utf-8 -*-
"""
Created on Sat Feb 23 08:04:59 2019

@author: jusirkka
"""

from lxml import etree
from lxml.cssselect import CSSSelector

class Parser(object):
    def __init__(self, fname):
        sel = CSSSelector('action')
        self.tree = {}
        tree = etree.parse(fname)
        for e in sel(tree):
            self.tree[e.get('name')] = False
            for p in e:
                if p.get('name') == 'checkable' and \
                   p.tag == 'property' and p[0].tag == 'bool' and p[0].text == 'true':
                       self.tree[e.get('name')] = True

    def print_header_template(self):
        for k, v in self.tree.items():
            if v:
                print('void on_{}_toggled(bool on);'.format(k))
            else:
                print('void on_{}_triggered();'.format(k))
        
    def print_source_template(self):
        for k, v in self.tree.items():
            if v:
                print('void MainWindow::on_{}_toggled(bool on) {{}}'.format(k))
            else:
                print('void MainWindow::on_{}_triggered() {{}}'.format(k))


from sys import argv
from argparse import ArgumentParser as AP


if __name__ == '__main__':
    parser = AP()
    parser.add_argument('-c', action='store_true')
    parser.add_argument('-f', type=str)
    options = parser.parse_args(argv[1:])

    p = Parser(options.f)
    if options.c:
        p.print_source_template()
    else:
        p.print_header_template()

