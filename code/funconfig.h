#ifndef _FUNCONFIG_H
#define _FUNCONFIG_H

#define CH32V003           1
#define FUNCONF_USE_PLL 0               // Do not use built-in 2x PLL 
//#define FUNCONF_HPRE_DIV  RCC_HPRE_DIV32
#define FUNCONF_HPRE_DIV  RCC_HPRE_DIV8
#define FUNCONF_SYSTEM_CORE_CLOCK  3000000
#define USE_DEBUGPRINTF    1

#endif

