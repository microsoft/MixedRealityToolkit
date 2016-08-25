// Copyright (C) Microsoft. All rights reserved.
// Licensed under the MIT License. See LICENSE in the project root for license information.

#pragma once
#ifndef __FRG_UTILS_H__
#define __FRG_UTILS_H__

//------------------------------------------------------------------------------------
// FRGFindNearestFree
//---------------------------------------------------------------------------------
// _daGrid must be a 2D grid where a cell is TRUE if there is a wall
Bool FRGFindNearestFree(const BoolDA& _daGrid, const Vec2i& _vSize, const Vec2i& _vPos, Float _fMaxDist, Vec2i& _outPos);

//------------------------------------------------------------------------------------
// FRGSetNeighbors
//---------------------------------------------------------------------------------
// _daGrid must be a 2D grid where a cell is TRUE if there is a wall
void FRGSetNeighbors(BoolDA& _daGrid, const Vec2i& _vSize, S32 _idxPos, S32 _radius);

//------------------------------------------------------------------------------------
// FRGAVL
//------------------------------------------------------------------------------------
// An AVL is a container class which work with tree
// An AVL is tree is automaticaly equilibrate so we are sure that we are able to add and access each element in O(log(n))
template <class Key, class Value>
class FRGAVL
{
public:
    FRGAVL() : m_root(NULL), m_nbElems(0) {}
    
    ~FRGAVL()
    {
        Delete_Z m_root;
    }

	void SetRootNull()
	{
		m_root = NULL;
		m_nbElems = 0;
	}

    void Clear()
    {
        Delete_Z m_root;
        m_root = NULL;
		m_nbElems = 0;
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

	Value& Add(const Key& _key, const Value& _value)
	{
		Value& v = Add(_key);
	    v = _value;
		return v;
	}

	struct Node;

	class Iterator
	{
	public:
		friend class FRGAVL<Key, Value>;
		
		Iterator():	m_pPrevious(),	m_pNode(){}
		
		const Key& GetKey() const
		{
			return m_pNode->m_key;
		}
		
		const Key& GetValue() const
		{
			return m_pNode->m_value;
		}
		
		const Node* GetNode() const
		{
			return m_pNode;
		}
		
		const Node* GetPrevious() const
		{
			return m_pPrevious;
		}

		Bool IsValid() const
		{
			return m_pNode!=NULL;
		}

		const Value& operator*() const
		{
			return m_pNode->m_value;
		}

		const Value* operator->() const
		{
			return &m_pNode->m_value;
		}

		Bool operator!=(const Iterator& _other) const
		{
			return (m_pNode != _other.m_pNode || m_pPrevious != _other.m_pPrevious);
		}

		Iterator& operator++()
		{
			const Node* next;
			if (m_pNode->m_left == NULL)
			{
				// if no left son tree
				next = m_pNode;
				while(next->m_parent && next->m_parent->m_left==next) // if current node is on parent left, then the parent has already been parsed
				{
					next = next->m_parent;
				}
				next = next->m_parent;

			}
			else
			{
				// find the rightest node in the left son tree
				next = m_pNode->m_left;
				while (next->m_right != NULL)
					next = next->m_right;
			}

			m_pPrevious = m_pNode;
			m_pNode = next;

			return *this;
		}

		Iterator operator++(int)
		{
			Iterator cpy = *this;
			++(*this);
			return cpy;
		}

	private:

		Iterator(const Node* _pNode, const Node* _pPrevious) :
			m_pPrevious(_pPrevious),
			m_pNode(_pNode)
		{
			if (m_pNode != NULL)
			{
				while (m_pNode->m_right != NULL)
					m_pNode = m_pNode->m_right;
			}
		}

		const Node* m_pPrevious;
		const Node* m_pNode;
	};

	Iterator MaxToMinBegin() const
	{
		return Iterator(m_root, NULL);
	}

	Iterator MaxToMinEnd() const
	{
		const Node* Previous;
		Previous = m_root;
		if (Previous != NULL)
		{
			while (Previous->m_left != NULL)
				Previous = Previous->m_left;
		}
		return Iterator(NULL, Previous);
	}

private:
    struct Node
    {
		Node() :m_key(), m_height(0), value() 
		{
			m_parent = NULL;
			m_left = NULL;
			m_right = NULL;
		}
        Node(const Key& _key, Node* _parent) : m_key(_key), m_height(0)
		{
			m_parent = _parent;
			m_left = NULL;
			m_right = NULL;
		}
        ~Node()
        {
			if(m_left)
			{
				Delete_Z m_left;
				m_left = NULL;
			}
			if(m_right)
			{
				Delete_Z m_right;
				m_right = NULL;
			}
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

        if (leftHeight > rightHeight)
        {
            if (_node->m_left->GetLeftHeight() < _node->m_left->GetRightHeight())
                RotateLeft(_node->m_left);

            RotateRight(_node);
        }
        else
        {
            if (_node->m_right->GetLeftHeight() > _node->m_right->GetRightHeight())
                RotateRight(_node->m_right);

            RotateLeft(_node);
        }
    }

    Value& AddRec(const  Key& _key, Node* _node)
    {
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

        return *tmp;
    }

    Node* m_root;
    S32 m_nbElems; 
};

#endif