//////////////////////////////////////////////////////////////////////////
// SyncSphere.java
//
// Test class for validating the XTools sync system.  Defines a 
//
// Copyright (C) 2014 Microsoft Corp.  All Rights Reserved
//////////////////////////////////////////////////////////////////////////

package HoloToolkit.Sharing;

import com.microsoft.holotoolkit.sharing.*;

 public class SyncSphere extends ObjectElementListener {
	private ObjectElement mElement;

	public SyncSphere(ObjectElement element) {
		mElement = element;

		mElement.AddListener(this);

		System.gc();
    }

	public void OnIntElementChanged(long elementID, int newValue) {
		System.out.println("SyncSphere.OnIntElementChanged");
	}

	public void OnFloatElementChanged(long elementID, float newValue) {
		System.out.println("SyncSphere.OnFloatElementChanged");
	}

	public void OnStringElementChanged(long elementID, XString newValue) {
		System.out.println("SyncSphere.OnStringElementChanged");
	}

	public void OnElementAdded(Element element) {
		System.out.println("SyncSphere.OnElementAdded");
	}

	public void OnElementDeleted(Element element) {
		System.out.println("SyncSphere.OnElementDeleted");
	}
 }
