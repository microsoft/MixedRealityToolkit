// Copyright (C) Microsoft. All rights reserved.
// Licensed under the MIT License. See LICENSE in the project root for license information.

#ifndef	_FIFO_Z
#define	_FIFO_Z

// FIFO en banked.

template<class T, int Granularity=64,Bool DeleteObject = TRUE,int align = _ALLOCDEFAULTALIGN> class FIFO_Z
//template<class T, int Granularity=64> class FIFO_Z
{
private:
	class FIFOBank
	{
		public:
		T			Data[Granularity];
		FIFOBank	*Next;
	};

	FIFOBank	*FreeBank;

	S32			StartElem;
	FIFOBank	*StartBank;

	S32			EndElem;
	FIFOBank	*EndBank;

	S32			NbElem;
	
	void	Init()
	{
		NbElem = 0;

		FreeBank = 0;

		StartElem = 0;
		StartBank = 0;

		EndElem = Granularity-1;
		EndBank = 0;
	}

public:
	FIFO_Z()
	{
		Init();
	}
	~FIFO_Z()
	{
		Empty(TRUE);
	}

	void	Push(const T &Ele)
	{
		EndElem++;
		NbElem++;

		if (EndElem >= Granularity)
		{
			// New bank...
			FIFOBank *NewEnd;

			if (FreeBank)
			{
				NewEnd = FreeBank;
				FreeBank = NewEnd->Next;
			}
			else
			{
				NewEnd=(FIFOBank*)AllocAlign_Z(sizeof(FIFOBank),align);
			}

			// Init.

			if (!EndBank)
			{
				StartElem = 0;
				StartBank = NewEnd;
			}
			else
			{
				EndBank->Next = NewEnd;
			}

			NewEnd->Next = 0;
			EndBank = NewEnd;

			EndElem = 0;
		}

		EndBank->Data[EndElem] = Ele;
	}
	T		*Push()
	{
		EndElem++;
		NbElem++;

		if (EndElem >= Granularity)
		{
			// New bank...
			FIFOBank *NewEnd;

			if (FreeBank)
			{
				NewEnd = FreeBank;
				FreeBank = NewEnd->Next;
			}
			else
			{
				NewEnd=(FIFOBank*)AllocAlign_Z(sizeof(FIFOBank),align);
			}

			// Init.

			if (!EndBank)
			{
				StartElem = 0;
				StartBank = NewEnd;
			}
			else
			{
				EndBank->Next = NewEnd;
			}

			NewEnd->Next = 0;
			EndBank = NewEnd;

			EndElem = 0;
		}

		return EndBank->Data+EndElem;
	}
	T		&GetLastPushed()
	{
		ASSERT_Z(NbElem>0);
		return 	EndBank->Data[EndElem];
	}

	Bool	Pop(T	&Ele)
	{
		if (!NbElem) return FALSE;

		Ele = StartBank->Data[StartElem];
		if (DeleteObject) StartBank->Data[StartElem].~T();

		StartElem++;
		if (StartElem>=Granularity)
		{
			StartElem = 0;

			FIFOBank *NewStart = StartBank->Next;

			StartBank->Next = FreeBank;
			FreeBank = StartBank;

			StartBank = NewStart;
			if (!StartBank) EndBank = 0;
		}

		NbElem--;
		if (!NbElem && StartBank)
		{
			StartBank->Next = FreeBank;
			FreeBank = StartBank;

			StartElem = 0;
			StartBank = 0;

			EndElem = Granularity-1;
			EndBank = 0;
		}

		return TRUE;
	}
	T		&GetPopable()
	{
		ASSERT_Z(NbElem>0);
		return 	StartBank->Data[StartElem];
	}

	void	Empty(Bool minimize=FALSE)
	{
		T	work;
		while (Pop(work)) {;};

		if (minimize)
		{
			while (FreeBank)
			{
				FIFOBank *NewFree;
				NewFree = FreeBank->Next;
				Free_Z(FreeBank);
				FreeBank = NewFree;
			}
			Init();
		}
	}

	inline	void	Flush(void)
	{
		Empty();
	}

	inline	void	Minimize(void)
	{
		Empty(TRUE);
	}

	S32 GetNbElement() const {return NbElem;}
};
#endif
