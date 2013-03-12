// Honeycomb, Copyright (C) 2013 Daniel Carter.  Distributed under the Boost Software License v1.0.
#pragma hdrstop

#include "Honey/Object/ComObject.h"

namespace honey
{

const vector<Component::Ptr> ComObject::_nullComs;

void ComObject::insertCom_priv(Component& com, int index, bool createDeps)
{
    static mt::Void _ = ComRegistry::inst().buildDepGraph();  //Build dep graph the first time this function is called
    mt_unused(_);
    
    Component::Ptr comPtr = &com;   //Prevent destruction in scope
    //Remove from previous obj
    if (com.hasObj()) com._comObj->removeCom(com);

    #ifdef DEBUG
        static const bool debug = true;
    #else
        static const bool debug = false;
    #endif
    if (createDeps || debug)
    {
        //Add missing component deps
        auto vertex = ComRegistry::inst().depGraph().vertex(com.comType());
        assert(vertex, sout() << "Component not registered: " << com.comType());
        for (auto& e : vertex->links())
        {
            auto& key = *e->keys().begin();
            if (hasComInSlot(key)) continue;
            if (createDeps)
                addCom(ComRegistry::inst().create(key), true);
            else
                error(sout()    << "Component dependency missing: " << key
                                << ".  Add the missing component first, or add with createDeps = true.");
        } 
    }

    //Insert into each type slot in hierarchy
    auto& typeHier = com.comType().hierarchy();
    for (auto& type : typeHier)
    {
        auto& slot = _slotMap[*type];
        if (!slot.type) slot.type = type;
        if (type == typeHier.back())
            //Main type, insert into place
            slot.list.insert(index != -1 ? slot.list.begin() + index : slot.list.end(), comPtr);
        else
            //Supertype, add to end
            slot.list.push_back(comPtr);

        //Add instance id to map
        if (com.getInstId() != idnull) slot.map[com.getInstId()] = &com;
    }

    com._comObj = this;
    com.onComInsert();
    listeners().dispatch<sigComInsert>(*this, com);
}

void ComObject::removeComInSlot(SlotMap::iterator& slotIt, Slot::List::iterator it, bool removeDeps, bool eraseSlot)
{
    Component::Ptr comPtr = *it;  //Prevent destruction in scope
    auto& com = *comPtr;
    ComRegistry::Type* slotType = slotIt->second.type;

    #ifdef DEBUG
        static const bool debug = true;
    #else
        static const bool debug = false;
    #endif
    if (removeDeps || debug)
    {
        //Remove dependent components, check each type slot in hierarchy
        for (auto& type : reversed(com.comType().hierarchy()))
        {
            //only remove deps if this is the last component
            auto slot = type == slotType ? &slotIt->second : this->slot(*type);
            assert(slot);
            if (slot->list.size() > 1) break; //checking supertypes would be redundant
            auto vertex = ComRegistry::inst().depGraph().vertex(*type);
            assert(vertex, sout() << "Component not registered: " << *type);
            for (auto& e : vertex->links(ComRegistry::DepNode::DepType::in))
            {
                auto& key = *e->keys().begin();
                if (removeDeps)
                    removeComsInSlot(key, removeDeps);
                else
                    assert(!hasComInSlot(key),
                        sout()  << "Dependent component still in object: " << key
                                << ".  Remove the dependent component first, or remove with removeDeps = true.");   
            }
        }
    }

    com.onComRemove();
    com._comObj = nullptr;

    //Remove from each type slot in hierarchy
    for (auto& type : com.comType().hierarchy())
    {
        //Get slot, use iterator if available, otherwise perform search
        auto slotIt_ = type == slotType ? slotIt : _slotMap.find(*type);
        assert(slotIt_ != _slotMap.end());
        auto& slot = slotIt_->second;

        //Erase from list, use iterator if available, otherwise perform search
        if (type == slotType)
            slot.list.erase(it);
        else
        {
            //search assuming com is at back
            auto range = reversed(slot.list);
            auto it = std::find(range.begin(), range.end(), comPtr);
            if (it != range.end()) stdutil::erase(slot.list, it);
        }

        //Erase instance id from map
        if (com.getInstId() != idnull)
        {
            auto itMap = slot.map.find(com.getInstId());
            if (itMap != slot.map.end() && itMap->second == &com)
                slot.map.erase(itMap);
        }

        //Erase slot if it doesn't hold any more components
        if (eraseSlot && slot.list.empty())
        {
            _slotMap.erase(slotIt_);
            if (type == slotType) slotIt = _slotMap.end(); //signal to caller that current slot has been erased
        }
    }

    listeners().dispatch<sigComRemove>(*this, com);
}

void ComObject::removeComsInSlot(SlotMap::iterator slotIt, bool removeDeps, bool eraseSlot)
{
    while (slotIt != _slotMap.end() && !slotIt->second.list.empty())
        removeComInSlot(slotIt, --slotIt->second.list.end(), removeDeps, eraseSlot);
}

void ComObject::removeComs()
{
    //Remove slots in dep order
    vector<SlotMap::iterator, SmallAllocator<SlotMap::iterator>> slots;
    slots.reserve(_slotMap.size());
    for (auto it = _slotMap.begin(); it != _slotMap.end(); ++it) slots.push_back(it);
    std::sort(slots.begin(), slots.end(), [](mt_elemOf(slots)& e, mt_elemOf(slots)& e2) { return e->second.type->depOrder() < e2->second.type->depOrder(); });
    for (auto& e : reversed(slots)) removeComsInSlot(e, false, false);
    _slotMap.clear();
}

void ComObject::updateComMap(Component& com, const Id& id)
{
    //Update each type slot in hierarchy
    for (auto& type : com.comType().hierarchy())
    {
        auto slot = this->slot(*type);
        assert(slot);
        auto it = slot->map.find(com.getInstId());
        if (it != slot->map.end() && it->second == &com) slot->map.erase(it);
        slot->map[id] = &com;
    }
}

}