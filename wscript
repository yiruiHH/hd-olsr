## -*- Mode: python; py-indent-offset: 4; indent-tabs-mode: nil; coding: utf-8; -*-

def build(bld):
    module = bld.create_ns3_module('hd-olsr', ['internet', 'mobility'])
    module.includes = '.'
    module.source = [
        'model/hd-olsr-header.cc',
        'model/hd-olsr-state.cc',
        'model/hd-olsr-routing-protocol.cc',
        'helper/hd-olsr-helper.cc',
        ]

    module_test = bld.create_ns3_module_test_library('hd-olsr')
    module_test.source = [
        
        ]

    headers = bld(features='ns3header')
    headers.module = 'hd-olsr'
    headers.source = [
        'model/hd-olsr-routing-protocol.h',
        'model/hd-olsr-header.h',
        'model/hd-olsr-state.h',
        'model/hd-olsr-repositories.h',
        'helper/hd-olsr-helper.h',
        ]


    if bld.env['ENABLE_EXAMPLES']:
        bld.recurse('examples')

    bld.ns3_python_bindings()
