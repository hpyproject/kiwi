/*-----------------------------------------------------------------------------
| Copyright (c) 2013-2019, Nucleic Development Team.
| Copyright (c) 2022-2023, Oracle and/or its affiliates.
|
| Distributed under the terms of the Modified BSD License.
|
| The full license is in the file LICENSE, distributed with this software.
|----------------------------------------------------------------------------*/
#include <kiwi/kiwi.h>
#include "symbolics.h"
#include "types.h"
#include "util.h"


namespace kiwisolver
{


namespace
{


HPyDef_SLOT(Variable_new, HPy_tp_new)
static HPy
Variable_new_impl( HPyContext *ctx, HPy type, const HPy *args, HPy_ssize_t nargs, HPy kwargs )
{
	static const char *kwlist[] = { "name", "context", 0 };
	HPy context = HPy_NULL;
	HPy name = HPy_NULL;

    HPyTracker ht;
    if (!HPyArg_ParseKeywords(ctx, &ht, args, nargs,
		kwargs, "|OO:__new__", (const char **) kwlist,
		&name, &context ) )
		return HPy_NULL;

	Variable* self;
	HPy pyvar = HPy_New(ctx, type, &self);
	if( HPy_IsNull(pyvar) )
	{
		HPyTracker_Close( ctx , ht );
		return HPy_NULL;
	}

	HPyField_Store(ctx, pyvar, &self->context, context);

	if( !HPy_IsNull( name ) )
	{
		if( !HPyUnicode_Check( ctx, name ) ) {
            // PyErr_Format(
            //     PyExc_TypeError,
            //     "Expected object of type `str`. Got object of type `%s` instead.",
            //     name->ob_type->tp_name );
            HPyErr_SetString( ctx,
                ctx->h_TypeError,
                "Expected object of type `str`.");
            HPy_Close( ctx , pyvar );
            HPyTracker_Close(ctx, ht);
            return HPy_NULL;
        }
		std::string c_name;
		if( !convert_pystr_to_str(ctx, name, c_name) ) {
			HPy_Close( ctx , pyvar );
			HPyTracker_Close(ctx, ht);
			return HPy_NULL;  // LCOV_EXCL_LINE
		}
		new( &self->variable ) kiwi::Variable( c_name );
	}
	else
	{
		new( &self->variable ) kiwi::Variable();
	}

	HPyTracker_Close( ctx , ht );
	return pyvar;
}


HPyDef_SLOT(Variable_traverse, HPy_tp_traverse)
static int
Variable_traverse_impl( void* obj, HPyFunc_visitproc visit, void* arg )
{
    Variable* self = (Variable*) obj;
	HPy_VISIT( &self->context );
	return 0;
}


HPyDef_SLOT(Variable_dealloc, HPy_tp_destroy)
static void
Variable_dealloc_impl(void *data)
{
    Variable *self = (Variable *)data;
	self->variable.~Variable();
}


HPyDef_SLOT(Variable_repr, HPy_tp_repr)
static HPy
Variable_repr_impl( HPyContext *ctx, HPy h_self )
{
    Variable* self = Variable::AsStruct( ctx, h_self );
	return HPyUnicode_FromString( ctx, self->variable.name().c_str() );
}


HPyDef_METH(Variable_name, "name", HPyFunc_NOARGS,
	.doc = "Get the name of the variable.")
static HPy
Variable_name_impl( HPyContext *ctx, HPy h_self )
{
    Variable* self = Variable::AsStruct( ctx, h_self );
	return HPyUnicode_FromString( ctx, self->variable.name().c_str() );
}


HPyDef_METH(Variable_setName, "setName", HPyFunc_O,
	.doc = "Set the name of the variable.")
static HPy
Variable_setName_impl( HPyContext *ctx, HPy h_self, HPy pystr )
{
	Variable* self = Variable::AsStruct( ctx, h_self );
	if( !HPyUnicode_Check( ctx, pystr ) ) {
		// PyErr_Format(
		//     PyExc_TypeError,
		//     "Expected object of type `str`. Got object of type `%s` instead.",
		//     expected,
		//     pystr->ob_type->tp_name );
		HPyErr_SetString( ctx,
			ctx->h_TypeError,
			"Expected object of type `str`.");
		return HPy_NULL;
	}
	std::string str;
	if( !convert_pystr_to_str( ctx, pystr, str ) )
		return HPy_NULL;
	self->variable.setName( str );
	return HPy_Dup( ctx, ctx->h_None );
}


HPyDef_METH(Variable_context, "context", HPyFunc_NOARGS,
	.doc = "Get the context object associated with the variable.")
static HPy
Variable_context_impl( HPyContext *ctx, HPy h_self )
{
    Variable* self = Variable::AsStruct( ctx, h_self );
	if( !HPyField_IsNull(self->context) ) {
		HPy context = HPyField_Load(ctx, h_self, self->context);
		if (!HPy_IsNull(context)) {
			return context;
		}
	}
	return HPy_Dup( ctx, ctx->h_None );
}


HPyDef_METH(Variable_setContext, "setContext", HPyFunc_O,
	.doc = "Set the context object associated with the variable.")
static HPy
Variable_setContext_impl( HPyContext *ctx, HPy h_self, HPy value )
{
    Variable* self = Variable::AsStruct( ctx, h_self );
	if( HPyField_IsNull(self->context) || !HPy_Is( ctx, value, HPyField_Load(ctx, h_self, self->context) ) )
	{
		HPyField_Store(ctx, h_self, &self->context, value);
	}
	return HPy_Dup( ctx, ctx->h_None );
}


HPyDef_METH(Variable_value, "value", HPyFunc_NOARGS,
	.doc = "Get the current value of the variable.")
static HPy
Variable_value_impl( HPyContext *ctx, HPy h_self )
{
    Variable* self = Variable::AsStruct( ctx, h_self );
	return HPyFloat_FromDouble( ctx, self->variable.value() );
}


HPyDef_SLOT(Variable_add, HPy_nb_add)
static HPy
Variable_add_impl( HPyContext *ctx, HPy first, HPy second )
{
	return BinaryInvoke<BinaryAdd, Variable>()( ctx, first, second );
}


HPyDef_SLOT(Variable_sub, HPy_nb_subtract)
static HPy
Variable_sub_impl( HPyContext *ctx, HPy first, HPy second )
{
	return BinaryInvoke<BinarySub, Variable>()( ctx, first, second );
}


HPyDef_SLOT(Variable_mul, HPy_nb_multiply)
static HPy
Variable_mul_impl( HPyContext *ctx, HPy first, HPy second )
{
	return BinaryInvoke<BinaryMul, Variable>()( ctx, first, second );
}


HPyDef_SLOT(Variable_div, HPy_nb_true_divide)
static HPy
Variable_div_impl( HPyContext *ctx, HPy first, HPy second )
{
	return BinaryInvoke<BinaryDiv, Variable>()( ctx, first, second );
}


HPyDef_SLOT(Variable_neg, HPy_nb_negative)
static HPy
Variable_neg_impl( HPyContext *ctx, HPy value )
{
	return UnaryInvoke<UnaryNeg, Variable>()( ctx, value );
}


HPyDef_SLOT(Variable_richcmp, HPy_tp_richcompare)
static HPy
Variable_richcmp_impl( HPyContext *ctx, HPy first, HPy second, HPy_RichCmpOp op )
{
	switch( op )
	{
		case HPy_EQ:
			return BinaryInvoke<CmpEQ, Variable>()( ctx, first, second );
		case HPy_LE:
			return BinaryInvoke<CmpLE, Variable>()( ctx, first, second );
		case HPy_GE:
			return BinaryInvoke<CmpGE, Variable>()( ctx, first, second );
		default:
			break;
	}
	HPy first_type = HPy_Type(ctx, first);
	HPy second_type = HPy_Type(ctx, second);
	HPyErr_Format(ctx,
	 	ctx->h_TypeError,
	 	"unsupported operand type(s) for %s: "
	 	"'%.100s' and '%.100s'",
	 	pyop_str( op ),
	 	HPyType_GetName(ctx, first_type),
	 	HPyType_GetName(ctx, second_type)
	 );
	HPy_Close(ctx, first_type);
	HPy_Close(ctx, second_type);
	return HPy_NULL;
}





static HPyDef* Variable_defines[] = {
    // slots
	&Variable_dealloc,
	&Variable_traverse,
	&Variable_repr,
	&Variable_richcmp,
	&Variable_new,
	&Variable_add,
	&Variable_sub,
	&Variable_mul,
	&Variable_neg,
	&Variable_div,
	// &Variable_clear

    // methods
	&Variable_name,
	&Variable_setName,
	&Variable_context,
	&Variable_setContext,
	&Variable_value,
	NULL
};
} // namespace


// Declare static variables (otherwise the compiler eliminates them)
HPyGlobal Variable::TypeObject;


HPyType_Spec Variable::TypeObject_Spec = {
	.name = "kiwisolver.Variable",
	.basicsize = sizeof( Variable ),
	.itemsize = 0,
	.flags = HPy_TPFLAGS_DEFAULT | HPy_TPFLAGS_HAVE_GC | HPy_TPFLAGS_BASETYPE,
    .defines = Variable_defines
};


bool Variable::Ready( HPyContext *ctx, HPy m )
{
    return add_type( ctx , m , &TypeObject , "Variable" , &TypeObject_Spec );
}

}  // namespace kiwisolver
