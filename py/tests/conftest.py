#------------------------------------------------------------------------------
# Copyright (c) 2013-2020, Nucleic Development Team.
# Copyright (c) 2022-2023, Oracle and/or its affiliates.
#
# Distributed under the terms of the Modified BSD License.
#
# The full license is in the file LICENSE, distributed with this software.
#------------------------------------------------------------------------------
import pytest
from hpy.debug.pytest import LeakDetector

# This fixtue has effect only if the tests are run in HPy debug mode
# Otherwise it is benign

@pytest.fixture(autouse=True)
def auto_hpy_debug(request):
    with LeakDetector() as ld:
        yield ld
