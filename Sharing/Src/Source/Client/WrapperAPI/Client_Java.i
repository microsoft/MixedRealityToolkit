%module(directors="1") SharingClient

%{
#include "../Common/Common.h"
#include "ClientWrapperAPI.h"
using namespace XTools;
%}

// Prevent the java runtime from hanging on exit
%insert("runtime") %{
#define SWIG_JAVA_ATTACH_CURRENT_THREAD_AS_DAEMON 
%}

%include <windows.i>
%include "arrays_java.i"
%include "typemaps.i"
%include "various.i"
%include "Common.i"

%include "enums.swg"
%javaconst(1);

// Swig defaults to treating signed chars as bytes and unsigned chars as shorts.  This code will map
// bytes in C++ to bytes in Java
%apply signed char { XTools::byte value };
%apply signed char { XTools::byte messageType };
%apply signed char[] { XTools::byte *data, XTools::byte *image };  // This will cause SWIG to generate interfaces using Java's builtin array type.  

// Define typemaps for parameter to register and unregister listener types
%define %listener_declare(ListenerType, RegisterName, UnregisterName)
%feature("director") ListenerType;
%typemap(javain) ListenerType *RegisterName "getCPtrAndAddReference($javainput)"
%typemap(javain) ListenerType *UnregisterName "getCPtrAndRemoveReference($javainput)"
%typemap(javain) const ref_ptr<ListenerType>& RegisterName "getCPtrAndAddReference($javainput)"
%typemap(javain) const ref_ptr<ListenerType>& UnregisterName "getCPtrAndRemoveReference($javainput)"
%enddef

#define LISTENER_LIST_NAME(TYPE) _list_##TYPE

// Add a member variable to the wrapper class to hold references to objects that are passed in as listeners
%define %listener_user_declare(UserType, ListenerType)

%typemap(javacode) UserType %{

	private java.util.List<ListenerType> LISTENER_LIST_NAME(ListenerType);

	private long getCPtrAndAddReference(ListenerType element) {
		if (element != null) {
			if (LISTENER_LIST_NAME(ListenerType) == null) {
				LISTENER_LIST_NAME(ListenerType) = java.util.Collections.synchronizedList(new java.util.ArrayList<ListenerType>());
			}

			LISTENER_LIST_NAME(ListenerType).add(element);
		}

		return ListenerType.getCPtr(element);
	}

	private long getCPtrAndRemoveReference(ListenerType element) {
		if (element != null && LISTENER_LIST_NAME(ListenerType) != null) {
			LISTENER_LIST_NAME(ListenerType).remove(element);
		}

		return ListenerType.getCPtr(element);
	}

%}
%enddef // listener_user_declare

%include "../ClientWrapperAPI.h"
