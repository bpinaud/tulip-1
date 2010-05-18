//-*-c++-*-
/**
 Author: Morgan Mathiaut
 Email : mathiaut@labri.fr
 Last modification : 23/10/2007
 This program is free software; you can redistribute it and/or modify
 it under the terms of the GNU General Public License as published by  
 the Free Software Foundation; either version 2 of the License, or     
 (at your option) any later version.
*/
#ifndef Tulip_GLBOUNDINGBOXSCENEVISITOR_H
#define Tulip_GLBOUNDINGBOXSCENEVISITOR_H
#ifndef DOXYGEN_NOTFOR_DEVEL

#include <tulip/BoundingBox.h>

#include "tulip/GlSceneVisitor.h"

namespace tlp {

  class GlGraphInputData;

  /** \brief Visitor to collect boundingBox of all GlEntities
   *
   * Visitor to collect boundingBox of all GlEntities
   * At end, boundingBox member contains the scene boundingBox
   */
  class TLP_GL_SCOPE GlBoundingBoxSceneVisitor : public GlSceneVisitor{
    
  public:
    
    /**
     * Constructor 
     */
    GlBoundingBoxSceneVisitor(GlGraphInputData* inputData):inputData(inputData){
      threadSafe=true;
    }
    
    /**
     * Method used for GlSimpleEntity
     */
    virtual void visit(GlSimpleEntity *entity);
    /**
     * Method used for GlNodes (and GlMetaNodes)
     */
    virtual void visit(GlNode *glNode);
    /**
     * Method used for GlEdges
     */
    virtual void visit(GlEdge *glEdge);
    
    /**
     * Return the scene boundingBox
     */
    BoundingBox getBoundingBox() {return boundingBox;}
    
  private:
    
    BoundingBox boundingBox;
    GlGraphInputData* inputData;

  };
  
}

#endif // DOXYGEN_NOTFOR_DEVEL

#endif // Tulip_GLLODSCENEVISITOR_H
