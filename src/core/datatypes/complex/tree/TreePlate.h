/**
 * @file
 * This file contains the declaration of a TreePlate a class that contains a topology and a set of variables.
 *
 * @brief Declaration of the TreePlate
 *
 * (c) Copyright 2009- under GPL version 3
 * @date Last modified: $Date:$
 * @author The RevBayes Development Core Team
 * @license GPL version 3
 * @version 1.0
 * @since 2011-04-13, version 1.0
 *
 * $Id$
 */

#ifndef TreePlate_H
#define TreePlate_H

#include "MutableMemberObject.h"
#include "Topology.h"
#include <set>
#include <string>

class TopologyNode;
class ValueRule;



const std::string TreePlate_name = "Tree Plate";

class TreePlate: public MutableMemberObject {

public:
                                    TreePlate(void);                                                                //!< Default constructor
                                    TreePlate(Topology* top);                                                       //!< constructor
                                    TreePlate(const TreePlate& t);                                                  //!< Copy constructor
                                   ~TreePlate(void);                                                                //!< Destructor

    // Basic utility functions
    TreePlate*                      clone(void) const;                                                              //!< Clone object
    const VectorString&             getClass(void) const;                                                           //!< Get class vector
    const TypeSpec&                 getTypeSpec(void) const;                                                        //!< Get language type of the object
    void                            printValue(std::ostream& o) const;                                              //!< Print value for user

    // Member variable functions
    RbLanguageObject*               executeOperation(const std::string& name, Environment* args);                   //!< Override to map member methods to internal functions
    const MemberRules*              getMemberRules(void) const;                                                     //!< Get member rules
    void                            setMemberVariable(const std::string& name, Variable* var);                      //!< Catch setting of the topology

    // Member method inits
    const MethodTable*              getMethods(void) const;                                                         //!< Get methods

    // Tree plate functions
    const Topology*                 getTopology(void) const;                                                        //!< Get the topology for this tree
    std::string                     buildNewickString(const TopologyNode* node) const;                              //!< compute the newick string for a tree rooting at this node
    
protected:
    RbLanguageObject*               executeOperationSimple(const std::string& name, Environment* args);             //!< Execute method

private:
    static const TypeSpec           typeSpec;
    
    std::vector<std::string>        nodeVariableNames;                                                              //!< The vector of variables names for each node
    
    void                            orderPlateWithTopology(Topology* t) { orderingTopology = t; }                   //!< Order the topology
    size_t                          getNodeIndex(const TopologyNode* theNode) const;                                //!< Get the index of the node
    size_t                          getTipIndex(const TopologyNode* theNode) const;                                 //!< Get the index of the node

    Topology*                       orderingTopology;                                                               //!< The topology that orders the tree plate
};

#endif

