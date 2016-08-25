// Copyright (C) Microsoft. All rights reserved.
// Licensed under the MIT License. See LICENSE in the project root for license information.

#pragma once
#ifndef __SHAPE_ANALYZER_TOOLS_H__
#define __SHAPE_ANALYZER_TOOLS_H__

#ifdef _DEBUG
    #define SHAPE_ANALYZER_ASSERT(c) EXCEPTION_Z(c)
#else
    #define SHAPE_ANALYZER_ASSERT(c) (void*)(0)
#endif

namespace ShapeReco
{
//------------------------------------------------------------------------------------
// Bool operator ==(const DynArray_Z<T>& _array1, const DynArray_Z<T>& _array2)
//------------------------------------------------------------------------------------
// Test if two DynArray are equals
//
// This is an helper for use a DynArray like a key in a AVL
template <class T>
Bool operator ==(const DynArray_Z<T>& _array1, const DynArray_Z<T>& _array2)
{
    if (_array1.GetSize() != _array2.GetSize())
        return FALSE;

    for (S32 i = 0; i < _array1.GetSize(); ++i)
    {
        if (_array1[i] != _array2[i])
            return FALSE;
    }

    return TRUE;
}

//------------------------------------------------------------------------------------
// Bool operator <(const DynArray_Z<T>& _array1, const DynArray_Z<T>& _array2)
//------------------------------------------------------------------------------------
// Test if one DynArray is bigger than another
//
// This is an helper for use a DynArray like a key in a AVL
template <class T>
Bool operator <(const DynArray_Z<T>& _array1, const DynArray_Z<T>& _array2)
{
    if (_array1.GetSize() < _array2.GetSize())
        return TRUE;

    if (_array1.GetSize() > _array2.GetSize())
        return FALSE;

    for (S32 i = 0; i < _array1.GetSize(); ++i)
    {
        if (_array1[i] < _array2[i])
            return TRUE;

        if (_array1[i] > _array2[i])
            return FALSE;
    }

    return FALSE;
}

//------------------------------------------------------------------------------------
// SharedPtr
//------------------------------------------------------------------------------------
// A shared pointer manage an dynamicly allocated object
// When the pointer is copied the object is shared between all pointer
// The object is automaticaly deleted when all pointer are destroy
// Carful we have to add mutex if we want to be thread safe
//
// This class is use for save already calculated characteristics
template <class T>
class SharedPtr
{
public :
    SharedPtr() :
        m_ptr(NULL),
        m_nbWatchers(NULL)
    {
    }

    SharedPtr(T* _ptr) :
        m_ptr(_ptr),
        m_nbWatchers((_ptr != NULL ? New_Z S32(1) : NULL))
    {
    }

    SharedPtr(const SharedPtr& _other) :
        m_ptr(_other.m_ptr),
        m_nbWatchers(_other.m_nbWatchers)
    {
        if (m_nbWatchers != NULL)
            ++(*m_nbWatchers);
    }

    const SharedPtr& operator=(const SharedPtr& _other)
    {
        if (this != &_other)
        {
            SharedPtr tmp(_other);
            Swap(m_ptr, tmp.m_ptr);
            Swap(m_nbWatchers, tmp.m_nbWatchers);
        }

        return *this;
    }

    ~SharedPtr()
    {
        if (m_nbWatchers != NULL && --(*m_nbWatchers) == 0)
        {
            Delete_Z m_nbWatchers;
            Delete_Z m_ptr;
        }
    }

    T* Get() const
    {
        return m_ptr;
    }

    void Reset(T* _ptr = NULL)
    {
        SharedPtr tmp(_ptr);
        Swap(m_ptr, tmp.m_ptr);
        Swap(m_nbWatchers, tmp.m_nbWatchers);
    }

private :
    T* m_ptr;
    S32* m_nbWatchers;
};

//------------------------------------------------------------------------------------
// CloneablePtr
//------------------------------------------------------------------------------------
// A cloneable pointer manage an dynamicly allocated object
// The object must have a "clone" method
// When the pointer is copied the object is automaticaly cloned
// The object is automaticaly deleted when its pointer is destroy
//
// This class is use for manage polymorphic constraints and simplify copy and alocation/desalocation
template <class T>
class CloneablePtr
{
public:
    CloneablePtr() :
        m_ptr(NULL)
    {
    }

    CloneablePtr(const T& _cloneable) :
        m_ptr(_cloneable.clone())
    {
    }

    CloneablePtr(const CloneablePtr& _other) :
        m_ptr((_other.m_ptr == NULL ? NULL : _other.m_ptr->clone()))
    {
    }

    const CloneablePtr& operator=(const CloneablePtr& _other)
    {
        CloneablePtr tmp(_other);
        Swap(m_ptr, tmp.m_ptr);
        return *this;
    }
    
    ~CloneablePtr()
    {
        Delete_Z m_ptr;
    }

protected:
    T* m_ptr;
};

//------------------------------------------------------------------------------------
// AVL
//------------------------------------------------------------------------------------
// An AVL is a container class which work with tree
// An AVL is tree is automaticaly equilibrate so we are sure that we are able to add and access each element in O(log(n))
//
// This class is use for store saved characteristics
template <class Key, class Value>
class AVL
{
public:
    AVL() : m_root(NULL), m_nbElems(0) {}
    
    ~AVL()
    {
        Delete_Z m_root;
    }

    void Clear()
    {
        Delete_Z m_root;
        m_root = NULL;
    }

    S32 GetNbElems() const
    {
        return m_nbElems;
    }

    S32 GetHeight() const
    {
        if (m_root == NULL)
            return 0;
        return m_root->m_height;
    }
        
    Value* Find(const Key& _key)
    {
        return FindRec(_key, m_root);
    }

    Value& Add(const Key& _key)
    {
        ++m_nbElems;

        if (m_root != NULL)
            return AddRec(_key, m_root);
        else
        {
            m_root = New_Z Node(_key, NULL);
            return m_root->m_value;
        }
    }

private:
    struct Node
    {
        Node(const Key& _key, Node* _parent) : m_key(_key), m_height(0), m_parent(_parent), m_left(NULL), m_right(NULL) {}
        ~Node()
        {
            Delete_Z m_left;
            Delete_Z m_right;
        }

        S32 GetLeftHeight() const
        {
            if (m_left == NULL)
                return 0;

            return 1 + m_left->m_height;
        }

        S32 GetRightHeight() const
        {
            if (m_right == NULL)
                return 0;

            return 1 + m_right->m_height;
        }

        void UpdateHeight()
        {
            m_height = Max(GetLeftHeight(), GetRightHeight());
        }

        Key m_key;
        Value m_value;

        S32 m_height;

        Node* m_parent;
        Node* m_left;
        Node* m_right;
    };

    Value* FindRec(const Key& _key, Node* _node)
    {
        if (_node == NULL)
            return NULL;

        if (_key == _node->m_key)
            return &_node->m_value;

        if (_key < _node->m_key)
            return FindRec(_key, _node->m_left);

        return FindRec(_key, _node->m_right);
    }

    ///////////////////////////////////////////////////////////////////////////////
    //                                                                           //
    //               parent                                  parent              //
    //                 |                                       |                 //
    //                 |                                       |                 //
    //                (A)                                     (B)                //
    //              ___|___                                 ___|___              //
    //             /       \                               /       \             //
    //            /         \            ==>              /         \            //
    //           /          (B)      RotateLeft(A)      (A)          \           //
    //          /          __|___        ==>          ___|__          \          //
    //         /          /      \                   /      \          \         //
    //       _/         _/        \_               _/        \_         \_       //
    //      / \        / \        / \             / \        / \        / \      //
    //     /   \      /   \      /   \           /   \      /   \      /   \     //
    //    /     \    /     \    /     \         /     \    /     \    /     \    //
    //   / alpha \  / beta  \  / gamma \       / alpha \  / beta  \  / gamma \   //
    //  /_________\/_________\/_________\     /_________\/_________\/_________\  //
    //                                                                           //
    ///////////////////////////////////////////////////////////////////////////////
    void RotateLeft(Node* _node)
    {
        SHAPE_ANALYZER_ASSERT(_node != NULL);
        SHAPE_ANALYZER_ASSERT(_node->m_right != NULL);

        Node* parent = _node->m_parent;
        Node* a = _node;
        Node* b = _node->m_right;
        Node* beta = _node->m_right->m_left;

        // Parent
        if (parent == NULL)
        {
            m_root = b;
            b->m_parent = NULL;
        }
        else
        {
            if (parent->m_left == a)
                parent->m_left = b;
            else
                parent->m_right = b;
            b->m_parent = parent;
        }

        // A
        a->m_right = beta;
        if (beta != NULL)
            beta->m_parent = a;

        // B
        b->m_left = a;
        a->m_parent = b;

        // Heights
        a->UpdateHeight();
        b->UpdateHeight();
    }

    ///////////////////////////////////////////////////////////////////////////////
    //                                                                           //
    //                 parent                              parent                //
    //                   |                                   |                   //
    //                   |                                   |                   //
    //                  (A)                                 (B)                  //
    //                ___|___                             ___|___                //
    //               /       \                           /       \               //
    //              /         \          ==>            /         \              //
    //            (B)          \     RotateRight(A)    /          (A)            //
    //          ___|__          \        ==>          /          __|___          //
    //         /      \          \                   /          /      \         //
    //       _/        \_         \_               _/         _/        \_       //
    //      / \        / \        / \             / \        / \        / \      //
    //     /   \      /   \      /   \           /   \      /   \      /   \     //
    //    /     \    /     \    /     \         /     \    /     \    /     \    //
    //   / alpha \  / beta  \  / gamma \       / alpha \  / beta  \  / gamma \   //
    //  /_________\/_________\/_________\     /_________\/_________\/_________\  //
    //                                                                           //
    ///////////////////////////////////////////////////////////////////////////////
    void RotateRight(Node* _node)
    {
        SHAPE_ANALYZER_ASSERT(_node != NULL);
        SHAPE_ANALYZER_ASSERT(_node->m_left != NULL);

        Node* parent = _node->m_parent;
        Node* a = _node;
        Node* b = _node->m_left;
        Node* beta = _node->m_left->m_right;

        // Parent
        if (parent == NULL)
        {
            m_root = b;
            b->m_parent = NULL;
        }
        else
        {
            if (parent->m_left == a)
                parent->m_left = b;
            else
                parent->m_right = b;
            b->m_parent = parent;
        }

        // A
        a->m_left = beta;
        if (beta != NULL)
            beta->m_parent = a;

        // B
        b->m_right = a;
        a->m_parent = b;

        // Heights
        a->UpdateHeight();
        b->UpdateHeight();
    }

    void EquilibrateAfterAdd(Node* _node)
    {
        if (_node == NULL)
            return;

        S32 leftHeight = _node->GetLeftHeight();
        S32 rightHeight = _node->GetRightHeight();
        S32 diff = Abs(leftHeight - rightHeight);

        if (diff == 0)
            return;

        if (diff == 1)
        {
            _node->m_height = Max(leftHeight, rightHeight);
            return EquilibrateAfterAdd(_node->m_parent);
        }

        SHAPE_ANALYZER_ASSERT(diff == 2);

        if (leftHeight > rightHeight)
        {
            SHAPE_ANALYZER_ASSERT(_node->m_left != NULL);

            if (_node->m_left->GetLeftHeight() < _node->m_left->GetRightHeight())
                RotateLeft(_node->m_left);

            RotateRight(_node);
        }
        else
        {
            SHAPE_ANALYZER_ASSERT(_node->m_right != NULL);

            if (_node->m_right->GetLeftHeight() > _node->m_right->GetRightHeight())
                RotateRight(_node->m_right);

            RotateLeft(_node);
        }
    }

    Value& AddRec(const  Key& _key, Node* _node)
    {
        SHAPE_ANALYZER_ASSERT(_node != NULL);
        SHAPE_ANALYZER_ASSERT(!(_key == _node->m_key));

        Value* tmp = NULL;

        if (_key < _node->m_key)
        {
            if (_node->m_left != NULL)
                return AddRec(_key, _node->m_left);
            else
            {
                _node->m_left = New_Z Node(_key, _node);
                tmp = &_node->m_left->m_value;
                EquilibrateAfterAdd(_node);
            }
        }
        else if (_node->m_right != NULL)
            return AddRec(_key, _node->m_right);
        else
        {
            _node->m_right = New_Z Node(_key, _node);
            tmp = &_node->m_right->m_value;
            EquilibrateAfterAdd(_node);
        }

        SHAPE_ANALYZER_ASSERT(tmp != NULL);
        return *tmp;
    }

    Node* m_root;
    S32 m_nbElems; 
};
} // namespace ShapeReco

#endif