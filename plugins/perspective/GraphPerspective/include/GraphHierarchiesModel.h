/**
 *
 * This file is part of Tulip (www.tulip-software.org)
 *
 * Authors: David Auber and the Tulip development Team
 * from LaBRI, University of Bordeaux 1 and Inria Bordeaux - Sud Ouest
 *
 * Tulip is free software; you can redistribute it and/or modify
 * it under the terms of the GNU Lesser General Public License
 * as published by the Free Software Foundation, either version 3
 * of the License, or (at your option) any later version.
 *
 * Tulip is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.
 * See the GNU General Public License for more details.
 *
 */
#ifndef GRAPHHIERARCHIESMODEL_H
#define GRAPHHIERARCHIESMODEL_H

#include <QtCore/QAbstractItemModel>
#include <tulip/Observable.h>
#include <QtCore/QList>

namespace tlp {
class Graph;
class TulipProject;
class PluginProgress;
}

class GraphHierarchiesModel : public QAbstractItemModel, public tlp::Observable {
  Q_OBJECT

  QList<tlp::Graph *> _graphs;
  QString generateName(tlp::Graph *) const;

  tlp::Graph *_currentGraph;

public:
  static void setApplicationDefaults(tlp::Graph*);

  explicit GraphHierarchiesModel(QObject *parent=0);
  GraphHierarchiesModel(const GraphHierarchiesModel &);
  virtual ~GraphHierarchiesModel();

  // Allows the model to behave like a list and to be iterable
  typedef QList<tlp::Graph *>::iterator iterator;
  typedef QList<tlp::Graph *>::const_iterator const_iterator;
  tlp::Graph *operator[](int i) const {
    return _graphs[i];
  }
  tlp::Graph *operator[](int i) {
    return _graphs[i];
  }
  int size() const {
    return _graphs.size();
  }

  iterator begin() {
    return _graphs.begin();
  }
  iterator end() {
    return _graphs.end();
  }
  const_iterator begin() const {
    return _graphs.begin();
  }
  const_iterator end() const {
    return _graphs.end();
  }

  QList<tlp::Graph *> graphs() const {
    return _graphs;
  }

  // Methods re-implemented from QAbstractItemModel
  QModelIndex index(int row, int column,const QModelIndex &parent = QModelIndex()) const;
  QModelIndex parent(const QModelIndex &child) const;
  int rowCount(const QModelIndex &parent = QModelIndex()) const;
  int columnCount(const QModelIndex &parent = QModelIndex()) const;
  QVariant data(const QModelIndex &index, int role = Qt::DisplayRole) const;
  QVariant headerData(int section, Qt::Orientation orientation, int role) const;

  // Methods inherited from the observable system
  void treatEvent(const tlp::Event &);

  // active graph handling
  void setCurrentGraph(tlp::Graph *);
  tlp::Graph *currentGraph() const;

signals:
  void currentGraphChanged(tlp::Graph *);

public slots:
  void addGraph(tlp::Graph *);
  void removeGraph(tlp::Graph *);

  void readProject(tlp::TulipProject *,tlp::PluginProgress *);
  bool writeProject(tlp::TulipProject *, tlp::PluginProgress *);
};

#endif // GRAPHHIERARCHIESMODEL_H
