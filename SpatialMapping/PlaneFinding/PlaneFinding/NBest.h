#pragma once

// This class represents a sorted list of a maximum of N elements
// We use it to find the best planes, by finding many potential planes, and throwing those out that aren't considered
// good enough.
template <unsigned int N, typename T>
class NBest
{
public:
    virtual void Add(T data)
    {
        static UINT32 indexToInsert = 0;
        indexToInsert = 0;
        static bool fShouldInsert = false;
        fShouldInsert = false;

        for (UINT32 i = 0; i < num && !fShouldInsert; ++i)
        {
            if (best[i] < data)
            {
                indexToInsert = i;
                fShouldInsert = true;
            }
        }

        if (!fShouldInsert && num < N)
        {
            indexToInsert = num; // insert at the last element
            fShouldInsert = true;
        }

        if (fShouldInsert)
        {
            // add an extra bucket on the end if necessary
            if (num < N)
            {
                num++;
            }

            // insert the item, and shift everything down so we don't overwrite
            for (UINT32 i = indexToInsert; i < num; ++i)
            {
                T temp = best[i];
                best[i] = data;
                data = temp;
            }
        }
    }

    NBest()
    {}

    UINT32 num = 0;
    T best[N];
};