/**
 * @file RbMathLogic
 * This file contains some logic function on numbers.
 *
 * The following functions check whether two double-precision real
 * numbers are equal. They are described in:
 *
 * Knuth, D. E. 1981. The art of computer programming: Seminumerical
 *    algorithms, Volume 2. Addison-Wesley.
 *
 * Note that approximately equal to is more stringent than essentially
 * equal to.
 *
 * @brief Implementation of simple RlBoolean algebra on numbers.
 *
 * (c) Copyright 2009- under GPL version 3
 * @date Last modified: $Date$
 * @author The RevBayes core development team
 * @license GPL version 3
 * @version 1.0
 * @since 2011-03-17, version 1.0
 *
 * $Id$
 */

#include "RbConstants.h"
#include "RbException.h"
#include "RbMathLogic.h"
#include "RbSettings.h"
#include <cassert>
#include <cmath>


using namespace RevBayesCore;

/** Compares two doubles for equality */
bool RbMath::compApproximatelyEqual(double a, double b) {
    
    double epsilon = RbSettings::userSettings().getTolerance();
    return fabs(a - b) <= ( (fabs(a) < fabs(b) ? fabs(b) : fabs(a)) * epsilon);
}


/** Compares two doubles for equality */
bool RbMath::compApproximatelyEqual(double a, double b, double epsilon) {
    
    return fabs(a - b) <= ( (fabs(a) < fabs(b) ? fabs(b) : fabs(a)) * epsilon);
}


/** Compares two doubles for equality */
bool RbMath::compEssentiallyEqual(double a, double b) {
    
    double epsilon = RbSettings::userSettings().getTolerance();
    return fabs(a - b) <= ( (fabs(a) > fabs(b) ? fabs(b) : fabs(a)) * epsilon);
}


/** Compares two doubles for equality */
bool RbMath::compEssentiallyEqual(double a, double b, double epsilon) {
    
    return fabs(a - b) <= ( (fabs(a) > fabs(b) ? fabs(b) : fabs(a)) * epsilon);
}


/** Tests whether one number is greater than another */
bool RbMath::compDefinitelyGreaterThan(double a, double b) {
    
    double epsilon = RbSettings::userSettings().getTolerance();
    return (a - b) > ( (fabs(a) < fabs(b) ? fabs(b) : fabs(a)) * epsilon);
}


/** Tests whether one number is greater than another */
bool RbMath::compDefinitelyGreaterThan(double a, double b, double epsilon) {
    
    return (a - b) > ( (fabs(a) < fabs(b) ? fabs(b) : fabs(a)) * epsilon);
}


/** Tests whether one number is less than another */
bool RbMath::compDefinitelyLessThan(double a, double b) {
    
    double epsilon = RbSettings::userSettings().getTolerance();
    return (b - a) > ( (fabs(a) < fabs(b) ? fabs(b) : fabs(a)) * epsilon);
}


/** Tests whether one number is less than another */
bool RbMath::compDefinitelyLessThan(double a, double b, double epsilon) {
    
    return (b - a) > ( (fabs(a) < fabs(b) ? fabs(b) : fabs(a)) * epsilon);
}


bool RbMath::isAComputableNumber(double x)
{
    if ( x != x )
        return false;
    if ( x == RbConstants::Double::neginf )
        return false;
    if ( x == RbConstants::Double::nan )
        return false;
    if ( x == RbConstants::Double::inf )
        return false;
    
    return true;
}

/** Tests whether a double is finite */
bool RbMath::isFinite(double x) {
    
    return x == x;
}


/** Tests whether a size_t is finite */
bool RbMath::isFinite(size_t x)
{
    if (x == 0)
        return true;
    if (x == RbConstants::Size_t::inf)
        return false;
    if (x == RbConstants::Size_t::nan)
        return false;
    return true;
}


/** Tests whether a double is actually an interger */
bool RbMath::isInt(double x){

    double int_part;
    return ( modf ( x, &int_part ) == 0.0 );
}


/** Tests whether a double is NAN (not a number) */
bool RbMath::isNan(double x) {

    return x != x;
}


/** Returns the maximum of two real numbers */
double RbMath::max(double x, double y){

	return (x < y) ? y : x;
}


/** Returns the minimum of two real numbers */
double RbMath::min(double x, double y) {

    return (x < y) ? x : y;
}


