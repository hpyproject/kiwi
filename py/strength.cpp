/*-----------------------------------------------------------------------------
| Copyright (c) 2013-2019, Nucleic Development Team.
| Copyright (c) 2022-2023, Oracle and/or its affiliates.
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


HPyDef_GET(strength_weak, "weak", .doc = "The predefined weak strength.")
static HPy
strength_weak_get(HPyContext *ctx, HPy h_self, void *closure)
{
	return HPyFloat_FromDouble( ctx, kiwi::strength::weak );
}


HPyDef_GET(strength_medium, "medium", .doc = "The predefined medium strength.")
static HPy
strength_medium_get(HPyContext *ctx, HPy h_self, void *closure)
{
	return HPyFloat_FromDouble( ctx, kiwi::strength::medium );
}


HPyDef_GET(strength_strong, "strong", .doc = "The predefined strong strength.")
static HPy
strength_strong_get(HPyContext *ctx, HPy h_self, void *closure)
{
	return HPyFloat_FromDouble( ctx, kiwi::strength::strong );
}


HPyDef_GET(strength_required, "required", .doc = "The predefined required strength.")
static HPy
strength_required_get(HPyContext *ctx, HPy h_self, void *closure)
{
	return HPyFloat_FromDouble( ctx, kiwi::strength::required );
}


HPyDef_METH(strength_create, "create", HPyFunc_VARARGS,
	.doc = "Create a strength from constituent values and optional weight.")
static HPy
strength_create_impl( HPyContext *ctx, HPy h_self, const HPy *args, size_t nargs )
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



static HPyDef* strength_defines[] = {
	// getsets
	&strength_weak,
	&strength_medium,
	&strength_strong,
	&strength_required,

	// methods
	&strength_create,
	NULL
};
} // namespace


// Declare static variables (otherwise the compiler eliminates them)
HPyGlobal strength::TypeObject;


HPyType_Spec strength::TypeObject_Spec = {
	.name = "kiwisolver.strength",
	.basicsize = sizeof( strength ),
	.itemsize = 0,
	.flags = HPy_TPFLAGS_DEFAULT,
    .defines = strength_defines,
};


bool strength::Ready( HPyContext *ctx, HPy m )
{
    if ( ! add_type( ctx , m , &TypeObject , "strength" , &TypeObject_Spec ) ) {
        return false;
    }

	strength* s;
	HPy h_strength = new_from_global(ctx, TypeObject, &s);
    if( HPy_IsNull(h_strength) ) {
		return false;
	}
	
	if ( HPy_SetAttr_s(ctx, m, "strength", h_strength) )
    {
		HPy_Close(ctx, h_strength);
        return false;
    }
	HPy_Close(ctx, h_strength);
    return true;
}


}  // namespace
