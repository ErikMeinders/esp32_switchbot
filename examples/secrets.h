// with realsecrets.h in .gitignore I can keep my secrets out of the repo
// yet still have a template for others to use

#if __has_include("realsecrets.h")
#include "realsecrets.h"
#else
#define sb_token  "GET YOUR OWN TOKEN IN THE APP"
#define sb_secret "SAME FOR SECRET"
#endif