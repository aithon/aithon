
#ifndef _AICONF_H_
#define _AICONF_H_


#if !defined(AI_USE_LCD) || defined(__DOXYGEN__)
#define AI_USE_LCD                     TRUE
#endif

#if !defined(AI_USE_MOTORS) || defined(__DOXYGEN__)
#define AI_USE_MOTORS                  TRUE
#endif

#if !defined(AI_USE_SERVOS) || defined(__DOXYGEN__)
#define AI_USE_SERVOS                  TRUE
#endif

#if !defined(AI_USE_MUSIC) || defined(__DOXYGEN__)
#define AI_USE_MUSIC                   TRUE
#endif

#if !defined(AI_USE_IMU) || defined(__DOXYGEN__)
#define AI_USE_IMU                     TRUE
#endif

#if !defined(AI_USE_ANALOG) || defined(__DOXYGEN__)
#define AI_USE_ANALOG                  TRUE
#endif

#if !defined(AI_USE_UART1_PROGRAMMING) || defined(__DOXYGEN__)
#define AI_USE_UART1_PROGRAMMING			FALSE
#endif

#if !defined(AI_USE_UART2_PROGRAMMING) || defined(__DOXYGEN__)
#define AI_USE_UART2_PROGRAMMING			FALSE
#endif


#endif  /* _AICONF_H_ */
