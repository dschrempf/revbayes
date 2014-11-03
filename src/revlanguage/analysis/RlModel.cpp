
#include "ArgumentRule.h"
#include "ArgumentRules.h"
#include "ConstantNode.h"
#include "Ellipsis.h"
#include "Model.h"
#include "ModelVector.h"
#include "RevObject.h"
#include "RbException.h"
#include "RlModel.h"
#include "RlString.h"
#include "TypeSpec.h"

#include <vector>
#include <fstream>
#include <algorithm>
#include <string>

using namespace RevLanguage;

Model::Model() : WorkspaceToCoreWrapperObject<RevBayesCore::Model>()
{
 
    ArgumentRules* dotArgRules = new ArgumentRules();
    dotArgRules->push_back( new ArgumentRule("file", RlString::getClassTypeSpec()  , ArgumentRule::BY_VALUE ) );
    dotArgRules->push_back( new ArgumentRule("verbose", RlBoolean::getClassTypeSpec(), ArgumentRule::BY_VALUE, ArgumentRule::ANY, new RlBoolean(false) ) );
    dotArgRules->push_back( new ArgumentRule("bg", RlString::getClassTypeSpec(), ArgumentRule::BY_VALUE, ArgumentRule::ANY, new RlString("lavenderblush2") ) );
    methods.addFunction("graph", new MemberProcedure( RlUtils::Void, dotArgRules) );

}


/** Clone object */
Model* Model::clone(void) const {
    
	return new Model(*this);
}


void Model::constructInternalObject( void ) {
    // we free the memory first
    delete value;
    
    // now allocate a model
    std::set<const RevBayesCore::DagNode*> s;
    for (std::set<RevPtr<const Variable> >::iterator it = sources.begin(); it != sources.end(); ++it) {
        RevBayesCore::DagNode* n = (*it)->getRevObject().getDagNode();
        s.insert( n );
    }
    value = new RevBayesCore::Model( s );
//    printModelDotGraph();
}

/* Map calls to member methods */
RevPtr<Variable> Model::executeMethod(std::string const &name, const std::vector<Argument> &args, bool &found)
{
    
    if (name == "graph")
    {
        found = true;
        
        const std::string&   fn      = static_cast<const RlString &>( args[0].getVariable()->getRevObject() ).getValue();
        bool vb = static_cast< const RlBoolean &>( args[1].getVariable()->getRevObject() ).getValue();
        const std::string&   bg      = static_cast<const RlString &>( args[2].getVariable()->getRevObject() ).getValue();
        printModelDotGraph(fn, vb, bg);
        
        return NULL;
    }
    
    return RevObject::executeMethod( name, args, found );
}

/** Get Rev type of object */
const std::string& Model::getClassType(void) { 
    
    static std::string revType = "Model";
    
	return revType; 
}

/** Get class type spec describing type of object */
const TypeSpec& Model::getClassTypeSpec(void) { 
    
    static TypeSpec revTypeSpec = TypeSpec( getClassType(), new TypeSpec( WorkspaceToCoreWrapperObject<RevBayesCore::Model>::getClassTypeSpec() ) );
    
	return revTypeSpec; 
}



/** Return member rules (no members) */
const MemberRules& Model::getParameterRules(void) const {
    
    static MemberRules modelMemberRules;
    static bool rulesSet = false;
    
    if ( !rulesSet )
    {
        
        modelMemberRules.push_back( new ArgumentRule("x", RevObject::getClassTypeSpec(), ArgumentRule::BY_CONSTANT_REFERENCE ) );
        modelMemberRules.push_back( new Ellipsis( RevObject::getClassTypeSpec() ) );
        
        rulesSet = true;
    }
    
    return modelMemberRules;
}


/** Get type spec */
const TypeSpec& Model::getTypeSpec( void ) const {
    
    static TypeSpec typeSpec = getClassTypeSpec();
    
    return typeSpec;
}


/** Print a simplified representation of the model for the user. */
void Model::printValue(std::ostream &o) const {
    
    const std::vector<RevBayesCore::DagNode*>& theNodes = value->getDagNodes();
    std::vector<RevBayesCore::DagNode*>::const_iterator it;

    o << std::endl;
    std::stringstream s;
    
    // compute the number of nodes by only counting nodes that are not hidden
    size_t numNodes = 0;
    for ( it=theNodes.begin(); it!=theNodes.end(); ++it )
    {
    
        if ( (*it)->isHidden() == false )
        {
            ++numNodes;
        }
    
    }
    
    s << "Model with " << numNodes << " nodes";
    o << s.str() << std::endl;
    for ( size_t i = 0; i < s.str().size(); ++i )
        o << "=";
    o << std::endl << std::endl;
    
    for ( it=theNodes.begin(); it!=theNodes.end(); ++it )
    {
        RevBayesCore::DagNode *theNode = *it;
        // skip hidden nodes
        if ( theNode->isHidden() == true )
        {
            continue;
        }
        
        if ( theNode->getName() != "" )
        {
            o << theNode->getName() <<  " :" << std::endl;
        }
        else
        {
            o << "<" << theNode << "> :" << std::endl;
        }
        
        o << "_value        = ";
        std::ostringstream o1;
        theNode->printValueElements( o1, ", " );
        o << StringUtilities::oneLiner( o1.str(), 54 ) << std::endl;

        theNode->printStructureInfo( o, false );

        o << std::endl;
    }
}


/** Set a member variable */
void Model::setConstParameter(const std::string& name, const RevPtr<const Variable> &var) {

    if ( name == "" || name == "x") {
        sources.insert( var );
    }
    else {
        RevObject::setConstParameter(name, var);
    }
}

/* Write a file in DOT format for viewing the model DAG in graphviz */
//   This requires the user to have graphviz installed, or they can paste the file contents
//   into http://graphviz-dev.appspot.com/
void Model::printModelDotGraph(const std::string &fn, bool vb, const std::string &bgc){
    
    const std::vector<RevBayesCore::DagNode*>& theNodes = value->getDagNodes();
    std::vector<RevBayesCore::DagNode*>::const_iterator it;
    
    std::ofstream o;
    o.open(fn.c_str());
    o << "/* Graphical model description in DOT language                                    */\n";
    o << "/*    To view graph:                                                              */\n";
    o << "/*       open this file in the program Graphviz: http://www.graphviz.org          */\n";
    o << "/*       or paste contents into an online viewer: http://stamm-wilbrandt.de/GraphvizFiddle */\n\n";
	o << "digraph REVDAG {\n";
    std::string nrank = "   {rank=same";
    for ( it=theNodes.begin(); it!=theNodes.end(); ++it ){
        if( !(*it)->isHidden() || vb){
            std::stringstream nname;
            if ( (*it)->getName() != "" )
                nname << (*it)->getName();
            else
                nname << (*it);
            std::string stname = nname.str();
            std::replace( stname.begin(), stname.end(), '[', '_');
			std::replace( stname.begin(), stname.end(), '.', '_');

            stname.erase(std::remove(stname.begin(), stname.end(), ']'), stname.end());  
                  
            std::stringstream rl;
			if((*it)->getName() == "" && !vb){
				rl << "function" ;
			}
			else
				rl << nname.str() ;
           
            // only print values of constant nodes (only simple numeric values)
            if((*it)->getDagNodeType() == "constant"){
                std::stringstream trl;
                if((*it)->isSimpleNumeric())  
                    (*it)->printValueElements(trl," ");
                else 
                    trl << " ... ";
                if(trl.str() != "" || vb){
                    std::string val = trl.str();
                    if(val == "")
                        val = "constant\\ndefault member";
                    if(!(*it)->isSimpleNumeric())
                        val = "...";
                    o << "   n_" << stname;
                    o << " [shape=";
                    if( (*it)->getName() == "" ){
                        o << "box, style=filled, fillcolor=white, ";
                        o << "label=\"" << val << "\"]\n";
                    }
                    else {
                        o << "record, style=filled, fillcolor=white, ";
                        rl << "|" << val;
                        o << "label=\"{" << rl.str() << "}\"]\n";
                    }
                }
            }
            else if((*it)->getDagNodeType() == "deterministic"){
                o << "   n_" << stname;
                o << " [shape=";
                std::stringstream strss;
                (*it)->printStructureInfo(strss);
                if(strss.str().find("function",0) < strss.str().npos){
                    std::string w;
                    while(strss >> w){
                        if(w == "_function"){
                            strss >> w;
                            strss >> w;
                            strss >> w;
                            rl << "\\n[ " << w << "( ) ]";
                        }
                    }
                }
                else{
                    std::string w;
                    while(strss >> w){
                        if(w == "_dagType"){
                            strss >> w;
                            strss >> w;
                            rl << "\\n[ " << w;
                            strss >> w;
                            if(w != "DAG")
                                rl << " " << w;
                            rl << " ]";
                        }
                    }
                }
                o << "oval, style=\"dashed,filled\", fillcolor=white, label=\"" << rl.str() << "\"]\n";
            }
            else if((*it)->getDagNodeType() == "stochastic"){
                o << "   n_" << stname;
                o << " [shape=";
                o << "oval, ";
                if((*it)->isClamped()){
                    o << "style=filled, fillcolor=gray, ";
                    nrank += "; n_" + stname;
                }
                else 
                    o << "style=filled, fillcolor=white, ";
                o << "label=\"" << rl.str() << "\"]\n";
            }
        }
    }
    for ( it=theNodes.begin(); it!=theNodes.end(); ++it ){
        if( !(*it)->isHidden() || vb){
            std::stringstream trl;
            (*it)->printValueElements(trl,",");
            if(trl.str() != "" || vb){
                std::stringstream nname;
                if ( (*it)->getName() != "" )
                    nname << (*it)->getName() ;
                else
                    nname << (*it);
                std::string stname = nname.str();
                std::replace( stname.begin(), stname.end(), '[', '_');
				std::replace( stname.begin(), stname.end(), '.', '_');
                stname.erase(std::remove(stname.begin(), stname.end(), ']'), stname.end());  

                if((*it)->getNumberOfChildren() > 0){
                    const std::set<RevBayesCore::DagNode*>& childeren = (*it)->getChildren();
                    std::set<RevBayesCore::DagNode*>::const_iterator ci;
                    for ( ci=childeren.begin(); ci!=childeren.end(); ++ci ){
                        if( (*ci)->isHidden() && vb==false ){
                            RevBayesCore::DagNode *ch = (*ci)->getFirstChild();
                            while (ch->isHidden()) {
                                ch = ch->getFirstChild();
                            }
                            std::stringstream cn;
                            if ( ch->getName() != "" )
                                cn << ch->getName();
                            else
                                cn << ch;
                            std::string stcn = cn.str();
                            std::replace( stcn.begin(), stcn.end(), '[', '_');
                            stcn.erase(std::remove(stcn.begin(), stcn.end(), ']'), stcn.end());  
                            
                            o << "   n_" << stname << " -> n_";
                            o << stcn;
                            if(ch->getDagNodeType() == "deterministic")
                                o << "[style=dashed]";
                            o << "\n";
                        }
                        else{
                            std::stringstream cn;
                            if ( (*ci)->getName() != "" )
                                cn << (*ci)->getName();
                            else
                                cn << (*ci);
                            std::string stcn = cn.str();
                            std::replace( stcn.begin(), stcn.end(), '[', '_');
                            stcn.erase(std::remove(stcn.begin(), stcn.end(), ']'), stcn.end());  

                            o << "   n_" << stname << " -> n_";
                            o << stcn;
                            if((*ci)->getDagNodeType() == "deterministic")
                                o << "[style=dashed]";
                            o << "\n";
                        }
                    }
                
                }
            }
        }
        
    }
    if(nrank.size() > 13){
        nrank += ";}\n";
        o << nrank;
    }
    o << "   graph [bgcolor=" << bgc << ", pad=0.25]\n";
    o << "}";
    o.close();
    
    
}