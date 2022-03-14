/*-----------------------------------------------------------------------------
| Copyright (c) 2013-2019, Nucleic Development Team.
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


static HPy
Term_new( HPyContext *ctx, HPy type, HPy* args, HPy_ssize_t nargs, HPy kwargs )
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
	self->variable = HPy_Dup( ctx, pyvar );
	self->coefficient = coefficient;
	return pyterm;
}



static int
Term_traverse( void* obj, HPyFunc_visitproc visit, void* arg )
{
	// Term* self = (Term*)obj;
	// PyObject_GC_UnTrack( self );
	return 0;
}


static HPy
Term_repr( HPyContext *ctx, HPy h_self )
{
	Term* self = Term::AsStruct(ctx, h_self);
	std::stringstream stream;
	stream << self->coefficient << " * ";
	stream << Variable::AsStruct( ctx, self->variable )->variable.name();
	return HPyUnicode_FromString( ctx, stream.str().c_str() );
}


static HPy
Term_variable( HPyContext *ctx, HPy h_self )
{
	Term* self = Term::AsStruct(ctx, h_self);
	return HPy_Dup( ctx, self->variable );
}


static HPy
Term_coefficient( HPyContext *ctx, HPy h_self )
{
	Term* self = Term::AsStruct(ctx, h_self);
	return HPyFloat_FromDouble( ctx, self->coefficient );
}


static HPy
Term_value( HPyContext *ctx, HPy h_self )
{
	Term* self = Term::AsStruct(ctx, h_self);
	Variable* pyvar = Variable::AsStruct( ctx, self->variable );
	return HPyFloat_FromDouble( ctx, self->coefficient * pyvar->variable.value() );
}


static HPy
Term_add( HPyContext *ctx, HPy first, HPy second )
{
	return BinaryInvoke<BinaryAdd, Term>()( ctx, first, second );
}


static HPy
Term_sub( HPyContext *ctx, HPy first, HPy second )
{
	return BinaryInvoke<BinarySub, Term>()( ctx, first, second );
}


static HPy
Term_mul( HPyContext *ctx, HPy first, HPy second )
{
	return BinaryInvoke<BinaryMul, Term>()( ctx, first, second );
}


static HPy
Term_div( HPyContext *ctx, HPy first, HPy second )
{
	return BinaryInvoke<BinaryDiv, Term>()( ctx, first, second );
}


static HPy
Term_neg( HPyContext *ctx, HPy value )
{
	return UnaryInvoke<UnaryNeg, Term>()( ctx, value );
}


static HPy
Term_richcmp( HPyContext *ctx, HPy first, HPy second, HPy_RichCmpOp op )
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


HPyDef_METH(Term_variable_def, "variable", Term_variable, HPyFunc_NOARGS,
	.doc = "Get the variable for the term.")
HPyDef_METH(Term_coefficient_def, "coefficient", Term_coefficient, HPyFunc_NOARGS,
	.doc = "Get the coefficient for the term.")
HPyDef_METH(Term_value_def, "value", Term_value, HPyFunc_NOARGS,
	.doc = "Get the value for the term.")


HPyDef_SLOT(Term_traverse_def, Term_traverse, HPy_tp_traverse)    /* tp_traverse */
HPyDef_SLOT(Term_repr_def, Term_repr, HPy_tp_repr)            /* tp_repr */
HPyDef_SLOT(Term_richcmp_def, Term_richcmp, HPy_tp_richcompare)  /* tp_richcompare */
HPyDef_SLOT(Term_new_def, Term_new, HPy_tp_new)              /* tp_new */
HPyDef_SLOT(Term_add_def, Term_add, HPy_nb_add)              /* nb_add */
HPyDef_SLOT(Term_sub_def, Term_sub, HPy_nb_subtract)         /* nb_subatract */
HPyDef_SLOT(Term_mul_def, Term_mul, HPy_nb_multiply)         /* nb_multiply */
HPyDef_SLOT(Term_neg_def, Term_neg, HPy_nb_negative)         /* nb_negative */
HPyDef_SLOT(Term_div_def, Term_div, HPy_nb_true_divide)      /* nb_true_divide */


static HPyDef* Term_defines[] = {
    // slots
	&Term_traverse_def,
	&Term_repr_def,
	&Term_richcmp_def,
	&Term_new_def,
	&Term_add_def,
	&Term_sub_def,
	&Term_mul_def,
	&Term_neg_def,
	&Term_div_def,
	// Term_clear_def,

    // methods
	&Term_variable_def,
	&Term_coefficient_def,
	&Term_value_def,
    NULL
};
} // namespace


// Initialize static variables (otherwise the compiler eliminates them)
HPy Term::TypeObject = HPy_NULL;


HPyType_Spec Term::TypeObject_Spec = {
	.name = "kiwisolver.Term",
	.basicsize = sizeof( Term ),
	.itemsize = 0,
	.flags = HPy_TPFLAGS_DEFAULT | HPy_TPFLAGS_HAVE_GC | HPy_TPFLAGS_BASETYPE,
    .defines = Term_defines
};


bool Term::Ready( HPyContext *ctx, HPy m )
{
    // The reference will be handled by the module to which we will add the type
    if (!HPyHelpers_AddType(ctx, m, "Term", &TypeObject_Spec, NULL)) {
        return false;
    }

    TypeObject = HPy_GetAttr_s(ctx, m, "Term");
    if( HPy_IsNull(TypeObject) )
    {
        return false;
    }
    return true;
}

}  // namespace kiwisolver
