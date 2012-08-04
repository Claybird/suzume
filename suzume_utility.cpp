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

#include <stdlib.h>
#include <string>
#include <atlstr.h>
#include <vector>
#include <map>

#include "suzume_utility.h"
#include "suzume_colorname.h"


namespace suzume{

float toFloat(LPCTSTR lpData,float defaultValue)
{
	if(!lpData || !*lpData)return defaultValue;
	return (float)_tstof(lpData);
}

int toInt(LPCTSTR lpData)
{
	return _tstoi(lpData);
}


void makeDictFromString(LPCTSTR lpString,std::map<std::wstring,std::wstring> &rDict)
{
	CAtlString strKey,strValue;
	bool bKey=true;
	for(;*lpString;lpString++){
		if(*lpString==_T('\n') || *lpString==_T('\r')){
			continue;
		}else if(*lpString==_T(':')){
			bKey=false;
		}else if(*lpString==_T(';')){
			if(!strKey.IsEmpty()){
				bKey=true;
				strKey=strKey.TrimLeft().TrimRight();
				strValue=strValue.TrimLeft().TrimRight();
				rDict[(LPCWSTR)strKey]=(LPCWSTR)strValue;
				strKey.Empty();
				strValue.Empty();
			}
		}else{
			if(bKey){
				strKey+=*lpString;
			}else{
				strValue+=*lpString;
			}
		}
	}

	if(!strKey.IsEmpty()){
		strKey=strKey.TrimLeft().TrimRight();
		strValue=strValue.TrimLeft().TrimRight();
		rDict[(LPCWSTR)strKey]=(LPCWSTR)strValue;
	}
}

//Split a string with one of the cSeparator
void splitString(LPCTSTR lpString,TCHAR cSeparator,std::vector<CAtlString> &rArray)
{
	rArray.clear();

	CAtlString strTmp;
	for(;*lpString;lpString++){
		if(*lpString==cSeparator){
			if(!strTmp.IsEmpty()){
				rArray.push_back(strTmp);
				strTmp.Empty();
			}
		}else{
			strTmp+=*lpString;
		}
	}

	if(!strTmp.IsEmpty()){
		rArray.push_back(strTmp);
	}
}

//Split a string with one of the lpSeparators
void splitString(LPCTSTR lpString,LPCTSTR lpSeparators,std::vector<CAtlString> &rArray)
{
	rArray.clear();

	CAtlString strTmp;
	for(;*lpString;lpString++){
		bool bSeparator=false;
		for(LPCTSTR p=lpSeparators;*p;p++){
			if(*p==*lpString){
				if(!strTmp.IsEmpty()){
					rArray.push_back(strTmp);
					strTmp.Empty();
				}
				bSeparator=true;
				break;
			}
		}
		if(!bSeparator){
			strTmp+=*lpString;
		}
	}

	if(!strTmp.IsEmpty()){
		rArray.push_back(strTmp);
	}
}

int lookupColorTable(LPCTSTR lpName,COLORREF &color)
{
	for(size_t i=0;i<COUNTOF(s_Colortable);i++){
		if(_tcsicmp(lpName,s_Colortable[i].name)==0){
			color=s_Colortable[i].color;
			return (int)i;
		}
	}
	return -1;
}

};
