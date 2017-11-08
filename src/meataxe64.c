/*
 * meataxe64: meataxe64
 */

#include "src/compiled.h"          /* GAP headers */

#include <assert.h>
#include "mtx64/field.h"
#include "mtx64/slab.h"

/* The slab level interface with meataxe 64 works uses mainly
   interfaces defined in mtx64/field.h and mtx64/slab.h.  
   This defines four types of
   object of interest, a FIELD (a large structure), a FELT (a 64 bit
   value representing a field element, a DSpace (a small structure
   that gathers some information about a bunch of vectors) and a a
   DFmt object, which is a char pointer to a block of bytes which
   contain vectors. We create the DSpace on the fly when we need if
   we store the number of columns with the Dfmt data and also 
   an int for the number of rowns. We call this a MTX64_Matrix in GAP.

   We wrap FELTS (slightly inefficient, but we dont expect working
   with them to be performance critical), FIELDS, and blocks
   of Dfmt data in T_DATOBJ objects. We store a FIELD in the family of each MTX64_Matrix
   and FELT. 

   For the time being, we make no attempt to unite these objects with
   FFE and List types in GAP. They are entirely separate objects
   accessed only by their own functions (and maybe a few OtherMethods
   for convenience).


   We might want to change this later and unite these with the
   MatrixObj development, or pay for another layer of wrapping.

   TODO -- consts and asserts in appropriate places.
   

   API for SLEch isn't documentred.
   SLEch  a input mx, rs, cs pre-allocated to size (2x 64 + space for bits)
          det for return of determinant (not used), m, c get multiplier and cleaner
          r remnant, nor number of rows of a

   SLEchS required to produce standard row select
 */

static Obj TYPE_MTX64_Field;     // global variable. All FIELDS have same type 
static Obj TYPE_MTX64_Felt;      // function, takes field
static Obj TYPE_MTX64_Matrix;    // function takes field

static inline FIELD * DataOfFieldObject( Obj f) {
    return (FIELD *)(ADDR_OBJ(f)+1);
}

static inline FELT GetFELTFromFELTObject (Obj f) {
    return *(FELT *) (ADDR_OBJ(f)+1);
}

static inline void SetFELTOfFELTObject (Obj f, FELT x) {
    *(FELT *)(ADDR_OBJ(f)+1) = x;
}

typedef struct {
    UInt noc;
    UInt nor;
} MTX64_Matrix_Header;

static inline MTX64_Matrix_Header *HeaderOfMTX64_Matrix (Obj mx) {
    return (MTX64_Matrix_Header *)(ADDR_OBJ(mx)+1);
}


static inline Obj NEW_MTX64_Matrix(Obj f, UInt noc, UInt nor) {
    DSPACE ds;
    Obj m;
    DSSet(DataOfFieldObject(f), noc, &ds);
    m = NewBag(T_DATOBJ, sizeof(Obj) + sizeof(MTX64_Matrix_Header) + ds.nob*nor);
    SET_TYPE_DATOBJ(m, CALL_1ARGS(TYPE_MTX64_Matrix,INTOBJ_INT(DataOfFieldObject(f)->fdef)));
    HeaderOfMTX64_Matrix(m)->noc = noc;
    HeaderOfMTX64_Matrix(m)->nor = nor;
    return m;
}

static inline Dfmt * DataOfMTX64_Matrix ( Obj m) {
    return (Dfmt *)(HeaderOfMTX64_Matrix(m) + 1);
}

static Obj MakeMtx64Field(UInt field_order) {
    Obj field = NewBag(T_DATOBJ, FIELDLEN + sizeof(Obj));
    SET_TYPE_DATOBJ(field, TYPE_MTX64_Field);
    FieldSet(field_order, DataOfFieldObject(field));
    return field;
}


static Obj MTX64_CreateField(Obj self, Obj field_order) {
    return MakeMtx64Field(INT_INTOBJ(field_order));
}

static Obj MakeMtx64Felt(Obj field, FELT x) {
    Obj f = NewBag(T_DATOBJ, sizeof(FELT)+sizeof(Obj));
    UInt q = DataOfFieldObject(field)->fdef;
    Obj type = CALL_1ARGS(TYPE_MTX64_Felt,
                          INTOBJ_INT(q));
    SET_TYPE_DATOBJ(f,type );
    SetFELTOfFELTObject(f, x);
    return f;
}

static Obj FieldOfMTX64Matrix;

static inline void SetDSpaceOfMTX64_Matrix( Obj m, DSPACE *ds) {
    /* Only safe until next garbage collection */
    Obj field = CALL_1ARGS(FieldOfMTX64Matrix, m);
    DSSet(DataOfFieldObject(field), HeaderOfMTX64_Matrix(m)->noc, ds);
}


Obj MTX64_FieldOrder(Obj self, Obj field)
{
    FIELD * _f = DataOfFieldObject(field);
    return INTOBJ_INT(_f->fdef);
}

Obj MTX64_FieldCharacteristic(Obj self, Obj field)
{
    FIELD * _f = DataOfFieldObject(field);
    return INTOBJ_INT(_f->charc);
}

Obj MTX64_FieldDegree(Obj self, Obj field)
{
    FIELD * _f = DataOfFieldObject(field);
    return INTOBJ_INT(_f->pow);
}


Obj MTX64_CreateFieldElement(Obj self, Obj field, Obj elt)
{
    return MakeMtx64Felt(field, INT_INTOBJ(elt));
}

Obj MTX64_ExtractFieldElement(Obj self, Obj elt)
{
    return INTOBJ_INT(GetFELTFromFELTObject(elt));
}

Obj MTX64_FieldAdd(Obj self, Obj f, Obj a, Obj b)
{
    FIELD *_f = DataOfFieldObject(f);
    FELT _a = GetFELTFromFELTObject(a);
    FELT _b = GetFELTFromFELTObject(b);

    return MakeMtx64Felt(f, FieldAdd(_f, _a, _b));
}

Obj MTX64_FieldNeg(Obj self, Obj field, Obj a)
{
    FIELD *_f = DataOfFieldObject(field);
    FELT _a = GetFELTFromFELTObject(a);

    return MakeMtx64Felt(field, FieldNeg(_f, _a));
}

Obj MTX64_FieldSub(Obj self, Obj field, Obj a, Obj b)
{
    FIELD *_f = DataOfFieldObject(field);
    FELT _a = GetFELTFromFELTObject(a);
    FELT _b = GetFELTFromFELTObject(b);

    return MakeMtx64Felt(field, FieldSub(_f, _a, _b));
}

Obj MTX64_FieldMul(Obj self, Obj field, Obj a, Obj b)
{
    FIELD *_f = DataOfFieldObject(field);
    FELT _a = GetFELTFromFELTObject(a);
    FELT _b = GetFELTFromFELTObject(b);

    return MakeMtx64Felt(field, FieldMul(_f, _a, _b));
}

Obj MTX64_FieldInv(Obj self, Obj field, Obj a)
{
    FIELD *_f = DataOfFieldObject(field);
    FELT _a = GetFELTFromFELTObject(a);

    return MakeMtx64Felt(field, FieldInv(_f, _a));
}

Obj MTX64_FieldDiv(Obj self, Obj field, Obj a, Obj b)
{
    FIELD *_f = DataOfFieldObject(field);
    FELT _a = GetFELTFromFELTObject(a);
    FELT _b = GetFELTFromFELTObject(b);

    return MakeMtx64Felt(field, FieldDiv(_f, _a, _b));
}

// Add GAP callable matrix constructors and inspectors

Obj MTX64_NewMatrix(Obj self, Obj field, Obj nor, Obj noc) {
    // checks, or at least asserts
    return NEW_MTX64_Matrix(field, INT_INTOBJ(nor), INT_INTOBJ(noc));
}

Obj MTX64_Matrix_NumRows(Obj self, Obj m) {
    // checks, or at least asserts
    return INTOBJ_INT(HeaderOfMTX64_Matrix(m)->nor);
}

Obj MTX64_Matrix_NumCols(Obj self, Obj m) {
    // checks, or at least asserts
    return INTOBJ_INT(HeaderOfMTX64_Matrix(m)->noc);
}

// 0 based adressing
static FELT GetEntryMTX64(Obj m, UInt col, UInt row)
{
    DSPACE ds;
    Dfmt * d;
    SetDSpaceOfMTX64_Matrix(m, &ds);
    d = DataOfMTX64_Matrix(m);
    d = DPAdv(&ds, row, d);
    return DUnpak(&ds, col, d);
}

Obj MTX64_GetEntry(Obj self, Obj m, Obj col, Obj row)
{
    Obj f = CALL_1ARGS(FieldOfMTX64Matrix,m);
    return MakeMtx64Felt(f,GetEntryMTX64(m, INT_INTOBJ(col), INT_INTOBJ(row)));
}

void SetEntryMTX64(Obj m, UInt col, UInt row, FELT entry)
{
    DSPACE ds;
    Dfmt * d;
    SetDSpaceOfMTX64_Matrix(m, &ds);
    d = DataOfMTX64_Matrix(m);
    d = DPAdv(&ds, row, d);
    DPak(&ds, col, d, entry);
}

Obj MTX64_SetEntry(Obj self, Obj m, Obj col, Obj row, Obj entry)
{
    SetEntryMTX64(m, INT_INTOBJ(col), INT_INTOBJ(row),GetFELTFromFELTObject(entry));
    return 0;
}

#if 0
Obj MTX64_DCpy(Obj self, Obj src, Obj dst, Obj nrows)
{
    Obj ds = DSpaceOfDfmt(src);
    FixDSpace(ds);
    DCpy(DataOfDSPaceObject(ds), DataOfDfmtObject(src), INT_INTOBJ(nrows),
         DataOfDfmtObject(dst));
    return 0;
}

Obj MTX64_DCut(Obj self, Obj m, Obj nrows, Obj startcol, Obj clip ) 
{
    Obj ms = DSpaceOfDfmt(m);
    Obj cbs = DSpaceOfDfmt(clip);
    FixDSpace(ms);
    FixDSpace(cbs);
    DCut(DataOfDSPaceObject(ms), INT_INTOBJ(nrows), INT_INTOBJ(startcol),
         DataOfDfmtObject(m), DataOfDSPaceObject(cbs), DataOfDfmtObject(cb));
    return 0;
}

Obj MTX64_DPaste(Obj self, Obj clip, Obj nrows, Obj startcol, Obj m)
{
    Obj ms = DSpaceOfDfmt(m);
    Obj cbs = DSpaceOfDfmt(cb);
    FixDSpace(ms);
    FixDSpace(cbs);
    DPaste(DataOfDSPaceObject(cbs), DataOfDfmtObject(cb),
           INT_INTOBJ(nrows), INT_INTOBJ(startcol),
         DataOfSpaceObject(ms), DataOfDfmtObject(m), );
    return 0;
    
}

Obj MTX64_DAdd(Obj self, Obj nrows, Obj d1, Obj d2)
{
    Obj ds = DSpaceOfDfmt(d1);
    Obj d = MTX64_MakeDfmt(ds, INT_INTOBJ(nrows));
    FixDSpace(ds);
    DAdd( DataOfDSpaceObject(ds), INT_INTOBJ(nrows), DataOfDfmtObject(d1),
          DataOfDfmtObject(d2), DataOfDfmtObject(d));
    return d;
}

Obj MTX64_DSub(Obj self, Obj nrows, Obj d1, Obj d2)
{
    Obj ds = DSpaceOfDfmt(d1);
    Obj d = MTX64_MakeDfmt(ds, INT_INTOBJ(nrows));
    FixDSpace(ds);
    DSub( DataOfDSpaceObject(ds), INT_INTOBJ(nrows), DataOfDfmtObject(d1),
          DataOfDfmtObject(d2), DataOfDfmtObject(d));
    return d;
}

Obj MTX64_DSMad(Obj self, Obj nrows, Obj scalar, Obj d1, Obj d2)
{
    Obj ds = DSpaceOfDfmt(d1);
    FixDSpace(ds);
    DSMad( DataOfDSpaceObject(ds), GetFELTFromFELTObject(scalar),
           INT_INTOBJ(nrows), DataOfDfmtObject(d1),
          DataOfDfmtObject(d2));
    return 0;
}

// In place?
Obj MTX64_DSMul(Obj self, Obj nrows, Obj scalar, Obj d1)
{
    Obj ds = DSpaceOfDfmt(d1);
    FixDSpace(ds);
    DSMul( DataOfDSpaceObject(ds), GetFELTFromFELTObject(scalar),
           INT_INTOBJ(nrows), DataOfDfmtObject(d1));
    return 0;
    
}


// Higher level stuff
Obj MTX64_SLEchelize(Obj self, Obj a, Obj nrows, Obj ncols)
{
    Obj result;
    uint64_t rank;
    uint64_t *rs, *cs;
    Dfmt *multiply, *remnant, *cleaner;

    // NewBaggin?
    rs = malloc((nrows >> 3) + 2);
    cs = malloc((ncols >> 3) + 2);

    multiply = malloc();
    remnant = malloc();
    cleaner = malloc();

    rank = SLEch(f, a, rs, cs, multiply, cleaner, remnant, nrows, ncols);

    result = NewPRec(4);



    return result;
}

Obj MTX64_SLMultiply(Obj self, Obj field, Obj a, Obj b)
{
    
}

Obj MTX64_SLTranspose(Obj self, Obj field, Obj mat)
{

}

#endif

typedef Obj (* GVarFunc)(/*arguments*/);
#define GVAR_FUNC_TABLE_ENTRY(srcfile, name, nparam, params) \
  {#name, nparam, \
   params, \
   (GVarFunc)name, \
   srcfile ":Func" #name }

// Table of functions to export
static StructGVarFunc GVarFuncs [] = {
    GVAR_FUNC_TABLE_ENTRY("meataxe64.c", MTX64_CreateField, 1, "q"),

    GVAR_FUNC_TABLE_ENTRY("meataxe64.c", MTX64_FieldOrder, 1, "f"),
    GVAR_FUNC_TABLE_ENTRY("meataxe64.c", MTX64_FieldCharacteristic, 1, "f"),
    GVAR_FUNC_TABLE_ENTRY("meataxe64.c", MTX64_FieldDegree, 1, "f"),

    GVAR_FUNC_TABLE_ENTRY("meataxe64.c", MTX64_CreateFieldElement, 2, "f,x"),
    GVAR_FUNC_TABLE_ENTRY("meataxe64.c", MTX64_ExtractFieldElement, 1, "e"),

    GVAR_FUNC_TABLE_ENTRY("meataxe64.c", MTX64_FieldAdd, 3, "f,a,b"),
    GVAR_FUNC_TABLE_ENTRY("meataxe64.c", MTX64_FieldNeg, 2, "f,a"),
    GVAR_FUNC_TABLE_ENTRY("meataxe64.c", MTX64_FieldSub, 3, "f,a,b"),
    GVAR_FUNC_TABLE_ENTRY("meataxe64.c", MTX64_FieldMul, 3, "f,a,b"),
    GVAR_FUNC_TABLE_ENTRY("meataxe64.c", MTX64_FieldInv, 2, "f,a"),
    GVAR_FUNC_TABLE_ENTRY("meataxe64.c", MTX64_FieldDiv, 3, "f,a,b"),

    GVAR_FUNC_TABLE_ENTRY("meataxe64.c", MTX64_NewMatrix, 3, "f,nor,noc"),
    GVAR_FUNC_TABLE_ENTRY("meataxe64.c", MTX64_Matrix_NumRows, 1, "m"),
    GVAR_FUNC_TABLE_ENTRY("meataxe64.c", MTX64_Matrix_NumCols, 1, "m"),
    GVAR_FUNC_TABLE_ENTRY("meataxe64.c", MTX64_GetEntry, 3, "m,i,j"),
    GVAR_FUNC_TABLE_ENTRY("meataxe64.c", MTX64_SetEntry, 4, "m,i,j,x"),
    { 0 } /* Finish with an empty entry */

};

/******************************************************************************
*F  InitKernel( <module> )  . . . . . . . . initialise kernel data structures
*/
static Int InitKernel( StructInitInfo *module )
{
    /* init filters and functionsi */
    InitHdlrFuncsFromTable( GVarFuncs );

    ImportGVarFromLibrary( "MTX64_FieldType", &TYPE_MTX64_Field);
    ImportFuncFromLibrary( "MTX64_FieldEltType", &TYPE_MTX64_Felt);
    ImportFuncFromLibrary( "MTX64_MatrixType", &TYPE_MTX64_Matrix);
    ImportFuncFromLibrary( "FieldOfMTX64Matrix", &FieldOfMTX64Matrix);

    /* return success */
    return 0;
}

/******************************************************************************
*F  InitLibrary( <module> ) . . . . . . .  initialise library data structures
*/
static Int InitLibrary( StructInitInfo *module )
{
    /* init filters and functions */
    InitGVarFuncsFromTable( GVarFuncs );

    /* return success */
    return 0;
}

/******************************************************************************
*F  InitInfopl()  . . . . . . . . . . . . . . . . . table of init functions
*/
static StructInitInfo module = {
 /* type        = */ MODULE_DYNAMIC,
 /* name        = */ "meataxe64",
 /* revision_c  = */ 0,
 /* revision_h  = */ 0,
 /* version     = */ 0,
 /* crc         = */ 0,
 /* initKernel  = */ InitKernel,
 /* initLibrary = */ InitLibrary,
 /* checkInit   = */ 0,
 /* preSave     = */ 0,
 /* postSave    = */ 0,
 /* postRestore = */ 0
};

StructInitInfo *Init__Dynamic( void )
{
    return &module;
}