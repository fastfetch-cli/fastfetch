#pragma once

#ifndef FASTFETCH_INCLUDED_UTIL_PWD
#define FASTFETCH_INCLUDED_UTIL_PWD

#include <Lmcons.h>
#include <minwindef.h>

struct passwd
{
    char    pw_name[UNLEN + 1];    /* username */
    // char   *pw_passwd[1];       /* user password */
    // int     pw_uid;             /* user ID */
    // int     pw_gid;             /* group ID */
    // char   *pw_gecos;           /* user information */
    char    pw_dir[MAX_PATH];      /* home directory */
    // char   *pw_shell;           /* shell program */
};

struct passwd* ffGetPasswd();

#endif
