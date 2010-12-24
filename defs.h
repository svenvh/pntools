/*
 * def.h
 *
 *  Created on: Oct 6, 2010
 *      Author: Teddy Zhai,
 *              Sven van Haastregt
 */

#ifndef DEF_H_
#define DEF_H_

#include <limits>

typedef double THR_t;

#define thr_min(a,b) ((a)>=(b) ? (b) : (a))

#define THR_INF (std::numeric_limits<double>::max())

#endif /* DEF_H_ */
