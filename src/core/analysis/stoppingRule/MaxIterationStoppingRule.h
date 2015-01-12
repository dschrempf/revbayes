#ifndef MaxIterationStoppingRule_H
#define MaxIterationStoppingRule_H

#include "StoppingRule.h"

#include <vector>

namespace RevBayesCore {
    
    /**
     * @brief The max-iterations stopping rule.
     *
     * This stopping rule returns true when the maximum number of iterations has been reached.
     * This rule is most useful if you want to run for at most a given number of iterations
     * or if you do not apply any other stopping rule.
     *
     *
     * @copyright Copyright 2009-
     * @author The RevBayes Development Core Team (Sebastian Hoehna)
     * @since Version 1.0, 2015-01-11
     *
     */
    class MaxIterationStoppingRule : public StoppingRule {
        
    public:
        MaxIterationStoppingRule(size_t g);
        virtual                                            ~MaxIterationStoppingRule(void);                             //!< Virtual destructor
        
        
        // public methods
        virtual MaxIterationStoppingRule*                   clone(void) const;                                          //!< Clone function. This is similar to the copy constructor but useful in inheritance.
        bool                                                stop(void);                                                 //!< Should we stop now?
        
    private:
        size_t                                              maxGenerations;
        
    };
    
    // Global functions using the class
    std::ostream&                       operator<<(std::ostream& o, const MaxIterationStoppingRule& x);                 //!< Overloaded output operator
    
}

#endif