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
#ifndef TULIPFILEDESCRIPTORWIDGET_H
#define TULIPFILEDESCRIPTORWIDGET_H

#include <tulip/TulipMetaTypes.h>
#include <QtGui/QWidget>

class TLP_QT_SCOPE TulipFileDescriptorWidget: public QWidget {
  Q_OBJECT

  TulipFileDescriptor _data;
  QWidget* _display;
public:
  explicit TulipFileDescriptorWidget(QWidget* parent = 0);

  void setData(TulipFileDescriptor);
  TulipFileDescriptor data() const;

protected slots:
  void browse();
};

#endif // TULIPFILEDESCRIPTORWIDGET_H
