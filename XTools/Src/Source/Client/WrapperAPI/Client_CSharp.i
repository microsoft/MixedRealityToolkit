%module(directors="1") SharingClient

%{
#include "../Common/Common.h"
#include "ClientWrapperAPI.h"
using namespace XTools;
%}

%include <windows.i>
%include <stl.i>
%include "arrays_csharp.i"
%include "typemaps.i"
%include "Common.i"

%csconst(1);

%csmethodmodifiers XTools::NetworkOutMessage::WriteArray "public unsafe";
%csmethodmodifiers XTools::NetworkInMessage::ReadArray "public unsafe";
%csmethodmodifiers XTools::VisualPairConnector::ProcessImage "public unsafe";
%csmethodmodifiers XTools::TagImage::CopyImageData "public unsafe";
%csmethodmodifiers XTools::ImageTagManager::FindTags "public unsafe";
%csmethodmodifiers XTools::AnchorDownloadRequest::GetData "public unsafe";
%csmethodmodifiers XTools::RoomManager::UploadAnchor "public unsafe";
%apply unsigned char FIXED[] { ::XTools::byte *data, const ::XTools::byte *data, const ::XTools::byte *image, ::XTools::byte *image, byte *data, const byte *data };


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


%include "../ClientWrapperAPI.h"
