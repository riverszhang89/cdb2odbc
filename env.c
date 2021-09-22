#include "driver.h"
#include <stdint.h>
SQLRETURN SQLSetEnvAttr(
     SQLHENV      EnvironmentHandle,  
     SQLINTEGER   Attribute,  
     SQLPOINTER   ValuePtr,  
     SQLINTEGER   StringLength)
{
   /* stub to please the DM */
   __debug("enters method attr %d val %d", Attribute, (int)(uintptr_t)ValuePtr);
   __debug("leaves method.");
   return SQL_SUCCESS;
}
