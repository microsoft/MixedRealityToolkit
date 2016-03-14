//////////////////////////////////////////////////////////////////////////
// SyncRoot.java
//
// Wraps the root sync object element
//
// Copyright (C) 2014 Microsoft Corp.  All Rights Reserved
//////////////////////////////////////////////////////////////////////////

package HoloToolkit.Sharing;

import com.microsoft.holotoolkit.sharing.*;
import java.util.*;

 public class SyncRoot extends ObjectElementListener {
	private ObjectElement mRootElement;
	private Map<Long, SyncSphere> mSpheres;

	private IntElement mIntElement;
	private int mIntValue = 0;

	public SyncRoot(ObjectElement rootElement) {
		mRootElement = rootElement;

		mRootElement.AddListener(this);

		mIntElement = mRootElement.CreateIntElement(new XString("TestInt"), mIntValue);

		mSpheres = new Hashtable<Long, SyncSphere>();

		System.gc();
    }

	public int GetIntValue()
	{
		return mIntValue;
	}

	public void SetIntValue(int newValue)
	{
		mIntValue = newValue;
		mIntElement.SetValue(newValue);
	}

	public void OnIntElementChanged(long elementID, int newValue) {
		if (elementID == mIntElement.GetGUID())
		{
			mIntValue = newValue;
			System.out.println("Int Value changed to " + newValue);
		}
		else
		{
			System.out.println("Unknown int changed to " + newValue);
		}
	}

	public void OnFloatElementChanged(long elementID, float newValue) {
		System.out.println("SyncRoot.OnFloatElementChanged");
	}

	public void OnStringElementChanged(long elementID, XString newValue) {
		System.out.println("SyncRoot.OnStringElementChanged");
	}

	public void OnElementAdded(Element element) {
		System.out.println("Object " + element.GetName().GetString() + " of type " + element.GetElementType().toString() + " added to root");

		if (element.GetElementType().swigValue() == ElementType.ObjectType.swigValue())
		{
			mSpheres.put(element.GetGUID(), new SyncSphere(ObjectElement.Cast(element)));
		}

		System.gc();
	}

	public void OnElementDeleted(Element element) {
		System.out.println("Object Deleted from root");

		mSpheres.remove(element.GetGUID());
	}
 }