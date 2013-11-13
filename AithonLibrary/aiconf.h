
#ifndef _AICONF_H_
#define _AICONF_H_


#if !defined(AI_USE_LCD) || defined(__DOXYGEN__)
#define AI_USE_LCD                     TRUE
#endif

#if !defined(AI_USE_MOTORS) || defined(__DOXYGEN__)
#define AI_USE_MOTORS                  FALSE
#endif

#if !defined(AI_USE_SERVOS) || defined(__DOXYGEN__)
#define AI_USE_SERVOS                  FALSE
#endif

#if !defined(AI_USE_MUSIC) || defined(__DOXYGEN__)
#define AI_USE_MUSIC                   FALSE
#endif

#if !defined(AI_USE_IMU) || defined(__DOXYGEN__)
#define AI_USE_IMU                     FALSE
#endif

#if !defined(AI_USE_ANALOG) || defined(__DOXYGEN__)
#define AI_USE_ANALOG                  FALSE
#endif


#endif  /* _AICONF_H_ */
