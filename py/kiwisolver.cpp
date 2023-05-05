/*-----------------------------------------------------------------------------
| Copyright (c) 2013-2019, Nucleic Development Team.
| Copyright (c) 2022-2023, Oracle and/or its affiliates.
|
| Distributed under the terms of the Modified BSD License.
|
| The full license is in the file LICENSE, distributed with this software.
|----------------------------------------------------------------------------*/
#include <kiwi/kiwi.h>
#include "types.h"

#define PY_KIWI_VERSION "1.3.2"

namespace
{


bool ready_types( HPyContext *ctx, HPy m )
{
    using namespace kiwisolver;
    if( !Variable::Ready( ctx, m ) )
    {
        return false;
    }
    if( !Term::Ready( ctx, m ) )
    {
        return false;
    }
    if( !Expression::Ready( ctx, m ) )
    {
        return false;
    }
    if( !Constraint::Ready( ctx, m ) )
    {
        return false;
    }
    if( !strength::Ready( ctx, m ) )
    {
        return false;
    }
    if( !Solver::Ready( ctx, m ) )
    {
        return false;
    }
    return true;
}

bool add_objects( HPyContext *ctx, HPy mod )
{
	using namespace kiwisolver;

    HPy kiwiversion = HPyUnicode_FromString( ctx, KIWI_VERSION );
    if( HPy_IsNull(kiwiversion) )
    {
        return false;
    }

    if (HPy_SetAttr_s(ctx, mod, "__kiwi_version__", kiwiversion)) {
        HPy_Close(ctx, kiwiversion);
		return false;
    }
    HPy_Close(ctx, kiwiversion);

    HPy pyversion = HPyUnicode_FromString( ctx, PY_KIWI_VERSION );
    if( HPy_IsNull(pyversion) )
    {
        return false;
    }

    if (HPy_SetAttr_s(ctx, mod, "__version__", pyversion)) {
        HPy_Close(ctx, pyversion);
		return false;
    }
    HPy_Close(ctx, pyversion);

	return true;
}


HPyDef_SLOT(kiwisolver_exec, HPy_mod_exec)
static int kiwisolver_exec_impl(HPyContext *ctx, HPy mod)
{
    if( !ready_types( ctx, mod ) )
    {
        return -1;
    }
    if( !kiwisolver::init_exceptions( ctx, mod ) )
    {
        return -1;
    }
    if( !add_objects( ctx, mod ) )
    {
        return -1;
    }


    return 0;
}

static HPyDef *defines[] = {
    &kiwisolver_exec,
    NULL
};

static HPyGlobal *globals[] = {
    &kiwisolver::DuplicateConstraint,
    &kiwisolver::UnsatisfiableConstraint,
    &kiwisolver::UnknownConstraint,
    &kiwisolver::DuplicateEditVariable,
    &kiwisolver::UnknownEditVariable,
    &kiwisolver::BadRequiredStrength,
    &kiwisolver::Term::TypeObject,
    &kiwisolver::Variable::TypeObject,
    &kiwisolver::strength::TypeObject,
    &kiwisolver::Expression::TypeObject,
    &kiwisolver::Constraint::TypeObject,
    &kiwisolver::Solver::TypeObject,
    NULL,
};


static HPyModuleDef moduledef = {
    .doc = "kiwisolver extension module",
    .size = 0,
    .defines = defines,
    .globals = globals,
};

}  // namespace

extern "C" {

HPy_MODINIT(kiwisolver, moduledef)

}
