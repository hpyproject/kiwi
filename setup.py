# --------------------------------------------------------------------------------------
# Copyright (c) 2013-2022, Nucleic Development Team.
#
# Distributed under the terms of the Modified BSD License.
#
# The full license is in the file LICENSE, distributed with this software.
# --------------------------------------------------------------------------------------
import os

from setuptools import Extension, setup

try:
    from cppy import CppyBuildExt
except ImportError as e:
    raise RuntimeError(
        "Missing setup required dependencies: cppy. "
        "Installing through pip as recommended ensure one never hits this issue."
    ) from e

# Before releasing the version needs to be updated in kiwi/version.h, if the changes
# are not limited to the solver.

# Use the env var KIWI_DISABLE_FH4 to disable linking against VCRUNTIME140_1.dll

if "KIWI_DISABLE_FH4" in os.environ:
    os.environ.setdefault("CPPY_DISABLE_FH4", "1")

ext_modules = [
    Extension(
        "kiwisolver",
        ['py/kiwisolver.cpp',
         'py/constraint.cpp',
         'py/expression.cpp',
         'py/solver.cpp',
         'py/strength.cpp',
         'py/term.cpp',
         'py/variable.cpp'],
        include_dirs=["."],
        language="c++",
    ),
]


setup(
    hpy_ext_modules=ext_modules,
    cmdclass={"build_ext": CppyBuildExt},
)
