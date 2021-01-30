#ifndef _PageType_h_
#define _PageType_h_

typedef enum _PageTypeEnum
{
   UNDEFINED_PAGE_TYPE = -1,
   DEFAULT_PAGE_TYPE = 0,      /* Used in the absence of PageType property
                                  in the loaded drawing. */
   PROCESS_PAGE,
   AERATION_PAGE,
   CIRCUIT_PAGE,
   RT_CHART_PAGE,
   TEST_COMMANDS_PAGE
} PageTypeEnum;

#endif
