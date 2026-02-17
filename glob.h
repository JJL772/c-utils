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

#ifdef __cplusplus
extern "C" {
#endif

static inline int
globish(const char* pattern, const char* str)
{
  const char* p = pattern;
  const char* s = str;
  
  /* special cases that should always match */
  if (*pattern == '*' && !pattern[1])
    return 1;
  
  while (*p && *s) {
    switch (*p) {
    case '*':
      /* check for a match between the next pattern char and the current 's'
       * if it's a hit, advance the pattern past the wildcard and repeat the
       * compare. */
      if (toupper(*(p+1)) == toupper(*s)) {
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
      if (toupper(*p) != toupper(*s))
        return 0;
      ++p, ++s;
      break;
    }
  }
  
  /* if any chars remain in the pattern, it wasn't a match */
  if (*p)
    return 0;

  return 1;
}

#ifdef __cplusplus
}
#endif