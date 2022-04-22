#------------------------------------------------------------------------------
# Copyright (c) 2013-2020, Nucleic Development Team.
# Copyright (c) 2022, Oracle and/or its affiliates.
#
# Distributed under the terms of the Modified BSD License.
#
# The full license is in the file LICENSE, distributed with this software.
#------------------------------------------------------------------------------
import os
import sys
from setuptools import setup, Extension
from setuptools.command.build_ext import build_ext

# Before releasing the version needs to be updated in:
# - setup.py
# - py/kiwisolver.cpp
# - kiwi/version.h
# - docs/source/conf.py

# Use the env var KIWI_DISABLE_FH4 to disable linking against VCRUNTIME140_1.dll

ext_modules = [
    Extension(
        'kiwisolver',
        ['py/kiwisolver.cpp',
         'py/constraint.cpp',
         'py/expression.cpp',
         'py/solver.cpp',
         'py/strength.cpp',
         'py/term.cpp',
         'py/variable.cpp'],
        include_dirs=['.'],
        language='c++',
    ),
]


setup(
    name='kiwisolver',
    version='1.3.2',
    author='The Nucleic Development Team',
    author_email='sccolbert@gmail.com',
    url='https://github.com/nucleic/kiwi',
    description='A fast implementation of the Cassowary constraint solver',
    long_description=open('README.rst').read(),
    license='BSD',
    classifiers=[
          # https://pypi.org/pypi?%3Aaction=list_classifiers
          'License :: OSI Approved :: BSD License',
          'Programming Language :: Python',
          'Programming Language :: Python :: 3',
          'Programming Language :: Python :: 3.7',
          'Programming Language :: Python :: 3.8',
          'Programming Language :: Python :: 3.9',
          'Programming Language :: Python :: 3.10',
          'Programming Language :: Python :: Implementation :: CPython',
          'Programming Language :: Python :: Implementation :: PyPy',
      ],
    python_requires='>=3.7',
    setup_requires=['hpy>0.0.2'],
    hpy_ext_modules=ext_modules,
)
