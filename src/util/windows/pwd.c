#include "pwd.h"

#define WIN32_LEAN_AND_MEAN
#include <Windows.h>
#include <shlobj.h>

struct passwd* ffGetPasswd()
{
    static struct passwd res;

    DWORD len = sizeof(res.pw_name);
    GetUserNameA(res.pw_name, &len);

    PWSTR pPath;
    if(SUCCEEDED(SHGetKnownFolderPath(&FOLDERID_Profile, KF_FLAG_DEFAULT, NULL, &pPath)))
        WideCharToMultiByte(CP_UTF8, 0, pPath, -1, res.pw_dir, sizeof(res.pw_dir), NULL, NULL); //res has been NULL inited
    CoTaskMemFree(pPath);
    for (char *current_pos = strchr(res.pw_dir, '\\'); current_pos; current_pos = strchr(current_pos + 1, '\\'))
        *current_pos = '/';

    return &res;
}
