/*-----------------------------------------------------------------------------
| Copyright (c) 2013-2019, Nucleic Development Team.
| Copyright (c) 2022-2023, Oracle and/or its affiliates.
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

extern HPyGlobal DuplicateConstraint;

extern HPyGlobal UnsatisfiableConstraint;

extern HPyGlobal UnknownConstraint;

extern HPyGlobal DuplicateEditVariable;

extern HPyGlobal UnknownEditVariable;

extern HPyGlobal BadRequiredStrength;


struct strength
{
    static HPyType_Spec TypeObject_Spec;

    static HPyGlobal TypeObject;

	static bool Ready( HPyContext *ctx, HPy m );
};


struct Variable
{
	HPyField context;
	kiwi::Variable variable;

    static HPyType_Spec TypeObject_Spec;

    static HPyGlobal TypeObject;

	static bool Ready( HPyContext *ctx, HPy m );

	static bool TypeCheck( HPyContext *ctx, HPy obj )
	{
		HPy h_type = HPyGlobal_Load( ctx , TypeObject );
		bool result = HPy_TypeCheck( ctx, obj, h_type ) != 0;
		HPy_Close( ctx , h_type );
		return result;
	}

	static Variable* AsStruct(HPyContext *ctx, HPy obj) {
		return (Variable*)_HPy_AsStruct_Object(ctx, obj);
	}
};

HPyType_HELPERS(Variable)

struct Term
{
	HPyField variable;
	double coefficient;

    static HPyType_Spec TypeObject_Spec;

    static HPyGlobal TypeObject;

	static bool Ready( HPyContext *ctx, HPy m );

	static bool TypeCheck( HPyContext *ctx, HPy obj )
	{
		HPy h_type = HPyGlobal_Load( ctx , TypeObject );
		bool result = HPy_TypeCheck( ctx, obj, h_type ) != 0;
		HPy_Close( ctx , h_type );
		return result;
	}

	static Term* AsStruct(HPyContext *ctx, HPy obj) {
		return (Term*)_HPy_AsStruct_Object(ctx, obj);
	}
};

HPyType_HELPERS(Term)

struct Expression
{
	HPyField terms;
	double constant;

    static HPyType_Spec TypeObject_Spec;

    static HPyGlobal TypeObject;

	static bool Ready( HPyContext *ctx, HPy m );

	static bool TypeCheck( HPyContext *ctx, HPy obj )
	{
		HPy h_type = HPyGlobal_Load( ctx , TypeObject );
		bool result = HPy_TypeCheck( ctx, obj, h_type ) != 0;
		HPy_Close( ctx , h_type );
		return result;
	}

	static Expression* AsStruct(HPyContext *ctx, HPy obj) {
		return (Expression*)_HPy_AsStruct_Object(ctx, obj);
	}
};

HPyType_HELPERS(Expression)

struct Constraint
{
	HPyField expression;
	kiwi::Constraint constraint;

    static HPyType_Spec TypeObject_Spec;

    static HPyGlobal TypeObject;

	static bool Ready( HPyContext *ctx, HPy m );

	static bool TypeCheck( HPyContext *ctx, HPy obj )
	{
		HPy h_type = HPyGlobal_Load( ctx , TypeObject );
		bool result = HPy_TypeCheck( ctx, obj, h_type ) != 0;
		HPy_Close( ctx , h_type );
		return result;
	}

	static Constraint* AsStruct(HPyContext *ctx, HPy obj) {
	    return (Constraint *)_HPy_AsStruct_Object(ctx, obj);
	}
};

HPyType_HELPERS(Constraint)

struct Solver
{
	kiwi::Solver solver;

    static HPyType_Spec TypeObject_Spec;

    static HPyGlobal TypeObject;

	static bool Ready( HPyContext *ctx, HPy m );

	static bool TypeCheck( HPyContext *ctx, HPy obj )
	{
		HPy h_type = HPyGlobal_Load( ctx , TypeObject );
		bool result = HPy_TypeCheck( ctx, obj, h_type ) != 0;
		HPy_Close( ctx , h_type );
		return result;
	}
};

HPyType_HELPERS(Solver)

bool init_exceptions( HPyContext *ctx, HPy mod );


}  // namespace kiwisolver
