//-----------------------------------------------------------------------------------------
// Copyright (C) 2025 Jeremy Lorelli
//-----------------------------------------------------------------------------------------
// Purpose: Simple macro tools for C
//-----------------------------------------------------------------------------------------
// This file is part of 'c-utils'. It is subject to the license terms in the
// LICENSE file found in the top-level directory of this distribution.
// No part of 'c-utils', including this file, may be copied, modified, propagated,
// or otherwise distributed except according to the terms contained in the LICENSE file.
//
// SPDX-License-Identifier: MIT
//-----------------------------------------------------------------------------------------

#pragma once

#include <stddef.h>
#include <stdlib.h>
#include <string.h>

#define MAX_MACRO_NAME_LEN 128

#ifdef __cplusplus
namespace macro {
#endif

/**
 * Expands a string with macros in it.
 * Macros must be surrouned by ${}, such as ${PWD} or ${TEST}. Escape
 * sequences are also supported (i.e. \${PWD} will not be expanded).
 * Macros in the format $TEST are not recognized or expanded.
 * \param str Input string
 * \param resolveMacro Function to return the value of a macro. Returns NULL
 * if the macro doesn't exist.
 * \returns heap allocated string containing the transformed input.
 * The underlying buffer may be larger than the string contained within.
 */
static inline char*
expand_macro_string(const char* str, const char*(*resolveMacro)(const char*))
{
  /* result buffer */
  size_t ml = strlen(str) + 128;
  char* buf = (char*)malloc(ml);
  size_t bl = 0;

  for (const char* s = str; *s; ++s) {
    if (*s == '$' && *(s+1) == '{') {
      const char* start = s;
      s+=2; /* skip starting chars */

      /* read macro name */
      char macroName[MAX_MACRO_NAME_LEN];
      int n = 0;
      for (; *s && *s != '}' && n < MAX_MACRO_NAME_LEN-1; ++s)
        macroName[n++] = *s;
      macroName[n] = 0;

      /* incomplete macro? or macro too long? */
      if (!*s || n == MAX_MACRO_NAME_LEN-1) {
        s = start;
        goto append_single;
      }

      const char* e = resolveMacro(macroName);
      if (!e)
        e = "";

      /* resize if required */
      const size_t macLen = strlen(e);
      if (bl + macLen + 1 >= ml) {
        ml = bl + macLen + 128;
        buf = (char*)realloc(buf, ml);
      }

      /* append the macro value */
      for (; *e; ++e)
        buf[bl++] = *e;
      continue;
    }
    else if (*s == '\\' && *(s+1) == '$')
      /* escaped macro name ${} */
      s++;

append_single:
    if (bl >= (ml-1)) {
      ml += 128;
      buf = (char*)realloc(buf, ml);
    }
    buf[bl++] = *s; /* simply append */
  }

  buf[bl] = 0;

  return buf;
}

static inline const char*
strtools__getenv(const char* n)
{
  return getenv(n);
}

/**
 * Expand macro string using the environment.
 * @see expand_macro_string
 */
static inline char*
expand_macro_string_env(const char* str)
{
  return expand_macro_string(str, strtools__getenv);
}

#ifdef __cplusplus
} // namespace macro
#endif
