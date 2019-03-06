#!/bin/python

from waflib import *
import os, sys

top = '.'
out = 'build'

g_cflags = ["-Wall", "-Wextra", "-std=c++17"]
def btype_cflags(ctx):
	return {
		"DEBUG"   : g_cflags + ["-Og", "-ggdb3", "-march=core2", "-mtune=native"],
		"NATIVE"  : g_cflags + ["-Ofast", "-march=native", "-mtune=native"],
		"RELEASE" : g_cflags + ["-O3", "-march=core2", "-mtune=generic"],
	}.get(ctx.env.BUILD_TYPE, g_cflags)

def options(opt):
	opt.load("g++")
	opt.add_option('--build_type', dest='build_type', type="string", default='RELEASE', action='store', help="DEBUG, NATIVE, RELEASE")

def configure(ctx):
	ctx.load("g++")
	ctx.check(features='c cprogram', lib='asterales', uselib_store='ASTERALES')
	ctx.check(features='c cprogram', lib='openal', uselib_store='OPENAL')
	btup = ctx.options.build_type.upper()
	if btup in ["DEBUG", "NATIVE", "RELEASE"]:
		Logs.pprint("PINK", "Setting up environment for known build type: " + btup)
		ctx.env.BUILD_TYPE = btup
		ctx.env.CXXFLAGS = btype_cflags(ctx)
		Logs.pprint("PINK", "CXXFLAGS: " + ' '.join(ctx.env.CXXFLAGS))
		if btup == "DEBUG":
			ctx.define("DEBUG", 1)
		else:
			ctx.define("NDEBUG", 1)
	else:
		Logs.error("UNKNOWN BUILD TYPE: " + btup)
		
def build(bld):
	
	bld_install_files = bld.path.ant_glob('src/lyrebird/*.hh')
	bld.install_files('${PREFIX}/include/lyrebird', bld_install_files)
	
	lyrebird_files = bld.path.ant_glob('src/*.cc')
	lyrebird = bld (
		features = "cxx cxxshlib",
		target = 'lyrebird',
		source = lyrebird_files,
		uselib = ['ASTERALES'],
		vnum = '0.0.1'
	)
	
	lyrebird_test_files = bld.path.ant_glob('test/*.cc')
	lyrebird_test = bld (
		features = "cxx cxxprogram",
		target = 'lyrebird_test',
		source = lyrebird_test_files,
		use = ['lyrebird'],
		uselib = ['OPENAL', 'ASTERALES']
	)
