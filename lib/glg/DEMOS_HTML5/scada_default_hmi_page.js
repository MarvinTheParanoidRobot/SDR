//////////////////////////////////////////////////////////////////////////////
// Extends HMIPageBase with functionality to handle pages that need to handle
// tag-based data updates, but do not need any other special handling.
//////////////////////////////////////////////////////////////////////////////
function DefaultHMIPage( /* GlgObject */ viewport ){}

DefaultHMIPage.prototype = Object.create( HMIPageBase.prototype );
DefaultHMIPage.prototype.constructor = DefaultHMIPage;

// Returns an update interval in msec for animating drawing with data.
DefaultHMIPage.prototype.GetUpdateInterval = function()   /* int */
{
   switch( PageType )
   {
    case AERATION_PAGE:      return 2000;
    case CIRCUIT_PAGE:       return 1000;
    case RT_CHART_PAGE:      return 30;
    case TEST_COMMANDS_PAGE: return 100;
    default: return 500;   /* Use default update interval. */
   }
}

//////////////////////////////////////////////////////////////////////////////
// Returns true if tag sources need to be remapped for the page.
//////////////////////////////////////////////////////////////////////////////
DefaultHMIPage.prototype.NeedTagRemapping = function()   /* boolean */
{
   // In demo mode, unset tags need to be remapped to enable animation.
   if( RandomData )
     return true;
   else
     return false;   // Remap tags only if necessary.
}

//////////////////////////////////////////////////////////////////////////////
// Reassign TagSource parameter for a given tag object to a new
// TagSource value. tag_source and tag_name parameters are the current 
// TagSource and TagName of the tag_obj.
//////////////////////////////////////////////////////////////////////////////
DefaultHMIPage.prototype.RemapTagObject =
  function( /* GlgObject */ tag_obj, /* String */ tag_name,
            /* String */ tag_source )
{
   if( RandomData )
   {
      // Skip tags with undefined TagName.
      if( IsUndefined( tag_name ) )
        return;
         
      /* In demo mode, assign unset tag sources to be the same as tag names
         to enable animation with demo data.
      */
      if( IsUndefined( tag_source ) )
        AssignTagSource( tag_obj, tag_name );
   }
   else
   {
      // Assign new TagSource as needed.
      // AssignTagSource( tag_obj, new_tag_source );
   }
}
