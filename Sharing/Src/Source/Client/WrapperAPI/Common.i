
#pragma SWIG nowarn=401
#pragma SWIG nowarn=520  //Base class <> of <> is not similarly marked as a smart pointer.

%include "std_string.i"
%include "ref_ptr.i"

// SWIG types for unicode string interop.  This is necessary to ensure strings are marshalled correctly
// and so SWIG doesn't generate it's own wrapper class for wchar_t.  It will treat wchar_t as a C# string.
%typemap(imtype,
	inattributes = "[global::System.Runtime.InteropServices.MarshalAs(global::System.Runtime.InteropServices.UnmanagedType.LPWStr)]",
	outattributes = "[return: global::System.Runtime.InteropServices.MarshalAs(global::System.Runtime.InteropServices.UnmanagedType.LPWStr)]") const wchar_t * "string"
	%typemap(cstype) const wchar_t * "string"
	%typemap(csin) const wchar_t * "$csinput"
	%typemap(csout) const wchar_t * "{
	return $imcall;
  }"

%typemap(imtype,
         inattributes="[global::System.Runtime.InteropServices.MarshalAs(System.Runtime.InteropServices.UnmanagedType.LPStr)]",
         outattributes="[return: global::System.Runtime.InteropServices.MarshalAs(System.Runtime.InteropServices.UnmanagedType.LPStr)]") const char * "string"

%typemap(imtype,
         inattributes="[global::System.Runtime.InteropServices.MarshalAs(System.Runtime.InteropServices.UnmanagedType.I1)]",
         outattributes="[return: global::System.Runtime.InteropServices.MarshalAs(System.Runtime.InteropServices.UnmanagedType.I1)]") bool "bool"

%apply signed short { XTools::int16 };
%apply unsigned short { XTools::uint16 };
%apply signed int { XTools::int32 };
%apply unsigned int { XTools::uint32 };
%apply long long { XTools::int64 };
%apply unsigned long long { XTools::uint64 };

%ignore XTools::NetworkOutMessage::Write(uint16 value);
%ignore XTools::NetworkOutMessage::Write(uint32 value);
%ignore XTools::NetworkOutMessage::Write(uint64 value);
%ignore XTools::NetworkOutMessage::GetData();
%ignore XTools::NetworkOutMessage::GetSize() const;
%ignore XTools::NetworkOutMessage::Write(const std::string& value);

%ignore XTools::NetworkInMessage::ReadUInt16;
%ignore XTools::NetworkInMessage::ReadUInt32;
%ignore XTools::NetworkInMessage::ReadUInt64;
%ignore XTools::NetworkInMessage::GetData;
%ignore XTools::NetworkInMessage::ReadStdString;

%ignore XTools::XValue;
%ignore operator==(const XValue&, const XValue&);
%ignore operator!=(const XValue&, const XValue&);

%ignore XTools::Element::GetXValue;
%ignore XTools::Element::SetXValue;
%ignore XTools::Element::SetParent;

%ignore XTools::Reflection::XTObject;
