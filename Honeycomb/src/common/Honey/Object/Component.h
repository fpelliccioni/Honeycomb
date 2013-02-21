// Honeycomb, Copyright (C) 2013 Daniel Carter.  Distributed under the Boost Software License v1.0.
#pragma once

#include "Honey/Object/PropertyObject.h"
#include "Honey/Object/ListenerList.h"
#include "Honey/Graph/Tree.h"
#include "Honey/Graph/Dep.h"

namespace honey
{

/// Methods to create and operate on components and component objects
/**
  * \defgroup Component Component System
  */
/// @{

/// Forwards to COMPONENT_\#args
#define COMPONENT(...)                          EVAL(TOKCAT(COMPONENT_, NUMARGS(__VA_ARGS__))(__VA_ARGS__))
/// Declare a component. Call inside class definition.
/**
  * Every component type must have a unique type id (uses class name by default).
  */
#define COMPONENT_1(Class)                      COMPONENT_2(Class, #Class)
/// Declare a component with a custom type id
#define COMPONENT_2(Class, typeId)              COMPONENT_SUB(, Class, typeId)

/// Forwards to COMPONENT_SUB_\#args
#define COMPONENT_SUB(...)                      EVAL(TOKCAT(COMPONENT_SUB_, NUMARGS(__VA_ARGS__))(__VA_ARGS__))
/// Declare a derived component that inherits from component class `SuperCom`.  A derived component can be typecasted to its super component.
#define COMPONENT_SUB_2(SuperCom, Class)        COMPONENT_SUB_3(SuperCom, Class, #Class)
/// Declare a derived component with a custom type id
#define COMPONENT_SUB_3(SuperCom, Class, typeId)                                                                    \
    friend class ComRegistry;                                                                                       \
                                                                                                                    \
    typedef SharedPtr<Class> Ptr;                                                                                   \
    typedef SharedPtr<const Class> ConstPtr;                                                                        \
                                                                                                                    \
    /** Static function to get type */                                                                              \
    mt_staticObj(ComRegistry::Type, s_comType,                                                                      \
        (ComRegistry::Type(typeId, static_cast<Class*>(nullptr) IFEMPTY(, COMMA static_cast<SuperCom*>(nullptr), SuperCom))))   \
    /** Override from Component */                                                                                  \
    virtual const ComRegistry::Type& comType() const        { return s_comType(); }                                 \
    

/// Define to enable auto-registration 
/**
  * Define this for the main translation unit only and include Honeycomb.h and all user component headers.
  * In this way duplicate registration is avoided, which slows down program startup.
  */
#ifdef Component_reg
    /// Register a component class.  Call outside class definition.
    #define COMPONENT_REG(Class)                static mt::Void TOKCAT(__autoReg_,__COUNTER__)(ComRegistry::inst().reg<UNBRACKET(Class)>());
#else
    #define COMPONENT_REG(Class)
#endif

class Component;

/// Component methods
namespace component
{ 
    /// Called by registry to create a component.  May be specialized for a component type.
    template<class Com>
    typename std::enable_if<std::is_default_constructible<Com>::value && !std::is_abstract<Com>::value, Component&>::type
        create() { return *new Com; }
    template<class Com>
    typename mt::disable_if<std::is_default_constructible<Com>::value && !std::is_abstract<Com>::value, Component&>::type
        create() { error("ComRegistry can't create non-default-constructible components. Must specialize component::create()."); return *static_cast<Component*>(nullptr); }
}

/// Holds global list of all component types
class ComRegistry
{
    friend class ComObject;
public:
    /// Component type
    class Type
    {
        friend class ComRegistry;
    public:
        template<class Com>
        Type(const Id& id, Com*) :
            _id(id), _node(this), _create(&component::create<Com>),
            _depNode(this, id), _depCreate(&Com::createTypeDep), _depOrder(-1)  {}

        template<class Com, class SuperCom>
        Type(const Id& id, Com* com, SuperCom*)             : Type(id, com) { _node.setParent(&SuperCom::s_comType()._node); }

        bool operator==(const Type& rhs) const              { return _id == rhs._id; }
        bool operator!=(const Type& rhs) const              { return !operator==(rhs); }
        bool operator==(const Id& rhs) const                { return _id == rhs; }
        bool operator!=(const Id& rhs) const                { return !operator==(rhs); }

        /// Get name
        const Id& id() const                                { return _id; }
        /// Get hierarchy: this type and all supertypes.  Base supertype is at front, this type is at back.
        const vector<Type*>& hierarchy() const              { return _hierarchy; }
        /// Get dependency order.  A component type can depend only on those with a lower order.
        int depOrder() const                                { return _depOrder; }

        operator const Id&() const                          { return _id; }

        /// Returns true if this type is `base` or inherits from `base`
        bool isSubtypeOf(const Id& base) const
        {
            //For an early out, start at deepest subtype
            auto range = reversed(_hierarchy);
            return find(range, [&](elemtype(range)& e) { return *e == base; }) != range.end();
        }

        /// To string
        friend StringStream& operator<<(StringStream& os, const Type& val)  { return os << val.id(); }

    private:
        typedef DepNode<Type*> DepNode;

        Id _id;
        TreeNode<Type*> _node;
        vector<Type*> _hierarchy;
        function<Component& ()> _create;
        DepNode _depNode;
        function<DepNode ()> _depCreate;
        int _depOrder;
    };
    /// Component type dep node.  Type name is used as key.
    typedef Type::DepNode DepNode;
    typedef DepGraph<DepNode> DepGraph;

    /// Get singleton
    mt_staticObj(ComRegistry, inst,);

    /// Register a component type
    template<class Com>
    mt::Void reg()
    {
        Type& type = Com::s_comType();
        if (_types.find(type) != _types.end()) return mt::Void(); //Already registered
        _types[type] = &type;
        //build hierarchy
        for (auto* parent = &type._node; parent; parent = parent->getParent())
            type._hierarchy.insert(type._hierarchy.begin(), **parent);
        return mt::Void();
    }

    /// Create a component from a type
    Component& create(const Id& type) const
    {
        auto it = _types.find(type);
        assert(it != _types.end(), sout() << "Component type not registered: " << type);
        return it->second->_create();
    }

    /// Get component type with id
    Type& type(const Id& id) const
    {
        auto it = _types.find(id);
        assert(it != _types.end(), sout() << "Component type not registered: " << id);
        return *it->second;
    }

    /// Get dependency graph for all component types
    const DepGraph& depGraph() const                        { return _depGraph; }

private:
    mt::Void buildDepGraph();

    unordered_map<Id, Type*> _types;
    DepGraph _depGraph;
};

class ComObject;

/// Base class for components.  Components can only be attached to one ComObject at a time.
class Component : public PropertyObject
{
    friend class ComRegistry;
    friend class ComObject;
public:
    typedef SharedPtr<Component> Ptr;
    typedef SharedPtr<const Component> ConstPtr;

    Component()                                             : _comObj(nullptr) {}
    virtual ~Component() {}

    /// Get type of component
    virtual const ComRegistry::Type& comType() const = 0;

    /// Override from object, updates ComObject map
    virtual void setInstId(const Id& id);

    /// Get ComObject that this component belongs to
    ComObject& obj() const                                  { assert(hasObj()); return *_comObj; }
    /// Check if component is attached to a ComObject
    bool hasObj() const                                     { return _comObj; }

    /// Get listener list
    ListenerList& listeners()                               { return _listeners; }

protected:
    /// Called when dep graph is built to determine component type dependencies.  May be overridden by subclass.
    static ComRegistry::DepNode createTypeDep()             { return ComRegistry::DepNode(); }

    /// Called after component is inserted into ComObj
    virtual void onComInsert() {}
    /// Called before component is removed from ComObj
    virtual void onComRemove() {}

private:
    ComObject* _comObj;
    ListenerList _listeners;
};

/// @}

}
