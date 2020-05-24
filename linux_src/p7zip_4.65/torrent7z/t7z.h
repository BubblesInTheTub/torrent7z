#include "../CPP/Common/MyString.h"

namespace torrent7z
{

extern const char*t7zsig;
extern const char*t7zsig_str;
const int t7zsig_size=16+1+9+4+4;

//extern char*buffer;
const int tmpbufsize=(32+1)*1024*sizeof(TCHAR);

#ifdef _WIN32
int IsParentGui();
#endif

bool MyCreateDirectory(const CSysString&src);
bool MyRemoveDirectory(const CSysString&src);

#ifdef DEBUG
const bool debug=true;
#else
const bool debug=false;
#endif

#ifdef _UNICODE
//#define text(x) TEXT(x)
#define u2a(a) (a)
#define a2u(u) (u)
#else
#define text(x) (x)
AString u2a(const UString&u);
UString a2u(const AString&a);
#endif

extern TCHAR*usage_info;

}
