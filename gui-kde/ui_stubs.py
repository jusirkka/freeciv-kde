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
            props = {}
            for p in e:
                if p.get('name') == 'checkable':
                    props[p.get('name')] = p[0].text
                elif p.get('name') == 'checked':
                    props[p.get('name')] = p[0].text
                elif p.get('name') == 'enabled':
                    props[p.get('name')] = p[0].text
                elif p.get('name') == 'text':
                    props[p.get('name')] = p[0].text
                elif p.get('name') == 'shortcut':
                    props[p.get('name')] = p[0].text
                elif p.get('name') == 'toolTip':
                    props[p.get('name')] = p[0].text
                elif p.get('name') == 'icon':
                    props[p.get('name')] = p[0].get('theme')
            self.tree[e.get('name')] = props
    
        self.actions = {}
        self.menus = []
        sel = CSSSelector('widget[class="QMenuBar"] > widget[class="QMenu"]')
        tree = etree.parse(fname)
        for menu in sel(tree):
            menuActions = []
            name = ""
            for p in menu:
                if p.tag == 'property' and p.get('name') == 'title':
                    name = p[0].text
                    self.menus.append(name)
                elif p.tag == 'addaction':
                    menuActions.append(p.get('name'))
            self.actions[name] = menuActions


    def print_header_template(self):
        for k, v in self.tree.items():
            if 'checkable' in v and v['checkable']:
                print('void on_{}_toggled(bool on);'.format(k))
            else:
                print('void on_{}_triggered();'.format(k))
        
    def print_source_template(self):
        for k, v in self.tree.items():
            if 'checkable' in v and v['checkable']:
                print('void MainWindow::on_{}_toggled(bool on) {{}}'.format(k))
            else:
                print('void MainWindow::on_{}_triggered() {{}}'.format(k))

    def print_actions_template(self):
        for k, v in self.tree.items():
            # name, title, shortcut, icon, enabled, checkable, checked
            fmt = '"{}"'
            data = [fmt.format(k), 
                    fmt.format(v.get('text', '')), 
                    fmt.format(v.get('shortcut', '')), 
                    fmt.format(v.get('icon', '')),
                    fmt.format(v.get('toolTip', '')),
                    v.get('enabled', 'true'),
                    v.get('checkable', 'false'), 
                    v.get('checked', 'false')]
            print('{{{}}},'.format(', '.join(data)))

#    <Menu name="file" >
#      <Action name="clearAction" />
#    </Menu>

    def print_menus_template(self):
        for k in self.menus:
            print('    <Menu name="{}">'.format(k))
            for a in self.actions[k]:
                print('      <Action name="{}"/>'.format(a))
            print('    </Menu>')


from sys import argv
from argparse import ArgumentParser as AP


if __name__ == '__main__':
    parser = AP()
    parser.add_argument('-c', action='store_true')
    parser.add_argument('-f', type=str)
    parser.add_argument('-a', action='store_true')
    parser.add_argument('-m', action='store_true')
    options = parser.parse_args(argv[1:])

    p = Parser(options.f)
    if options.c:
        p.print_source_template()
    elif options.a:
        p.print_actions_template()
    elif options.m:
        p.print_menus_template()
    else:
        p.print_header_template()

