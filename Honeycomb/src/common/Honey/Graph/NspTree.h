// Honeycomb, Copyright (C) 2013 Daniel Carter.  Distributed under the Boost Software License v1.0.
#pragma once

#include "Honey/Math/Alge/Alge.h"
#include "Honey/Math/Geom/Box.h"
#include "Honey/Memory/SmallAllocator.h"
#include "Honey/Thread/Lock/Spin.h"

namespace honey
{
/** \cond */
namespace priv { template<class Real, int Dim> struct NspTreeNode {}; }
/** \endcond */

/// 3D spatial partitioning tree. Recursively divides space in N-dimensions at each depth level.
/**
  * The data is split by nodes and referenced in the leaves at max depth.
  * If the data box is so large that every node above depth X will need a ref, then it will be stored at depth X.
  * ex. Data could have a few refs at depth 1 and 2 instead of hundreds in the leaves at depth 5.
  *
  * All nodes up to max depth are pre-allocated in one contiguous memory block for better performance.
  *
  * During enumeration, data must be checked to see if it has already been visited to avoid duplicate visits.
  * For performance this visited check is done in constant time, the tradeoff is memory and a limit on concurrent enums (through recursion or threading)
  *
  * \tparam Dim     Dimension of 3D space divisions. Range [1,3]
  * \see            BspTree, QuadTree, OctTree
  */
template<int Dim>
class NspTree
{
    typedef priv::NspTreeNode<Real, Dim> NspTreeNode;

public:
    static const int dim = Dim;

    struct DataBase
    {
        DataBase(int concurMax)                         : visited(concurMax, false) {}
        virtual ~DataBase() {}

        Box box;                ///< Bounding box
        vector<bool> visited;   ///< Visited marks, one for each concurrent id
    };

    /// User data holder. User must provide a bounding box for adding to the tree and enumerating
    template<class DataT>
    struct Data : DataBase
    {
        /**
          * \param concurMax    must be >= to the concurMax of any tree this data is added to
          * \param data         user data
          */
        Data(int concurMax, const DataT& data)          : DataBase(concurMax), data(data) {}

        DataT data;
    };

    /// Visitor functor for enum function.
    class EnumVisitor
    {
        friend class NspTree;
        friend class NodeBase;
        template<int> friend class Node;
    public:
        #define ENUM_LIST(e,_)  \
            e(_, cont)          \
            e(_, stop)          \

        /**
          * \retval cont    continue enumeration
          * \retval stop    stop enumeration
          */
        ENUM(NspTree::EnumVisitor, State);
        #undef ENUM_LIST

        EnumVisitor() :
            _tree(nullptr),
            _state(State::cont),
            _concurId(-1),
            _visited(nullptr) {}

        /// Visit data
        virtual void operator()(const typename NspTree::DataBase* data) = 0;

        /// Get tree that is being enumerated
        const NspTree& tree() const             { return *_tree; }
        
        /// Get box used in enumeration
        const Box& box() const                  { return _box; }

        /// Set state of enumeration
        void setState(State state)              { _state = state; }
        State getState() const                  { return _state; }

    private:
        const NspTree* _tree;
        Box _box;
        State _state;
        int _concurId;
        vector<DataBase*>* _visited;
    };

    /// Holds box and data list
    /**
      * Data order is guaranteed (first added is first returned by enume()).
      * A map is used to track the list position for quick removal.
      */
    class Node
    {
        friend class NspTree;
        template<int> friend class Node;
    public:
        Node()                                                      : _children(nullptr), _activeFirst(activeEnd), _activeNext(inactive) {}
        ~Node() {}

        /// Set up box and children pointers recursively
        void init(const Box& box, vector<Node>& nodes, int depth, int depthOffset);

        /// Add data to node recursively
        void add(DataBase& data, int depth, int depthMax);
        /// Remove data from node recursively
        void remove(DataBase& data, int depth, int depthMax);
        /// Enumerate data recursively
        void enume(EnumVisitor& visitor, int depth, int depthMax) const;
        /// Enum for nodes that are completely bounded. We can safely enum the entire tree without intersection tests.
        void enumBounded(EnumVisitor& visitor) const;

        /// Add data to this node
        void add(DataBase& data)
        {
            //Check if data already exists while inserting temp list position into map
            pair<Map::iterator, bool> pair = _dataMap.insert(make_pair(&data, _data.end()));
            if (!pair.second) return;
            //Data doesn't exist, add to list and update list position in map
            _data.push_back(&data);
            pair.first->second = --_data.end();
            return;
        }

        /// Remove data from this node
        void remove(DataBase& data)
        {
            //Use map to quickly find position in list
            Map::iterator it = _dataMap.find(&data);
            if (it == _dataMap.end()) return;
            _data.erase(it->second);
            _dataMap.erase(it);
        }

        /// Enumerate data in this node
        void enume(EnumVisitor& visitor) const;

    private:
        typedef DataBase* MapKey;
        typedef list<MapKey, SmallAllocator<MapKey>> List;
        typedef typename List::iterator MapVal;
        typedef typename UnorderedMap<MapKey, MapVal, SmallAllocator>::Type Map;

        bool hasActiveChild() const                                 { return _activeFirst != activeEnd; }

        Node* _children;
        int8 _activeFirst;
        int8 _activeNext;
        Box _box;
        List _data;
        Map _dataMap;

        static const int8 activeEnd = -1;
        static const int8 inactive = -2;
    };

    /**
      * \param treeBox      box that bounds volume of entire tree
      * \param depthMax     max tree depth, number of leaf nodes is 2^(Dim*DepthMax)
      * \param concurMax    see concurMax()
      */
    NspTree(const Box& treeBox, int depthMax, int concurMax) :
        _depthMax(depthMax),
        _concurIds(concurMax),
        _visited(concurMax),
        _concurCount(0)
    {
        //Get total number of nodes by adding nodes at each level
        int count = 0;
        for (auto i: range(depthMax+1)) count += 1 << (dim*i);
        _nodes.resize(count);
        _root = &_nodes[0];
        _root->init(treeBox, _nodes, 0, 0);
        for (auto i: range(size(_concurIds))) _concurIds[i] = i;
    }

    ~NspTree() {}

    /// Add data to tree.  Data is pointed to by tree, not stored.  Data must be removed from the tree before changing its box.
    void add(DataBase& data)
    {
        assert(size(data.visited) >= concurMax(), "Data concur max not large enough");
        if (NspTreeNode::nodeBounded(_root->_box, data.box))
            _root->add(data);
        else
            _root->add(data, 0, _depthMax);
    }

    /// Remove data from tree.
    void remove(DataBase& data)
    {
        if (NspTreeNode::nodeBounded(_root->_box, data.box))
            _root->remove(data);
        else
            _root->remove(data, 0, _depthMax);
    }

    /// Enumerate data objects in tree within bounding box.  Visitor functor is called for every data object found.
    void enume(EnumVisitor& visitor, const Box& box) const          { const_cast<NspTree*>(this)->enume(visitor, box); }

    /// Get box that bounds entire tree volume
    const Box& bounds() const                                       { return _root->_box; }
    /// Get max number of simultaneous enumerations allowed through recursion or threading
    const int concurMax() const                                     { return size(_concurIds); }

private:
    /// Enumerate data objects in tree within bounding box.  Visitor functor is called for every data object found.
    void enume(EnumVisitor& visitor, const Box& box);

    int                         _depthMax;
    vector<Node>                _nodes;
    Node*                       _root;
    vector<int>                 _concurIds;
    int                         _concurCount;
    SpinLock                    _concurLock;
    vector<vector<DataBase*>>   _visited;
};


/// Tree structure that recursively divides 3D space into 2 lines at each depth level.
/**
  * The bsp tree divides a line on the X axis. The line is extruded along the YZ plane (making a 3D box) to fill a desired YZ range.
  */
class BspTree : public NspTree<1>
{
public:
    BspTree(const Box& treeBox, int depthMax, int concurMax) : NspTree<1>(treeBox,depthMax,concurMax) {}
};

/// Tree structure that recursively divides 3D space into 4 rectangles at each depth level.
/**
  * The quad tree divides a rectangle on the XZ plane. The rectangle is extruded along the Y axis (making a 3D box) to fill a desired Y range.
  */
class QuadTree : public NspTree<2>
{
public:
    QuadTree(const Box& treeBox, int depthMax, int concurMax) : NspTree<2>(treeBox,depthMax,concurMax) {}
};

/// Tree structure that recursively divides 3D space into 8 cubes at each depth level.
class OctTree : public NspTree<3>
{
public:
    OctTree(const Box& treeBox, int depthMax, int concurMax) : NspTree<3>(treeBox,depthMax,concurMax) {}
};

/** \cond */
namespace priv
{
    template<class Real>
    struct NspTreeNode<Real,1>
    {
        typedef Vec<3,Real> Vec3;
        typedef Box_<Real>  Box;

        #define ENUM_LIST(e,_)  \
            e(_, left)          \
            e(_, right)         \

        ENUM(NspTreeNode<1>, Child);
        #undef ENUM_LIST

        static void getChildBox(const Vec3& min, const Vec3& max, const Vec3& center, Child index, Box& childBox)
        {
            switch(index)
            {
            case Child::left:
                childBox.min = min;
                childBox.max = Vec3(center.x, max.y, max.z);
                break;
            case Child::right:
                childBox.min = Vec3(center.x, min.y, min.z);
                childBox.max = max;
                break;
            }
        }

        static bool nodeBounded(const Box& node, const Box& data)
        {
            return  data.min.x <= node.min.x &&
                    data.max.x >= node.max.x;
        }
    };

    template<class Real>
    struct NspTreeNode<Real,2>
    {
        typedef Vec<3,Real> Vec3;
        typedef Box_<Real>  Box;

        #define ENUM_LIST(e,_)  \
            e(_, ne)            \
            e(_, se)            \
            e(_, sw)            \
            e(_, nw)            \

        ENUM(NspTreeNode<2>, Child);
        #undef ENUM_LIST

        static void getChildBox(const Vec3& min, const Vec3& max, const Vec3& center, Child index, Box& childBox)
        {
            switch(index)
            {
            case Child::ne:
                childBox.min = Vec3(center.x, min.y, center.z);
                childBox.max = max;
                break;
            case Child::se:
                childBox.min = Vec3(center.x, min.y, min.z);
                childBox.max = Vec3(max.x, max.y, center.z);
                break;
            case Child::sw:
                childBox.min = min;
                childBox.max = Vec3(center.x, max.y, center.z);
                break;
            case Child::nw:
                childBox.min = Vec3(min.x, min.y, center.z);
                childBox.max = Vec3(center.x, max.y, max.z);
                break;
            }
        }

        static bool nodeBounded(const Box& node, const Box& data)
        {
            return  data.min.x <= node.min.x &&
                    data.max.x >= node.max.x &&
                    data.min.z <= node.min.z &&
                    data.max.z >= node.max.z;
        }
    };

    template<class Real>
    struct NspTreeNode<Real,3>
    {
        typedef Vec<3,Real> Vec3;
        typedef Box_<Real>  Box;

        #define ENUM_LIST(e,_)  \
            e(_, une)           \
            e(_, use)           \
            e(_, usw)           \
            e(_, unw)           \
            e(_, lsw)           \
            e(_, lnw)           \
            e(_, lne)           \
            e(_, lse)           \

        ENUM(NspTreeNode<3>, Child);
        #undef ENUM_LIST

        static void getChildBox(const Vec3& min, const Vec3& max, const Vec3& center, Child index, Box& childBox)
        {
            switch(index)
            {
            case Child::une:
                childBox.min = center;
                childBox.max = max;
                break;
            case Child::use:
                childBox.min = Vec3(center.x, min.y, center.z);
                childBox.max = Vec3(max.x, center.y, max.z);
                break;
            case Child::usw:
                childBox.min = Vec3(min.x, min.y, center.z);
                childBox.max = Vec3(center.x, center.y, max.z);
                break;
            case Child::unw:
                childBox.min = Vec3(min.x, center.y, center.z);
                childBox.max = Vec3(center.x, max.y, max.z);
                break;
            case Child::lsw:
                childBox.min = min;
                childBox.max = center;
                break;
            case Child::lnw:
                childBox.min = Vec3(min.x, center.y, min.z);
                childBox.max = Vec3(center.x, max.y, center.z);
                break;
            case Child::lne:
                childBox.min = Vec3(center.x, center.y, min.z);
                childBox.max = Vec3(max.x, max.y, center.z);
                break;
            case Child::lse:
                childBox.min = Vec3(center.x, min.y, min.z);
                childBox.max = Vec3(max.x, center.y, center.z);
                break;
            }
        }

        static bool nodeBounded(const Box& node, const Box& data)
        {
            return  data.min.x <= node.min.x &&
                    data.max.x >= node.max.x &&
                    data.min.y <= node.min.y &&
                    data.max.y >= node.max.y &&
                    data.min.z <= node.min.z &&
                    data.max.z >= node.max.z;
        }
    };
}
/** \endcond */

}
