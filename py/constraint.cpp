/*-----------------------------------------------------------------------------
| Copyright (c) 2013-2019, Nucleic Development Team.
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

static HPy
Constraint_new(HPyContext *ctx, HPy type, HPy* args, HPy_ssize_t nargs, HPy kwargs)
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
        HPyTracker_Close(ctx, ht);
        return HPy_NULL;
    }
    HPyField_Store(ctx, pycn, &cn->expression, red_expr);
    kiwi::Expression expr(convert_to_kiwi_expression(ctx, red_expr));
    new (&cn->constraint) kiwi::Constraint(expr, op, strength);
    return pycn;
}

// static void Constraint_clear(HPyContext *ctx, Constraint *self) TODO
// {
//     HPy_CLEAR(ctx, self->expression);
// }

static int Constraint_traverse(void *obj, HPyFunc_visitproc visit, void *arg)
{
    Constraint *self = (Constraint *)obj;
    HPy_VISIT(&self->expression);
// #if PY_VERSION_HEX >= 0x03090000
    // This was not needed before Python 3.9 (Python issue 35810 and 40217)
    // Py_VISIT(Py_TYPE(self)); TODO
// #endif
    return 0;
}

static void Constraint_dealloc(void *obj)
{
    // PyObject_GC_UnTrack(self);
    // Constraint_clear(self);
    // self->constraint.~Constraint();
    // Py_TYPE(self)->tp_free(pyobject_cast(self));
}

static HPy 
Constraint_repr(HPyContext *ctx, HPy h_self)
{
    Constraint* self = Constraint_AsStruct(ctx, h_self);
    std::stringstream stream;
    Expression *expr = Expression::AsStruct(ctx, HPyField_Load(ctx, h_self, self->expression));
    HPy_ssize_t size = HPy_Length(ctx, expr->terms);
    for (HPy_ssize_t i = 0; i < size; ++i)
    {
        HPy item = HPy_GetItem_i(ctx, expr->terms, i);
        Term *term = Term::AsStruct(ctx, item);
        stream << term->coefficient << " * ";
        stream << Variable::AsStruct(ctx, term->variable)->variable.name();
        stream << " + ";
    }
    stream << expr->constant;
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

static HPy 
Constraint_expression(HPyContext *ctx, HPy h_self)
{
    Constraint* self = Constraint_AsStruct(ctx, h_self);
    return HPy_Dup(ctx, HPyField_Load(ctx, h_self, self->expression));
}

static HPy 
Constraint_op(HPyContext *ctx, HPy h_self)
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

static HPy 
Constraint_strength(HPyContext *ctx, HPy h_self)
{
    Constraint* self = Constraint::AsStruct(ctx, h_self);
    return HPyFloat_FromDouble(ctx, self->constraint.strength());
}

static HPy 
Constraint_or(HPyContext *ctx, HPy pyoldcn, HPy value)
{
    if (!Constraint::TypeCheck(ctx, pyoldcn))
        std::swap(pyoldcn, value);
    double strength;
    if (!convert_to_strength(ctx, value, strength))
        return HPy_NULL;
    Constraint *newcn;
    HPy pynewcn = HPy_New(ctx, Constraint::TypeObject, &newcn);
    if (HPy_IsNull(pynewcn))
        return HPy_NULL;
    Constraint *oldcn = Constraint::AsStruct(ctx, pyoldcn);
    HPyField_Store(ctx, pynewcn, &newcn->expression, HPyField_Load(ctx, pyoldcn, oldcn->expression));
    new (&newcn->constraint) kiwi::Constraint(oldcn->constraint, strength);
    return pynewcn;
}


HPyDef_METH(Constraint_expression_def, "expression", Constraint_expression, HPyFunc_NOARGS,
.doc = "Get the expression object for the constraint.")
HPyDef_METH(Constraint_op_def, "op", Constraint_op, HPyFunc_NOARGS,
.doc = "Get the relational operator for the constraint.")
HPyDef_METH(Constraint_strength_def, "strength", Constraint_strength, HPyFunc_NOARGS,
.doc = "Get the strength for the constraint.")


HPyDef_SLOT(Constraint_dealloc_def, Constraint_dealloc, HPy_tp_destroy)
HPyDef_SLOT(Constraint_traverse_def, Constraint_traverse, HPy_tp_traverse)
HPyDef_SLOT(Constraint_repr_def, Constraint_repr, HPy_tp_repr)
HPyDef_SLOT(Constraint_new_def, Constraint_new, HPy_tp_new)
HPyDef_SLOT(Constraint_or_def, Constraint_or, HPy_nb_or)
// HPyDef_SLOT(Constraint_clear_def, Constraint_clear, HPy_tp_clear)       TODO /* tp_clear */

static HPyDef* Constraint_defines[] = {
    // slots
    &Constraint_dealloc_def,
    &Constraint_traverse_def,
    &Constraint_repr_def,
    &Constraint_new_def,
    &Constraint_or_def,
    // &Constraint_clear_def,

    // methods
    &Constraint_expression_def,
    &Constraint_op_def,
    &Constraint_strength_def,
    NULL
};
} // namespace

// Initialize static variables (otherwise the compiler eliminates them)
HPy Constraint::TypeObject = HPy_NULL;

HPyType_Spec Constraint::TypeObject_Spec = {
    .name = "kiwisolver.Constraint", /* tp_name */
    .basicsize = sizeof(Constraint),      /* tp_basicsize */
    .itemsize = 0,                       /* tp_itemsize */
    .flags = HPy_TPFLAGS_DEFAULT /* | HPy_TPFLAGS_HAVE_GC */ | HPy_TPFLAGS_BASETYPE,
    .defines = Constraint_defines    /* slots */
};

bool Constraint::Ready(HPyContext *ctx, HPy m)
{
    // The reference will be handled by the module to which we will add the type
    if (!HPyHelpers_AddType(ctx, m, "Constraint", &TypeObject_Spec, NULL)) {
        return false;
    }

    TypeObject = HPy_GetAttr_s(ctx, m, "Constraint");
    if( HPy_IsNull(TypeObject) )
    {
        return false;
    }
    return true;
}

} // namespace kiwisolver
