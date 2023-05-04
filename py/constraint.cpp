/*-----------------------------------------------------------------------------
| Copyright (c) 2013-2019, Nucleic Development Team.
| Copyright (c) 2022-2023, Oracle and/or its affiliates.
|
| Distributed under the terms of the Modified BSD License.
|
| The full license is in the file LICENSE, distributed with this software.
|----------------------------------------------------------------------------*/
#include <algorithm>
#include <sstream>
#include <kiwi/kiwi.h>
#include "types.h"
#include "util.h"

namespace kiwisolver
{

namespace
{

HPyDef_SLOT(Constraint_new, HPy_tp_new)
static HPy
Constraint_new_impl(HPyContext *ctx, HPy type, const HPy *args, HPy_ssize_t nargs, HPy kwargs)
{
    static const char *kwlist[] = {"expression", "op", "strength", 0};
    HPy pyexpr;
    HPy pyop;
    HPy pystrength = HPy_NULL;
    HPyTracker ht;
    if (!HPyArg_ParseKeywords(ctx, &ht, args, nargs,
            kwargs, "OO|O:__new__", (const char **)kwlist,
            &pyexpr, &pyop, &pystrength))
        return HPy_NULL;
    if (!Expression::TypeCheck(ctx, pyexpr)){
        // PyErr_Format(
        //     PyExc_TypeError,
        //     "Expected object of type `Expression`. Got object of type `%s` instead.",
        //     ob->ob_type->tp_name );
        HPyTracker_Close( ctx , ht );
        HPyErr_SetString( ctx, ctx->h_TypeError, "Expected object of type `Expression`." );
        return HPy_NULL;
    }
    kiwi::RelationalOperator op;
    if (!convert_to_relational_op(ctx, pyop, op)) {
        HPyTracker_Close(ctx, ht);
        return HPy_NULL;
    }
    double strength = kiwi::strength::required;
    if (!HPy_IsNull(pystrength) && !convert_to_strength(ctx, pystrength, strength)) {
        HPyTracker_Close(ctx, ht);
        return HPy_NULL;
    }
    Constraint *cn;
    HPy pycn = HPy_New(ctx, type, &cn);
    if (HPy_IsNull(pycn)) {
        HPyTracker_Close(ctx, ht);
        return HPy_NULL;
    }
    HPy red_expr = reduce_expression(ctx, pyexpr);
    if (HPy_IsNull(red_expr)) {
        HPy_Close( ctx , pycn );
        HPyTracker_Close(ctx, ht);
        return HPy_NULL;
    }
    HPyField_Store(ctx, pycn, &cn->expression, red_expr);
    kiwi::Expression expr(convert_to_kiwi_expression(ctx, red_expr));
    HPyTracker_Close( ctx , ht );
    HPy_Close( ctx , red_expr );
    new (&cn->constraint) kiwi::Constraint(expr, op, strength);
    return pycn;
}

HPyDef_SLOT(Constraint_traverse, HPy_tp_traverse)
static int Constraint_traverse_impl(void *obj, HPyFunc_visitproc visit, void *arg)
{
    Constraint *self = (Constraint *)obj;
    HPy_VISIT(&self->expression);
    return 0;
}

HPyDef_SLOT(Constraint_dealloc, HPy_tp_destroy)
static void Constraint_dealloc_impl(void *data)
{
    Constraint* self = (Constraint *)data;
    self->constraint.~Constraint();
}

HPyDef_SLOT(Constraint_repr, HPy_tp_repr)
static HPy 
Constraint_repr_impl(HPyContext *ctx, HPy h_self)
{
    Constraint* self = Constraint_AsStruct(ctx, h_self);
    std::stringstream stream;
    HPy h_expr = HPyField_Load(ctx, h_self, self->expression);
    Expression *expr = Expression_AsStruct(ctx, h_expr);
    double expr_constant = expr->constant;
    HPy expr_terms = HPyField_Load(ctx, h_expr, expr->terms);
    HPy_ssize_t size = HPy_Length(ctx, expr_terms);
    HPy_Close( ctx , h_expr );
    for (HPy_ssize_t i = 0; i < size; ++i)
    {
        HPy item = HPy_GetItem_i(ctx, expr_terms, i);
        if ( HPy_IsNull( item ) )
        {
            HPy_Close( ctx , expr_terms );
            return HPy_NULL;
        }
        Term *term = Term::AsStruct(ctx, item);
        stream << term->coefficient << " * ";
        HPy var = HPyField_Load( ctx , item , term->variable );
        stream << Variable::AsStruct(ctx, var)->variable.name();
        stream << " + ";
        HPy_Close( ctx , var );
        HPy_Close( ctx , item );
    }
    HPy_Close( ctx , expr_terms );
    stream << expr_constant;
    switch (self->constraint.op())
    {
    case kiwi::OP_EQ:
        stream << " == 0";
        break;
    case kiwi::OP_LE:
        stream << " <= 0";
        break;
    case kiwi::OP_GE:
        stream << " >= 0";
        break;
    }
    stream << " | strength = " << self->constraint.strength();
    return HPyUnicode_FromString(ctx, stream.str().c_str());
}

HPyDef_METH(Constraint_expression, "expression", HPyFunc_NOARGS,
        .doc = "Get the expression object for the constraint.")
static HPy 
Constraint_expression_impl(HPyContext *ctx, HPy h_self)
{
    Constraint* self = Constraint_AsStruct(ctx, h_self);
    return HPyField_Load(ctx, h_self, self->expression);
}

HPyDef_METH(Constraint_op, "op", HPyFunc_NOARGS,
        .doc = "Get the relational operator for the constraint.")
static HPy 
Constraint_op_impl(HPyContext *ctx, HPy h_self)
{
    Constraint* self = Constraint_AsStruct(ctx, h_self);
    HPy res = HPy_NULL;
    switch (self->constraint.op())
    {
    case kiwi::OP_EQ:
        res = HPyUnicode_FromString(ctx, "==");
        break;
    case kiwi::OP_LE:
        res = HPyUnicode_FromString(ctx, "<=");
        break;
    case kiwi::OP_GE:
        res = HPyUnicode_FromString(ctx, ">=");
        break;
    }
    return res;
}

HPyDef_METH(Constraint_strength, "strength", HPyFunc_NOARGS,
        .doc = "Get the strength for the constraint.")
static HPy 
Constraint_strength_impl(HPyContext *ctx, HPy h_self)
{
    Constraint* self = Constraint_AsStruct(ctx, h_self);
    return HPyFloat_FromDouble(ctx, self->constraint.strength());
}

HPyDef_SLOT(Constraint_or, HPy_nb_or)
static HPy 
Constraint_or_impl(HPyContext *ctx, HPy pyoldcn, HPy value)
{
    if (!Constraint::TypeCheck(ctx, pyoldcn))
        std::swap(pyoldcn, value);
    double strength;
    if (!convert_to_strength(ctx, value, strength))
        return HPy_NULL;
    Constraint *newcn;
    HPy pynewcn = new_from_global(ctx, Constraint::TypeObject, &newcn);
    if (HPy_IsNull(pynewcn))
        return HPy_NULL;
    Constraint *oldcn = Constraint_AsStruct(ctx, pyoldcn);
    HPy pyoldcn_expr = HPyField_Load(ctx, pyoldcn, oldcn->expression);
    HPyField_Store(ctx, pynewcn, &newcn->expression, pyoldcn_expr);
    HPy_Close( ctx , pyoldcn_expr );
    new (&newcn->constraint) kiwi::Constraint(oldcn->constraint, strength);
    return pynewcn;
}

static HPyDef* Constraint_defines[] = {
    // slots
    &Constraint_dealloc,
    &Constraint_traverse,
    &Constraint_repr,
    &Constraint_new,
    &Constraint_or,
    // &Constraint_clear,

    // methods
    &Constraint_expression,
    &Constraint_op,
    &Constraint_strength,
    NULL
};
} // namespace

// Declare static variables (otherwise the compiler eliminates them)
HPyGlobal Constraint::TypeObject;

HPyType_Spec Constraint::TypeObject_Spec = {
    .name = "kiwisolver.Constraint", /* tp_name */
    .basicsize = sizeof(Constraint),      /* tp_basicsize */
    .itemsize = 0,                       /* tp_itemsize */
    .flags = HPy_TPFLAGS_DEFAULT | HPy_TPFLAGS_HAVE_GC | HPy_TPFLAGS_BASETYPE,
    .defines = Constraint_defines    /* slots */
};

bool Constraint::Ready(HPyContext *ctx, HPy m)
{
    return add_type( ctx , m , &TypeObject , "Constraint" , &TypeObject_Spec );
}

} // namespace kiwisolver
