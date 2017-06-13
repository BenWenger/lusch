#ifndef LUSCH_GUI_EDITORTREEMODEL_H_INCLUDED
#define LUSCH_GUI_EDITORTREEMODEL_H_INCLUDED

#include <memory>
#include <vector>
#include <string>
#include <QAbstractItemModel>

namespace lsh
{

    class EditorTreeModel : public QAbstractItemModel
    {
        Q_OBJECT

    public:
        static const int    LaunchRole = Qt::UserRole + 0;

        EditorTreeModel(QObject* parent = Q_NULLPTR);

        ////////////////////////////
        // Qt Implementation

        virtual int         columnCount(const QModelIndex& parent) const override;
        virtual int         rowCount(const QModelIndex& parent) const override;
        virtual QVariant    data(const QModelIndex& data, int role) const override;
        virtual QModelIndex index(int row, int column, const QModelIndex& parent) const override;
        virtual QModelIndex parent(const QModelIndex& index) const override;

    private:
        ////////////////////////////
        // Data
        struct                          Node;
        typedef std::unique_ptr<Node>   NodePtr;
        typedef std::vector<NodePtr>    NodeVec;

        struct Node
        {
            Node*           parent = nullptr;
            QModelIndex     index;
            QString         label;
            QString         launchString;
            NodeVec         children;
        };
        NodeVec             nodes;

        Node*               appendItem(Node* parent, const QString& label, const QString& launch);
        
        static Node*        getNodeFromIndex(const QModelIndex& index);
        const NodeVec&      getChildrenFromIndex(const QModelIndex& index) const;
        NodeVec&            getChildrenFromIndex(const QModelIndex& index);
    };

}


#endif