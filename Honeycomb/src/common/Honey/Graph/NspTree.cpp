// Honeycomb, Copyright (C) 2013 Daniel Carter.  Distributed under the Boost Software License v1.0.
#pragma hdrstop

#include "Honey/Graph/NspTree.h"
#include "Honey/Math/Geom/Intersect.h"

namespace honey
{

template<int Dim>
void NspTree<Dim>::Node::init(const Box& box, vector<Node>& nodes, int depth, int depthOffset)
{
    _box = box;

    int offset = this - &nodes[depthOffset];
    depthOffset += 1 << dim*depth;
    depth += 1;
    //Leaf has no children
    if (depthOffset >= size(nodes)) return;
    _children = &nodes[depthOffset + offset*NspTreeNode::Child::valMax];

    Box childBox;
    Vec3 center = _box.getCenter();
    for (auto i: range(NspTreeNode::Child::valMax))
    {
        NspTreeNode::getChildBox(_box.min, _box.max, center, NspTreeNode::Child(i), childBox);
        _children[i].init(childBox, nodes, depth, depthOffset);
    }
}

template<int Dim>
void NspTree<Dim>::Node::add(DataBase& data, int depth, int depthMax)
{
    for (int i = 0; i < NspTreeNode::Child::valMax; ++i)
    {
        //The strategy here is to reference data as high as possible in the tree
        //If the data doesn't intersect the child box then we are done
        if (!Intersect::test(_children[i]._box, data.box)) continue;

                //Add data ref if child is leaf
        if (    depth == depthMax - 1 ||
                //Also, if the data box entirely contains the child (checking only the splitting N dimensions),
                //then we can add the data ref in the child and stop recursion
                NspTreeNode::nodeBounded(_children[i]._box, data.box)
                )
            _children[i].add(data);
        else
            _children[i].add(data, depth+1, depthMax); //Recurse

        // Add child to active linked list if not already active
        if (_children[i]._activeNext != inactive) continue;
        _children[i]._activeNext = _activeFirst;
        _activeFirst = i;
    }
}

template<int Dim>
void NspTree<Dim>::Node::remove(DataBase& data, int depth, int depthMax)
{
    //Iterate over linked list of active children, keep track of previous
    for (int8 i = _activeFirst, prev = activeEnd; i != activeEnd;)
    {
        if (!Intersect::test(_children[i]._box, data.box))
        {
            prev = i;
            i = _children[i]._activeNext;
            continue;
        }

        if (depth == depthMax - 1 || NspTreeNode::nodeBounded(_children[i]._box, data.box))
            _children[i].remove(data);
        else
            _children[i].remove(data, depth+1, depthMax); //Recurse

        //Check if child has data
        if (!_children[i]._data.empty() || _children[i].hasActiveChild())
        {
            prev = i;
            i = _children[i]._activeNext;
            continue;
        }

        //No data, deactivate child, hook up linked list
        if (_activeFirst == i)
        {
            _activeFirst = _children[i]._activeNext;
            _children[i]._activeNext = inactive;
            i = _activeFirst;
        }
        else
        {
            _children[prev]._activeNext = _children[i]._activeNext;
            _children[i]._activeNext = inactive;
            i = _children[prev]._activeNext;
        }
    }
}

template<int Dim>
void NspTree<Dim>::Node::enume(EnumVisitor& visitor, int depth, int depthMax) const
{
    enume(visitor);
    //Iterate over linked list of active children
    for (int8 i = _activeFirst; i != activeEnd; i = _children[i]._activeNext)
    {
        //Intersection test
        if (!Intersect::test(_children[i]._box, visitor.box())) continue;
        if (visitor.getState() == EnumVisitor::State::stop) return;
        //If the visitor bounds the entire child, then we don't need to perform any more intersection tests
        if (depth != depthMax - 1 && NspTreeNode::nodeBounded(_children[i]._box, visitor.box()))
            _children[i].enumBounded(visitor);
        else
            _children[i].enume(visitor, depth+1, depthMax); //Recurse
    }
}

template<int Dim>
void NspTree<Dim>::Node::enumBounded(EnumVisitor& visitor) const
{
    enume(visitor);
    //Iterate over linked list of active children
    for (int8 i = _activeFirst; i != activeEnd; i = _children[i]._activeNext)
    {
        if (visitor.getState() == EnumVisitor::State::stop) return;
        _children[i].enumBounded(visitor);
    }
}

template<int Dim>
void NspTree<Dim>::Node::enume(EnumVisitor& visitor) const
{
    for (auto it = _data.begin(); it != _data.end(); ++it)
    {
        //Check if already visited
        if ((*it)->visited[visitor._concurId]) continue;
        //Intersection test
        if (!Intersect::test((*it)->box, visitor.box())) continue;
        //Visit
        (*it)->visited[visitor._concurId] = true;
        visitor._visited->push_back(*it);
        visitor(*it);

        if (visitor.getState() == EnumVisitor::State::stop) return;
    }
}

template<int Dim>
void NspTree<Dim>::enume(EnumVisitor& visitor, const Box& box)
{
    //Init visitor
    visitor._tree = this;
    visitor._box = box;
    visitor.setState(EnumVisitor::State::cont);
        
    //Pop concurrent id from list (used to keep track of visited)
    assert(_concurCount < size(_concurIds), sout() << "Max number of concurrent enumerations reached: " << size(_concurIds));
    _concurLock.lock();
    visitor._concurId = _concurIds[_concurCount++];
    _concurLock.unlock();

    visitor._visited = &_visited[visitor._concurId];

    //Perform enumeration
    _root->enume(visitor, 0, _depthMax);

    //Reset data visited marks
    for (int i = 0; i < size(*visitor._visited); ++i)
        visitor._visited->at(i)->visited[visitor._concurId] = false;
    visitor._visited->clear();

    //Push id back onto concurrent list
    _concurLock.lock();
    _concurIds[--_concurCount] = visitor._concurId;
    _concurLock.unlock();
}

template class NspTree<1>;
template class NspTree<2>;
template class NspTree<3>;

}
