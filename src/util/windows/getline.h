#pragma once

#ifndef FASTFETCH_INCLUDED_UTIL_GETLINE
#define FASTFETCH_INCLUDED_UTIL_GETLINE

#include <stdint.h>
#include <stdio.h>

ssize_t getline(char **lineptr, size_t *n, FILE *stream);

#endif
