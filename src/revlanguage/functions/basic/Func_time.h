/**
 * @file
 * This file contains the declaration and implementation
 * of the Func_time, which is used to set the seed of the random number generator.
 *
 * @brief Declaration and implementation of Func_time
 *
 * (c) Copyright 2009- under GPL version 3
 * @date Last modified: $Date: 2012-06-12 10:25:58 +0200 (Tue, 12 Jun 2012) $
 * @author The RevBayes Development Core Team
 * @license GPL version 3
 * @version 1.0
 *
 * $Id: Func_time.h 1626 2012-06-12 08:25:58Z hoehna $
 */

#ifndef Func_time_H
#define Func_time_H

#include <iosfwd>
#include <vector>

#include "Procedure.h"
#include "RevPtr.h"

namespace RevBayesCore { class RbHelpReference; }

namespace RevLanguage {
class ArgumentRules;
class RevVariable;
class TypeSpec;
    
    class Func_time : public Procedure {
        
    public:
        Func_time();
        
        // Basic utility functions
        Func_time*                                      clone(void) const;                                          //!< Clone the object
        static const std::string&                       getClassType(void);                                         //!< Get Rev type
        static const TypeSpec&                          getClassTypeSpec(void);                                     //!< Get class type spec
        std::string                                     getFunctionName(void) const;                                //!< Get the primary name of the function in Rev
        const TypeSpec&                                 getTypeSpec(void) const;                                    //!< Get language type of the object
        
        // Regular functions
        const ArgumentRules&                            getArgumentRules(void) const;                               //!< Get argument rules
        const TypeSpec&                                 getReturnType(void) const;                                  //!< Get type of return value
        
        
        RevPtr<RevVariable>                             execute(void);                                              //!< Execute function
        
    protected:
        
        std::vector<std::string>                        getHelpAuthor(void) const;                                  //!< Get the author(s) of this function
        std::string                                     getHelpDescription(void) const;                             //!< Get the description for this function
        std::string                                     getHelpDetails(void) const;                                 //!< Get the more detailed description of the function
        std::string                                     getHelpExample(void) const;                                 //!< Get an executable and instructive example
        std::vector<RevBayesCore::RbHelpReference>      getHelpReferences(void) const;                              //!< Get some references/citations for this function
        std::vector<std::string>                        getHelpSeeAlso(void) const;                                 //!< Get suggested other functions
        std::string                                     getHelpTitle(void) const;                                   //!< Get the title of this help entry
        
    };
    
}


#endif


