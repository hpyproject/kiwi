/*-----------------------------------------------------------------------------
| Copyright (c) 2013-2019, Nucleic Development Team.
| Copyright (c) 2022-2023, Oracle and/or its affiliates.
|
| Distributed under the terms of the Modified BSD License.
|
| The full license is in the file LICENSE, distributed with this software.
|----------------------------------------------------------------------------*/
#include <sstream>
#include "symbolics.h"
#include "types.h"
#include "util.h"


namespace kiwisolver
{


namespace
{


HPyDef_SLOT(Term_new, HPy_tp_new)
static HPy
Term_new_impl( HPyContext *ctx, HPy type, const HPy *args, HPy_ssize_t nargs, HPy kwargs )
{
	static const char *kwlist[] = { "variable", "coefficient", 0 };
	HPy pyvar;
	HPy pycoeff = HPy_NULL;
	HPyTracker ht;
	if( !HPyArg_ParseKeywords(ctx, &ht, args, nargs,
		kwargs, "O|O:__new__", (const char **) kwlist,
		&pyvar, &pycoeff ) )
		return HPy_NULL;
	if( !Variable::TypeCheck( ctx, pyvar ) ){
		// PyErr_Format(
		// 	PyExc_TypeError,
		// 	"Expected object of type `Variable`. Got object of type `%s` instead.",
		// 	pyvar->ob_type->tp_name );
		HPyErr_SetString( ctx, ctx->h_TypeError, "Expected object of type `Variable`." );
		HPyTracker_Close(ctx, ht);
		return HPy_NULL;
	}
	double coefficient = 1.0;
	if( !HPy_IsNull(pycoeff) && !convert_to_double( ctx, pycoeff, coefficient ) ) {
		HPyTracker_Close(ctx, ht);
		return HPy_NULL;
	}
	Term* self;
	HPy pyterm = HPy_New(ctx, type, &self);
	if( HPy_IsNull(pyterm) ) {
		HPyTracker_Close(ctx, ht);
		return HPy_NULL;
	}
	HPyField_Store( ctx, pyterm , &self->variable , pyvar );
	self->coefficient = coefficient;
	HPyTracker_Close(ctx, ht);
	return pyterm;
}



HPyDef_SLOT(Term_traverse, HPy_tp_traverse)
static int
Term_traverse_impl( void* obj, HPyFunc_visitproc visit, void* arg )
{
	Term *t = (Term*) obj;
	HPy_VISIT(&t->variable);
	return 0;
}


HPyDef_SLOT(Term_repr, HPy_tp_repr)
static HPy
Term_repr_impl( HPyContext *ctx, HPy h_self )
{
	Term* self = Term::AsStruct(ctx, h_self);
	std::stringstream stream;
	stream << self->coefficient << " * ";
	HPy term_var = HPyField_Load( ctx , h_self, self->variable );
	stream << Variable::AsStruct( ctx, term_var )->variable.name();
	HPy_Close( ctx , term_var );
	return HPyUnicode_FromString( ctx, stream.str().c_str() );
}


HPyDef_METH(Term_variable, "variable", HPyFunc_NOARGS,
	.doc = "Get the variable for the term.")
static HPy
Term_variable_impl( HPyContext *ctx, HPy h_self )
{
	Term* self = Term::AsStruct(ctx, h_self);
	return HPyField_Load( ctx , h_self ,  self->variable );
}


HPyDef_METH(Term_coefficient, "coefficient", HPyFunc_NOARGS,
	.doc = "Get the coefficient for the term.")
static HPy
Term_coefficient_impl( HPyContext *ctx, HPy h_self )
{
	Term* self = Term::AsStruct(ctx, h_self);
	return HPyFloat_FromDouble( ctx, self->coefficient );
}


HPyDef_METH(Term_value, "value", HPyFunc_NOARGS,
	.doc = "Get the value for the term.")
static HPy
Term_value_impl( HPyContext *ctx, HPy h_self )
{
	Term* self = Term::AsStruct(ctx, h_self);
	HPy self_var = HPyField_Load( ctx , h_self , self->variable );
	Variable* pyvar = Variable::AsStruct( ctx, self_var );
	HPy result = HPyFloat_FromDouble( ctx, self->coefficient * pyvar->variable.value() );
	HPy_Close( ctx , self_var );
	return result;
}


HPyDef_SLOT(Term_add, HPy_nb_add)
static HPy
Term_add_impl( HPyContext *ctx, HPy first, HPy second )
{
	return BinaryInvoke<BinaryAdd, Term>()( ctx, first, second );
}


HPyDef_SLOT(Term_sub, HPy_nb_subtract)
static HPy
Term_sub_impl( HPyContext *ctx, HPy first, HPy second )
{
	return BinaryInvoke<BinarySub, Term>()( ctx, first, second );
}


HPyDef_SLOT(Term_mul, HPy_nb_multiply)
static HPy
Term_mul_impl( HPyContext *ctx, HPy first, HPy second )
{
	return BinaryInvoke<BinaryMul, Term>()( ctx, first, second );
}


HPyDef_SLOT(Term_div, HPy_nb_true_divide)
static HPy
Term_div_impl( HPyContext *ctx, HPy first, HPy second )
{
	return BinaryInvoke<BinaryDiv, Term>()( ctx, first, second );
}


HPyDef_SLOT(Term_neg, HPy_nb_negative)
static HPy
Term_neg_impl( HPyContext *ctx, HPy value )
{
	return UnaryInvoke<UnaryNeg, Term>()( ctx, value );
}


HPyDef_SLOT(Term_richcmp, HPy_tp_richcompare)
static HPy
Term_richcmp_impl( HPyContext *ctx, HPy first, HPy second, HPy_RichCmpOp op )
{
	switch( op )
	{
		case HPy_EQ:
			return BinaryInvoke<CmpEQ, Term>()( ctx, first, second );
		case HPy_LE:
			return BinaryInvoke<CmpLE, Term>()( ctx, first, second );
		case HPy_GE:
			return BinaryInvoke<CmpGE, Term>()( ctx, first, second );
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






static HPyDef* Term_defines[] = {
    // slots
	&Term_traverse,
	&Term_repr,
	&Term_richcmp,
	&Term_new,
	&Term_add,
	&Term_sub,
	&Term_mul,
	&Term_neg,
	&Term_div,
	// Term_clear,

    // methods
	&Term_variable,
	&Term_coefficient,
	&Term_value,
    NULL
};
} // namespace


// Declare static variables (otherwise the compiler eliminates them)
HPyGlobal Term::TypeObject;


HPyType_Spec Term::TypeObject_Spec = {
	.name = "kiwisolver.Term",
	.basicsize = sizeof( Term ),
	.itemsize = 0,
	.flags = HPy_TPFLAGS_DEFAULT | HPy_TPFLAGS_HAVE_GC | HPy_TPFLAGS_BASETYPE,
    .defines = Term_defines
};


bool Term::Ready( HPyContext *ctx, HPy m )
{
    return add_type(ctx, m, &TypeObject, "Term", &TypeObject_Spec);
}

}  // namespace kiwisolver
