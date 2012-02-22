/**
 * @file
 * This file contains the declaration of Container, a complex type
 * that acts as a base class for all Containers.
 *
 * @brief Declaration of Container
 *
 * (c) Copyright 2009- under GPL version 3
 * @date Last modified: $Date$
 * @author The RevBayes Development Core Team
 * @license GPL version 3
 * @version 1.0
 * @since 2009-12-04, version 1.0
 * @extends Container
 *
 * $Id$
 */

#include "Container.h"
#include "MemberFunction.h"
#include "Natural.h"
#include "RbException.h"
#include "RbObject.h"
#include "RbNullObject.h"
#include "RbUtil.h"
#include "TypeSpec.h"
#include "ValueRule.h"
#include "VectorString.h"

#include <algorithm>

/** Set type of elements */
Container::Container(const TypeSpec& elemType) : ConstantMemberObject(), elementType(elemType) {
    
}

/** Set type of elements */
Container::Container(const TypeSpec& elemType, const MemberRules& memberRules) : ConstantMemberObject(memberRules), elementType(elemType) {
    
}


/** Copy Constructor */
Container::Container(const Container &v) : ConstantMemberObject(v), elementType(v.elementType) {
    
}


/** Destructor. Free the memory of the elements. */
Container::~Container(void) {
    
}

/** Assignment operator; make sure we get independent elements */
Container& Container::operator=( const Container& x ) {
    
    if ( this != &x ) {
        
        if (elementType != x.elementType) {
            throw RbException("Cannot assign a vector to another vector of different type.");
        }
        
        returnValueSize = x.returnValueSize;
    }
    
    return ( *this );
}


/** Get class name of object */
const std::string& Container::getClassName(void) { 
    
    static std::string rbClassName = "Container";
    
	return rbClassName; 
}

/** Get class type spec describing type of object */
const TypeSpec& Container::getClassTypeSpec(void) { 
    
    static TypeSpec rbClass = TypeSpec( getClassName(), new TypeSpec( ConstantMemberObject::getClassTypeSpec() ) );
    
	return rbClass; 
}



/* Get method specifications */
const MethodTable& Container::getMethods(void) const {
    
    static MethodTable methods = MethodTable();
    static bool          methodsSet = false;
    
    if ( methodsSet == false ) 
    {
        ArgumentRules* sizeArgRules = new ArgumentRules();
        methods.addFunction("size", new MemberFunction( Natural::getClassTypeSpec(), sizeArgRules) );
        
        // add method for call "x[]" as a function
        ArgumentRules* squareBracketArgRules = new ArgumentRules();
        squareBracketArgRules->push_back( new ValueRule( "index" , Natural::getClassTypeSpec() ) );
        methods.addFunction("[]",  new MemberFunction( RbObject::getClassTypeSpec(), squareBracketArgRules) );
        
        // sort function
        ArgumentRules* sortArgRules = new ArgumentRules();
        methods.addFunction("sort", new MemberFunction( RbVoid_name, sortArgRules) );
        
        // unique function
        ArgumentRules* uniqueArgRules = new ArgumentRules();
        methods.addFunction("unique", new MemberFunction( RbVoid_name, uniqueArgRules) );
        
        // necessary call for proper inheritance
        methods.setParentTable( &ConstantMemberObject::getMethods() );
        methodsSet = true;
    }
    
    return methods;
}


/* Map calls to member methods */
const RbLanguageObject& Container::executeOperationSimple(const std::string& name, const std::vector<Argument>& args) {
    
    if (name == "size") {
        
        // we set our value
        returnValueSize.setValue(size());
        
        return returnValueSize;
    } else if ( name == "[]") {
        // get the member with give index
        const Natural& index = static_cast<const Natural&>( args[0].getVariable().getValue() );

        if (size() < (size_t)(index.getValue()) ) {
            throw RbException("Index out of bounds in []");
        }
        
        // TODO: Check what happens with DAGNodeContainers
        RbLanguageObject& element = static_cast<RbLanguageObject&>( getElement(index.getValue() - 1) );
        return element;
    } else if (name == "sort") {
            
            // we set our value
        sort();
            
        return RbNullObject::getInstance();
    } else if (name == "unique") {
        
        // we set our value
        unique();
        
        return RbNullObject::getInstance();
    }
    
    return ConstantMemberObject::executeOperationSimple( name, args );
}



