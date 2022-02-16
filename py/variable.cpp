/*-----------------------------------------------------------------------------
| Copyright (c) 2013-2019, Nucleic Development Team.
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


static HPy
Variable_new( HPyContext *ctx, HPy type, HPy* args, HPy_ssize_t nargs, HPy kwargs )
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
		return HPy_NULL;

	self->context = HPy_Dup( ctx, context );

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
            HPyTracker_Close(ctx, ht);
            return HPy_NULL;
        }
		std::string c_name;
		if( !convert_pystr_to_str(ctx, name, c_name) ) {
			HPyTracker_Close(ctx, ht);
			return HPy_NULL;  // LCOV_EXCL_LINE
		}
		new( &self->variable ) kiwi::Variable( c_name );
	}
	else
	{
		new( &self->variable ) kiwi::Variable();
	}

	return pyvar;
}


// static void
// Variable_clear( HPyContext *ctx, Variable* self )
// {
// 	HPy_CLEAR( ctx, self->context );
// }


// static int
// Variable_traverse( void* obj, HPyFunc_visitproc visit, void* arg )
// {
// //     Variable* self = (Variable*) obj;
// // 	HPy_VISIT( self->context );
// // #if PY_VERSION_HEX >= 0x03090000
//     // This was not needed before Python 3.9 (Python issue 35810 and 40217)
//     // HPy_VISIT(HPy_Type(ctx, self));
// // #endif
// 	return 0;
// }


static void
Variable_dealloc( void *obj )
{
	Variable* self = (Variable*)obj;
	// PyObject_GC_UnTrack( self );
	// Variable_clear( ctx, self );
	self->variable.~Variable();
	// Py_TYPE( self )->tp_free( pyobject_cast( self ) );
}


static HPy
Variable_repr( HPyContext *ctx, HPy h_self )
{
    Variable* self = Variable::AsStruct( ctx, h_self );
	return HPyUnicode_FromString( ctx, self->variable.name().c_str() );
}


static HPy
Variable_name( HPyContext *ctx, HPy h_self )
{
    Variable* self = Variable::AsStruct( ctx, h_self );
	return HPyUnicode_FromString( ctx, self->variable.name().c_str() );
}


static HPy
Variable_setName( HPyContext *ctx, HPy h_self, HPy pystr )
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


static HPy
Variable_context( HPyContext *ctx, HPy h_self )
{
    Variable* self = Variable::AsStruct( ctx, h_self );
	if( !HPy_IsNull(self->context) )
		return HPy_Dup( ctx, self->context );
	return HPy_Dup( ctx, ctx->h_None );
}


static HPy
Variable_setContext( HPyContext *ctx, HPy h_self, HPy value )
{
    Variable* self = Variable::AsStruct( ctx, h_self );
	if( !HPy_Is( ctx, value, self->context ) )
	{
		HPy temp = self->context;
		self->context = HPy_Dup( ctx, value );
		HPy_Close( ctx, temp );
	}
	return HPy_Dup( ctx, ctx->h_None );
}


static HPy
Variable_value( HPyContext *ctx, HPy h_self )
{
    Variable* self = Variable::AsStruct( ctx, h_self );
	return HPyFloat_FromDouble( ctx, self->variable.value() );
}


static HPy
Variable_add( HPyContext *ctx, HPy first, HPy second )
{
	return BinaryInvoke<BinaryAdd, Variable>()( ctx, first, second );
}


static HPy
Variable_sub( HPyContext *ctx, HPy first, HPy second )
{
	return BinaryInvoke<BinarySub, Variable>()( ctx, first, second );
}


static HPy
Variable_mul( HPyContext *ctx, HPy first, HPy second )
{
	return BinaryInvoke<BinaryMul, Variable>()( ctx, first, second );
}


static HPy
Variable_div( HPyContext *ctx, HPy first, HPy second )
{
	return BinaryInvoke<BinaryDiv, Variable>()( ctx, first, second );
}


static HPy
Variable_neg( HPyContext *ctx, HPy value )
{
	return UnaryInvoke<UnaryNeg, Variable>()( ctx, value );
}


static HPy
Variable_richcmp( HPyContext *ctx, HPy first, HPy second, HPy_RichCmpOp op )
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
	// PyErr_Format(
	// 	PyExc_TypeError,
	// 	"unsupported operand type(s) for %s: "
	// 	"'%.100s' and '%.100s'",
	// 	pyop_str( op ),
	// 	Py_TYPE( first )->tp_name,
	// 	Py_TYPE( second )->tp_name
	// );
    HPyErr_SetString( ctx, ctx->h_TypeError, "unsupported operand type(s)" );
	return HPy_NULL;
}


HPyDef_METH(Variable_name_def, "name", Variable_name, HPyFunc_NOARGS,
	.doc = "Get the name of the variable.")
HPyDef_METH(Variable_setName_def, "setName", Variable_setName, HPyFunc_O,
	.doc = "Set the name of the variable.")
HPyDef_METH(Variable_context_def, "context", Variable_context, HPyFunc_NOARGS,
	.doc = "Get the context object associated with the variable.")
HPyDef_METH(Variable_setContext_def, "setContext", Variable_setContext, HPyFunc_O,
	.doc = "Set the context object associated with the variable.")
HPyDef_METH(Variable_value_def, "value", Variable_value, HPyFunc_NOARGS,
	.doc = "Get the current value of the variable.")


HPyDef_SLOT(Variable_dealloc_def, Variable_dealloc, HPy_tp_destroy)
// HPyDef_SLOT(Variable_traverse_def, Variable_traverse, HPy_tp_traverse)
HPyDef_SLOT(Variable_repr_def, Variable_repr, HPy_tp_repr)
HPyDef_SLOT(Variable_richcmp_def, Variable_richcmp, HPy_tp_richcompare)
HPyDef_SLOT(Variable_new_def, Variable_new, HPy_tp_new)
HPyDef_SLOT(Variable_add_def, Variable_add, HPy_nb_add)
HPyDef_SLOT(Variable_sub_def, Variable_sub, HPy_nb_subtract)
HPyDef_SLOT(Variable_mul_def, Variable_mul, HPy_nb_multiply)
HPyDef_SLOT(Variable_neg_def, Variable_neg, HPy_nb_negative)
HPyDef_SLOT(Variable_div_def, Variable_div, HPy_nb_true_divide)
// HPyDef_SLOT(Variable_clear_def, Variable_clear, HPy_tp_clear)

static HPyDef* Variable_defines[] = {
    // slots
	&Variable_dealloc_def,
	// &Variable_traverse_def,
	&Variable_repr_def,
	&Variable_richcmp_def,
	&Variable_new_def,
	&Variable_add_def,
	&Variable_sub_def,
	&Variable_mul_def,
	&Variable_neg_def,
	&Variable_div_def,
	// &Variable_clear_def

    // methods
	&Variable_name_def,
	&Variable_setName_def,
	&Variable_context_def,
	&Variable_setContext_def,
	&Variable_value_def,
	NULL
};
} // namespace


// Initialize static variables (otherwise the compiler eliminates them)
HPy Variable::TypeObject = HPy_NULL;


HPyType_Spec Variable::TypeObject_Spec = {
	.name = "kiwisolver.Variable",
	.basicsize = sizeof( Variable ),
	.itemsize = 0,
	.flags = HPy_TPFLAGS_DEFAULT /* | HPy_TPFLAGS_HAVE_GC */ | HPy_TPFLAGS_BASETYPE,
    .defines = Variable_defines
};


bool Variable::Ready( HPyContext *ctx, HPy m )
{
    // The reference will be handled by the module to which we will add the type
    if (!HPyHelpers_AddType(ctx, m, "Variable", &TypeObject_Spec, NULL)) {
        return false;
    }

    TypeObject = HPy_GetAttr_s(ctx, m, "Variable");
    if( HPy_IsNull(TypeObject) )
    {
        return false;
    }
    return true;
}

}  // namespace kiwisolver
