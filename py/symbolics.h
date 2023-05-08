/*-----------------------------------------------------------------------------
| Copyright (c) 2013-2019, Nucleic Development Team.
| Copyright (c) 2022-2023, Oracle and/or its affiliates.
|
| Distributed under the terms of the Modified BSD License.
|
| The full license is in the file LICENSE, distributed with this software.
|----------------------------------------------------------------------------*/
#pragma once
#include "types.h"
#include "util.h"


namespace kiwisolver
{

template<typename Op, typename T>
struct UnaryInvoke
{
	HPy operator()( HPyContext *ctx, HPy value )
	{
		return Op()( ctx, T::AsStruct(ctx, value ), value );
	}
};


template<typename Op, typename T>
struct BinaryInvoke
{
	HPy operator()( HPyContext *ctx, HPy first, HPy second )
	{
		if( T::TypeCheck( ctx, first ) )
			return invoke<Normal>( ctx, T::AsStruct( ctx, first ), second, first, second );
		return invoke<Reverse>( ctx, T::AsStruct( ctx, second ), first, second, first );
	}

	struct Normal
	{
		template<typename U>
		HPy operator()( HPyContext *ctx, T* primary, U secondary, HPy h_primary, HPy h_secondary )
		{
			return Op()( ctx, primary, secondary, h_primary, h_secondary );
		}
	};

	struct Reverse
	{
		template<typename U>
		HPy operator()( HPyContext *ctx, T* primary, U secondary, HPy h_primary, HPy h_secondary )
		{
			return Op()( ctx, secondary, primary, h_secondary, h_primary );
		}
	};

	template<typename Invk>
	HPy invoke( HPyContext *ctx, T* primary, HPy secondary, HPy h_primary, HPy h_secondary )
	{
		if( Expression::TypeCheck( ctx, secondary ) )
			return Invk()( ctx, primary, Expression_AsStruct( ctx, secondary ), h_primary, h_secondary );
		if( Term::TypeCheck( ctx, secondary ) )
			return Invk()( ctx, primary, Term_AsStruct( ctx, secondary ), h_primary, h_secondary );
		if( Variable::TypeCheck( ctx, secondary ) )
			return Invk()( ctx, primary, Variable_AsStruct( ctx, secondary ), h_primary, h_secondary );
		if( HPyFloat_Check( ctx, secondary ) )
			return Invk()( ctx, primary, HPyFloat_AsDouble( ctx, secondary ), h_primary, h_secondary );
		if( HPyLong_Check( ctx, secondary ) )
		{
			double v = HPyLong_AsDouble( ctx, secondary );
			if( v == -1 && HPyErr_Occurred(ctx) )
				return HPy_NULL;
			return Invk()( ctx, primary, v, h_primary, HPy_NULL );
		}
		HPy_RETURN_NOTIMPLEMENTED( ctx );
	}
};


struct BinaryMul
{
	template<typename T, typename U>
	HPy operator()( HPyContext *ctx, T first, U second, HPy h_first, HPy h_second )
	{
		HPy_RETURN_NOTIMPLEMENTED( ctx );
	}
};


template<> inline
HPy BinaryMul::operator()( HPyContext *ctx, Variable* first, double second, HPy h_first, HPy h_second )
{
	Term* term;
	HPy pyterm =  new_from_global( ctx, Term::TypeObject, &term );
	if( HPy_IsNull(pyterm) )
		return HPy_NULL;
	HPyField_Store( ctx , pyterm , &term->variable , h_first );
	term->coefficient = second;
	return pyterm;
}


template<> inline
HPy BinaryMul::operator()( HPyContext *ctx, Term* first, double second, HPy h_first, HPy h_second )
{
	Term* term;
	HPy pyterm =  new_from_global( ctx, Term::TypeObject, &term );
	if( HPy_IsNull(pyterm) )
		return HPy_NULL;
	HPy first_var = HPyField_Load( ctx , h_first , first->variable );
	HPyField_Store( ctx , pyterm , &term->variable , first_var );
	HPy_Close( ctx , first_var );
	term->coefficient = first->coefficient * second;
	return pyterm;
}


template<> inline
HPy BinaryMul::operator()( HPyContext *ctx, Expression* first, double second, HPy h_first, HPy h_second )
{
	Expression* expr;
	HPy pyexpr =  new_from_global( ctx, Expression::TypeObject, &expr );
	if( HPy_IsNull(pyexpr) )
		return HPy_NULL;
	HPy first_terms = HPyField_Load(ctx, h_first, first->terms);
	HPy_ssize_t end = HPy_Length( ctx, first_terms );
	HPyTupleBuilder terms = HPyTupleBuilder_New( ctx, end );
	for( HPy_ssize_t i = 0; i < end; ++i )
	{
		HPy item = HPy_GetItem_i( ctx, first_terms, i );
		if( HPy_IsNull(item) ) {
			HPyTupleBuilder_Cancel( ctx , terms );
			HPy_Close( ctx , first_terms );
			HPy_Close( ctx , pyexpr );
			return HPy_NULL;
		}
		HPy term = BinaryMul()( ctx, Term_AsStruct( ctx, item ), second, item, h_second );
		HPy_Close( ctx , item );
		if( HPy_IsNull(term) ) {
			HPyTupleBuilder_Cancel( ctx , terms );
			HPy_Close( ctx , first_terms );
			HPy_Close( ctx , pyexpr );
			return HPy_NULL;
		}
		HPyTupleBuilder_Set( ctx , terms , i , term );
		HPy_Close( ctx , term );
	}
	HPy_Close( ctx , first_terms );
	HPy terms_tuple = HPyTupleBuilder_Build( ctx, terms );
	if ( HPy_IsNull( terms_tuple ) )
	{
		HPy_Close( ctx , pyexpr );
		return HPy_NULL;
	}

	HPyField_Store(ctx, pyexpr, &expr->terms, terms_tuple);
	HPy_Close( ctx , terms_tuple );
	expr->constant = first->constant * second;
	return pyexpr;
}


template<> inline
HPy BinaryMul::operator()( HPyContext *ctx, double first, Variable* second, HPy h_first, HPy h_second )
{
	return operator()( ctx, second, first, h_second, h_first );
}


template<> inline
HPy BinaryMul::operator()( HPyContext *ctx, double first, Term* second, HPy h_first, HPy h_second )
{
	return operator()( ctx, second, first, h_second, h_first );
}


template<> inline
HPy BinaryMul::operator()( HPyContext *ctx, double first, Expression* second, HPy h_first, HPy h_second )
{
	return operator()( ctx, second, first, h_second, h_first );
}


struct BinaryDiv
{
	template<typename T, typename U>
	HPy operator()( HPyContext *ctx, T first, U second, HPy h_first, HPy h_second )
	{
		HPy_RETURN_NOTIMPLEMENTED( ctx );
	}
};


template<> inline
HPy BinaryDiv::operator()( HPyContext *ctx, Variable* first, double second, HPy h_first, HPy h_second )
{
	if( second == 0.0 )
	{
		HPyErr_SetString( ctx, ctx->h_ZeroDivisionError, "float division by zero" );
		return HPy_NULL;
	}
	return BinaryMul()( ctx, first, 1.0 / second, h_first, HPy_NULL );
}


template<> inline
HPy BinaryDiv::operator()( HPyContext *ctx, Term* first, double second, HPy h_first, HPy h_second )
{
	if( second == 0.0 )
	{
		HPyErr_SetString( ctx, ctx->h_ZeroDivisionError, "float division by zero" );
		return HPy_NULL;
	}
	return BinaryMul()( ctx, first, 1.0 / second, h_first, HPy_NULL );
}


template<> inline
HPy BinaryDiv::operator()( HPyContext *ctx, Expression* first, double second, HPy h_first, HPy h_second )
{
	if( second == 0.0 )
	{
		HPyErr_SetString( ctx, ctx->h_ZeroDivisionError, "float division by zero" );
		return HPy_NULL;
	}
	return BinaryMul()( ctx, first, 1.0 / second, h_first, HPy_NULL );
}


struct UnaryNeg
{
	template<typename T>
	HPy operator()( HPyContext *ctx, T value, HPy h_value )
	{
		HPy_RETURN_NOTIMPLEMENTED( ctx );
	}
};


template<> inline
HPy UnaryNeg::operator()( HPyContext *ctx, Variable* value, HPy h_value )
{
	return BinaryMul()( ctx, value, -1.0, h_value, HPy_NULL );
}


template<> inline
HPy UnaryNeg::operator()( HPyContext *ctx, Term* value, HPy h_value )
{
	return BinaryMul()( ctx, value, -1.0, h_value, HPy_NULL );
}


template<> inline
HPy UnaryNeg::operator()( HPyContext *ctx, Expression* value, HPy h_value )
{
	return BinaryMul()( ctx, value, -1.0, h_value, HPy_NULL );
}


struct BinaryAdd
{
	template<typename T, typename U>
	HPy operator()( HPyContext *ctx, T first, U second, HPy h_first, HPy h_second )
	{
		HPy_RETURN_NOTIMPLEMENTED( ctx );
	}
};


template<> inline
HPy BinaryAdd::operator()( HPyContext *ctx, Expression* first, Expression* second, HPy h_first, HPy h_second )
{
	Expression* expr;
	HPy pyexpr = new_from_global( ctx, Expression::TypeObject, &expr );
	if( HPy_IsNull(pyexpr) )
		return HPy_NULL;
	expr->constant = first->constant + second->constant;

	HPy first_terms = HPyField_Load(ctx, h_first, first->terms);
	HPy_ssize_t first_len = HPy_Length( ctx, first_terms );
	HPy second_terms = HPyField_Load(ctx, h_second, second->terms);
	HPy_ssize_t second_len = HPy_Length( ctx, second_terms );
	HPyTupleBuilder terms = HPyTupleBuilder_New( ctx, first_len + second_len );

	for( HPy_ssize_t i = 0; i < first_len; ++i )
	{
		HPy item = HPy_GetItem_i( ctx, first_terms, i );
		if ( HPy_IsNull(item) )
		{
			HPy_Close( ctx , pyexpr );
			HPy_Close( ctx , first_terms );
			HPyTupleBuilder_Cancel( ctx , terms );
			return HPy_NULL;
		}
		HPyTupleBuilder_Set( ctx, terms, i, item );
		HPy_Close( ctx, item );
	}
	HPy_Close( ctx , first_terms );

	for( HPy_ssize_t j = 0; j < second_len; ++j )
	{
		HPy item = HPy_GetItem_i( ctx, second_terms, j );
		if ( HPy_IsNull(item) )
		{
			HPy_Close( ctx , pyexpr );
			HPy_Close( ctx , second_terms );
			HPyTupleBuilder_Cancel( ctx , terms );
			return HPy_NULL;
		}
		HPyTupleBuilder_Set( ctx, terms, j + first_len, item );
		HPy_Close( ctx, item );
	}
	HPy_Close( ctx , second_terms );

	HPy terms_tuple = HPyTupleBuilder_Build( ctx, terms );
	if( HPy_IsNull(terms_tuple) )
	{
		HPy_Close( ctx , pyexpr );
		return HPy_NULL;
	}
	HPyField_Store(ctx, pyexpr, &expr->terms, terms_tuple);
	HPy_Close( ctx , terms_tuple );
	return pyexpr;
}


template<> inline
HPy BinaryAdd::operator()( HPyContext *ctx, Expression* first, Term* second, HPy h_first, HPy h_second )
{
	Expression* expr;
	HPy pyexpr =  new_from_global( ctx, Expression::TypeObject, &expr );
	if( HPy_IsNull(pyexpr) )
		return HPy_NULL;
	HPy first_terms = HPyField_Load(ctx, h_first, first->terms);
	HPy_ssize_t end = HPy_Length( ctx, first_terms );
	HPyTupleBuilder terms = HPyTupleBuilder_New( ctx, end + 1 );
	for( HPy_ssize_t i = 0; i < end; ++i )
	{
		HPy item = HPy_GetItem_i( ctx, first_terms, i );
		if( HPy_IsNull(item) ) {
			HPy_Close( ctx , first_terms );
			HPy_Close( ctx , pyexpr );
			HPyTupleBuilder_Cancel( ctx, terms );
			return HPy_NULL;
		}
		HPyTupleBuilder_Set( ctx, terms, i, item );
		HPy_Close( ctx , item );
	}
	HPy_Close( ctx , first_terms );
	HPyTupleBuilder_Set( ctx, terms, end, h_second);
	HPy terms_tuple = HPyTupleBuilder_Build( ctx, terms );
	if ( HPy_IsNull( terms_tuple ) )
	{
		HPy_Close( ctx , pyexpr );
		return HPy_NULL;
	}
	HPyField_Store(ctx, pyexpr, &expr->terms, terms_tuple);
	HPy_Close( ctx , terms_tuple );
	expr->constant = first->constant;
	return pyexpr;
}


template<> inline
HPy BinaryAdd::operator()( HPyContext *ctx, Expression* first, Variable* second, HPy h_first, HPy h_second )
{
	HPy temp = BinaryMul()( ctx, second, 1.0, h_second, HPy_NULL );
	if( HPy_IsNull(temp) )
		return HPy_NULL;
	HPy result = operator()( ctx, first, Term_AsStruct( ctx, temp ), h_first, temp );
	HPy_Close( ctx , temp );
	return result;
}


template<> inline
HPy BinaryAdd::operator()( HPyContext *ctx, Expression* first, double second, HPy h_first, HPy h_second )
{
	Expression* expr;
	HPy pyexpr =  new_from_global( ctx, Expression::TypeObject, &expr );
	if( HPy_IsNull(pyexpr) )
		return HPy_NULL;
	HPy first_terms = HPyField_Load(ctx, h_first, first->terms);
	HPyField_Store(ctx, pyexpr, &expr->terms, first_terms);
	HPy_Close(ctx, first_terms);
	expr->constant = first->constant + second;
	return pyexpr;
}


template<> inline
HPy BinaryAdd::operator()( HPyContext *ctx, Term* first, double second, HPy h_first, HPy h_second )
{
	Expression* expr;
	HPy pyexpr =  new_from_global( ctx, Expression::TypeObject, &expr );
	if( HPy_IsNull(pyexpr) )
		return HPy_NULL;
	expr->constant = second;
	HPy items[] = {
		h_first,
	};
	HPy terms_tuple = HPyTuple_FromArray( ctx, items, 1 );
	if( HPy_IsNull(terms_tuple) )
	{
		HPy_Close( ctx , pyexpr );
		return HPy_NULL;
	}
	HPyField_Store(ctx, pyexpr, &expr->terms, terms_tuple);
	HPy_Close(ctx, terms_tuple);
	return pyexpr;
}


template<> inline
HPy BinaryAdd::operator()( HPyContext *ctx, Term* first, Expression* second, HPy h_first, HPy h_second )
{
	return operator()( ctx, second, first, h_second, h_first );
}


template<> inline
HPy BinaryAdd::operator()( HPyContext *ctx, Term* first, Term* second, HPy h_first, HPy h_second )
{
	Expression* expr;
	HPy pyexpr =  new_from_global( ctx, Expression::TypeObject, &expr );
	if( HPy_IsNull(pyexpr) )
		return HPy_NULL;
	expr->constant = 0.0;
	HPy items[] = {
		h_first,
		h_second
	};
	HPy terms_tuple = HPyTuple_FromArray( ctx, items, 2 );
	if( HPy_IsNull(terms_tuple) )
	{
		HPy_Close( ctx , pyexpr );
		return HPy_NULL;
	}
	HPyField_Store(ctx, pyexpr, &expr->terms, terms_tuple);
	HPy_Close(ctx, terms_tuple);
	return pyexpr;
}


template<> inline
HPy BinaryAdd::operator()( HPyContext *ctx, Term* first, Variable* second, HPy h_first, HPy h_second )
{
	HPy temp = BinaryMul()( ctx, second, 1.0, h_second, HPy_NULL );
	if( HPy_IsNull(temp) )
		return HPy_NULL;
	HPy result = BinaryAdd()( ctx, first, Term_AsStruct( ctx, temp ), h_first, temp );
	HPy_Close( ctx , temp );
	return result;
}


template<> inline
HPy BinaryAdd::operator()( HPyContext *ctx, Variable* first, double second, HPy h_first, HPy h_second )
{
	HPy temp = BinaryMul()( ctx, first, 1.0, h_first, HPy_NULL );
	if( HPy_IsNull(temp) )
		return HPy_NULL;
	HPy result = operator()( ctx, Term_AsStruct( ctx, temp ), second, temp, h_second );
	HPy_Close( ctx , temp );
	return result;
}


template<> inline
HPy BinaryAdd::operator()( HPyContext *ctx, Variable* first, Variable* second, HPy h_first, HPy h_second )
{
	HPy temp = BinaryMul()( ctx, first, 1.0, h_first, HPy_NULL );
	if( HPy_IsNull(temp) )
		return HPy_NULL;
	HPy result = operator()( ctx, Term_AsStruct( ctx, temp ), second, temp, h_second );
	HPy_Close( ctx , temp );
	return result;
}


template<> inline
HPy BinaryAdd::operator()( HPyContext *ctx, Variable* first, Term* second, HPy h_first, HPy h_second )
{
	HPy temp = BinaryMul()( ctx, first, 1.0, h_first, HPy_NULL );
	if( HPy_IsNull(temp) )
		return HPy_NULL;
	HPy result = operator()( ctx, Term_AsStruct( ctx, temp ), second, temp, h_second );
	HPy_Close( ctx , temp );
	return result;
}


template<> inline
HPy BinaryAdd::operator()( HPyContext *ctx, Variable* first, Expression* second, HPy h_first, HPy h_second )
{
	HPy temp = BinaryMul()( ctx, first, 1.0, h_first, HPy_NULL );
	if( HPy_IsNull(temp) )
		return HPy_NULL;
	HPy result = operator()( ctx, Term_AsStruct( ctx, temp ), second, temp, h_second );
	HPy_Close( ctx , temp );
	return result;
}


template<> inline
HPy BinaryAdd::operator()( HPyContext *ctx, double first, Variable* second, HPy h_first, HPy h_second )
{
	return operator()( ctx, second, first, h_second, h_first );
}


template<> inline
HPy BinaryAdd::operator()( HPyContext *ctx, double first, Term* second, HPy h_first, HPy h_second )
{
	return operator()( ctx, second, first, h_second, h_first );
}


template<> inline
HPy BinaryAdd::operator()( HPyContext *ctx, double first, Expression* second, HPy h_first, HPy h_second )
{
	return operator()( ctx, second, first, h_second, h_first );
}


struct BinarySub
{
	template<typename T, typename U>
	HPy operator()( HPyContext *ctx, T first, U second, HPy h_first, HPy h_second )
	{
		HPy_RETURN_NOTIMPLEMENTED( ctx );
	}
};


template<> inline
HPy BinarySub::operator()( HPyContext *ctx, Variable* first, double second, HPy h_first, HPy h_second )
{
	return BinaryAdd()( ctx, first, -second, h_first, HPy_NULL );
}


template<> inline
HPy BinarySub::operator()( HPyContext *ctx, Variable* first, Variable* second, HPy h_first, HPy h_second )
{
	HPy temp = UnaryNeg()( ctx, second, h_second );
	if( HPy_IsNull(temp) )
		return HPy_NULL;
	HPy result = BinaryAdd()( ctx, first, Term_AsStruct( ctx, temp ), h_first, temp );
	HPy_Close( ctx , temp );
	return result;
}


template<> inline
HPy BinarySub::operator()( HPyContext *ctx, Variable* first, Term* second, HPy h_first, HPy h_second )
{
	HPy temp = UnaryNeg()( ctx, second, h_second );
	if( HPy_IsNull(temp) )
		return HPy_NULL;
	HPy result = BinaryAdd()( ctx, first, Term_AsStruct( ctx, temp ), h_first, temp );
	HPy_Close( ctx , temp );
	return result;
}


template<> inline
HPy BinarySub::operator()( HPyContext *ctx, Variable* first, Expression* second, HPy h_first, HPy h_second )
{
	HPy temp = UnaryNeg()( ctx, second, h_second );
	if( HPy_IsNull(temp) )
		return HPy_NULL;
	HPy result = BinaryAdd()( ctx, first, Expression_AsStruct( ctx, temp ), h_first, temp );
	HPy_Close( ctx , temp );
	return result;
}


template<> inline
HPy BinarySub::operator()( HPyContext *ctx, Term* first, double second, HPy h_first, HPy h_second )
{
	return BinaryAdd()( ctx, first, -second, h_first, HPy_NULL );
}


template<> inline
HPy BinarySub::operator()( HPyContext *ctx, Term* first, Variable* second, HPy h_first, HPy h_second )
{
	HPy temp = UnaryNeg()( ctx, second, h_second );
	if( HPy_IsNull(temp) )
		return HPy_NULL;
	HPy result = BinaryAdd()( ctx, first, Term_AsStruct( ctx, temp ), h_first, temp );
	HPy_Close( ctx , temp );
	return result;
}


template<> inline
HPy BinarySub::operator()( HPyContext *ctx, Term* first, Term* second, HPy h_first, HPy h_second )
{
	HPy temp = UnaryNeg()( ctx, second, h_second );
	if( HPy_IsNull(temp) )
		return HPy_NULL;
	HPy result = BinaryAdd()( ctx, first, Term_AsStruct( ctx, temp ), h_first, temp );
	HPy_Close( ctx , temp );
	return result;
}


template<> inline
HPy BinarySub::operator()( HPyContext *ctx, Term* first, Expression* second, HPy h_first, HPy h_second )
{
	HPy temp = UnaryNeg()( ctx, second, h_second );
	if( HPy_IsNull(temp) )
		return HPy_NULL;
	HPy result = BinaryAdd()( ctx, first, Expression_AsStruct( ctx, temp ), h_first, temp );
	HPy_Close( ctx , temp );
	return result;
}


template<> inline
HPy BinarySub::operator()( HPyContext *ctx, Expression* first, double second, HPy h_first, HPy h_second )
{
	return BinaryAdd()( ctx, first, -second, h_first, HPy_NULL );
}


template<> inline
HPy BinarySub::operator()( HPyContext *ctx, Expression* first, Variable* second, HPy h_first, HPy h_second )
{
	HPy temp = UnaryNeg()( ctx, second, h_second );
	if( HPy_IsNull(temp) )
		return HPy_NULL;
	HPy result = BinaryAdd()( ctx, first, Term_AsStruct( ctx, temp ), h_first, temp );
	HPy_Close( ctx , temp );
	return result;
}


template<> inline
HPy BinarySub::operator()( HPyContext *ctx, Expression* first, Term* second, HPy h_first, HPy h_second )
{
	HPy temp = UnaryNeg()( ctx, second, h_second );
	if( HPy_IsNull(temp) )
		return HPy_NULL;
	HPy result = BinaryAdd()( ctx, first, Term_AsStruct( ctx, temp ), h_first, temp );
	HPy_Close( ctx , temp );
	return result;
}


template<> inline
HPy BinarySub::operator()( HPyContext *ctx, Expression* first, Expression* second, HPy h_first, HPy h_second )
{
	HPy temp = UnaryNeg()( ctx, second, h_second );
	if( HPy_IsNull(temp) )
		return HPy_NULL;
	HPy result = BinaryAdd()( ctx, first, Expression_AsStruct( ctx, temp ), h_first, temp );
	HPy_Close( ctx , temp );
	return result;
}


template<> inline
HPy BinarySub::operator()( HPyContext *ctx, double first, Variable* second, HPy h_first, HPy h_second )
{
	HPy temp = UnaryNeg()( ctx, second, h_second );
	if( HPy_IsNull(temp) )
		return HPy_NULL;
	HPy result = BinaryAdd()( ctx, first, Term_AsStruct( ctx, temp ), HPy_NULL, temp );
	HPy_Close( ctx , temp );
	return result;
}


template<> inline
HPy BinarySub::operator()( HPyContext *ctx, double first, Term* second, HPy h_first, HPy h_second )
{
	HPy temp = UnaryNeg()( ctx, second, h_second );
	if( HPy_IsNull(temp) )
		return HPy_NULL;
	HPy result = BinaryAdd()( ctx, first, Term_AsStruct( ctx, temp ), HPy_NULL, temp );
	HPy_Close( ctx , temp );
	return result;
}


template<> inline
HPy BinarySub::operator()( HPyContext *ctx, double first, Expression* second, HPy h_first, HPy h_second )
{
	HPy temp = UnaryNeg()( ctx, second, h_second );
	if( HPy_IsNull(temp) )
		return HPy_NULL;
	HPy result = BinaryAdd()( ctx, first, Expression_AsStruct( ctx, temp ), HPy_NULL, temp );
	HPy_Close( ctx , temp );
	return result;
}


template<typename T, typename U>
HPy makecn( HPyContext *ctx, T first, U second, kiwi::RelationalOperator op, HPy h_first, HPy h_second )
{
	HPy pyexpr = BinarySub()( ctx, first, second, h_first, h_second );
	if( HPy_IsNull(pyexpr) )
		return HPy_NULL;
	HPy constraint_type = HPyGlobal_Load( ctx , Constraint::TypeObject );
	HPy pycn =  HPyType_GenericNew( ctx, constraint_type , NULL, 0, HPy_NULL );
	HPy_Close( ctx , constraint_type );
	if( HPy_IsNull(pycn) )
	{
		HPy_Close( ctx , pyexpr );
		return HPy_NULL;
	}
	Constraint* cn = Constraint_AsStruct( ctx, pycn );
	HPy red_expr = reduce_expression( ctx, pyexpr );
	if( HPy_IsNull(red_expr) )
	{
		HPy_Close( ctx , pyexpr );
		HPy_Close( ctx , pycn );
		return HPy_NULL;
	}
	HPyField_Store(ctx, pycn, &cn->expression, red_expr);
	kiwi::Expression expr( convert_to_kiwi_expression( ctx, red_expr ) );
	HPy_Close( ctx , pyexpr );
	HPy_Close( ctx , red_expr );
	new( &cn->constraint ) kiwi::Constraint( expr, op, kiwi::strength::required );
	return pycn;
}


struct CmpEQ
{
	template<typename T, typename U>
	HPy operator()( HPyContext *ctx, T first, U second, HPy h_first, HPy h_second )
	{
		return makecn( ctx, first, second, kiwi::OP_EQ, h_first, h_second );
	}
};


struct CmpLE
{
	template<typename T, typename U>
	HPy operator()( HPyContext *ctx, T first, U second, HPy h_first, HPy h_second )
	{
		return makecn( ctx, first, second, kiwi::OP_LE, h_first, h_second );
	}
};


struct CmpGE
{
	template<typename T, typename U>
	HPy operator()( HPyContext *ctx, T first, U second, HPy h_first, HPy h_second )
	{
		return makecn( ctx, first, second, kiwi::OP_GE, h_first, h_second );
	}
};


}  // namespace kiwisolver
