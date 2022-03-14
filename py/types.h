/*-----------------------------------------------------------------------------
| Copyright (c) 2013-2019, Nucleic Development Team.
|
| Distributed under the terms of the Modified BSD License.
|
| The full license is in the file LICENSE, distributed with this software.
|----------------------------------------------------------------------------*/
#pragma once
#include "hpy.h"
#include <kiwi/kiwi.h>

#define HPyFloat_Check( ctx, obj ) HPy_TypeCheck( ctx, obj, ctx->h_FloatType )
#define HPyLong_Check( ctx, obj ) HPy_TypeCheck( ctx, obj, ctx->h_LongType )
#define HPy_RETURN_NOTIMPLEMENTED( ctx ) return HPy_Dup(ctx, ctx->h_NotImplemented)

namespace kiwisolver
{

extern HPy DuplicateConstraint;

extern HPy UnsatisfiableConstraint;

extern HPy UnknownConstraint;

extern HPy DuplicateEditVariable;

extern HPy UnknownEditVariable;

extern HPy BadRequiredStrength;


struct strength
{
    static HPyType_Spec TypeObject_Spec;

    static HPy TypeObject;

	static bool Ready( HPyContext *ctx, HPy m );
};


struct Variable
{
	HPyField context;
	kiwi::Variable variable;

    static HPyType_Spec TypeObject_Spec;

    static HPy TypeObject;

	static bool Ready( HPyContext *ctx, HPy m );

	static bool TypeCheck( HPyContext *ctx, HPy obj )
	{
		return HPy_TypeCheck( ctx, obj, TypeObject ) != 0;
	}

	static Variable* AsStruct(HPyContext *ctx, HPy obj) {
		return (Variable*)HPy_AsStruct(ctx, obj);
	}
};

HPyType_HELPERS(Variable)

struct Term
{
	HPy variable;
	double coefficient;

    static HPyType_Spec TypeObject_Spec;

    static HPy TypeObject;

	static bool Ready( HPyContext *ctx, HPy m );

	static bool TypeCheck( HPyContext *ctx, HPy obj )
	{
		return HPy_TypeCheck( ctx, obj, TypeObject ) != 0;
	}

	static Term* AsStruct(HPyContext *ctx, HPy obj) {
		return (Term*)HPy_AsStruct(ctx, obj);
	}
};

HPyType_HELPERS(Term)

struct Expression
{
	HPyField terms;
	double constant;

    static HPyType_Spec TypeObject_Spec;

    static HPy TypeObject;

	static bool Ready( HPyContext *ctx, HPy m );

	static bool TypeCheck( HPyContext *ctx, HPy obj )
	{
		return HPy_TypeCheck( ctx, obj, TypeObject ) != 0;
	}

	static Expression* AsStruct(HPyContext *ctx, HPy obj) {
		return (Expression*)HPy_AsStruct(ctx, obj);
	}
};

HPyType_HELPERS(Expression)

struct Constraint
{
	HPyField expression;
	kiwi::Constraint constraint;

    static HPyType_Spec TypeObject_Spec;

    static HPy TypeObject;

	static bool Ready( HPyContext *ctx, HPy m );

	static bool TypeCheck( HPyContext *ctx, HPy obj )
	{
		return HPy_TypeCheck( ctx, obj, TypeObject ) != 0;
	}

	static Constraint* AsStruct(HPyContext *ctx, HPy obj) {
		return (Constraint*)HPy_AsStruct(ctx, obj);
	}
};

HPyType_HELPERS(Constraint)

struct Solver
{
	kiwi::Solver solver;

    static HPyType_Spec TypeObject_Spec;

    static HPy TypeObject;

	static bool Ready( HPyContext *ctx, HPy m );

	static bool TypeCheck( HPyContext *ctx, HPy obj )
	{
		return HPy_TypeCheck( ctx, obj, TypeObject ) != 0;
	}

	static Solver* AsStruct(HPyContext *ctx, HPy obj) {
		return (Solver*)HPy_AsStruct(ctx, obj);
	}
};

HPyType_HELPERS(Solver)

bool init_exceptions( HPyContext *ctx, HPy mod );


}  // namespace kiwisolver
