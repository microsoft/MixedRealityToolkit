%module(directors="1") SharingProfiler

%{
#include "../Common/Common.h"
#include "ProfilerAPI.h"
using namespace XTools;
%}

%include <windows.i>
%include <stl.i>
%include "../../Client/WrapperAPI/Common.i"
%include "../../Client/WrapperAPI/arrays_csharp_pinned.i"

// Apply the PINNED typemap the function parameters that match the following signatures
%apply unsigned char PINNED[] { ::XTools::byte *data, const ::XTools::byte *data, const ::XTools::byte *image, ::XTools::byte *image, byte *data, const byte *data };

// Define typemaps for parameter to register and unregister listener types
%define %listener_declare(ListenerType, RegisterName, UnregisterName)
%feature("director") ListenerType;
%typemap(csin) ListenerType *RegisterName "getCPtrAndAddReference($csinput)"
%typemap(csin) ListenerType *UnregisterName "getCPtrAndRemoveReference($csinput)"
%typemap(csin) const ref_ptr<ListenerType>& RegisterName "getCPtrAndAddReference($csinput)"
%typemap(csin) const ref_ptr<ListenerType>& UnregisterName "getCPtrAndRemoveReference($csinput)"
%enddef

#define LISTENER_LIST_NAME(TYPE) _list_##TYPE

// Add a member variable to the wrapper class to hold references to objects that are passed in as listeners
%define %listener_user_declare(UserType, ListenerType)

%typemap(cscode) UserType %{

	private System.Collections.Generic.List<ListenerType> LISTENER_LIST_NAME(ListenerType) = new System.Collections.Generic.List<ListenerType>();

	private System.Runtime.InteropServices.HandleRef getCPtrAndAddReference(ListenerType element) 
	{
		lock(LISTENER_LIST_NAME(ListenerType))
		{
			if (element != null) 
			{
				LISTENER_LIST_NAME(ListenerType).Add(element);
			}

			return ListenerType.getCPtr(element);
		}
	}

	private System.Runtime.InteropServices.HandleRef getCPtrAndRemoveReference(ListenerType element) 
	{
		lock(LISTENER_LIST_NAME(ListenerType))
		{
			if (element != null) 
			{
				LISTENER_LIST_NAME(ListenerType).Remove(element);
			}

			return ListenerType.getCPtr(element);
		}
	}

%}
%enddef // listener_user_declare

// Add C# methods to the XString class to make it easier to convert to and from normal C# strings
%typemap(cscode) XTools::XString %{
  public override string ToString() { 
    return GetString(); 
  }

  public override bool Equals(object obj)   
  {
    XString s = obj as XString; 
    if (ReferenceEquals(s, null)) {
      return false;
    }
    else {
      return IsEqual(s);
    }
  }

  public override int GetHashCode()
  {
    return GetString().GetHashCode(); 
  }

  public static bool operator ==(XString lhs, XString rhs)
  {
    if (ReferenceEquals(lhs, rhs)) {
      return true;
    }
    else if (ReferenceEquals(lhs, null) || ReferenceEquals(rhs, null)) {
      return false;
    } else {
      return lhs.IsEqual(rhs);
    }
   }
  
  public static bool operator !=(XString lhs, XString rhs)
  {
    return !(lhs == rhs);
  }
  
  public static implicit operator XString(string s) {
    return new XString(s);
  } 

  public static implicit operator string(XString s)
  {
    return s.GetString();
  }
%}

%include "../ProfilerAPI.h"
