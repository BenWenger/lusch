
#include "editortreemodel.h"

namespace lsh
{
    
    EditorTreeModel::EditorTreeModel(QObject* parent)
        : QAbstractItemModel(parent)
    {
        ///  Put in some dummy data (this is temporary)
        
        appendItem(nullptr, "Root Node", "Launch:  Root");

        Node* f1 = appendItem(nullptr, "Folder 1", "");
        appendItem(f1, "Item 1-A", "Launch:  1-A");
        appendItem(f1, "Item 1-B", "Launch:  1-B");
        appendItem(f1, "Item 1-C", "Launch:  1-C");

        Node* f2 = appendItem(nullptr, "Folder 2", "");
        appendItem(f2, "Item 2-A", "Launch:  2-A");
        appendItem(f2, "Item 2-B", "Launch:  2-B");
        appendItem(f2, "Item 2-C", "Launch:  2-C");
        appendItem(f2, "Item 2-D", "Launch:  2-D");
        appendItem(f2, "Item 2-E", "Launch:  2-E");
    }

    auto EditorTreeModel::appendItem(Node* parent, const QString& label, const QString& launch) -> Node*
    {
        NodeVec& vec = (parent ? parent->children : nodes);

        auto item = std::make_unique<Node>();
        auto ptr = item.get();
        ptr->label =            label;
        ptr->launchString =     launch;
        ptr->parent =           parent;
        ptr->index =            createIndex( static_cast<int>(vec.size()), 0, ptr );

        vec.emplace_back( std::move(item) );
        return ptr;
    }
    
    auto EditorTreeModel::getNodeFromIndex(const QModelIndex& index) -> Node*
    {
        return reinterpret_cast<Node*>(index.internalPointer());
    }
    
    auto EditorTreeModel::getChildrenFromIndex(const QModelIndex& index) const -> const NodeVec&
    {
        if(index.isValid())     return getNodeFromIndex(index)->children;
        else                    return nodes;
    }

    auto EditorTreeModel::getChildrenFromIndex(const QModelIndex& index) -> NodeVec&
    {
        if(index.isValid())     return getNodeFromIndex(index)->children;
        else                    return nodes;
    }
    
    //////////////////////////////////////////////
    //////////////////////////////////////////////

    int EditorTreeModel::columnCount(const QModelIndex& parent) const
    {
        return 1;
    }

    int EditorTreeModel::rowCount(const QModelIndex& parent) const
    {
        return static_cast<int>(getChildrenFromIndex(parent).size());
    }

    QVariant EditorTreeModel::data(const QModelIndex& data, int role) const
    {
        if(!data.isValid())
            return QVariant();

        auto node = getNodeFromIndex(data);
        switch(role)
        {
        case Qt::DisplayRole:   return node->label;
        case LaunchRole:        return node->launchString;
        }

        return QVariant();
    }

    QModelIndex EditorTreeModel::index(int row, int column, const QModelIndex& parent) const
    {
        if(column != 0)         return QModelIndex();
        if(row < 0)             return QModelIndex();

        auto& vec = getChildrenFromIndex(parent);
        int siz = static_cast<int>(vec.size());
        if(row >= siz)          return QModelIndex();

        return vec[row]->index;
    }

    QModelIndex EditorTreeModel::parent(const QModelIndex& index) const
    {
        if(!index.isValid())    return QModelIndex();

        Node* node = getNodeFromIndex(index)->parent;
        if(node)                return node->index;
        else                    return QModelIndex();
    }
}
