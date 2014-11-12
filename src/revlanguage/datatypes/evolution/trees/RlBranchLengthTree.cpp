#include "ModelVector.h"
#include "Natural.h"
#include "RlBranchLengthTree.h"
#include "RbUtil.h"
#include "RlString.h"
#include "RealPos.h"
#include "TypeSpec.h"

#include <sstream>

using namespace RevLanguage;

/** Default constructor */
BranchLengthTree::BranchLengthTree(void) : ModelObject<RevBayesCore::BranchLengthTree>()
{

    ArgumentRules* nnodesArgRules = new ArgumentRules();
    methods.addFunction("nnodes", new MemberProcedure(Natural::getClassTypeSpec(),       nnodesArgRules              ) );
    
    ArgumentRules* heightArgRules = new ArgumentRules();
    methods.addFunction("height", new MemberProcedure(Natural::getClassTypeSpec(),       heightArgRules              ) );
    
    ArgumentRules* namesArgRules = new ArgumentRules();
    methods.addFunction("names", new MemberProcedure(ModelVector<RlString>::getClassTypeSpec(),  namesArgRules       ) );
    
    ArgumentRules* rerootArgRules = new ArgumentRules();
    rerootArgRules->push_back( new ArgumentRule("leaf"    , RlString::getClassTypeSpec(), ArgumentRule::BY_VALUE) );
    
    methods.addFunction("reroot", new MemberProcedure(RlUtils::Void,  rerootArgRules       ) );


}

/** Construct from bool */
BranchLengthTree::BranchLengthTree(RevBayesCore::BranchLengthTree *t) : ModelObject<RevBayesCore::BranchLengthTree>( t )
{

    ArgumentRules* nnodesArgRules = new ArgumentRules();
    methods.addFunction("nnodes", new MemberProcedure(Natural::getClassTypeSpec(),       nnodesArgRules              ) );
    
    ArgumentRules* heightArgRules = new ArgumentRules();
    methods.addFunction("height", new MemberProcedure(Natural::getClassTypeSpec(),       heightArgRules              ) );
    
    ArgumentRules* namesArgRules = new ArgumentRules();
    methods.addFunction("names", new MemberProcedure(ModelVector<RlString>::getClassTypeSpec(),  namesArgRules       ) );
    
    ArgumentRules* rerootArgRules = new ArgumentRules();
    rerootArgRules->push_back( new ArgumentRule("leaf"    , RlString::getClassTypeSpec(), ArgumentRule::BY_VALUE) );
    
    methods.addFunction("reroot", new MemberProcedure(RlUtils::Void,  rerootArgRules       ) );


}

/** Construct from bool */
BranchLengthTree::BranchLengthTree(const RevBayesCore::BranchLengthTree &t) : ModelObject<RevBayesCore::BranchLengthTree>( new RevBayesCore::BranchLengthTree( t ) ) {
    
}

/** Construct from bool */
BranchLengthTree::BranchLengthTree(RevBayesCore::TypedDagNode<RevBayesCore::BranchLengthTree> *n) : ModelObject<RevBayesCore::BranchLengthTree>( n )
{

    ArgumentRules* nnodesArgRules = new ArgumentRules();
    methods.addFunction("nnodes", new MemberProcedure(Natural::getClassTypeSpec(),       nnodesArgRules              ) );
    
    ArgumentRules* heightArgRules = new ArgumentRules();
    methods.addFunction("height", new MemberProcedure(Natural::getClassTypeSpec(),       heightArgRules              ) );
    
    ArgumentRules* namesArgRules = new ArgumentRules();
    methods.addFunction("names", new MemberProcedure(ModelVector<RlString>::getClassTypeSpec(),  namesArgRules       ) );
    
    ArgumentRules* rerootArgRules = new ArgumentRules();
    rerootArgRules->push_back( new ArgumentRule("leaf"    , RlString::getClassTypeSpec(), ArgumentRule::BY_VALUE) );
    
    methods.addFunction("reroot", new MemberProcedure(RlUtils::Void,  rerootArgRules       ) );


}


/** Clone object */
BranchLengthTree* BranchLengthTree::clone(void) const {
    
	return new BranchLengthTree(*this);
}


/* Map calls to member methods */
RevLanguage::RevPtr<RevLanguage::RevVariable> BranchLengthTree::executeMethod(std::string const &name, const std::vector<Argument> &args, bool &found)
{
    
    if (name == "nnodes")
    {
        found = true;
        
        size_t n = this->dagNode->getValue().getNumberOfNodes();
        return new RevVariable( new Natural( n ) );
    }
    else if (name == "height")
    {
        found = true;
        
        const RevBayesCore::TopologyNode& r = this->dagNode->getValue().getTipNode( 0 );
        return new RevVariable( new RealPos( r.getBranchLength() ) );
    } 
    else if (name == "names")
    {
        found = true;
        
        const std::vector<std::string>& n = this->dagNode->getValue().getTipNames();
        return new RevVariable( new ModelVector<RlString>( n ) );
    }
    else if (name == "reroot")
    {
        
        const RevObject& st = args[0].getVariable()->getRevObject();
        if ( st.isType( RlString::getClassTypeSpec() ) )
        {
            std::string n = std::string( static_cast<const RlString&>( st ).getValue() );
            this->dagNode->getValue().reroot(n);
        }
        return NULL;
        
    }
    
    return ModelObject<RevBayesCore::BranchLengthTree>::executeMethod( name, args, found );
}


/** Get Rev type of object */
const std::string& BranchLengthTree::getClassType(void) { 
    
    static std::string revType = "BranchLengthTree";
    
	return revType; 
}

/** Get class type spec describing type of object */
const TypeSpec& BranchLengthTree::getClassTypeSpec(void) { 
    
    static TypeSpec revTypeSpec = TypeSpec( getClassType(), new TypeSpec( RevObject::getClassTypeSpec() ) );
    
	return revTypeSpec; 
}


/** Get type spec */
const TypeSpec& BranchLengthTree::getTypeSpec( void ) const {
    
    static TypeSpec typeSpec = getClassTypeSpec();
    
    return typeSpec;
}

