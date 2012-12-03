/*
* The Suzume SVG Library
* A tiny SVG parser and renderer using MSXML and GDI+
* Released under zlib License as:

Copyright (c) 2012. Claybird <claybird.without.wing@gmail.com>

This software is provided 'as-is', without any express or implied
warranty. In no event will the authors be held liable for any damages
arising from the use of this software.

Permission is granted to anyone to use this software for any purpose,
including commercial applications, and to alter it and redistribute it
freely, subject to the following restrictions:

   1. The origin of this software must not be misrepresented; you must not
   claim that you wrote the original software. If you use this software
   in a product, an acknowledgment in the product documentation would be
   appreciated but is not required.

   2. Altered source versions must be plainly marked as such, and must not be
   misrepresented as being the original software.

   3. This notice may not be removed or altered from any source
   distribution.

*/

#pragma once
namespace suzume{

#ifndef fmax
inline float fmax(float x,float y){return x>y ? x : y;}
#endif fmax

#ifndef fmin
inline float fmin(float x,float y){return x<y ? x : y;}
#endif fmin

#ifndef COUNTOF
#define COUNTOF(x) (sizeof(x)/sizeof(x[0]))
#endif

#ifndef ASSERT
#define ASSERT ATLASSERT
#endif

inline float pow2(float x){return x*x;}

float toFloat(LPCTSTR lpData,float defaultValue=0.0f);

int toInt(LPCTSTR lpData);

void makeDictFromString(LPCTSTR lpString,std::map<std::wstring,std::wstring> &rDict);

//Split a string with one of the cSeparator
void splitString(LPCTSTR lpString,TCHAR cSeparator,std::vector<CAtlString> &rArray);

//Split a string with one of the lpSeparators
void splitString(LPCTSTR lpString,LPCTSTR lpSeparators,std::vector<CAtlString> &rArray);

int lookupColorTable(LPCTSTR lpName,COLORREF &color);
};
