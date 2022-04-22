/*-----------------------------------------------------------------------------
| Copyright (c) 2013-2019, Nucleic Development Team.
| Copyright (c) 2022, Oracle and/or its affiliates.
|
| Distributed under the terms of the Modified BSD License.
|
| The full license is in the file LICENSE, distributed with this software.
|----------------------------------------------------------------------------*/
#include <kiwi/kiwi.h>
#include "util.h"


#ifdef __clang__
#pragma clang diagnostic ignored "-Wdeprecated-writable-strings"
#endif

#ifdef __GNUC__
#pragma GCC diagnostic ignored "-Wwrite-strings"
#endif


namespace kiwisolver
{


namespace
{


static void
strength_dealloc( void* obj )
{
	// Py_TYPE( self )->tp_free( self );
}


static HPy
strength_weak(HPyContext *ctx, HPy h_self, void *closure)
{
	return HPyFloat_FromDouble( ctx, kiwi::strength::weak );
}


static HPy
strength_medium(HPyContext *ctx, HPy h_self, void *closure)
{
	return HPyFloat_FromDouble( ctx, kiwi::strength::medium );
}


static HPy
strength_strong(HPyContext *ctx, HPy h_self, void *closure)
{
	return HPyFloat_FromDouble( ctx, kiwi::strength::strong );
}


static HPy
strength_required(HPyContext *ctx, HPy h_self, void *closure)
{
	return HPyFloat_FromDouble( ctx, kiwi::strength::required );
}


static HPy
strength_create( HPyContext *ctx, HPy h_self, HPy* args, HPy_ssize_t nargs )
{
	HPy pya;
	HPy pyb;
	HPy pyc;
	HPy pyw = HPy_NULL;
	if( !HPyArg_Parse(ctx, NULL, args, nargs, "OOO|O", &pya, &pyb, &pyc, &pyw ) )
		return HPy_NULL;
	double a, b, c;
	double w = 1.0;
	if( !convert_to_double( ctx, pya, a ) )
		return HPy_NULL;
	if( !convert_to_double( ctx, pyb, b ) )
		return HPy_NULL;
	if( !convert_to_double( ctx, pyc, c ) )
		return HPy_NULL;
	if( !HPy_IsNull(pyw) && !convert_to_double( ctx, pyw, w ) )
		return HPy_NULL;
	return HPyFloat_FromDouble( ctx, kiwi::strength::create( a, b, c, w ) );
}


HPyDef_GET(strength_weak_def, "weak", strength_weak, .doc = "The predefined weak strength.")
HPyDef_GET(strength_medium_def, "medium", strength_medium, .doc = "The predefined medium strength.")
HPyDef_GET(strength_strong_def, "strong", strength_strong, .doc = "The predefined strong strength.")
HPyDef_GET(strength_required_def, "required", strength_required, .doc = "The predefined required strength.")

HPyDef_METH(strength_create_def, "create", strength_create, HPyFunc_VARARGS,
	.doc = "Create a strength from constituent values and optional weight.")

HPyDef_SLOT(strength_dealloc_def, strength_dealloc, HPy_tp_destroy)

static HPyDef* strength_defines[] = {
    // slots
	&strength_dealloc_def,

	// getsets
	&strength_weak_def,
	&strength_medium_def,
	&strength_strong_def,
	&strength_required_def,

	// methods
	&strength_create_def,
	NULL
};
} // namespace


// Initialize static variables (otherwise the compiler eliminates them)
HPy strength::TypeObject = HPy_NULL;


HPyType_Spec strength::TypeObject_Spec = {
	.name = "kiwisolver.strength",
	.basicsize = sizeof( strength ),
	.itemsize = 0,
	.flags = HPy_TPFLAGS_DEFAULT,
    .defines = strength_defines
};


bool strength::Ready( HPyContext *ctx, HPy m )
{
    // The reference will be handled by the module to which we will add the type
    if (!HPyHelpers_AddType(ctx, m, "strength", &TypeObject_Spec, NULL)) {
        return false;
    }

    TypeObject = HPy_GetAttr_s(ctx, m, "strength");
	if( HPy_IsNull(TypeObject) ) {
		return false;
	}

	strength* s;
	HPy h_strength = HPy_New(ctx, TypeObject, &s);
    if( HPy_IsNull(h_strength) ) {
		HPy_Close(ctx, TypeObject);
		return false;
	}
	HPy_Close(ctx, TypeObject);
	
	if ( HPy_SetAttr_s(ctx, m, "strength", h_strength) )
    {
		HPy_Close(ctx, h_strength);
        return false;
    }
	HPy_Close(ctx, h_strength);
    return true;
}


}  // namespace
