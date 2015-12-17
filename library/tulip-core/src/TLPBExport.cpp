/**
 *
 * This file is part of Tulip (www.tulip-software.org)
 *
 * Authors: David Auber and the Tulip development Team
 * from LaBRI, University of Bordeaux
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
#include <tulip/TLPBExportImport.h>
#include <tulip/TlpTools.h>
#include <tulip/PropertyTypes.h>
#include <tulip/GraphProperty.h>


PLUGIN(TLPBExport)

using namespace tlp;
using namespace std;

//================================================================================
void TLPBExport::getSubGraphs(Graph* g, vector<Graph*>& vsg) {
  // get subgraphs in a vector
  Graph *sg;
  forEach(sg, g->getSubGraphs()) {
    vsg.push_back(sg);
    getSubGraphs(sg, vsg);
  }
}
//================================================================================
void TLPBExport::writeAttributes(ostream &os, Graph *g) {
  const DataSet& attributes = g->getAttributes();

  if (!attributes.empty()) {
    // If nodes and edges are stored as graph attributes
    // we need to update their ids before serializing them
    // as nodes and edges have been reindexed
    pair<string, DataType*> attribute;
    forEach(attribute, attributes.getValues()) {
      if (attribute.second->getTypeName() == string(typeid(node).name())) {
        node *n = reinterpret_cast<node*>(attribute.second->value);
        n->id = getNode(*n).id;
      }
      else if (attribute.second->getTypeName() == string(typeid(edge).name())) {
        edge *e = reinterpret_cast<edge*>(attribute.second->value);
        e->id = getEdge(*e).id;
      }
      else if (attribute.second->getTypeName() == string(typeid(vector<node>).name())) {
        vector<node> *vn = reinterpret_cast<vector<node>*>(attribute.second->value);

        for (size_t i = 0 ; i < vn->size() ; ++i) {
          (*vn)[i].id = getNode((*vn)[i]).id;
        }
      }
      else if (attribute.second->getTypeName() == string(typeid(vector<edge>).name())) {
        vector<edge> *ve = reinterpret_cast<vector<edge>*>(attribute.second->value);

        for (size_t i = 0 ; i < ve->size() ; ++i) {
          (*ve)[i].id = getEdge((*ve)[i]).id;
        }
      }
    }
  }

  unsigned int id = g->getSuperGraph() == g ? 0 : g->getId();
  // write graph id
  os.write((char *) &id, sizeof(id));
  // write graph attributes
  DataSet::write(os, attributes);
  // do not forget to write the end marker
  os.put(')');

  // write subgraph attributes
  Graph* sg;
  forEach(sg, g->getSubGraphs())
  writeAttributes(os, sg);
}
//================================================================================
bool TLPBExport::exportGraph(std::ostream &os) {

  // change graph parent in hierarchy temporarily to itself as
  // it will be the new root of the exported hierarchy
  Graph *superGraph = graph->getSuperGraph();
  graph->setSuperGraph(graph);

  // header
  TLPBHeader header(graph->numberOfNodes(), graph->numberOfEdges());
  // write header
  os.write((char *) &header, sizeof(header));
  // reindex nodes/edges
  {
    unsigned int i = 0;
    node n;
    forEach(n, graph->getNodes()) {
      nodeIndex.set(n.id, node(i));
      ++i;
    }
    i = 0;
    edge e;
    forEach(e, graph->getEdges()) {
      edgeIndex.set(e.id, edge(i));
      ++i;
    }
  }
  // loop to write edges
  {
    pluginProgress->setComment("writing edges...");
    // use a vector as buffer
    std::vector< std::pair<node, node> > vEdges(MAX_EDGES_TO_WRITE);
    edge e;
    unsigned int edgesToWrite = 0, nbWrittenEdges = 0;
    forEach(e, graph->getEdges()) {
      std::pair<node, node> ends = graph->ends(e);
      ends.first = getNode(ends.first);
      ends.second = getNode(ends.second);
      vEdges[edgesToWrite] = ends;

      if (++edgesToWrite == MAX_EDGES_TO_WRITE) {
        // write already buffered edges
        os.write((char *) vEdges.data(),
                 MAX_EDGES_TO_WRITE * sizeof(vEdges[0]));
        nbWrittenEdges += edgesToWrite;

        if (pluginProgress->progress(nbWrittenEdges, header.numNodes)
            != TLP_CONTINUE)
          return pluginProgress->state()!=TLP_CANCEL;

        edgesToWrite = 0;
      }
    }

    if (edgesToWrite) {
      // write last buffered edges
      os.write((char *) vEdges.data(),
               edgesToWrite * sizeof(std::pair<node, node>));
    }
  }
  // write subgraphs
  std::vector<Graph*> vSubGraphs;
  // get subgraphs in a vector
  getSubGraphs(graph, vSubGraphs);
  unsigned int numSubGraphs = vSubGraphs.size();
  {
    pluginProgress->setComment("writing subgraphs...");
    // write nb subgraphs
    os.write((char *) &numSubGraphs, sizeof(numSubGraphs));

    for(unsigned int i = 0; i < numSubGraphs; ++i) {
      Graph* sg = vSubGraphs[i];
      std::pair<unsigned int, unsigned int> ids(sg->getId(),
          sg->getSuperGraph()->getSuperGraph() == sg->getSuperGraph() ? 0 : sg->getSuperGraph()->getId());
      // write ids
      os.write((char *) &ids, sizeof(ids));
      // loop to write sg nodes ranges
      {
        // use a vector as buffer

        std::vector<std::vector<std::pair<node, node> > > vRangesVec;
        vRangesVec.push_back(std::vector<std::pair<node, node> >(MAX_RANGES_TO_WRITE));
        std::vector<std::pair<node, node> > &vRanges = vRangesVec.back();

        unsigned int rangesToWrite = 0;
        unsigned int numRanges = 0;

        bool pendingWrite = false;
        node beginNode, lastNode, current;
        forEach(current, sg->getNodes()) {
          current = getNode(current);
          pendingWrite = true;

          if (!beginNode.isValid())
            beginNode = lastNode = current;
          else {
            if (current.id == lastNode.id + 1)
              lastNode = current;
            else {
              vRanges[rangesToWrite++] = std::pair<node, node>(beginNode, lastNode);
              ++numRanges;
              beginNode = lastNode = current;

              if (rangesToWrite == MAX_RANGES_TO_WRITE) {
                vRangesVec.push_back(std::vector<std::pair<node, node> >(MAX_RANGES_TO_WRITE));
                vRanges = vRangesVec.back();
                rangesToWrite = 0;
                pendingWrite = false;
              }
            }
          }
        }

        if (pendingWrite) {
          // insert last range in buffer
          vRanges[rangesToWrite++] = std::pair<node, node>(beginNode, lastNode);
          ++numRanges;
        }

        // mark nb ranges
        os.write((char *) &numRanges, sizeof(numRanges));
        // write already buffered ranges
        int numRangesV = numRanges / MAX_RANGES_TO_WRITE;

        for (int j = 0 ; j < numRangesV ; ++j) {
          os.write((char *) vRangesVec[j].data(),
                   MAX_RANGES_TO_WRITE * sizeof(vRangesVec[j][0]));
        }

        // write last buffered ranges
        os.write((char *) vRanges.data(),
                 rangesToWrite * sizeof(vRanges[0]));

      }
      // loop to write sg edges ranges
      {
        // use a vector as buffer
        std::vector<std::vector<std::pair<edge, edge> > > vRangesVec;
        vRangesVec.push_back(std::vector<std::pair<edge, edge> >(MAX_RANGES_TO_WRITE));
        std::vector<std::pair<edge, edge> > &vRanges = vRangesVec.back();

        unsigned int rangesToWrite = 0;
        unsigned int numRanges = 0;

        bool pendingWrite = false;
        edge beginEdge, lastEdge, current;
        forEach(current, sg->getEdges()) {
          current = getEdge(current);
          pendingWrite = true;

          if (!beginEdge.isValid())
            beginEdge = lastEdge = current;
          else {
            if (current.id == lastEdge.id + 1)
              lastEdge = current;
            else {
              vRanges[rangesToWrite++] = std::pair<edge, edge>(beginEdge, lastEdge);
              ++numRanges;
              beginEdge = lastEdge = current;

              if (rangesToWrite == MAX_RANGES_TO_WRITE) {
                vRangesVec.push_back(std::vector<std::pair<edge, edge> >(MAX_RANGES_TO_WRITE));
                vRanges = vRangesVec.back();
                rangesToWrite = 0;
                pendingWrite = false;
              }
            }
          }
        }

        if (pendingWrite) {
          // insert last range in buffer
          vRanges[rangesToWrite++] = std::pair<edge, edge>(beginEdge, lastEdge);
          ++numRanges;
        }

        // mark nb ranges
        os.write((char *) &numRanges, sizeof(numRanges));
        // write already buffered ranges
        int numRangesV = numRanges / MAX_RANGES_TO_WRITE;

        for (int j = 0 ; j < numRangesV ; ++j) {
          os.write((char *) vRangesVec[j].data(),
                   MAX_RANGES_TO_WRITE * sizeof(vRangesVec[j][0]));
        }

        // write last buffered ranges
        os.write((char *) vRanges.data(),
                 rangesToWrite * sizeof(vRanges[0]));
      }

      if (pluginProgress->progress(i, numSubGraphs)
          != TLP_CONTINUE)
        return pluginProgress->state()!=TLP_CANCEL;
    }
  }
  // write properties
  {
    pluginProgress->setComment("writing properties...");
    unsigned int numProperties = 0;
    std::vector<PropertyInterface*> props;
    std::set<PropertyInterface*> rootProps;
    PropertyInterface* prop;
    // get local properties in a vector
    forEach(prop, graph->getObjectProperties()) {
      props.push_back(prop);
      rootProps.insert(prop);
      ++numProperties;
    }

    // get subgraphs local properties too
    for (unsigned int i = 0; i < numSubGraphs; ++i) {
      Graph* sg = vSubGraphs[i];
      forEach(prop, sg->getLocalObjectProperties()) {
        props.push_back(prop);
        ++numProperties;
      }
    }

    // write nb properties
    os.write((char *) &numProperties, sizeof(numProperties));

    // loop on properties
    for (unsigned int i = 0; i < numProperties; ++i) {
      prop = props[i];
      std::string nameOrType = prop->getName();
      unsigned int size = nameOrType.size();
      // write property name
      os.write((char *) &size, sizeof(size));
      os.write((char *) nameOrType.data(), size);
      // write graph id
      size = rootProps.find(prop) != rootProps.end() ? 0 : prop->getGraph()->getId();
      os.write((char *) &size, sizeof(size));
      // special treament for pathnames view properties
      bool pnViewProp = (nameOrType == string("viewFont") ||
                         nameOrType == string("viewTexture"));
      // write type
      nameOrType = prop->getTypename();
      size = nameOrType.size();
      os.write((char *) &size, sizeof(size));
      os.write((char *) nameOrType.data(), size);

      if (pnViewProp && !TulipBitmapDir.empty()) {
        string defVal = prop->getNodeDefaultStringValue();
        size_t pos = defVal.find(TulipBitmapDir);

        if (pos != string::npos)
          defVal.replace(pos, TulipBitmapDir.size(), "TulipBitmapDir/");

        StringType::writeb(os, defVal);

        defVal = prop->getEdgeDefaultStringValue();
        pos = defVal.find(TulipBitmapDir);

        if (pos != string::npos)
          defVal.replace(pos, TulipBitmapDir.size(), "TulipBitmapDir/");

        StringType::writeb(os, defVal);
      }
      else {
        // write node default value
        prop->writeNodeDefaultValue(os);
        // write edge default value
        prop->writeEdgeDefaultValue(os);
      }

      // write nodes values
      {
        // write nb of non default values
        size = prop->numberOfNonDefaultValuatedNodes(rootProps.find(prop) != rootProps.end() ? graph : NULL);
        os.write((char *) &size, sizeof(size));
        // prepare ouput stream
        stringstream vs;

        // seems there is a bug in emscripten that prevents to use the stringstream buffer optimization,
        // so fallback  writing directly to the output stream in that case
        bool emscripten = false;
#ifdef __EMSCRIPTEN__
        emscripten = true;
        std::ostream &s = os;
#else
        std::ostream &s = vs;
#endif
        char* vBuf = NULL;
        unsigned int valueSize = prop->nodeValueSize();

        if (valueSize && !emscripten) {
          // allocate a special buffer for values
          // this will ease the write of a bunch of values
          vBuf = (char *) malloc(MAX_VALUES_TO_WRITE * (sizeof(unsigned int) + valueSize));
          vs.rdbuf()->pubsetbuf((char *) vBuf,
                                MAX_VALUES_TO_WRITE * (sizeof(unsigned int) + valueSize));
        }

        // loop on nodes
        node n;
        unsigned int nbValues = 0;
        forEach(n, prop->getNonDefaultValuatedNodes(rootProps.find(prop) != rootProps.end() ? graph : NULL)) {
          size = getNode(n).id;
          s.write((char *) &size, sizeof(size));

          if (pnViewProp && !TulipBitmapDir.empty()) { // viewFont || viewTexture
            string sVal = prop->getNodeStringValue(n);
            size_t pos = sVal.find(TulipBitmapDir);

            if (pos != string::npos)
              sVal.replace(pos, TulipBitmapDir.size(), "TulipBitmapDir/");

            StringType::writeb(s, sVal);
          }
          else
            prop->writeNodeValue(s, n);

          ++nbValues;

          if (nbValues == MAX_VALUES_TO_WRITE && !emscripten) {
            // write already buffered values
            if (vBuf)
              os.write(vBuf, MAX_VALUES_TO_WRITE * (sizeof(unsigned int) + valueSize));
            else {
              std::string sbuf = vs.str();
              size = (unsigned int) vs.tellp();
              // write buffer
              os.write(sbuf.c_str(), size);
            }

            // reset for next write
            vs.seekp(0, ios_base::beg);
            nbValues = 0;
          }
        }

        if (nbValues && !emscripten) {
          // write last buffered values
          if (vBuf) {
            os.write(vBuf, nbValues * (sizeof(unsigned int) + valueSize));
            free(vBuf);
          }
          else {
            std::string sbuf = vs.str();
            size = (unsigned int) vs.tellp();
            // write buffer
            os.write(sbuf.c_str(), size);
          }
        }
      }
      // write edges values
      {
        // write nb of non default values
        size = prop->numberOfNonDefaultValuatedEdges(rootProps.find(prop) != rootProps.end() ? graph : NULL);
        os.write((char *) &size, sizeof(size));
        // prepare ouput stream
        stringstream vs;

        // seems there is a bug in emscripten that prevents to use the stringstream buffer optimization,
        // so fallback  writing directly to the output stream in that case
        bool emscripten = false;
#ifdef __EMSCRIPTEN__
        emscripten = true;
        ostream &s = os;
#else
        ostream &s = vs;
#endif
        char* vBuf = NULL;
        unsigned int valueSize = prop->edgeValueSize();
        bool isGraphProperty = false;

        if (valueSize && !emscripten) {
          // allocate a special buffer for values
          // this will ease the write of a bunch of values
          vBuf = (char *) malloc(MAX_VALUES_TO_WRITE * (sizeof(unsigned int) + valueSize));
          vs.rdbuf()->pubsetbuf((char *) vBuf,
                                MAX_VALUES_TO_WRITE * (sizeof(unsigned int) + valueSize));
        }
        else {
          if (prop->getTypename() == GraphProperty::propertyTypename)
            isGraphProperty = true;
        }

        // loop on edges
        edge e;
        unsigned int nbValues = 0;
        forEach(e, prop->getNonDefaultValuatedEdges(rootProps.find(prop) != rootProps.end() ? graph : NULL)) {
          size = getEdge(e).id;
          s.write((char *) &size, sizeof(size));

          if (isGraphProperty) {
            // re-index embedded edges
            const set<edge>& edges = ((GraphProperty*)prop)->getEdgeValue(e);
            set<edge> rEdges;
            set<edge>::const_iterator its;

            for (its = edges.begin(); its != edges.end(); ++its) {
              rEdges.insert(getEdge(*its));
            }

            // finally save set
            EdgeSetType::writeb(s, rEdges);
          }
          else if (pnViewProp && !TulipBitmapDir.empty()) {   // viewFont || viewTexture
            string sVal = prop->getEdgeStringValue(e);
            size_t pos = sVal.find(TulipBitmapDir);

            if (pos != string::npos)
              sVal.replace(pos, TulipBitmapDir.size(), "TulipBitmapDir/");

            StringType::writeb(s, sVal);
          }
          else
            prop->writeEdgeValue(s, e);

          ++nbValues;

          if (nbValues == MAX_VALUES_TO_WRITE && !emscripten) {
            // write already buffered values
            if (vBuf)
              os.write(vBuf, MAX_VALUES_TO_WRITE * (sizeof(unsigned int) + valueSize));
            else {
              std::string sbuf = vs.str();
              size = (unsigned int) vs.tellp();
              // write buffer
              os.write(sbuf.c_str(), size);
            }

            // reset for next write
            vs.seekp(0, ios_base::beg);
            nbValues = 0;
          }
        }

        if (nbValues && !emscripten) {
          // write last buffered values
          if (vBuf) {
            os.write(vBuf, nbValues * (sizeof(unsigned int) + valueSize));
            free(vBuf);
          }
          else {
            std::string sbuf = vs.str();
            size = (unsigned int) vs.tellp();
            // write buffer
            os.write(sbuf.c_str(), size);
          }
        }
      }

      if (pluginProgress->progress(i, numProperties)
          != TLP_CONTINUE)
        return pluginProgress->state()!=TLP_CANCEL;
    }
  }
  // write graph attributes
  writeAttributes(os, graph);

  graph->setSuperGraph(superGraph);
  return true;
}
