/*
Copyright (C) 2018 Adrian Michel
http://www.amichel.com

This program is free software: you can redistribute it and/or modify
it under the terms of the GNU General Public License as published by
the Free Software Foundation, either version 3 of the License, or
(at your option) any later version.

This program is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
GNU General Public License for more details.

You should have received a copy of the GNU General Public License
along with this program.  If not, see <http://www.gnu.org/licenses/>.
*/

#pragma once

#include "tree.h"
#include "plugintree.h"

// T is the node type (Node or DocumentNode )
template <class T>
class Event {
 public:
  enum Type { add, remove, change };

 private:
  const Type _type;
  const T* _node;
  const UniqueId* _id;
  const UniqueId* _parentId;
  const bool _ensureVisible;

 public:
  Event(Type type, const T* node, const UniqueId* id, const UniqueId* parentId,
        bool ensureVisible)
      : _type(type),
        _node(node),
        _parentId(parentId),
        _ensureVisible(ensureVisible),
        _id(id) {}
  Type type() const { return _type; }
  const T* node() const { return _node; }
  const UniqueId* parentId() const { return _parentId; }
  bool ensureVisible() const { return _ensureVisible; }
  const UniqueId* id() const { return _id; }
};

template <class T>
class View {
 public:
  virtual ~View() {}

  virtual void update(const Event<T>& event) = 0;
  virtual bool allowRemove(const UniqueIdPtrVector& ids) const = 0;
  virtual bool allowChange(const UniqueIdPtrVector& ids) const = 0;
};

class Command {};

class Node : public Info {
 public:
  enum NodeType {
    MIN_NODE_TYPES,
    DATASOURCE,
    SYMBOLSSOURCE,
    RUNNABLE,
    SLIPPAGE,
    COMMISSION,
    SIGNALHANDLER,
    MAX_NODE_TYPES
  };

  enum NodeSubtype {
    MIN_NODE_SUBTYPES,
    CONFIG,
    PLUGIN,
    ROOT,
    MAX_NODE_SUBTYPES
  };

 public:
  Node(const Info& info) : Info(info) {}

  virtual ~Node() {}
  virtual NodeType type() const = 0;
  virtual NodeSubtype subtype() const = 0;

  static const std::string typeToString(NodeType type) {
    switch (type) {
      case DATASOURCE:
        return "Data Source";
      case SYMBOLSSOURCE:
        return "Symbols Source";
      case RUNNABLE:
        return "Runnable";
      case SLIPPAGE:
        return "Slippage";
      case COMMISSION:
        return "Commission";
      case SIGNALHANDLER:
        return "Signal";
      default:
        ASSERT(false);
        return "";
    }
  }
};

template <Node::NodeType t>
class NodeBase : public Node {
 public:
  NodeBase(const Info& info) : Node(info) {}
  NodeType type() const { return t; }
};

// config info
template <Node::NodeType t>
class ConfigNode : public NodeBase<t> {
 public:
  ConfigNode(const Info& info) : NodeBase<t>(info) {}

  virtual NodeSubtype subtype() const { return CONFIG; }
};

class Pathable {
 public:
  virtual ~Pathable() {}

  virtual const std::string& path() const = 0;
};

template <Node::NodeType t>
class PluginNode : public NodeBase<t>, public Pathable {
 private:
  const std::string _path;

 public:
  PluginNode(const Info& info, const std::string& path)
      : NodeBase<t>(info), _path(path) {}
  virtual NodeSubtype subtype() const { return PLUGIN; }
  const std::string& path() const { return _path; }
};

template <Node::NodeType t>
class PluginsRootNode : public NodeBase<t> {
 public:
  PluginsRootNode() : NodeBase<t>(Info(makeName(), makeDescription())) {}

 private:
  static const std::string makeName() {
    switch (t) {
      case DATASOURCE:
        return "Data Source Plugins";
      case SYMBOLSSOURCE:
        return "Symbols Source Plugins";
      case RUNNABLE:
        return "Runnable Plugins";
      case SLIPPAGE:
        return "Slippage Plugins";
      case COMMISSION:
        return "Commission Plugins";
      case SIGNALHANDLER:
        return "Signal Handler Plugins";
      default:
        ASSERT(false);
        return "";
    }
  }
  static const std::string makeDescription() {
    switch (t) {
      case DATASOURCE:
        return "Data Source Plugins";
      case SYMBOLSSOURCE:
        return "Symbols Source Plugins";
      case RUNNABLE:
        return "Runnable Plugins";
      case SLIPPAGE:
        return "Slippage Plugins";
      case COMMISSION:
        return "Commission Plugins";
      case SIGNALHANDLER:
        return "Signal Handler Plugins";
      default:
        ASSERT(false);
        return "";
    }
  }

 public:
  virtual NodeSubtype subtype() const { return ROOT; }
};

typedef ManagedPtr<Node> NodePtr;

inline bool operator==(const NodePtr& node, const UniqueId& id) {
  return node->id() == id;
}

typedef ConfigNode<Node::NodeType::DATASOURCE> DataSourceConfigNode;
typedef ConfigNode<Node::NodeType::SYMBOLSSOURCE> SymbolsSourceConfigNode;
typedef ConfigNode<Node::NodeType::RUNNABLE> RunnableConfigNode;
typedef ConfigNode<Node::NodeType::COMMISSION> CommissionConfigNode;
typedef ConfigNode<Node::NodeType::SLIPPAGE> SlippageConfigNode;
typedef ConfigNode<Node::NodeType::SIGNALHANDLER> SignalHandlerConfigNode;

typedef PluginNode<Node::NodeType::DATASOURCE> DataSourcePluginNode;
typedef PluginNode<Node::NodeType::SYMBOLSSOURCE> SymbolsSourcePluginNode;
typedef PluginNode<Node::NodeType::RUNNABLE> RunnablePluginNode;
typedef PluginNode<Node::NodeType::COMMISSION> CommissionPluginNode;
typedef PluginNode<Node::NodeType::SLIPPAGE> SlippagePluginNode;
typedef PluginNode<Node::NodeType::SIGNALHANDLER> SignalHandlerPluginNode;

typedef PluginsRootNode<Node::NodeType::DATASOURCE> DataSourcePluginRootNode;
typedef PluginsRootNode<Node::NodeType::SYMBOLSSOURCE>
    SymbolsSourcePluginRootNode;
typedef PluginsRootNode<Node::NodeType::RUNNABLE> RunnablePluginRootNode;
typedef PluginsRootNode<Node::NodeType::COMMISSION> CommissionPluginRootNode;
typedef PluginsRootNode<Node::NodeType::SLIPPAGE> SlippagePluginRootNode;
typedef PluginsRootNode<Node::NodeType::SIGNALHANDLER>
    SignalHandlerPluginRootNode;

class Traverser {
 public:
  virtual ~Traverser() {}
  virtual void node(const Node* node) = 0;
};

class WrongPluginTypeException {
 private:
  const std::string _message;

 public:
  WrongPluginTypeException(const std::string& message) : _message(message) {}

  const std::string& message() const { return _message; }
};

class IdNotFoundException {
 private:
  const UniqueId _id;
  std::string _message;

 public:
  IdNotFoundException(const UniqueId& id) : _id(id) {
    std::ostringstream o;
    o << "Id " << id.toString() << " not found";
    _message = o.str();
  }

  const std::string& message() const { return _message; }

  const UniqueId& id() const { return _id; }
};

class PluginTree : public tree<NodePtr>, public PluginExplorer {
 private:
  const iterator _dataSourcePluginRoot;
  const iterator _symbolsSourcePluginRoot;
  const iterator _runnablePluginRoot;
  const iterator _slippagePluginRoot;
  const iterator _commissionPluginRoot;
  const iterator _signalHandlerPluginRoot;

 public:
  PluginTree()
      : _dataSourcePluginRoot(
            insert(begin(), NodePtr(new DataSourcePluginRootNode()))),
        _symbolsSourcePluginRoot(
            insert(begin(), NodePtr(new SymbolsSourcePluginRootNode()))),
        _runnablePluginRoot(
            insert(begin(), NodePtr(new RunnablePluginRootNode()))),
        _commissionPluginRoot(
            insert(begin(), NodePtr(new CommissionPluginRootNode()))),
        _slippagePluginRoot(
            insert(begin(), NodePtr(new SlippagePluginRootNode()))),
        _signalHandlerPluginRoot(
            insert(begin(), NodePtr(new SignalHandlerPluginRootNode())))

  {}

  template <class T, class U, class V>
  void f(const std::string& filePath, iterator root) {
    try {
      PluginInstance<T> pi(filePath);
      addPlugin<Plugin<T>, U, V>(*pi, filePath, root);
    } catch (const PluginInstanceException& e) {
      LOG(log_debug, "PluginInstanceException: " << filePath << ", message: "
                                                 << e.message());
    }
  }

  virtual void process(const std::string& filePath,
                       PluginLoadingStatusHandler* loadingStatusHandler,
                       std::vector<ManagedPtr<Info> >& duplicates) {
    try {
      if (loadingStatusHandler != 0) loadingStatusHandler->event(filePath);
      try {
        //				LOG( log_debug, _T( "trying data source
        // plugin: ") << filePath );
        f<DataSource, DataSourcePluginNode, DataSourceConfigNode>(
            filePath, _dataSourcePluginRoot);
      } catch (const PluginTreeException& e) {
        duplicates.insert(duplicates.end(), e.info().begin(), e.info().end());
      }

      try {
        f<SymbolsSource, SymbolsSourcePluginNode, SymbolsSourceConfigNode>(
            filePath, _symbolsSourcePluginRoot);
      } catch (const PluginTreeException& e) {
        duplicates.insert(duplicates.end(), e.info().begin(), e.info().end());
      }
      try {
        //				LOG( log_debug, _T( "trying runnable
        // plugin:
        //") << filePath );
        f<Runnable, RunnablePluginNode, RunnableConfigNode>(
            filePath, _runnablePluginRoot);
      } catch (const PluginTreeException& e) {
        duplicates.insert(duplicates.end(), e.info().begin(), e.info().end());
      }
      try {
        f<Commission, CommissionPluginNode, CommissionConfigNode>(
            filePath, _commissionPluginRoot);
      } catch (const PluginTreeException& e) {
        duplicates.insert(duplicates.end(), e.info().begin(), e.info().end());
      }
      try {
        f<Slippage, SlippagePluginNode, SlippageConfigNode>(
            filePath, _slippagePluginRoot);
      } catch (const PluginTreeException& e) {
        duplicates.insert(duplicates.end(), e.info().begin(), e.info().end());
      }
      try {
        f<SignalHandler, SignalHandlerPluginNode, SignalHandlerConfigNode>(
            filePath, _signalHandlerPluginRoot);
      } catch (const PluginTreeException& e) {
        duplicates.insert(duplicates.end(), e.info().begin(), e.info().end());
      }
    } catch (const PluginMethodException&) {
      // the plugin doesn't implement at least one method
      //			LOG( log_debug, "PluginMethodException" );
    } catch (const PluginException&) {
      //			LOG( log_debug, "PluginException" );
    }
  }

  void init() {}

  virtual void command(Command cmd) {}

  // TODO: implement and check assumptions: level of various nodes, no duplicate
  // ids etc
  bool assertValid() { return true; }

  template <class T, Node::NodeType U>
  std::auto_ptr<PluginInstance<T> > getPlugin(const UniqueId& id) const
      throw(PluginMethodException, PluginInstanceException,
            WrongPluginTypeException, IdNotFoundException) {
    iterator i = std::find(begin(), end(), id);
    // todo: replace this assert with an exception - this is an error
    if (i != end()) {
      const Node* node = i->get();
      // if the node is of the wrong type, throw exception
      if (node->type() != U) {
        std::ostringstream o;

        o << "Plugin " << id.toString() << " is of type "
          << Node::typeToString(node->type()) << ", expected "
          << Node::typeToString(U);
        throw WrongPluginTypeException(o.str());
      }
      assert(node->subtype() != Node::NodeSubtype::ROOT);
      // go down the tree until we find a parent that is of plugin type
      while (node->subtype() != Node::NodeSubtype::PLUGIN) {
        // call the base class parent
        i = tree<NodePtr>::parent(i);
        node = i->get();
      }

      const Pathable* pluginNode = dynamic_cast<const Pathable*>(node);

      return std::auto_ptr<PluginInstance<T> >(
          new PluginInstance<T>(pluginNode->path()));
    } else
      // couldn't find the id
      throw IdNotFoundException(id);
  }

  bool traverseChildren(const UniqueId& id, Traverser& traverser) const {
    for (iterator i = begin(); i != end(); i++) {
      if (*i == id) {
        for (sibling_iterator si = i; si != end(i); si++)
          traverser.node(si->get());

        return true;
      }
    }
    return false;
  }

 public:
  template <Node::NodeType T, Node::NodeSubtype U, class V>
  const V* findNode(const UniqueId& id) const {
    for (iterator i = begin(); i != end(); i++) {
      if (*i == id) {
        ASSERT((*i)->type() == T);
        ASSERT((*i)->subtype() == U);
        return dynamic_cast<const V*>(i->get());
      }
    }
    return 0;
  }

  const Node* findNode(const UniqueId& id) const {
    iterator i = std::find(begin(), end(), id);
    return i != end() ? i->get() : 0;
  }

  bool hasId(const UniqueId& id) const { return findNode(id) != 0; }

  template <Node::NodeType T>
  const PluginNode<T>* findPluginNode(const UniqueId& id) const {
    return findNode<T, Node::NodeSubtype::PLUGIN, PluginNode<T> >(id);
  }

  template <Node::NodeType T>
  const ConfigNode<T>* findConfigNode(const UniqueId& id) const {
    return findNode<T, Node::NodeSubtype::CONFIG, ConfigNode<T> >(id);
  }

  template <Node::NodeType T>
  const PluginsRootNode<T>* findPluginRootNode(const UniqueId& id) const {
    return findNode<T, Node::NodeSubtype::ROOT, PluginRootNode<T> >(id);
  }

  template <class V>
  void addConfig(const UniqueId& parent,
                 const Info& info) throw(PluginTreeException) {
    if (std::find(begin(), end(), info.id()) != end())
      // duplicate ids - throw exception
      throw PluginTreeException(info);
    iterator i = std::find(begin(), end(), parent);
    // parent must be in there already - assert if not
    assert(i != end());
    iterator k = append_child(i, NodePtr(new V(info)));
  }

  template <class V>
  void removeConfig(const UniqueId& id) {
    iterator i = std::find(begin(), end(), id);
    assert(i != end());
    __super::erase(i);
  }

  template <class V>
  void replaceConfig(const Info& info) throw(PluginTreeException) {
    iterator i = std::find(begin(), end(), info.id());
    assert(i != end());
    __super::replace(i, NodePtr(new V(info)));
  }

  bool checkIds(const UniqueId& id) {
    static UniqueId unauthorizedIds[] = {
        UniqueId(),
        UniqueId("02284BC4-CE53-4dd5-8925-B6B874799360")  // Greg's plugin
    };

    for (int n = 0; n < sizeof(unauthorizedIds) / sizeof(UniqueId); n++) {
      if (id == unauthorizedIds[n]) return false;
    }
    return true;
  }

  template <class T, class U, class V>
  void addPlugin(const T& plugin, const std::string& path,
                 const iterator& root) throw(PluginTreeException) {
    LOG_PLUGIN(log_debug, "trying to add plugin type: \""
                              << boost::typeindex::type_id<T>().pretty_name()
                              << "\", path" << path
                              << ", id: " << plugin.id().toString());

    PluginTreeException e;
    // check that the plug-in is not on the black  list
    // if it is, just ignore it
    if (!checkIds(plugin.id())) return;
    // the node should not be there when adding
    const Node* node = findNode(plugin.id());

    // if the node is found, add it to the exception, the id should not be
    // duplicated
    if (node != 0) {
      LOG(log_debug, "duplicate plugin found: " << path << ", id: "
                                                << plugin.id().toString());
      e.add(*node);
    } else {
      iterator i = append_child(root, NodePtr(new U(plugin, path)));

      for (InfoPtr info = plugin.first(); info.get() != 0;
           info = plugin.next()) {
        try {
          LOG_PLUGIN(log_debug,
                     "adding plugin type: \""
                         << boost::typeindex::type_id<T>().pretty_name()
                         << "\", path: \"" << path
                         << "\", info: " << info->toString())
          addConfig<V>((*i)->id(), *info);
        } catch (const PluginTreeException& ex) {
          LOG(log_error, "PluginTreeException while adding plugin id: "
                             << info->toString()
                             << ", message: " << ex.message())
          e.add(ex.info()[0]);
        }
      }
    }
    if (e.info().size() > 0) throw e;

    //		LOG( log_debug, _T( "plugin added: ") << path );
  }

  // returns the id of the plugin whose parameter id is a configuration
  // if no parent, return 0
  const UniqueId* parent(const UniqueId& id) const throw(IdNotFoundException) {
    for (iterator i = begin(); i != end(); i++) {
      if (*i == id) {
        iterator p = tree<NodePtr>::parent(i);
        if (p != end())
          return &(*p)->id();
        else
          return 0;
      }
    }
    // the id wasn't found - throw and exception
    LOG(log_error, "id not found: " << id.toString())
    throw IdNotFoundException(id);
  }
};