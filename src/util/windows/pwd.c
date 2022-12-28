#include "pwd.h"

#define WIN32_LEAN_AND_MEAN
#include <Windows.h>
#include <shlobj.h>

struct passwd* ffGetPasswd()
{
    static struct passwd res;

    DWORD len = sizeof(res.pw_name);
    GetUserNameA(res.pw_name, &len);

    SHGetFolderPathA(NULL, CSIDL_PROFILE, NULL, 0, res.pw_dir);

    return &res;
}
