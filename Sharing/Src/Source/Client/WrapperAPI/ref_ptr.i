

#if !defined(XTOOLS_REF_PTR_NAMESPACE)
# define XTOOLS_REF_PTR_NAMESPACE XTools
#endif


// Users can provide their own XTOOLS_REF_PTR_TYPEMAPS or XTOOLS_REF_PTR_TYPEMAPS_NO_WRAP macros before including this file to change the
// visibility of the constructor and getCPtr method if desired to public if using multiple modules.
#ifndef XTOOLS_REF_PTR_TYPEMAPS
#define XTOOLS_REF_PTR_TYPEMAPS(CONST, TYPE...) XTOOLS_REF_PTR_TYPEMAPS_IMPLEMENTATION(protected, protected, CONST, TYPE)
#endif


// Workaround empty first macro argument bug
#define SWIGEMPTYHACK
// Main user macro for defining ref_ptr typemaps for both const and non-const pointer types
%define %ref_ptr(TYPE...)
//%feature("smartptr", noblock=1) TYPE { XTOOLS_REF_PTR_NAMESPACE::ref_ptr< TYPE > }
XTOOLS_REF_PTR_TYPEMAPS(SWIGEMPTYHACK, TYPE)
XTOOLS_REF_PTR_TYPEMAPS(const, TYPE)
%enddef

// Language specific macro implementing all the customisations for handling the smart pointer
%define XTOOLS_REF_PTR_TYPEMAPS_IMPLEMENTATION(PTRCTOR_VISIBILITY, CPTR_VISIBILITY, CONST, TYPE...)

// %naturalvar is as documented for member variables
%naturalvar TYPE;
%naturalvar XTOOLS_REF_PTR_NAMESPACE::ref_ptr< CONST TYPE >;

// Typemap customisations...

///////////////////////////////////////////////////
// Java-specific typemapping
///////////////////////////////////////////////////

#if defined(SWIGJAVA)

%typemap(in) XTOOLS_REF_PTR_NAMESPACE::ref_ptr< CONST TYPE > %{ 
  // ref_ptr by value in
	$1 = *(CONST TYPE **)&$input; 
%}
%typemap(directorin,descriptor="L$packagepath/TYPE;") XTOOLS_REF_PTR_NAMESPACE::ref_ptr< CONST TYPE > %{
  // ref_ptr by value directorin
  if ($1) { $1->AddRef(); }
   *( TYPE **)&$input = $1.get();
%}
%typemap(out) XTOOLS_REF_PTR_NAMESPACE::ref_ptr< CONST TYPE > %{ 
  // ref_ptr by value out
  if ($1) {
  	result->AddRef();
  	*(CONST TYPE **)&$result = result.get();
  } else {
   	*(CONST TYPE **)&$result = 0; 
  }
%}

%typemap(in) XTOOLS_REF_PTR_NAMESPACE::ref_ptr< CONST TYPE > & ($*1_ltype tempnull, $*1_ltype temp, CONST TYPE * smartarg) %{ 
  // ref_ptr by reference in
  if ( $input ) {
  	smartarg = *(CONST TYPE **)&$input; 
  	temp = XTOOLS_REF_PTR_NAMESPACE::ref_ptr< CONST TYPE >(smartarg);
  	$1 = &temp;
  } else {
	$1 = &tempnull;
  }
%}
%typemap(memberin) XTOOLS_REF_PTR_NAMESPACE::ref_ptr< CONST TYPE > & %{
  // ref_ptr by reference memberin
  #error "Where is this used?"
  delete &($1);
  if ($self) {
    XTOOLS_REF_PTR_NAMESPACE::ref_ptr< CONST TYPE > * temp = new XTOOLS_REF_PTR_NAMESPACE::ref_ptr< CONST TYPE >(*$input);
    $1 = *temp;
  }
%}
%typemap(directorin,descriptor="L$packagepath/TYPE;") XTOOLS_REF_PTR_NAMESPACE::ref_ptr< CONST TYPE > & %{
  // ref_ptr by reference directorin
  if ($1) { $1->AddRef(); }
  *( TYPE **)&$input = $1.get();
%}
%typemap(out) XTOOLS_REF_PTR_NAMESPACE::ref_ptr< CONST TYPE > & %{ 
  // ref_ptr by reference out
  if (*$1) {
    $1->get()->AddRef();
    *(CONST TYPE **)&$result = $1->get();
  } else {
    *(CONST TYPE **)&$result = 0;
  }
%} 


// various missing typemaps - If ever used (unlikely) ensure compilation error rather than runtime bug
%typemap(in) CONST TYPE[], CONST TYPE[ANY], CONST TYPE (CLASS::*) %{
#error "typemaps for $1_type not available"
%}
%typemap(out) CONST TYPE[], CONST TYPE[ANY], CONST TYPE (CLASS::*) %{
#error "typemaps for $1_type not available"
%}

%typemap(jni)    XTOOLS_REF_PTR_NAMESPACE::ref_ptr< CONST TYPE >,
                  XTOOLS_REF_PTR_NAMESPACE::ref_ptr< CONST TYPE > &,
                  XTOOLS_REF_PTR_NAMESPACE::ref_ptr< CONST TYPE > *,
                  XTOOLS_REF_PTR_NAMESPACE::ref_ptr< CONST TYPE > *& "jlong"
%typemap(jtype)  XTOOLS_REF_PTR_NAMESPACE::ref_ptr< CONST TYPE >, 
                  XTOOLS_REF_PTR_NAMESPACE::ref_ptr< CONST TYPE > &,
                  XTOOLS_REF_PTR_NAMESPACE::ref_ptr< CONST TYPE > *,
                  XTOOLS_REF_PTR_NAMESPACE::ref_ptr< CONST TYPE > *& "long"
%typemap(jstype) XTOOLS_REF_PTR_NAMESPACE::ref_ptr< CONST TYPE >,
                  XTOOLS_REF_PTR_NAMESPACE::ref_ptr< CONST TYPE > &,
                  XTOOLS_REF_PTR_NAMESPACE::ref_ptr< CONST TYPE > *,
                  XTOOLS_REF_PTR_NAMESPACE::ref_ptr< CONST TYPE > *& "TYPE"
%typemap(javain) XTOOLS_REF_PTR_NAMESPACE::ref_ptr< CONST TYPE >,
                 XTOOLS_REF_PTR_NAMESPACE::ref_ptr< CONST TYPE > &,
                 XTOOLS_REF_PTR_NAMESPACE::ref_ptr< CONST TYPE > *,
                 XTOOLS_REF_PTR_NAMESPACE::ref_ptr< CONST TYPE > *& "TYPE.getCPtr($javainput)"
%typemap(javadirectorin) XTOOLS_REF_PTR_NAMESPACE::ref_ptr< CONST TYPE >,
                 XTOOLS_REF_PTR_NAMESPACE::ref_ptr< CONST TYPE > &,
                 XTOOLS_REF_PTR_NAMESPACE::ref_ptr< CONST TYPE > *,
                 XTOOLS_REF_PTR_NAMESPACE::ref_ptr< CONST TYPE > *& "($jniinput == 0) ? null : new TYPE($jniinput, true)"

%typemap(javaout) XTOOLS_REF_PTR_NAMESPACE::ref_ptr< CONST TYPE > {
	// ref_ptr by value javaout
    long cPtr = $jnicall;
    return (cPtr == 0) ? null : new TYPE(cPtr, true);
  }
%typemap(javaout) XTOOLS_REF_PTR_NAMESPACE::ref_ptr< CONST TYPE > & {
	// ref_ptr by reference javaout
    long cPtr = $jnicall;
    return (cPtr == 0) ? null : new TYPE(cPtr, true);
  }
%typemap(javaout) XTOOLS_REF_PTR_NAMESPACE::ref_ptr< CONST TYPE > * {
	// ref_ptr by pointer javaout
    long cPtr = $jnicall;
    return (cPtr == 0) ? null : new TYPE(cPtr, true);
  }
%typemap(javaout) XTOOLS_REF_PTR_NAMESPACE::ref_ptr< CONST TYPE > *& {
	// ref_ptr by const pointer javaout
    long cPtr = $jnicall;
    return (cPtr == 0) ? null : new TYPE(cPtr, true);
  }

%typemap(javadirectorout) XTOOLS_REF_PTR_NAMESPACE::ref_ptr< CONST TYPE > %{ TYPE.getCPtr($javacall) %}
%typemap(javadirectorout) XTOOLS_REF_PTR_NAMESPACE::ref_ptr< CONST TYPE > & %{ TYPE.getCPtr($javacall) %}
%typemap(javadirectorout) XTOOLS_REF_PTR_NAMESPACE::ref_ptr< CONST TYPE > * %{ TYPE.getCPtr($javacall) %}
%typemap(javadirectorout) XTOOLS_REF_PTR_NAMESPACE::ref_ptr< CONST TYPE > *& %{ TYPE.getCPtr($javacall) %}

#endif //SWIGJAVA




///////////////////////////////////////////////////
// C#-specific typemapping
///////////////////////////////////////////////////

#if defined(SWIGCSHARP)

%typemap(in) XTOOLS_REF_PTR_NAMESPACE::ref_ptr< CONST TYPE > %{ 
  // ref_ptr by value in
  $1 = *(CONST TYPE **)&$input;
%}
%typemap(out) XTOOLS_REF_PTR_NAMESPACE::ref_ptr< CONST TYPE > %{ 
  // ref_ptr by value out
  if ($1) {
    $1->AddRef();
    *(CONST TYPE **)&$result = $1.get();
  } else {
    *(CONST TYPE **)&$result = 0; 
  }
%}

%typemap(in) XTOOLS_REF_PTR_NAMESPACE::ref_ptr< CONST TYPE > & ($*1_ltype tempnull, $*1_ltype temp, CONST TYPE * smartarg) %{ 
  // ref_ptr by reference in
  if ( $input ) {
    smartarg = *(CONST TYPE **)&$input; 
    temp = XTOOLS_REF_PTR_NAMESPACE::ref_ptr< CONST TYPE >(smartarg);
    $1 = &temp;
  } else {
    $1 = &tempnull;
  }
%}
%typemap(memberin) XTOOLS_REF_PTR_NAMESPACE::ref_ptr< CONST TYPE > & %{
  #error "Where is this used?"
  delete &($1);
  if ($self) {
    XTOOLS_REF_PTR_NAMESPACE::ref_ptr< CONST TYPE > * temp = new XTOOLS_REF_PTR_NAMESPACE::ref_ptr< CONST TYPE >(*$input);
    $1 = *temp;
  }
%}
%typemap(directorin) XTOOLS_REF_PTR_NAMESPACE::ref_ptr< CONST TYPE > & %{
  // ref_ptr by reference directorin
  if ($1) { $1->AddRef(); }
  $input = $1.get();
%}
%typemap(out) XTOOLS_REF_PTR_NAMESPACE::ref_ptr< CONST TYPE > & %{ 
  // ref_ptr by reference out
  if (*$1) {
    $1->get()->AddRef();
    *(CONST TYPE **)&$result = $1->get();
  } else {
    *(CONST TYPE **)&$result = 0;
  }
%} 

// various missing typemaps - If ever used (unlikely) ensure compilation error rather than runtime bug
%typemap(in) CONST TYPE[], CONST TYPE[ANY], CONST TYPE (CLASS::*) %{
#error "typemaps for $1_type not available"
%}
%typemap(out) CONST TYPE[], CONST TYPE[ANY], CONST TYPE (CLASS::*) %{
#error "typemaps for $1_type not available"
%}


%typemap (ctype)  XTOOLS_REF_PTR_NAMESPACE::ref_ptr< CONST TYPE >,
                  XTOOLS_REF_PTR_NAMESPACE::ref_ptr< CONST TYPE > &,
                  XTOOLS_REF_PTR_NAMESPACE::ref_ptr< CONST TYPE > *,
                  XTOOLS_REF_PTR_NAMESPACE::ref_ptr< CONST TYPE > *& "void *"
%typemap (imtype, out="global::System.IntPtr")  XTOOLS_REF_PTR_NAMESPACE::ref_ptr< CONST TYPE >, 
                  XTOOLS_REF_PTR_NAMESPACE::ref_ptr< CONST TYPE > &,
                  XTOOLS_REF_PTR_NAMESPACE::ref_ptr< CONST TYPE > *,
                  XTOOLS_REF_PTR_NAMESPACE::ref_ptr< CONST TYPE > *& "global::System.Runtime.InteropServices.HandleRef"
%typemap (cstype) XTOOLS_REF_PTR_NAMESPACE::ref_ptr< CONST TYPE >,
                  XTOOLS_REF_PTR_NAMESPACE::ref_ptr< CONST TYPE > &,
                  XTOOLS_REF_PTR_NAMESPACE::ref_ptr< CONST TYPE > *,
                  XTOOLS_REF_PTR_NAMESPACE::ref_ptr< CONST TYPE > *& "TYPE"
%typemap(csin)	  XTOOLS_REF_PTR_NAMESPACE::ref_ptr< CONST TYPE >,
                  XTOOLS_REF_PTR_NAMESPACE::ref_ptr< CONST TYPE > &,
                  XTOOLS_REF_PTR_NAMESPACE::ref_ptr< CONST TYPE > *,
                  XTOOLS_REF_PTR_NAMESPACE::ref_ptr< CONST TYPE > *& "TYPE.getCPtr($csinput)"
%typemap(csdirectorin) XTOOLS_REF_PTR_NAMESPACE::ref_ptr< CONST TYPE >,
                 XTOOLS_REF_PTR_NAMESPACE::ref_ptr< CONST TYPE > &,
                 XTOOLS_REF_PTR_NAMESPACE::ref_ptr< CONST TYPE > *,
                 XTOOLS_REF_PTR_NAMESPACE::ref_ptr< CONST TYPE > *& "($iminput == global::System.IntPtr.Zero) ? null : new TYPE($iminput, true)"

%typemap(csout, excode=SWIGEXCODE) XTOOLS_REF_PTR_NAMESPACE::ref_ptr< CONST TYPE > {
    global::System.IntPtr cPtr = $imcall;
    TYPE ret = (cPtr == global::System.IntPtr.Zero) ? null : new TYPE(cPtr, true);$excode
    return ret; 
  }
%typemap(csout, excode=SWIGEXCODE) XTOOLS_REF_PTR_NAMESPACE::ref_ptr< CONST TYPE > & {
    global::System.IntPtr cPtr = $imcall;
    TYPE ret = (cPtr == global::System.IntPtr.Zero) ? null : new TYPE(cPtr, true);$excode
    return ret; 
  }
%typemap(csout, excode=SWIGEXCODE) XTOOLS_REF_PTR_NAMESPACE::ref_ptr< CONST TYPE > * {
    global::System.IntPtr cPtr = $imcall;
    TYPE ret = (cPtr == global::System.IntPtr.Zero) ? null : new TYPE(cPtr, true);$excode
    return ret; 
  }
%typemap(csout, excode=SWIGEXCODE) XTOOLS_REF_PTR_NAMESPACE::ref_ptr< CONST TYPE > *& {
    global::System.IntPtr cPtr = $imcall;
    TYPE ret = (cPtr == global::System.IntPtr.Zero) ? null : new TYPE(cPtr, true);$excode
    return ret; 
  }
%typemap(csvarout, excode=SWIGEXCODE2) XTOOLS_REF_PTR_NAMESPACE::ref_ptr< CONST TYPE > %{
    get {
      TYPE ret = new TYPE($imcall, true);$excode
      return ret;
    } %}
%typemap(csvarout, excode=SWIGEXCODE2) XTOOLS_REF_PTR_NAMESPACE::ref_ptr< CONST TYPE >& %{
    get {
      TYPE ret = new TYPE($imcall, true);$excode
      return ret;
    } %}
%typemap(csvarout, excode=SWIGEXCODE2) XTOOLS_REF_PTR_NAMESPACE::ref_ptr< CONST TYPE >* %{
    get {
      TYPE ret = new TYPE($imcall, true);$excode
      return ret;
    } %}

%typemap(csdirectorout, excode=SWIGEXCODE) XTOOLS_REF_PTR_NAMESPACE::ref_ptr< CONST TYPE > %{ TYPE.getCPtr($cscall).Handle %}
%typemap(csdirectorout, excode=SWIGEXCODE) XTOOLS_REF_PTR_NAMESPACE::ref_ptr< CONST TYPE > & %{ TYPE.getCPtr($cscall).Handle %}
%typemap(csdirectorout, excode=SWIGEXCODE) XTOOLS_REF_PTR_NAMESPACE::ref_ptr< CONST TYPE > * %{ TYPE.getCPtr($cscall).Handle %}
%typemap(csdirectorout, excode=SWIGEXCODE) XTOOLS_REF_PTR_NAMESPACE::ref_ptr< CONST TYPE > *& %{ TYPE.getCPtr($cscall).Handle %}

#endif //SWIGCSHARP

%enddef
