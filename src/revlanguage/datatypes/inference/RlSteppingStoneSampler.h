#ifndef RlSteppingStoneSampler_H
#define RlSteppingStoneSampler_H

#include "SteppingStoneSampler.h"
#include "RlControlVariableWrapper.h"
#include "TypedDagNode.h"

#include <ostream>
#include <string>

namespace RevLanguage {
    
    
    /**
     * @brief Rev wrapper of the Stepping-Stone-Sampler class.
     *
     * The stepping stone sampler analyzes the output of a power posterior and
     * computes the marginal likelihood.
     *
     *
     * @copyright Copyright 2009-
     * @author The RevBayes Development Core Team (Sebastian Hoehna)
     * @since Version 1.0, 2012-06-17
     *
     */
    class SteppingStoneSampler : public RlControlVariableWrapper<RevBayesCore::SteppingStoneSampler> {
        
    public:
        
        SteppingStoneSampler(void);                                                                                         //!< Default constructor
        
        // Basic utility functions
        virtual SteppingStoneSampler*               clone(void) const;                                                      //!< Clone object
        void                                        constructInternalObject(void);                                          //!< We construct the a new internal PowerPosterior object.
        static const std::string&                   getClassName(void);                                                     //!< Get class name
        static const TypeSpec&                      getClassTypeSpec(void);                                                 //!< Get class type spec
        const MemberRules&                          getMemberRules(void) const;                                             //!< Get member rules (const)
        virtual const TypeSpec&                     getTypeSpec(void) const;                                                //!< Get language type of the object
        virtual void                                printValue(std::ostream& o) const;                                      //!< Print value (for user)
        
        // Member method inits
        const MethodTable&                          getMethods(void) const;                                                 //!< Get methods
        RbLanguageObject*                           executeMethod(const std::string& name, const std::vector<Argument>& args);  //!< Override to map member methods to internal functions
        
    protected:
        
        void                                        setConstMemberVariable(const std::string& name, const RbPtr<const Variable> &var);              //!< Set member variable
        
        RbPtr<const Variable>                       likelihoodColumnName;
        RbPtr<const Variable>                       powerColumnName;
        RbPtr<const Variable>                       delimmiter;
        RbPtr<const Variable>                       filename;
        
    };
    
}

#endif
