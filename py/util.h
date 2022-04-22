/*-----------------------------------------------------------------------------
| Copyright (c) 2013-2019, Nucleic Development Team.
| Copyright (c) 2022, Oracle and/or its affiliates.
|
| Distributed under the terms of the Modified BSD License.
|
| The full license is in the file LICENSE, distributed with this software.
|----------------------------------------------------------------------------*/
#pragma once
#include <map>
#include <string>
#include <kiwi/kiwi.h>
#include "types.h"


namespace kiwisolver
{

inline bool
convert_to_double( HPyContext *ctx, HPy obj, double& out )
{
    if( HPyFloat_Check( ctx, obj ) )
    {
        out = HPyFloat_AsDouble( ctx, obj );
        return true;
    }
    if( HPyLong_Check( ctx, obj ) )
    {
        out = HPyLong_AsDouble( ctx, obj );
        if( out == -1.0 && HPyErr_Occurred(ctx) )
            return false;
        return true;
    }
    // PyErr_Format(
    //     PyExc_TypeError,
    //     "Expected object of type `%s`. Got object of type `%s` instead.",
    //     expected,
    //     ob->ob_type->tp_name );
    HPyErr_SetString( ctx, ctx->h_TypeError, "Expected object of type `float, int, or long`." );
    return false;
}


inline bool
convert_pystr_to_str( HPyContext *ctx, HPy value, std::string& out )
{
    out = HPyUnicode_AsUTF8AndSize( ctx, value, NULL );
    return true;
}


inline bool
convert_to_strength( HPyContext *ctx, HPy value, double& out )
{
    if( HPyUnicode_Check( ctx, value ) )
    {
        std::string str;
        if( !convert_pystr_to_str( ctx, value, str ) )
            return false;
        if( str == "required" )
            out = kiwi::strength::required;
        else if( str == "strong" )
            out = kiwi::strength::strong;
        else if( str == "medium" )
            out = kiwi::strength::medium;
        else if( str == "weak" )
            out = kiwi::strength::weak;
        else
        {
            // PyErr_Format(
            //     PyExc_ValueError,
            //     "string strength must be 'required', 'strong', 'medium', "
            //     "or 'weak', not '%s'",
            //     str.c_str()
            // );
            HPyErr_SetString(ctx,
                ctx->h_ValueError,
                "string strength must be 'required', 'strong', 'medium', "
                "or 'weak'"
            );
            return false;
        }
        return true;
    }
    if( !convert_to_double( ctx, value, out ) )
        return false;
    return true;
}


inline bool
convert_to_relational_op( HPyContext *ctx, HPy value, kiwi::RelationalOperator& out )
{
    if( !HPyUnicode_Check( ctx, value ) )
    {
        // PyErr_Format(
        //     PyExc_TypeError,
        //     "Expected object of type `%s`. Got object of type `%s` instead.",
        //     expected,
        //     ob->ob_type->tp_name );
        HPyErr_SetString(ctx, ctx->h_TypeError, "Expected object of type `str`." );
        return false;
    }
    std::string str;
    if( !convert_pystr_to_str( ctx, value, str ) )
        return false;
    if( str == "==" )
        out = kiwi::OP_EQ;
    else if( str == "<=" )
        out = kiwi::OP_LE;
    else if( str == ">=" )
        out = kiwi::OP_GE;
    else
    {
        // PyErr_Format(
        //     PyExc_ValueError,
        //     "relational operator must be '==', '<=', or '>=', not '%s'",
        //     str.c_str()
        // );
        HPyErr_SetString(ctx, ctx->h_ValueError, "relational operator must be '==', '<=', or '>='" );
        return false;
    }
    return true;
}


inline HPy
make_terms( HPyContext *ctx, const std::map<HPy*, double>& coeffs )
{
    typedef std::map<HPy*, double>::const_iterator iter_t;
    HPy_ssize_t size = coeffs.size();
    HPyTupleBuilder terms = HPyTupleBuilder_New( ctx, size );
    // for( HPy_ssize_t i = 0; i < size; ++i ) // zero tuple for safe early return
    //     HPyTupleBuilder_Set( ctx, terms, i, 0 );
    HPy_ssize_t i = 0;
    iter_t it = coeffs.begin();
    iter_t end = coeffs.end();
    for( ; it != end; ++it, ++i )
    {
        Term* term;
        HPy pyterm = HPy_New(ctx, Term::TypeObject, &term);
        term->variable = HPy_Dup( ctx, *(it->first) );
        term->coefficient = it->second;
        HPyTupleBuilder_Set( ctx, terms, i, pyterm );
    }
    return HPyTupleBuilder_Build( ctx, terms );
}


inline HPy
reduce_expression( HPyContext *ctx, HPy pyexpr )  // pyexpr must be an Expression
{
    Expression* expr = Expression::AsStruct( ctx, pyexpr );
    std::map<HPy*, double> coeffs;
    HPy expr_terms = HPyField_Load(ctx, pyexpr, expr->terms);
    HPy_ssize_t size = HPy_Length( ctx, expr_terms );
    for( HPy_ssize_t i = 0; i < size; ++i )
    {
        HPy item = HPy_GetItem_i( ctx, expr_terms, i );
        Term* term = Term::AsStruct( ctx, item );
        coeffs[ &term->variable ] += term->coefficient;
    }
    HPy terms = make_terms( ctx, coeffs );
    if( HPy_IsNull(terms) )
        return HPy_NULL;
    Expression* newexpr;
    HPy pynewexpr = HPy_New(ctx, Expression::TypeObject, &newexpr);
    if( HPy_IsNull(pynewexpr) )
        return HPy_NULL;
    HPyField_Store(ctx, pynewexpr, &newexpr->terms, terms);
    newexpr->constant = expr->constant;
    return pynewexpr;
}


inline kiwi::Expression
convert_to_kiwi_expression( HPyContext *ctx, HPy pyexpr )  // pyexpr must be an Expression
{
    Expression* expr = Expression::AsStruct( ctx, pyexpr );
    std::vector<kiwi::Term> kterms;
    HPy expr_terms = HPyField_Load(ctx, pyexpr, expr->terms);
    HPy_ssize_t size = HPy_Length( ctx, expr_terms );
    for( HPy_ssize_t i = 0; i < size; ++i )
    {
        HPy item = HPy_GetItem_i( ctx, expr_terms, i );
        Term* term = Term::AsStruct( ctx, item );
        Variable* var = Variable::AsStruct( ctx, term->variable );
        kterms.push_back( kiwi::Term( var->variable, term->coefficient ) );
    }
    return kiwi::Expression( kterms, expr->constant );
}


inline const char*
pyop_str( int op )
{
    switch( op )
    {
        case HPy_LT:
            return "<";
        case HPy_LE:
            return "<=";
        case HPy_EQ:
            return "==";
        case HPy_NE:
            return "!=";
        case HPy_GT:
            return ">";
        case HPy_GE:
            return ">=";
        default:
            return "";
    }
}


}  // namespace kiwisolver
