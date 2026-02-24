//-----------------------------------------------------------------------------------------
// Copyright (C) 2025 Jeremy Lorelli
//-----------------------------------------------------------------------------------------
// Purpose: Tiny glob-ish implementation
//-----------------------------------------------------------------------------------------
// This file is part of 'c-utils'. It is subject to the license terms in the
// LICENSE file found in the top-level directory of this distribution.
// No part of 'c-utils', including this file, may be copied, modified, propagated,
// or otherwise distributed except according to the terms contained in the LICENSE file.
//
// SPDX-License-Identifier: MIT
//-----------------------------------------------------------------------------------------
#pragma once

#include <string.h>
#include <ctype.h>
#include <stdio.h>

#ifdef __cplusplus
extern "C" {
#endif

/**
 * @brief Basic, mostly-compliant, glob implementation
 * @param pattern Glob pattern to match with
 * @param str Pattern to match with
 * @returns 1 for match, 0 otherwise
 */
static inline int
globish(const char* pattern, const char* str)
{
  const char* p = pattern;
  const char* s = str;

  /* special cases that should always match */
  if (*pattern == '*' && !pattern[1])
    return 1;

  const char* rewind = NULL;
  while (*p && *s) {
    switch (*p) {
    case '*':
      /* check for a match between the next pattern char and the current 's'
       * if it's a hit, advance the pattern past the wildcard and repeat the
       * compare. */
      if (toupper(*(p+1)) == toupper(*s)) {
        /* if the coming match fails, rewind to this position in the pattern
         * and try again. */
        rewind = p;
        ++p;
        break;
      }
      ++s;
      if (!*s) /* advance the pattern if no more input chars are present */
        ++p;
      break;
    case '?': /* match any single char */
      ++p, ++s;
      break;
    default:  /* normal char compare */
      if (toupper(*p) != toupper(*s)) {
        /* if we have no match and a rewind pos set, go back to it and continue
         * matching against the wildcard */
        if (rewind) {
          p = rewind;
          rewind = NULL;
          continue;
        }
        return 0;
      }
      ++p, ++s;
      break;
    }
  }
  
  /* if any chars besides wildcard remain in the pattern, it wasn't a match */
  if (*p && *p != '*') {
    return 0;
  }

  return 1;
}

#ifdef __cplusplus
}
#endif