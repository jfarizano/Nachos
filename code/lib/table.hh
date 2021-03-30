/// A very simple map from non-negative integers to some type.
///
/// Copyright (c) 2018-2021 Docentes de la Universidad Nacional de Rosario.
/// All rights reserved.  See `copyright.h` for copyright notice and
/// limitation of liability and disclaimer of warranty provisions.

#ifndef NACHOS_LIB_TABLE__HH
#define NACHOS_LIB_TABLE__HH


#include "list.hh"


template <class T>
class Table {
public:
    static const unsigned SIZE = 20;

    /// Construct an empty table.
    Table();

    /// Add an item into a free index.
    ///
    /// Returns -1 if no space is left to add the item.
    int Add(T item);

    /// Get the item associated with a given index.
    T Get(int i) const;

    /// Check whether a given index has an associated item.
    bool HasKey(int i) const;

    /// Check whether the table is empty.
    bool IsEmpty() const;

    /// Remove the item associated with a given index.
    ///
    /// Returns the removed item, or `T()` if the index is already
    /// unoccupied.
    T Remove(int i);

    /// Updates the item associated with a given valid index.
    ///
    /// The index must be valid, i.e. it must be assigned to some value.
    ///
    /// Returns the old item.
    T Update(int i, T item);

private:
    /// Data items.
    T data[SIZE];

    /// Current greatest index for a new item.
    int current;

    /// A list to store indexes of items that have been freed and are not
    /// among those with greatest numbers, so it is not possible to modify
    /// `current`.  In other words, this keeps track of external
    /// fragmentation.
    List<int> freed;
};


template <class T>
Table<T>::Table()
{
    current = 0;
}


template <class T>
int
Table<T>::Add(T item)
{
    int i;

    if (!freed.IsEmpty()) {
        i = freed.Pop();
        data[i] = item;
        return i;
    } else if (current < static_cast<int>(SIZE)) {
        i = current++;
        data[i] = item;
        return i;
    } else {
        return -1;
    }
}

template <class T>
T
Table<T>::Get(int i) const
{
    ASSERT(i >= 0);

    return HasKey(i) ? data[i] : T();
}

template <class T>
bool
Table<T>::HasKey(int i) const
{
    ASSERT(i >= 0);

    return i < current && !freed.Has(i);
}

template <class T>
bool
Table<T>::IsEmpty() const
{
    return current == 0;
}

template <class T>
T
Table<T>::Remove(int i)
{
    ASSERT(i >= 0);

    if (!HasKey(i)) {
        return T();
    }

    if (i == current - 1) {
        current--;
        for (int j = current - 1; j >= 0 && !HasKey(j); j--) {
            ASSERT(freed.Has(j));
            freed.SortedPop(&j);
            current--;
        }
    } else {
        freed.SortedInsert(i, i);
    }
    return data[i];
}

template <class T>
T
Table<T>::Update(int i, T item)
{
    ASSERT(i >= 0);
    ASSERT(i < current);
    ASSERT(!freed.Has(i));

    T previous = data[i];
    data[i] = item;
    return previous;
}


#endif
