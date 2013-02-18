// Honeycomb, Copyright (C) 2013 Daniel Carter.  Distributed under the Boost Software License v1.0.
#pragma once

#include "Honey/String/Id.h"
#include "Honey/Object/ListenerList.h"
#include "Honey/Memory/SmallAllocator.h"

namespace honey
{

/// Unrooted acyclic tree.
/**
  * Every tree node has one parent node, multiple children, and a generic data value.
  * Nodes may contain a key for identification and fast retrieval. The key doesn't have to be unique.
  */ 
template<class Data, class Key = Id>
class TreeNode
{
    template<class Data, class Key>
    friend class TreeNode;

    /// Child linked list
    typedef list<TreeNode*, SmallAllocator<TreeNode*>> ChildList;
    typedef list<const TreeNode*, SmallAllocator<const TreeNode*>> ChildListConst;

    /// Map holding children at keys
    typedef typename UnorderedMultiMap<Key, TreeNode*, SmallAllocator>::Type ChildMap;
    typedef typename UnorderedMultiMap<Key, const TreeNode*, SmallAllocator>::Type ChildMapConst;

public:
    typedef Data Data;
    typedef Key Key;

    /// Iterators for child list
    typedef DerefIter<typename ChildList::const_iterator> ChildIter;
    typedef std::reverse_iterator<ChildIter> ChildIterR;

    typedef DerefIter<typename ChildListConst::const_iterator> ChildConstIter;
    typedef std::reverse_iterator<ChildConstIter> ChildConstIterR;

    /// Iterator and ranged iterator for children map
    typedef typename ChildMap::const_iterator ChildMapIter;
    typedef typename ChildMapConst::const_iterator ChildMapConstIter;

    SIGNAL_DECL(TreeNode)
    /// Called before class is destroyed
    SIGNAL(sigDestroy, (TreeNode& src));
    /// Called after data is set
    SIGNAL(sigSetData, (TreeNode& src, Data data));
    /// Called after key is set
    SIGNAL(sigSetKey, (TreeNode& src, Key key));
    /// Called after child is inserted
    SIGNAL(sigInsertChild, (TreeNode& src, TreeNode* childPos, TreeNode& child));
    /// Called before child is removed
    SIGNAL(sigRemoveChild, (TreeNode& src, TreeNode& child));
    /// Called before new parent is set (called from parent when inserting/removing child)
    SIGNAL(sigSetParent, (TreeNode& src, TreeNode* parent));

    TreeNode()                                                  { init(); }
    TreeNode(const Data& data)                                  : _data(data) { init(); }
    TreeNode(const Data& data, const Key& key)                  : _data(data), _key(key) { init(); }

    virtual ~TreeNode()
    {
        setParent(nullptr);
        clearChildren();
        if (hasListenerList()) _listenerList().dispatch<sigDestroy>(*this);
        delete_(_listenerList_p);
    }

    /// Set the data that this node contains
    void setData(const Data& data)
    {
        _data = data;
        if (hasListenerList()) _listenerList().dispatch<sigSetData>(*this, _data);
    }

    const Data& getData() const                                 { return _data; }
    Data& getData()                                             { return _data; }
    /// Implicit cast to the data at this node
    operator const Data&() const                                { return getData(); }
    operator Data&()                                            { return getData(); }
    /// Get the data at this node
    const Data& operator*() const                               { return getData(); }
    Data& operator*()                                           { return getData(); }
    const Data& operator->() const                              { return getData(); }
    Data& operator->()                                          { return getData(); }

    /// Set the key used to identify this node
    void setKey(const Key& key)
    {
        if (hasParent() && _key != _keyNull)
        {
            //Erase key in parent
            ChildMapIter itMap = _parent->childMapIter(*this);
            assert(itMap != _parent->_childMap.end(), sout() << "Child not found in parent's children map. Parent Id: " << _parent->_key << " ; Child Id: " << _key);
            _parent->_childMap.erase(itMap);
        }
        
        _key = key;

        //Insert the child at new key
        if (hasParent()) _parent->_childMap.insert(make_pair(_key, this));
        if (hasListenerList()) _listenerList().dispatch<sigSetKey>(*this, _key);
    }

    /// Get key used to identify this node
    Key& getKey()                                               { return _key; }
    const Key& getKey() const                                   { return _key; }

    /// Set parent node. Returns position in new parent's child list (or children().end() if parent is null)
    ChildIter setParent(TreeNode* parent)
    {
        if (hasParent()) _parent->removeChild(*this);
        if (parent) return parent->addChild(*this);
        return _childList.end();
    }

    /// Get parent node
    const TreeNode* getParent() const                           { return _parent; }
    TreeNode* getParent()                                       { return _parent; }
    /// Check if node has a parent
    bool hasParent() const                                      { return _parent; }

    ///Add child into list, returns position of child in list.
    ChildIter addChild(TreeNode& child)
    {
        assert(&child != this);
        if (child.hasParent()) child._parent->removeChild(child);
        return insertChild(_childList.end(), child);
    }

    ///Set child into list at position.  Returns position of child in list.
    ChildIter setChild(ChildIter pos, TreeNode& child)          { return insertChild(removeChild(pos), child); }

    /// Insert child into list at position.  Returns position of child in list.
    ChildIter insertChild(ChildIter pos, TreeNode& child)
    {
        assert(&child != this);
        //Remove child from parent
        if (child.hasParent()) child._parent->removeChild(child);
        if (child.hasListenerList()) child._listenerList().dispatch<sigSetParent>(child, this);
        child._parent = this;
        //Insert child into list
        child._itSib = _childList.insert(pos, &child);
        //Insert child key into map
        if (child._key != _keyNull) _childMap.insert(make_pair(child._key, &child));
        if (hasListenerList()) _listenerList().dispatch<sigInsertChild>(*this, pos != _childList.end() ? &*pos : nullptr, child);
        return child._itSib;
    }
    
    /// Remove child from children list.  Returns position of next child, or children().end() if child doesn't exist.
    ChildIter removeChild(TreeNode& child)
    {
        ChildIter itChild = childPos(child);
        if (itChild != _childList.end()) return removeChild(itChild);
        return _childList.end(); //Child not in parent
    }

    /// Remove child at position in list. Returns position of next child. Will fail if pos is invalid.
    ChildIter removeChild(ChildIter pos)
    {
        auto& child = *pos;
        assert(child._parent == this);
        if (hasListenerList()) _listenerList().dispatch<sigRemoveChild>(*this, child);
        if (child.hasListenerList()) child._listenerList().dispatch<sigSetParent>(child, nullptr);
        child._parent = nullptr;
        child._itSib = child._childList.end();
        //Erase from child list
        auto itRet = _childList.erase(pos);
        //Erase from map
        if (child._key != _keyNull)
        {
            ChildMapIter itMap = childMapIter(child);
            assert(itMap != _childMap.end(), sout() << "Child not found in parent's children map. Parent Id: " << _key << " ; Child Id: " << child._key);
            _childMap.erase(itMap);
        }
        return itRet;
    }

    /// Clear all children
    void clearChildren()                                        { while (hasChildren()) removeChild(--_childList.end()); }

    /// Get all children
    Range_<ChildIter, ChildIter> children()                     { return range(_childList.begin(), _childList.end()); }
    Range_<ChildConstIter, ChildConstIter> children() const     { auto ret = const_cast<TreeNode*>(this)->children(); return reinterpret_cast<Range_<ChildConstIter, ChildConstIter>&>(ret); }

    /// Get all children in reverse order
    Range_<ChildIterR, ChildIterR> childrenR()                  { return range(_childList.rbegin(), _childList.rend()); }
    Range_<ChildConstIterR, ChildConstIterR> childrenR() const  { auto ret = const_cast<TreeNode*>(this)->childrenR(); return reinterpret_cast<Range_<ChildConstIterR, ChildConstIterR>&>(ret);}

    /// Get number of children
    int childCount() const                                      { return _childList.size(); }

    /// Check if node has children
    bool hasChildren() const                                    { return childCount() > 0; }

    /// Get child position in list.  Returns first child found at key, or children().end() if not found.
    ChildIter childPos(const Key& key)
    {
        auto it = _childMap.find(key);
        return it != _childMap.end() ? it->second->_itSib : _childList.end();
    }

    ChildConstIter childPos(const Key& key) const               { auto ret = const_cast<TreeNode*>(this)->childPos(key); return reinterpret_cast<ChildConstIter&>(ret); }

    /// Get child position in list.  Returns children().end() if not in parent.
    ChildIter childPos(const TreeNode& child)
    {
        if (child._parent == this) return child._itSib;
        return _childList.end();
    }

    ChildConstIter childPos(const TreeNode& child) const        { auto ret = const_cast<TreeNode*>(this)->childPos(child); return reinterpret_cast<ChildConstIter&>(ret); }

    /// Get child at offset from another child.  Large offsets will be slow to process.
    ChildConstIter childPos(ChildConstIter itBase, int offset) const
    {
        if (offset > 0)     for (; offset > 0 && itBase != _childList.end(); --offset, ++itBase);
        else                for (; offset < 0 && itBase != _childList.begin(); ++offset, --itBase);
        return itBase;
    }

    ChildIter childPos(ChildIter itBase, int offset)            { auto ret = const_cast<const TreeNode*>(this)->childPos(reinterpret_cast<ChildConstIter&>(itBase), offset); return reinterpret_cast<ChildIter&>(ret); }

    /// Get offset of child `itPos` relative to another child `itBase`. Returns childCount() if not found.
    int childOffset(ChildConstIter itBase, ChildConstIter itPos) const
    {
        auto itBaseR = itBase;
        auto itBegin = _childList.begin();
        auto itEnd = _childList.end();
        //Check forward and backward at same time
        int count = 0;
        for (; itBase != itPos && itBaseR != itPos; ++count)
        {
            if (itBase == itEnd && itBaseR == itBegin) return childCount();
            if (itBase != itEnd) ++itBase;
            if (itBaseR != itBegin) --itBaseR;
        }
        return (itBase == itPos) ? count : -count;
    }

    int childOffset(ChildIter itBase, ChildIter itPos)          { return const_cast<const TreeNode*>(this)->childOffset(reinterpret_cast<ChildConstIter&>(itBase), reinterpret_cast<ChildConstIter&>(itPos)); }

    /// Get child in list. Returns first child found at key, or null if not found.
    TreeNode* child(const Key& key)
    {
        if (hasChildMap())
        {
            auto it = _childMap.find(key);
            if (it != _childMap.end()) return it->second;
        }
        return nullptr;
    }

    const TreeNode* child(const Key& key) const                 { return const_cast<TreeNode*>(this)->child(key); }

    /// Check if this node has a child in its list
    bool hasChild(const TreeNode& child) const                  { return childPos(child) != _childList.end(); }

    /// Get children that match the key
    /**
      * The child node `TreeNode*` is stored in the map pair `ChildMapIter->second`
      */
    Range_<ChildMapIter, ChildMapIter> children(const Key& key) { return range(_childMap.equal_range(key)); }

    Range_<ChildMapConstIter, ChildMapConstIter> children(const Key& key) const   { auto ret = const_cast<TreeNode*>(this)->children(key); return reinterpret_cast<Range_<ChildMapConstIter, ChildMapConstIter>&>(ret); }

    /// Returns forward iterator range starting at next sibling
    Range_<ChildIter, ChildIter> sibNext()                      { return hasParent() ?  range(next(_itSib), _parent->_childList.end()) :
                                                                                        range(_childList.end(), _childList.end()); }
    Range_<ChildConstIter, ChildConstIter> sibNext() const      { auto ret = const_cast<TreeNode*>(this)->sibNext(); return reinterpret_cast<Range_<ChildConstIter, ChildConstIter>&>(ret); }

    /// Returns reverse iterator range starting at previous sibling
    Range_<ChildIterR, ChildIterR> sibPrev()                    { return hasParent() ?  range(ChildIterR(_itSib), _parent->_childList.rend()) :
                                                                                        range(_childList.rend(), _childList.rend()); }
    Range_<ChildConstIterR, ChildConstIterR> sibPrev() const    { auto ret = const_cast<TreeNode*>(this)->sibPrev(); return reinterpret_cast<Range_<ChildConstIterR, ChildConstIterR>&>(ret); }

    /// Get number of siblings (count doesn't include this node)
    int sibCount() const                                        { return hasParent() ? _parent->childCount() - 1 : 0; }

    /// Check if node has a next sibling
    bool sibHasNext() const                                     { return hasParent() ? next(_itSib) != _parent->_childList.end() : false; }
    /// Check if node has a previous sibling
    bool sibHasPrev() const                                     { return hasParent() ? ChildIterR(_itSib) != _parent->_childList.rend() : false; }

    /// Get sibling at offset from another sibling.  Large offsets will be slow to process.
    ChildConstIter sibPos(ChildConstIter itBase, int offset) const      { return hasParent() ? _parent->childPos(itBase, offset) : _childList.end(); }
    ChildIter sibPos(ChildIter itBase, int offset)                      { return hasParent() ? _parent->childPos(itBase, offset) : _childList.end(); }

    /// Get offset of sibling `itPos` relative to another sibling `itBase`. Parent's childCount() is returned if position is not found.
    int sibOffset(ChildConstIter itBase, ChildConstIter itPos) const    { return hasParent() ? _parent->childOffset(itBase, itPos) : 0; }
    int sibOffset(ChildIter itBase, ChildIter itPos)                    { return hasParent() ? _parent->childOffset(itBase, itPos) : 0; }

    /// Get the root (top-most) node of the tree that contains this node
    TreeNode& root()
    {
        TreeNode* parent = this;
        while (parent->hasParent()) parent = parent->_parent;
        return *parent;
    }

    const TreeNode& root() const                                { return const_cast<TreeNode*>(this)->root(); }

    /// Check if this is a root node at the top of a tree
    bool isRoot() const                                         { return !hasParent(); }
    /// Check if this is a leaf node at the end of a tree branch
    bool isLeaf() const                                         { return !hasChildren(); }

    /// Test if `ancestor` is this node's ancestor.  Returns false if `ancestor` is same as this node.
    bool isAncestor(const TreeNode& ancestor) const
    {
        for (auto node = getParent(); node; node = node->getParent())
            if (node == &ancestor) return true;
        return false;
    }

    /// Test if this node is an ancestor of node.  Returns false if `node` is same as this node.
    bool isAncestorOf(const TreeNode& node) const               { return node.isAncestor(*this); }

    /// Returns first node found with key in tree of descendants (depth-first pre-order traversal)
    TreeNode* findNode(const Key& key)
    {
        if (_key == key) return this;
        for (auto& e : preOrd())
        {
            auto child = e.child(key);
            if (child) return child;
        }
        return nullptr;
    }

    const TreeNode* findNode(const Key& key) const              { return const_cast<TreeNode*>(this)->findNode(key); }

    /// Depth-first pre-order traversal.
    /** 
      * The first node returned is the root, followed by a path through all the first children.
      * The last node returned is the leaf node at the end of a path through all the last children.
      * Do not modify tree while traversing.
      */
    template<class TreeNode>
    class PreOrdIter_
    {
    public:
        typedef std::bidirectional_iterator_tag     iterator_category;
        typedef TreeNode                            value_type;
        typedef ptrdiff_t                           difference_type;
        typedef TreeNode*                           pointer;
        typedef TreeNode&                           reference;
        
        PreOrdIter_()                                           : _root(nullptr), _node(nullptr), _skipChildren(false), _count(0) {}

        PreOrdIter_(TreeNode* root, bool end) :
            _skipChildren(false),
            _count(0)
        {
            assert(root);
            _root = root;
            _node = (!end) ? _root : _root->_parent;
        }

        PreOrdIter_& operator++()
        {
            if (_node == _root->_parent) return *this;

            if (!_node->isLeaf() && !_skipChildren)
            {
                //Non-leaf, move into first child
                _node = *_node->_childList.begin();
            }
            else
            {
                //Leaf, select next sibling.  If there is none then backtrack through parents until sibling is found.
                _skipChildren = false;
                TreeNode* parent;
                for (parent = _node; parent != _root->_parent; parent = parent->_parent)
                {
                    //Don't select siblings of root
                    if (parent != _root && parent->sibHasNext())
                    {
                        parent = &*begin(parent->sibNext());
                        break;
                    }
                }

                _node = parent;
            }

            ++_count;
            return *this;
        }

        PreOrdIter_& operator--()
        {
            if (_node == _root) return *this;

            //If we are at traversal end / have a prev sibling...
            if (_node == _root->_parent || _node->sibHasPrev())
            {
                //Select deepest last child of root / prev sibling
                _node = (_node == _root->_parent) ? _root : &*begin(_node->sibPrev());
                while (_node->childCount() > 0) _node = *--_node->_childList.end();
            }
            else
            {
                //No previous sibling, go to parent
                _node = _node->_parent;
            }

            --_count;
            return *this;
        }

        PreOrdIter_ operator++(int)                             { auto tmp = *this; ++*this; return tmp; }
        PreOrdIter_ operator--(int)                             { auto tmp = *this; --*this; return tmp; }

        bool operator==(const PreOrdIter_& rhs) const           { return _node == rhs._node; }
        bool operator!=(const PreOrdIter_& rhs) const           { return !operator==(rhs); }

        reference operator*() const                             { assert(_node); return *_node; }
        pointer operator->() const                              { return &operator*(); }

        /// Skip the current node's children on next step of this iterator
        void skipChildren()                                     { _skipChildren = true; }

        /// Get the number of nodes between the current position and the beginning of the iteration
        int count() const                                       { return _count; }

    private:
        TreeNode*   _root;
        TreeNode*   _node;
        bool        _skipChildren;
        int         _count;
    };

    typedef PreOrdIter_<TreeNode> PreOrdIter;
    typedef PreOrdIter_<const TreeNode> PreOrdConstIter;

    typedef std::reverse_iterator<PreOrdConstIter> PreOrdConstIterR;
    typedef std::reverse_iterator<PreOrdIter> PreOrdIterR;

    /// Get a depth-first pre-order range
    Range_<PreOrdConstIter, PreOrdConstIter>    preOrd() const  { return range(PreOrdConstIter(this, false), PreOrdConstIter(this, true)); }
    Range_<PreOrdIter, PreOrdIter>              preOrd()        { return range(PreOrdIter(this, false), PreOrdIter(this, true)); }

    /// Get a reversed depth-first pre-order range
    Range_<PreOrdConstIterR, PreOrdConstIterR>  preOrdR() const { auto range_ = preOrd(); return range(PreOrdConstIterR(end(range_)), PreOrdConstIterR(begin(range_))); }
    Range_<PreOrdIterR, PreOrdIterR>            preOrdR()       { auto range_ = preOrd(); return range(PreOrdIterR(end(range_)), PreOrdIterR(begin(range_))); }

    /// Get the number of nodes in the entire subtree (ie. this node and its children, and its children's children...)
    int preOrdCount() const                                     { return reduce(preOrd(), 0, [](int a, const TreeNode&) { return ++a; }); }
    
    /// Get listener list
    ListenerList& listeners()                                   { return _listenerList(); }
private:

    void init()
    {
        _parent = nullptr;
        _itSib = _childList.end();
        _listenerList_p = nullptr;
    }

    ///There are potentially multiple children with same key, find the specific child pointer in map
    ChildMapIter childMapIter(TreeNode& child)
    {
        return stdutil::find(_childMap, child._key, &child);
    }

    /// \name Optional addons for the tree node, created automatically when requested
    /// @{
    ListenerList& _listenerList()                               { if (_listenerList_p) return *_listenerList_p; return *(_listenerList_p = new ListenerList()); }
    const ListenerList& _listenerList() const                   { return const_cast<TreeNode*>(this)->_listenerList(); }
    bool hasListenerList() const                                { return _listenerList_p; }
    /// @}

    Data _data;
    TreeNode* _parent;
    ChildIter _itSib;
    ChildList _childList;
    Key _key;
    ChildMap _childMap;
    ListenerList* _listenerList_p;

    static Key _keyNull;    ///< Used to check if key is set
};

template<class Data, class Key>
typename Key TreeNode<Data, Key>::_keyNull;

}
