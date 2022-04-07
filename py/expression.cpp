/*-----------------------------------------------------------------------------
| Copyright (c) 2013-2019, Nucleic Development Team.
| Copyright (c) 2022, Oracle and/or its affiliates.
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
Expression_new( HPyContext *ctx, HPy type, HPy* args, HPy_ssize_t nargs, HPy kwargs )
{
    static const char *kwlist[] = { "terms", "constant", 0 };
    HPy pyterms;
    HPy pyconstant = HPy_NULL;
    HPyTracker ht;
    if (!HPyArg_ParseKeywords(ctx, &ht, args, nargs,
        kwargs, "O|O:__new__", (const char **)kwlist,
        &pyterms, &pyconstant ) )
        return HPy_NULL;
    HPy_ssize_t end = HPy_Length( ctx, pyterms );
    HPyTupleBuilder terms = HPyTupleBuilder_New(ctx, end);
    for( HPy_ssize_t i = 0; i < end; ++i )
    {
        HPy item = HPy_GetItem_i( ctx, pyterms, i );
        if( HPy_IsNull( item ) || !Term::TypeCheck( ctx, item ) ) {
            // PyErr_Format(
            //     PyExc_TypeError,
            //     "Expected object of type `%s`. Got object of type `%s` instead.",
            //     expected,
            //     ob->ob_type->tp_name );
            HPyErr_SetString( ctx, ctx->h_TypeError, "Expected object of type `Term`.");
            HPy_Close( ctx , item ); // XCLOSE
            HPyTracker_Close(ctx, ht);
            HPyTupleBuilder_Cancel( ctx , terms );
            return HPy_NULL;
        }
        HPyTupleBuilder_Set( ctx, terms, i, item );
        HPy_Close( ctx, item );
    }
    double constant = 0.0;
    if( !HPy_IsNull(pyconstant) && !convert_to_double( ctx, pyconstant, constant ) ) {
        HPyTupleBuilder_Cancel( ctx , terms );
        HPyTracker_Close(ctx, ht);
        return HPy_NULL;
    }
    HPy terms_tuple = HPyTupleBuilder_Build( ctx, terms );
    if (HPy_IsNull(terms_tuple)) {
        HPyTracker_Close(ctx, ht);
        return HPy_NULL;
    }
    Expression* self;
    HPy pyexpr = HPy_New(ctx, type, &self);
    HPyField_Store(ctx, pyexpr, &self->terms, terms_tuple);
    HPy_Close( ctx , terms_tuple );
    self->constant = constant;
    return pyexpr;
}


static int
Expression_traverse( void* obj, HPyFunc_visitproc visit, void* arg )
{
    Expression* self = (Expression*) obj;
    HPy_VISIT( &self->terms );
    return 0;
}


static HPy
Expression_repr( HPyContext *ctx, HPy h_self )
{
    Expression* self = Expression::AsStruct( ctx, h_self );
    std::stringstream stream;
    HPy expr_terms = HPyField_Load(ctx, h_self, self->terms);
    HPy_ssize_t end = HPy_Length( ctx, expr_terms );
    for( HPy_ssize_t i = 0; i < end; ++i )
    {
        HPy item = HPy_GetItem_i( ctx, expr_terms, i );
        if ( HPy_IsNull( item ) )
        {
            HPy_Close( ctx , expr_terms );
            return HPy_NULL;
        }
        Term* term = Term::AsStruct( ctx, item );
        stream << term->coefficient << " * ";
        HPy term_var = HPyField_Load( ctx , item , term->variable );
        stream << Variable::AsStruct( ctx, term_var )->variable.name();
        stream << " + ";
        HPy_Close( ctx , item );
        HPy_Close( ctx , term_var );
    }
    HPy_Close( ctx , expr_terms );
    stream << self->constant;
    return HPyUnicode_FromString( ctx, stream.str().c_str() );
}


static HPy
Expression_terms( HPyContext *ctx, HPy h_self )
{
    Expression* self = Expression::AsStruct( ctx, h_self );
    return HPyField_Load(ctx, h_self, self->terms);
}


static HPy
Expression_constant( HPyContext *ctx, HPy h_self )
{
    Expression* self = Expression::AsStruct( ctx, h_self );
    return HPyFloat_FromDouble( ctx, self->constant );
}


static HPy
Expression_value( HPyContext *ctx, HPy h_self )
{
    Expression* self = Expression::AsStruct( ctx, h_self );
    double result = self->constant;
    HPy expr_terms = HPyField_Load(ctx, h_self, self->terms);
    HPy_ssize_t size = HPy_Length( ctx, expr_terms );
    for( HPy_ssize_t i = 0; i < size; ++i )
    {
        HPy item = HPy_GetItem_i( ctx, expr_terms, i );
        if ( HPy_IsNull( item ) )
        {
            HPy_Close( ctx , expr_terms );
            return HPy_NULL;
        }
        Term* term = Term::AsStruct( ctx, item );
        HPy term_var = HPyField_Load( ctx , item , term->variable );
        Variable* pyvar = Variable::AsStruct( ctx, term_var );
        result += term->coefficient * pyvar->variable.value();
        HPy_Close( ctx , term_var );
        HPy_Close( ctx , item );
    }
    HPy_Close( ctx , expr_terms );
    return HPyFloat_FromDouble( ctx, result );
}


static HPy
Expression_add( HPyContext *ctx, HPy first, HPy second )
{
    return BinaryInvoke<BinaryAdd, Expression>()( ctx, first, second );
}


static HPy
Expression_sub( HPyContext *ctx, HPy first, HPy second )
{
    return BinaryInvoke<BinarySub, Expression>()( ctx, first, second );
}


static HPy
Expression_mul( HPyContext *ctx, HPy first, HPy second )
{
    return BinaryInvoke<BinaryMul, Expression>()( ctx, first, second );
}


static HPy
Expression_div( HPyContext *ctx, HPy first, HPy second )
{
    return BinaryInvoke<BinaryDiv, Expression>()( ctx, first, second );
}


static HPy
Expression_neg( HPyContext *ctx, HPy value )
{
    return UnaryInvoke<UnaryNeg, Expression>()( ctx, value );
}


static HPy
Expression_richcmp( HPyContext *ctx, HPy first, HPy second, HPy_RichCmpOp op )
{
    switch( op )
    {
        case HPy_EQ:
            return BinaryInvoke<CmpEQ, Expression>()( ctx, first, second );
        case HPy_LE:
            return BinaryInvoke<CmpLE, Expression>()( ctx, first, second );
        case HPy_GE:
            return BinaryInvoke<CmpGE, Expression>()( ctx, first, second );
        default:
            break;
    }
    // PyErr_Format(
    //     PyExc_TypeError,
    //     "unsupported operand type(s) for %s: "
    //     "'%.100s' and '%.100s'",
    //     pyop_str( op ),
    //     Py_TYPE( first )->tp_name,
    //     Py_TYPE( second )->tp_name
    // );
    HPyErr_SetString( ctx, ctx->h_TypeError, "unsupported operand type(s)" );
    return HPy_NULL;
}


HPyDef_METH(Expression_terms_def, "terms", Expression_terms, HPyFunc_NOARGS,
    .doc = "Get the tuple of terms for the expression.")
HPyDef_METH(Expression_constant_def, "constant", Expression_constant, HPyFunc_NOARGS,
    .doc = "Get the constant for the expression.")
HPyDef_METH(Expression_value_def, "value", Expression_value, HPyFunc_NOARGS,
    .doc = "Get the value for the expression.")


HPyDef_SLOT(Expression_traverse_def, Expression_traverse, HPy_tp_traverse)  /* tp_traverse */
HPyDef_SLOT(Expression_repr_def, Expression_repr, HPy_tp_repr)              /* tp_repr */
HPyDef_SLOT(Expression_richcmp_def, Expression_richcmp, HPy_tp_richcompare) /* tp_richcompare */
HPyDef_SLOT(Expression_new_def, Expression_new, HPy_tp_new)                 /* tp_new */
HPyDef_SLOT(Expression_add_def, Expression_add, HPy_nb_add)                 /* nb_add */
HPyDef_SLOT(Expression_sub_def, Expression_sub, HPy_nb_subtract)            /* nb_sub */
HPyDef_SLOT(Expression_mul_def, Expression_mul, HPy_nb_multiply)            /* nb_mul */
HPyDef_SLOT(Expression_neg_def, Expression_neg, HPy_nb_negative)            /* nb_neg */
HPyDef_SLOT(Expression_div_def, Expression_div, HPy_nb_true_divide)         /* nb_div */

static HPyDef* Expression_defines[] = {
    // slots
    &Expression_traverse_def,
    &Expression_repr_def,
    &Expression_richcmp_def,
    &Expression_new_def,
    &Expression_add_def,
    &Expression_sub_def,
    &Expression_mul_def,
    &Expression_neg_def,
    &Expression_div_def,

    // methods
    &Expression_terms_def,
    &Expression_constant_def,
    &Expression_value_def,
    NULL
};
} // namespace


// Declare static variables (otherwise the compiler eliminates them)
HPyGlobal Expression::TypeObject;


HPyType_Spec Expression::TypeObject_Spec = {
	.name = "kiwisolver.Expression",
	.basicsize = sizeof( Expression ),
	.itemsize = 0,
	.flags = HPy_TPFLAGS_DEFAULT | HPy_TPFLAGS_HAVE_GC | HPy_TPFLAGS_BASETYPE,
    .defines = Expression_defines
};


bool Expression::Ready( HPyContext *ctx, HPy m )
{
    return add_type( ctx , m , &TypeObject , "Expression" , &TypeObject_Spec );
}

}  // namesapce kiwisolver
