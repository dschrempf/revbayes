#ifndef Func_freeSymmetricRateMatrix_H
#define Func_freeSymmetricRateMatrix_H

#include "RlTypedFunction.h"
#include "RlRateMatrix.h"

#include <map>
#include <string>

namespace RevLanguage {
    
    /**
     * The RevLanguage wrapper of the Free-K rate matrix function.
     *
     * The RevLanguage wrapper of the Free-K rate matrix connects
     * the variables/parameters of the function and creates the internal FreeSymmetricRateMatrixFunction object.
     * Please read the FreeSymmetricRateMatrixFunction.h for more info.
     *
     *
     * @copyright Copyright 2009-
     * @author The RevBayes Development Core Team (Michael Landis)
     * @since 2014-08-14, version 1.0
     *
     */
    class Func_freeSymmetricRateMatrix :  public TypedFunction<RateMatrix> {
        
    public:
        Func_freeSymmetricRateMatrix( void );
        
        // Basic utility functions
        Func_freeSymmetricRateMatrix*                                   clone(void) const;                                          //!< Clone the object
        static const std::string&                                       getClassType(void);                                         //!< Get Rev type
        static const TypeSpec&                                          getClassTypeSpec(void);                                     //!< Get class type spec
        std::string                                                     getFunctionName(void) const;                                //!< Get the primary name of the function in Rev
        const TypeSpec&                                                 getTypeSpec(void) const;                                    //!< Get the type spec of the instance
        
        // Function functions you have to override
        RevBayesCore::TypedFunction< RevBayesCore::RateGenerator >*     createFunction(void) const;                                 //!< Execute function
        const ArgumentRules&                                            getArgumentRules(void) const;                               //!< Get argument rules
        
    };
    
}

#endif
