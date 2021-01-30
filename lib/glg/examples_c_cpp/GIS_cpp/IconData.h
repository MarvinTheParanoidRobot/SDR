#pragma once

#include "GlgClass.h"

class IconData
{
public:
   IconData( void ) {};
   ~IconData( void ) {};
public:
   GlgObjectC icon_obj;
   GlgPoint lat_lon;   // Icon position in lat/lon coordinates
   double angle;       // Icon's rotation angle
};
