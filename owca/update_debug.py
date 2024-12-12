import os,sys

files = [q for q in os.listdir('.') if q.endswith(('.h','.cpp'))]
defines = {}

def load_debug_directive(fl):
	prefix = '#define RCDEBUG_'
	size = len('#define ')
	with open(fl,'rb') as input:
		lines = [q.rstrip() for q in input.readlines()]
	for l in lines:
		l = l.lstrip()
		if l.startswith(prefix):
			define = l[size:]
			defines[define] = fl
			break

def find_defines_in_lines(lines):
	todo = defines_set
	for l in lines:
		for f in todo:
			if l.find(f) >= 0:
				todo -= frozenset([f])
	return defines_set - todo

def remove_old_includes(lines):
	lines = [q for q in lines if q.find('#include "debug_') < 0]
	return lines
	
def add_new_includes(lines,defs):
	first_index = 0
	if lines:
		if lines[0].startswith('#ifndef '):
			first_index = 3
			assert not lines[2],lines[2]
		else:
			if lines[0].startswith('#include "stdafx.h"'):
				first_index = 1
	new = ['#include "%s"' % defines[q] for q in sorted(defs)]
	lines = lines[:first_index] + new + lines[first_index:]
	return lines
	
def update_file_with_directives(fl):
	with open(fl,'rb') as input:
		lines = [q.decode().rstrip() for q in input.readlines()]
	
	defs = find_defines_in_lines(lines)
	lines = remove_old_includes(lines)
	lines = add_new_includes(lines,defs)
	
	with open(fl,'wb') as output:
		for l in lines:
			output.write(l.encode())
			output.write(b'\r\n')
			
for f in files:
	if f.endswith('.h') and f.startswith('debug_'):
		load_debug_directive(f)

defines_set = frozenset(defines)
for d in sorted(defines_set):
	print('found ' + d)
	
for f in files:
	if not f.startswith('debug_') and f != 'stdafx.h':
		update_file_with_directives(f)