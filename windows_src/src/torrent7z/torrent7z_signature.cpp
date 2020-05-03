/*  #########################################################################  */

#include "StdAfx.h"

/*  #########################################################################  */

namespace torrent7z
{

/*  #########################################################################  */

//if you make *any* changes to the source, you *must* change both seed and signature to avoid any possible confusion
//                 16-byte seed                                                        signature
const char*t7zsig="\xa9\x9f\xd1\x57\x08\xa9\xd7\xea\x29\x64\xb2\x36\x1b\x83\x52\x33\x00torrent7z_0.9beta";
//const int t7zsig_size=16+1+9+4+4;
const char*t7zsig_str="torrent7z_0.9.2beta"
#ifdef DEBUG
"(debug)"
#else
""
#endif
;

/*  #########################################################################  */

}