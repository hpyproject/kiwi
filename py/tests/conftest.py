import pytest
from hpy.debug.pytest import LeakDetector

# This fixtue has effect only if the tests are run in HPy debug mode
# Otherwise it is benign

@pytest.fixture(autouse=True)
def auto_hpy_debug(request):
    with LeakDetector() as ld:
        yield ld
