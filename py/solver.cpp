/*-----------------------------------------------------------------------------
| Copyright (c) 2013-2019, Nucleic Development Team.
| Copyright (c) 2022, Oracle and/or its affiliates.
|
| Distributed under the terms of the Modified BSD License.
|
| The full license is in the file LICENSE, distributed with this software.
|----------------------------------------------------------------------------*/
#include <kiwi/kiwi.h>
#include "types.h"
#include "util.h"


namespace kiwisolver
{

namespace
{

static HPy
Solver_new( HPyContext *ctx, HPy type, HPy* args, HPy_ssize_t nargs, HPy kwargs )
{
	if( nargs != 0 || ( !HPy_IsNull(kwargs) && HPy_Length( ctx, kwargs ) != 0 ) ) {
		HPyErr_SetString( ctx, ctx->h_TypeError, "Solver.__new__ takes no arguments" );
		return HPy_NULL;
	}
	Solver* self;
	HPy pysolver = HPy_New( ctx, type, &self );
	if( HPy_IsNull( pysolver ) )
		return HPy_NULL;
	new( &self->solver ) kiwi::Solver();
	return pysolver;
}


static void
Solver_dealloc( void *obj )
{
    Solver* self = (Solver*)obj;
	self->solver.~Solver();
	// Py_TYPE( self )->tp_free( pyobject_cast( self ) );
}

static void
setObjectFromGlobal( HPyContext *ctx , HPyGlobal ex_type , HPy obj )
{
	HPy h_ex = HPyGlobal_Load(ctx, ex_type);
	HPyErr_SetObject( ctx, h_ex, obj );
	HPy_Close(ctx, h_ex);
}


static HPy
Solver_addConstraint( HPyContext *ctx, HPy h_self, HPy other )
{
    Solver* self = Solver::AsStruct( ctx, h_self );
	if( !Constraint::TypeCheck( ctx, other ) ) {
		// PyErr_Format(
		//     PyExc_TypeError,
		//     "Expected object of type `Constraint`. Got object of type `%s` instead.",
		//     other->ob_type->tp_name );
		HPyErr_SetString( ctx,
			ctx->h_TypeError,
			"Expected object of type `Constraint`.");
		return HPy_NULL;
	}
	Constraint* cn = Constraint::AsStruct( ctx, other );
	try
	{
		self->solver.addConstraint( cn->constraint );
	}
	catch( const kiwi::DuplicateConstraint& )
	{
		setObjectFromGlobal( ctx, DuplicateConstraint, other );
		return HPy_NULL;
	}
	catch( const kiwi::UnsatisfiableConstraint& )
	{
		setObjectFromGlobal( ctx, UnsatisfiableConstraint, other );
		return HPy_NULL;
	}
	return HPy_Dup( ctx, ctx->h_None );
}


static HPy
Solver_removeConstraint( HPyContext *ctx, HPy h_self, HPy other )
{
    Solver* self = Solver::AsStruct( ctx, h_self );
	if( !Constraint::TypeCheck( ctx, other ) ) {
		// PyErr_Format(
		//     PyExc_TypeError,
		//     "Expected object of type `Constraint`. Got object of type `%s` instead.",
		//     other->ob_type->tp_name );
		HPyErr_SetString( ctx, ctx->h_TypeError, "Expected object of type `Constraint`." );
		return HPy_NULL;
	}
	Constraint* cn = Constraint::AsStruct( ctx, other );
	try
	{
		self->solver.removeConstraint( cn->constraint );
	}
	catch( const kiwi::UnknownConstraint& )
	{
		setObjectFromGlobal( ctx, UnknownConstraint, other );
		return HPy_NULL;
	}
	return HPy_Dup( ctx, ctx->h_None );
}


static HPy
Solver_hasConstraint( HPyContext *ctx, HPy h_self, HPy other )
{
    Solver* self = Solver::AsStruct( ctx, h_self );
	if( !Constraint::TypeCheck( ctx, other ) ) {
		// PyErr_Format(
		//     PyExc_TypeError,
		//     "Expected object of type `Constraint`. Got object of type `%s` instead.",
		//     other->ob_type->tp_name );
		HPyErr_SetString( ctx, ctx->h_TypeError, "Expected object of type `Constraint`." );
		return HPy_NULL;
	}
	Constraint* cn = Constraint::AsStruct( ctx, other );
	return HPy_Dup( ctx, self->solver.hasConstraint( cn->constraint ) ? ctx->h_True : ctx->h_False );
}


static HPy
Solver_addEditVariable( HPyContext *ctx, HPy h_self, HPy* args, HPy_ssize_t nargs )
{
    Solver* self = Solver::AsStruct( ctx, h_self );
	HPy pyvar;
	HPy pystrength;
	if( !HPyArg_Parse(ctx, NULL, args, nargs, "OO", &pyvar, &pystrength ) )
		return HPy_NULL;
	if( !Variable::TypeCheck( ctx, pyvar ) ) {
		// PyErr_Format(
		//     PyExc_TypeError,
		//     "Expected object of type `Variable`. Got object of type `%s` instead.",
		//     ob->ob_type->tp_name );
		HPyErr_SetString( ctx, ctx->h_TypeError, "Expected object of type `Variable`.");
		return HPy_NULL;
	}
	double strength;
	if( !convert_to_strength( ctx, pystrength, strength ) )
		return HPy_NULL;
	Variable* var = Variable::AsStruct( ctx, pyvar );
	try
	{
		self->solver.addEditVariable( var->variable, strength );
	}
	catch( const kiwi::DuplicateEditVariable& )
	{
		setObjectFromGlobal( ctx, DuplicateEditVariable, pyvar );
		return HPy_NULL;
	}
	catch( const kiwi::BadRequiredStrength& e )
	{
		HPy h_ex = HPyGlobal_Load( ctx, BadRequiredStrength );
		HPyErr_SetString( ctx, h_ex, e.what() );
		HPy_Close( ctx , h_ex );
		return HPy_NULL;
	}
	return HPy_Dup( ctx, ctx->h_None );
}


static HPy
Solver_removeEditVariable( HPyContext *ctx, HPy h_self, HPy other )
{
    Solver* self = Solver::AsStruct( ctx, h_self );
	if( !Variable::TypeCheck( ctx, other ) ) {
		// PyErr_Format(
		//     PyExc_TypeError,
		//     "Expected object of type `Variable`. Got object of type `%s` instead.",
		//     other->ob_type->tp_name );
		HPyErr_SetString( ctx, ctx->h_TypeError, "Expected object of type `Variable`." );
		return HPy_NULL;
	}
	Variable* var = Variable::AsStruct( ctx, other );
	try
	{
		self->solver.removeEditVariable( var->variable );
	}
	catch( const kiwi::UnknownEditVariable& )
	{
		setObjectFromGlobal( ctx, UnknownEditVariable, other );
		return HPy_NULL;
	}
	return HPy_Dup( ctx, ctx->h_None );
}


static HPy
Solver_hasEditVariable( HPyContext *ctx, HPy h_self, HPy other )
{
    Solver* self = Solver::AsStruct( ctx, h_self );
	if( !Variable::TypeCheck( ctx, other ) ) {
		// PyErr_Format(
		//     PyExc_TypeError,
		//     "Expected object of type `Variable`. Got object of type `%s` instead.",
		//     other->ob_type->tp_name );
		HPyErr_SetString( ctx, ctx->h_TypeError, "Expected object of type `Variable`." );
		return HPy_NULL;
	}
	Variable* var = Variable::AsStruct( ctx, other );
	return HPy_Dup( ctx, self->solver.hasEditVariable( var->variable ) ? ctx->h_True : ctx->h_False );
}


static HPy
Solver_suggestValue( HPyContext *ctx, HPy h_self, HPy* args, HPy_ssize_t nargs )
{
    Solver* self = Solver::AsStruct( ctx, h_self );
	HPy pyvar;
	HPy pyvalue;
	if( !HPyArg_Parse(ctx, NULL, args, nargs, "OO", &pyvar, &pyvalue ) )
		return HPy_NULL;
	if( !Variable::TypeCheck( ctx, pyvar ) ) {
		// PyErr_Format(
		//     PyExc_TypeError,
		//     "Expected object of type `Variable`. Got object of type `%s` instead.",
		//     other->ob_type->tp_name );
		HPyErr_SetString( ctx, ctx->h_TypeError, "Expected object of type `Variable`." );
		return HPy_NULL;
	}
	double value;
	if( !convert_to_double( ctx, pyvalue, value ) )
		return HPy_NULL;
	Variable* var = Variable::AsStruct( ctx, pyvar );
	try
	{
		self->solver.suggestValue( var->variable, value );
	}
	catch( const kiwi::UnknownEditVariable& )
	{
		setObjectFromGlobal( ctx, UnknownEditVariable, pyvar );
		return HPy_NULL;
	}
	return HPy_Dup( ctx, ctx->h_None );
}


static HPy
Solver_updateVariables( HPyContext *ctx, HPy h_self )
{
    Solver* self = Solver::AsStruct( ctx, h_self );
	self->solver.updateVariables();
	return HPy_Dup( ctx, ctx->h_None );
}


static HPy
Solver_reset( HPyContext *ctx, HPy h_self )
{
    Solver* self = Solver::AsStruct( ctx, h_self );
	self->solver.reset();
	return HPy_Dup( ctx, ctx->h_None );
}


/* 
 Simple port of PyObject_Print. 
 Checks have been removed as Solver isn't checking the return of PyObject_Print
 */
void
Solver_Print(HPyContext *ctx, HPy op, FILE *fp)
{
    clearerr(fp); /* Clear any previous error condition */
    if (HPy_IsNull(op)) {
        HPy_BEGIN_LEAVE_PYTHON(ctx)
        fprintf(fp, "<nil>");
        HPy_END_LEAVE_PYTHON(ctx)
		return;
    }
    else {
		HPy s = HPy_Repr(ctx, op);
		if (HPy_IsNull(s))
			return;
		else if (HPyBytes_Check(ctx, s)) {
			fwrite(HPyBytes_AS_STRING(ctx, s), 1,
					HPyBytes_GET_SIZE(ctx, s), fp);
		}
		else if (HPyUnicode_Check(ctx, s)) {
			// PyObject *t;
			// t = PyUnicode_AsEncodedString(s, "utf-8", "backslashreplace");
			HPy t = HPyUnicode_AsUTF8String(ctx, s);
			if (HPy_IsNull(t)) {
				return;
			}
			else {
				fwrite(HPyBytes_AS_STRING(ctx, t), 1,
						HPyBytes_GET_SIZE(ctx, t), fp);
				HPy_Close(ctx, t);
			}
		}
		else {
			return;
		}
		HPy_Close(ctx, s);
    }
    return;
}

static HPy
Solver_dump( HPyContext *ctx, HPy h_self )
{
    Solver* self = Solver::AsStruct( ctx, h_self );
	HPy dump_str = HPyUnicode_FromString( ctx, self->solver.dumps().c_str() );
	Solver_Print( ctx, dump_str, stdout );
	HPy_Close( ctx, dump_str );
	return HPy_Dup( ctx, ctx->h_None );
}

static HPy
Solver_dumps( HPyContext *ctx, HPy h_self )
{
    Solver* self = Solver::AsStruct( ctx, h_self );
	return HPyUnicode_FromString( ctx, self->solver.dumps().c_str() );
}

HPyDef_METH(Solver_addConstraint_def, "addConstraint", Solver_addConstraint, HPyFunc_O,
	.doc = "Add a constraint to the solver.")
HPyDef_METH(Solver_removeConstraint_def, "removeConstraint", Solver_removeConstraint, HPyFunc_O,
	.doc = "Remove a constraint from the solver.")
HPyDef_METH(Solver_hasConstraint_def, "hasConstraint", Solver_hasConstraint, HPyFunc_O,
	.doc = "Check whether the solver contains a constraint.")
HPyDef_METH(Solver_addEditVariable_def, "addEditVariable", Solver_addEditVariable, HPyFunc_VARARGS,
	.doc = "Add an edit variable to the solver.")
HPyDef_METH(Solver_removeEditVariable_def, "removeEditVariable", Solver_removeEditVariable, HPyFunc_O,
	.doc = "Remove an edit variable from the solver.")
HPyDef_METH(Solver_hasEditVariable_def, "hasEditVariable", Solver_hasEditVariable, HPyFunc_O,
	.doc = "Check whether the solver contains an edit variable.")
HPyDef_METH(Solver_suggestValue_def, "suggestValue", Solver_suggestValue, HPyFunc_VARARGS,
	.doc = "Suggest a desired value for an edit variable.")
HPyDef_METH(Solver_updateVariables_def, "updateVariables", Solver_updateVariables, HPyFunc_NOARGS,
	.doc = "Update the values of the solver variables.")
HPyDef_METH(Solver_reset_def, "reset", Solver_reset, HPyFunc_NOARGS,
	.doc = "Reset the solver to the initial empty starting condition.")
HPyDef_METH(Solver_dump_def, "dump", Solver_dump, HPyFunc_NOARGS,
	.doc = "Dump a representation of the solver internals to stdout.")
HPyDef_METH(Solver_dumps_def, "dumps", Solver_dumps, HPyFunc_NOARGS,
	.doc = "Dump a representation of the solver internals to a string.")


HPyDef_SLOT(Solver_dealloc_def, Solver_dealloc, HPy_tp_destroy);
HPyDef_SLOT(Solver_new_def, Solver_new, HPy_tp_new);

static HPyDef* Solver_defines[] = {
	// slots
	&Solver_dealloc_def,
	&Solver_new_def,

	// methods
	&Solver_addConstraint_def,
	&Solver_removeConstraint_def,
	&Solver_hasConstraint_def,
	&Solver_addEditVariable_def,
	&Solver_removeEditVariable_def,
	&Solver_hasEditVariable_def,
	&Solver_suggestValue_def,
	&Solver_updateVariables_def,
	&Solver_reset_def,
	&Solver_dump_def,
	&Solver_dumps_def,
	NULL
};
} // namespace


// Initialize static variables (otherwise the compiler eliminates them)
HPy Solver::TypeObject = HPy_NULL;


HPyType_Spec Solver::TypeObject_Spec = {
	.name = "kiwisolver.Solver",
	.basicsize = sizeof( Solver ),
	.itemsize = 0,
	.flags = HPy_TPFLAGS_DEFAULT | HPy_TPFLAGS_BASETYPE,
    .defines = Solver_defines
};


bool Solver::Ready( HPyContext *ctx, HPy m )
{
    // The reference will be handled by the module to which we will add the type
    if ( !HPyHelpers_AddType( ctx, m, "Solver", &TypeObject_Spec, NULL ) ) {
        return false;
    }

    TypeObject = HPy_GetAttr_s( ctx, m, "Solver" );
    if( HPy_IsNull( TypeObject ) )
    {
        return false;
    }
    return true;
}


HPyGlobal DuplicateConstraint;

HPyGlobal UnsatisfiableConstraint;

HPyGlobal UnknownConstraint;

HPyGlobal DuplicateEditVariable;

HPyGlobal UnknownEditVariable;

HPyGlobal BadRequiredStrength;

static bool init_exception( HPyContext *ctx, HPy mod, HPyGlobal *global,
	const char *name, const char *attr_name )
{
 	HPy h = HPyErr_NewException( ctx, name, HPy_NULL, HPy_NULL );
 	if( HPy_IsNull( h ) )
    {
        return false;
    }
	HPyGlobal_Store( ctx , global , h );
	int set_attr_failed = HPy_SetAttr_s( ctx, mod, attr_name, h );
	HPy_Close(ctx, h);
	return !set_attr_failed;
}

bool init_exceptions( HPyContext *ctx, HPy mod )
{
 	init_exception( ctx, mod , &DuplicateConstraint ,
		"kiwisolver.DuplicateConstraint" , "DuplicateConstraint" );
  	init_exception( ctx, mod , &UnsatisfiableConstraint,
		"kiwisolver.UnsatisfiableConstraint" , "UnsatisfiableConstraint" );
  	init_exception( ctx, mod , &UnknownConstraint ,
		"kiwisolver.UnknownConstraint" , "UnknownConstraint" );
  	init_exception( ctx, mod , &DuplicateEditVariable ,
		"kiwisolver.DuplicateEditVariable" , "DuplicateEditVariable" );
  	init_exception( ctx, mod , &UnknownEditVariable ,
		"kiwisolver.UnknownEditVariable" , "UnknownEditVariable" );
  	init_exception( ctx, mod , &BadRequiredStrength,
		"kiwisolver.BadRequiredStrength" , "BadRequiredStrength" );
	return true;
}

}  // namespace
