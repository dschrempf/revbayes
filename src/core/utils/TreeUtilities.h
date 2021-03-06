/**
 * @file
 * This file contains helper functions for manipulating trees in RevBayes.
 *
 * @brief Namespace containing helper functions for trees
 *
 * (c) Copyright 2009- under GPL version 3
 * @date Last modified: $Date: 2012-07-05 16:47:08 +0200 (Thu, 05 Jul 2012) $
 * @author The RevBayes Development Core Team
 * @license GPL version 3
 *
 * $Id: RlStringUtilities.h 1651 2012-07-05 14:47:08Z hoehna $
 */

#ifndef TreeUtilities_H
#define TreeUtilities_H

#include "DistanceMatrix.h"
#include "MatrixReal.h"
#include "Tree.h"
#include "Tree.h"
#include "TopologyNode.h"
#include <string>
#include <vector>

#include <boost/unordered_set.hpp>


namespace RevBayesCore {

    namespace TreeUtilities {

        // these function are for public use
        double                  computeRobinsonFouldDistance(const Tree &a, const Tree &b);
        Tree*                   convertTree(const Tree &t, bool resetIndex=true);
//        AdmixtureTree*  convertToAdmixtureTree(const Tree &t, std::vector<std::string> names); // , const std::vector<std::string> names);
        void                    getAges(Tree *t, TopologyNode *n, std::vector<double>& ages, bool internalsOnly=true);
        DistanceMatrix*         getDistanceMatrix(const Tree& tree);
        size_t                  getMrcaIndex(const TopologyNode *l, const TopologyNode *r);
        void                    getOldestTip(Tree* t, TopologyNode *n, double& oldest);
        void                    getTaxaInSubtree(TopologyNode *n, std::vector<TopologyNode*> &taxa );
        void                    offsetTree(Tree *t, TopologyNode *n, double factor);
        void                    makeUltrametric(Tree *t); // make the tree ultrametric by extending terminal branches
        void                    rescaleSubtree(Tree *t, TopologyNode *n, double factor, bool v=false);
        void                    rescaleTree(Tree *t, TopologyNode *n, double factor);
        void                    setAges(Tree *t, TopologyNode *n, std::vector<double>& ages);
        void                    setAgesRecursively(Tree *t, TopologyNode *n, double age);

        // internal helper functions
        void                    constructTimeTreeRecursively(TopologyNode *tn, const TopologyNode &n, std::vector<TopologyNode*> &nodes, std::vector<double> &ages, double depth);
//        void            constructAdmixtureTreeRecursively(AdmixtureNode *tn, const TopologyNode &n, std::vector<AdmixtureNode*> &nodes, std::vector<double> &ages);


		void                    processDistsInSubtree(const TopologyNode& node, MatrixReal& matrix, std::vector< std::pair<std::string, double> >& distsToNodeFather, const std::map< std::string, int >& namesToId);
        void                    climbUpTheTree(const TopologyNode& node, boost::unordered_set <const TopologyNode* >& pathFromNodeToRoot) ;
        double                  getAgeOfMRCARecursive(const TopologyNode& node, boost::unordered_set <const TopologyNode* >& pathFromOtherNodeToRoot) ;
        double                  getAgeOfMRCA(const Tree &t, std::string first, std::string second) ;
        int                     getCollessMetric(const TopologyNode&, int& size);
        double                  getGammaStatistic(const Tree &t);



    }

}


#endif
