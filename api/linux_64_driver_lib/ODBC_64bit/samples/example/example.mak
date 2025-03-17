#
# Copyright (c) 2002-2022 Progress Software Corporation and/or its subsidiaries or affiliates. All rights reserved.
#
# Purpose:	Make file to build example application 
#
# Requirements:
#
# Shared Library Path Variable:
#	Environment variable should point to the lib directory
#	for this release. This will enable the linker to resolve the
#	location of the various shared libraries and the run-time linker to
#	resolve the location of run-time components.
#
# Note that you must set the make file variable CC in order to build this
# example program. Uncomment one of the platform specific lines in this
# make file.
# 
# To Build example, issue the following command:
#	%make -f example.mak
#
#
############################################################################

# For Linux
CC=g++
LIBC=-lc
CCFLAGS=-g

#
# Compiler options
#
# For 64-bit
DEFS=-DODBCVER=0x0350 -DODBC64
INCLUDE=-I../include -I../../include

#
# Definition for standard system linker
#
LD=ld

#
# Define Support Libraries used.
#
# NOTE:  The odbc library depends on the icu library, but this won't be located by a
# regular compile-time search path entry via '-L'.  This is normally addressed via
# specifying the LD_LIBRARY_PATH for runtime calls to a driver library.  However, this
# `example` program is compiling as an executable and needs this info now.  To handle this,
# we use 'rpath-link' to add the lib directory locations to the runtime search path
LIBPATH=-L../lib -L../../lib -Wl,-rpath-link=../lib:../../lib

# For Linux
LIBS=$(LIBPATH) -lodbc -lodbcinst

#
# Application that shows a single C source code connecting to different
# vendor databases by simply changing the passed in DSN.
#
#
# Sample program using C Compiler
#
example:	example.c
	$(CC) -o $@ $(DEFS) $(CCFLAGS) $(INCLUDE) example.c $(LIBS) $(LIBC)

clean:
	/bin/rm example
#
# End of Makefile
#
