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

#if !defined(_UNICODE) || defined(_MBCS)
#error The Suzume Library requires UNICODE build.
#endif

#define _CRT_SECURE_NO_WARNINGS
#define _USE_MATH_DEFINES
#include <math.h>
#include <stdlib.h>
#include <string>
#include <atlstr.h>
#include <vector>
#include <map>

#include <gdiplus.h>
#pragma comment(lib, "GdiPlus.lib")

#include "suzume.h"
#include "suzume_utility.h"

#import "msxml6.dll" named_guids raw_interfaces_only

/*
//TODO: temporary disabled
//NOTE: this code may help you if you are to implement gradations
struct GRADATIONINFO{
	Gdiplus::Color color;
	float offset;
};

bool operator<(const GRADATIONINFO& a,const GRADATIONINFO& b)
{
	return a.offset<b.offset;
}
*/

namespace suzume{

void domMakeAttributeDict(IXMLDOMElementPtr element,std::map<std::wstring,std::wstring> &rDict)
{
	IXMLDOMNamedNodeMapPtr attributes;
	element->get_attributes(&attributes);

	while(attributes){
		IXMLDOMNodePtr attribute;
		attributes->nextNode(&attribute);
		if(!attribute)break;

		CComBSTR strKey;
		attribute->get_nodeName(&strKey);
		CComVariant strValue;
		attribute->get_nodeValue(&strValue);
		strValue.ChangeType(VT_BSTR);

		rDict[(LPCWSTR)strKey]=strValue.bstrVal;
	}
}



CComBSTR domGetString(IXMLDOMElementPtr element,LPCTSTR key,LPCTSTR defaultValue=_T(""))
{
	CComVariant value;
	if(SUCCEEDED(element->getAttribute(CComBSTR(key),&value))){
		value.ChangeType(VT_BSTR);
		if(value.bstrVal)return value.bstrVal;
		else return _T("");
	}else{
		return defaultValue;
	}
}

float domGetFloat(IXMLDOMElementPtr element,LPCTSTR key,float defaultValue=0.0f)
{
	using namespace suzume;
	CComVariant value;
	if(SUCCEEDED(element->getAttribute(CComBSTR(key),&value))){
		//value.ChangeType(VT_R4);
		//return value.fltVal;
		value.ChangeType(VT_BSTR);
		if(value.bstrVal){
			return toFloat(value.bstrVal);
		}else{
			return defaultValue;
		}
	}else{
		return defaultValue;
	}
}

HRESULT domGetElementById(IXMLDOMNodePtr parent,LPCTSTR lpID,IXMLDOMNodePtr& p)
{
	DOMNodeType type;
	if(SUCCEEDED(parent->get_nodeType(&type))){
		if(type==NODE_ELEMENT){
			CAtlString strID=domGetString(parent,_T("id"));
			if(strID==lpID){
				p=parent;
				return S_OK;
			}
		}
	}

	IXMLDOMNodeListPtr children;
	if(SUCCEEDED(parent->get_childNodes(&children))){
		while(true){
			IXMLDOMNodePtr child;
			if(FAILED(children->nextNode(&child)) || !child)break;

			HRESULT hr=domGetElementById(child,lpID,p);
			if(hr==S_OK)return S_OK;
		}
	}
	return S_FALSE;
}

float parseDistance(LPCTSTR lpValue)
{
	using namespace suzume;
	CAtlString strValue=lpValue;
	float scale=1.0f;
	if(-1!=strValue.Find(_T("cm"))){
		scale=72.0f/2.54f;
	}else if(-1!=strValue.Find(_T("mm"))){
		scale=7.2f/2.54f;
	}else if(-1!=strValue.Find(_T("in"))){
		scale=72.0f;
	}else if(-1!=strValue.Find(_T("em"))){
		scale=12.0f;
	}else if(-1!=strValue.Find(_T("ex"))){
		scale=8.0f;
	}else if(-1!=strValue.Find(_T("pc"))){
		scale=8.0f;
	}
	//TODO: support of "%"
	//TODO: make a distance class which supports both absolute and relative distance.
	return toFloat(strValue)*scale;
}

float domGetDistance(IXMLDOMElementPtr element,LPCTSTR key,float defaultValue=0.0f)
{
	CComVariant value;
	if(SUCCEEDED(element->getAttribute(CComBSTR(key),&value))){
		value.ChangeType(VT_BSTR);
		if(value.bstrVal){
			return parseDistance(value.bstrVal);
		}else{
			return defaultValue;
		}
	}else{
		return defaultValue;
	}
}

Gdiplus::Color parseHexColor(LPCTSTR lpColor,LPCTSTR lpOpacity)
{
	using namespace suzume;
	CAtlString strColor=lpColor;
	BYTE r=(BYTE)_tcstol(strColor.Mid(0,2),NULL,16);
	BYTE g=(BYTE)_tcstol(strColor.Mid(2,2),NULL,16);
	BYTE b=(BYTE)_tcstol(strColor.Mid(4,2),NULL,16);
	BYTE a=255;
	if(lpOpacity && *lpOpacity){
		a=(BYTE)min(255,(int)(fmax(0.0,toFloat(lpOpacity,1.0f))*255.0f));
	}

	return Gdiplus::Color(a,r,g,b);
}


HRESULT recursiveMakePropDict(IXMLDOMElementPtr element,std::map<std::wstring,std::wstring> &propDict)
{
	if(!element)return E_POINTER;
	//inherit parent styles
	IXMLDOMNodePtr p;
	if(SUCCEEDED(element->get_parentNode(&p)) && p){
		recursiveMakePropDict(p,propDict);
	}

	suzume::makeDictFromString(domGetString(element,_T("style")),propDict);
	domMakeAttributeDict(element,propDict);

	return S_OK;
}


HRESULT parseTransform(Gdiplus::Matrix& mat,IXMLDOMElementPtr element,LPCTSTR lpKey=_T("transform"))
{
	if(!element)return E_POINTER;
	//inherit parent styles
	IXMLDOMNodePtr p;
	if(SUCCEEDED(element->get_parentNode(&p)) && p){
		parseTransform(mat,IXMLDOMElementPtr(p),lpKey);
	}

	CAtlString src=domGetString(element,lpKey);
	src.Remove(_T('\r'));
	src.Replace(_T("\t"),_T(" "));
	src.Replace(_T("\n"),_T(" "));
	src.Replace(_T(") "),_T(")\n"));
	src.Replace(_T("),"),_T(")\n"));
	src.Remove(_T(' '));

	std::vector<CAtlString> transforms;
	suzume::splitString(src,_T('\n'),transforms);
	float a,b,c,d,e,f,x,y,angle;

	for(size_t i=0;i<transforms.size();i++){
		const CAtlString &s=transforms[i];

		if(6==_stscanf(s,_T("matrix(%f,%f,%f,%f,%f,%f)"),&a,&b,&c,&d,&e,&f)){
			Gdiplus::Matrix matTmp(a,b,c,d,e,f);
			mat.Multiply(&matTmp);
		}else if(2==_stscanf(s,_T("translate(%f,%f)"),&x,&y)){
			mat.Translate(x,y);
		}else if(1==_stscanf(s,_T("translate(%f)"),&x)){
			mat.Translate(x,0.0f);
		}else if(3==_stscanf(s,_T("rotate(%f,%f,%f)"),&angle,&x,&y)){
			mat.RotateAt(angle,Gdiplus::PointF(x,y));
		}else if(1==_stscanf(s,_T("rotate(%f)"),&angle)){
			mat.Rotate(angle);
		}else if(2==_stscanf(s,_T("scale(%f,%f)"),&x,&y)){
			mat.Scale(x,y);
		}else if(1==_stscanf(s,_T("scale(%f)"),&x)){
			mat.Scale(x,x);
		}else if(1==_stscanf(s,_T("skewX(%f)"),&x)){
			Gdiplus::Matrix matTmp(1.0f,0.0f,tan(x),1.0f,0.0f,0.0f);
			mat.Multiply(&matTmp);
		}else if(1==_stscanf(s,_T("skewY(%f)"),&y)){
			Gdiplus::Matrix matTmp(1.0f,tan(y),0.0f,1.0f,0.0f,0.0f);
			mat.Multiply(&matTmp);
		}
	}

	return S_OK;
}

HRESULT parseTransform(CSVGElementBase& ve,IXMLDOMElementPtr element)
{
	Gdiplus::Matrix mat;
	parseTransform(mat,element);
	ve.setTransform(mat);
	return S_OK;
}

/*
//TODO: temporary disabled
//NOTE: this code may help you if you are to implement gradations
void parseGradationStops(IXMLDOMElementPtr element,std::vector<GRADATIONINFO>& colors)
{
	IXMLDOMNodeListPtr stopList;
	if(SUCCEEDED(element->getElementsByTagName(CComBSTR(_T("stop")),&stopList)) && stopList){
		while(true){
			IXMLDOMNodePtr stop;
			if(FAILED(stopList->nextNode(&stop)) || !stop)break;

			std::map<std::wstring,std::wstring> propDict;
			recursiveMakePropDict(stop,propDict);

			GRADATIONINFO info;
			//color
			CAtlString strColor=propDict[_T("stop-color")].c_str();
			if(strColor==_T("none") || strColor.IsEmpty()){
				return;
			}else if(strColor[0]==_T('#')){
				//color code
				info.color=parseHexColor((LPCTSTR)strColor+1,propDict[_T("stop-opacity")].c_str());
			}else{
				//try to lookup color table
				COLORREF refColor;
				int ret=lookupColorTable(strColor,refColor);
				if(ret==-1){
					return;
				}else{
					LPCTSTR lpOpacity=propDict[_T("stop-opacity")].c_str();
					int a=255;
					if(lpOpacity && *lpOpacity){
						a=min(255,(int)(fmax(0.0,toFloat(lpOpacity,1.0f))*255.0f));
					}

					info.color=Gdiplus::Color(a,GetRValue(refColor),GetGValue(refColor),GetBValue(refColor));
				}
			}

			//offset
			info.offset=toFloat(propDict[_T("offset")].c_str());

			colors.push_back(info);
		}
	}
}

void parseLinearGradient(IXMLDOMElementPtr element,std::vector<GRADATIONINFO>& colors,Gdiplus::PointF& start,Gdiplus::PointF& end,Gdiplus::Matrix& transform,Gdiplus::WrapMode& wrapMode)
{
	//NOTE: this version supports only gradientUnits="userSpaceOnUse"

	//inherit from other gradient object
	CAtlString href=domGetString(element,_T("xlink:href"));
	href.Remove(_T('#'));
	if(!href.IsEmpty()){
		IXMLDOMDocumentPtr document;
		if(SUCCEEDED(element->get_ownerDocument(&document)) && document){
			IXMLDOMNodeListPtr resultList;
			if(SUCCEEDED(document->getElementsByTagName(CComBSTR(_T("svg")),&resultList)) && resultList){
				IXMLDOMNodePtr nodeSVG,p;
				if(SUCCEEDED(resultList->get_item(0,&nodeSVG)) && nodeSVG && SUCCEEDED(domGetElementById(nodeSVG,href,p)) && p){
					parseLinearGradient(p,colors,start,end,transform,wrapMode);
				}
			}
		}
	}

	start.X=domGetDistance(element,_T("x1"),start.X);
	start.Y=domGetDistance(element,_T("y1"),start.Y);
	end.X=domGetDistance(element,_T("x2"),end.X);
	end.Y=domGetDistance(element,_T("y2"),end.Y);

	parseTransform(transform,element,_T("gradientTransform"));

	CAtlString strWrapMode=domGetString(element,_T("spreadMethod"));
	if(strWrapMode==_T("pad")){
		wrapMode=Gdiplus::WrapModeClamp;
	}else if(strWrapMode==_T("reflect")){
		wrapMode=Gdiplus::WrapModeTileFlipXY;
	}else if(strWrapMode==_T("repeat")){
		wrapMode=Gdiplus::WrapModeTile;
	}

	//parse each color level
	parseGradationStops(element,colors);
}
*/

void parseStyleFill(IXMLDOMElementPtr element,std::map<std::wstring,std::wstring> &propDict,Gdiplus::Brush **ppFill)
{
	using namespace suzume;
	CAtlString strColor=propDict[_T("fill")].c_str();

	if(strColor==_T("none") || strColor.IsEmpty()){
		return;
	}else if(strColor[0]==_T('#')){
		//color code
		Gdiplus::Color color=parseHexColor((LPCTSTR)strColor+1,propDict[_T("fill-opacity")].c_str());
		*ppFill=new Gdiplus::SolidBrush(color);
	}else if(-1!=strColor.Find(_T("url("))){
		if(-1==strColor.Find(_T('#'))){
			//external reference is not supported
			*ppFill=new Gdiplus::SolidBrush(Gdiplus::Color(0,0,0));
			return;
		}

		//TODO:gradations are not supported yet.
		//NOTE: The following code may work for some situations, but the LinearGradientBrush does always repeating.
		//      In order to implement a SVG style gradation, which does not repeat, a TextureBrush might work.
		/*strColor.Replace(_T("url(#"),_T(""));
		strColor.Replace(_T(")"),_T(""));

		IXMLDOMDocumentPtr document;
		if(SUCCEEDED(element->get_ownerDocument(&document)) && document){
			IXMLDOMNodeListPtr resultList;
			if(SUCCEEDED(document->getElementsByTagName(CComBSTR(_T("svg")),&resultList)) && resultList){
				IXMLDOMNodePtr nodeSVG,p;
				if(SUCCEEDED(resultList->get_item(0,&nodeSVG)) && nodeSVG && SUCCEEDED(domGetElementById(nodeSVG,strColor,p)) && p){
					//parsing gradient definitions
					std::vector<GRADATIONINFO> stops;
					Gdiplus::PointF start(0.0f,0.0f),end(1.0f,0.0f);
					Gdiplus::Matrix transform;
					Gdiplus::WrapMode wrapMode=Gdiplus::WrapModeTile;//Gdiplus::WrapModeClamp;	The linear gradient brush does not support this flag, i.e., we cannot use this brush for the SVG style gradation
					parseLinearGradient(p,stops,start,end,transform,wrapMode);
					if(stops.size()==0){
						*ppFill=new Gdiplus::SolidBrush(Gdiplus::Color(0,0,0));
						return;
					}else if(stops.size()==1){
						*ppFill=new Gdiplus::SolidBrush(stops[0].color);
						return;
					}

					*ppFill=new Gdiplus::LinearGradientBrush(start,end,Gdiplus::Color(),Gdiplus::Color());

					//setting gradient colors
					std::sort(stops.begin(),stops.end());
					std::vector<Gdiplus::Color> colors(stops.size());
					std::vector<float> offsets(stops.size());
					for(size_t i=0;i<stops.size();i++){
						colors[i]=stops[i].color;
						offsets[i]=stops[i].offset;
					}
					Gdiplus::LinearGradientBrush* pFill=(Gdiplus::LinearGradientBrush*)*ppFill;
					pFill->SetInterpolationColors(&colors[0],&offsets[0],(int)colors.size());

					pFill->SetTransform(&transform);
					pFill->SetWrapMode(wrapMode);
					return;
				}
			}
		}*/
		//Fallback to default color
		*ppFill=new Gdiplus::SolidBrush(Gdiplus::Color(0,0,0));
	}else{
		//try to lookup color table
		COLORREF refColor;
		int ret=suzume::lookupColorTable(strColor,refColor);
		if(ret==-1){
			//Fallback to default color
			*ppFill=new Gdiplus::SolidBrush(Gdiplus::Color(0,0,0));
			return;
		}else{
			LPCTSTR lpOpacity=propDict[_T("fill-opacity")].c_str();
			BYTE a=255;
			if(lpOpacity && *lpOpacity){
				a=(BYTE)min(255,(int)(fmax(0.0,toFloat(lpOpacity,1.0f))*255.0f));
			}

			Gdiplus::Color color=Gdiplus::Color(a,(BYTE)GetRValue(refColor),(BYTE)GetGValue(refColor),(BYTE)GetBValue(refColor));
			*ppFill=new Gdiplus::SolidBrush(color);
		}
	}
}


void parseStyleStroke(std::map<std::wstring,std::wstring> &propDict,Gdiplus::Pen **ppStroke)
{
	using namespace suzume;
	CAtlString strColor=propDict[_T("stroke")].c_str();

	//color
	Gdiplus::Color color(0,0,0);
	if(strColor==_T("none") || strColor.IsEmpty()){
		return;
	}else if(strColor[0]==_T('#')){
		//color code
		color=parseHexColor((LPCTSTR)strColor+1,propDict[_T("stroke-opacity")].c_str());
	}else if(-1!=strColor.Find(_T("url("))){
		//Unsupported format(including gradation)
		//return;
	}else{
		//try to lookup color table
		COLORREF refColor;
		int ret=suzume::lookupColorTable(strColor,refColor);
		if(ret==-1){
			return;
		}else{
			LPCTSTR lpOpacity=propDict[_T("stroke-opacity")].c_str();
			BYTE a=255;
			if(lpOpacity && *lpOpacity){
				a=(BYTE)min(255,(int)(fmax(0.0f,toFloat(lpOpacity,1.0f))*255.0f));
			}

			color=Gdiplus::Color(a,(BYTE)GetRValue(refColor),(BYTE)GetGValue(refColor),(BYTE)GetBValue(refColor));
		}
	}

	//width
	CAtlString strWidth=propDict[_T("stroke-width")].c_str();
	float width;
	if(strWidth.IsEmpty()){
		width=1.0f;
	}else{
		width=parseDistance(strWidth);
	}
	*ppStroke=new Gdiplus::Pen(color,width);

	//dash
	{
		float dashOffset=toFloat(propDict[_T("stroke-dashoffset")].c_str());
		(*ppStroke)->SetDashOffset(dashOffset);

		std::vector<CAtlString> dashStrArray;
		CAtlString str=propDict[_T("stroke-dasharray")].c_str();
		str.Remove(_T('\r'));
		str.Remove(_T('\n'));
		splitString(str,_T(','),dashStrArray);
		if(!dashStrArray.empty()){
			std::vector<float> dashArray;
			for(size_t i=0;i<dashStrArray.size();i++){
				dashArray.push_back(toFloat(dashStrArray[i]));
			}
			(*ppStroke)->SetDashPattern(&dashArray[0],(INT)dashArray.size());
		}
	}

	//miter limit
	float miterLimit=toFloat(propDict[_T("stroke-miterlimit")].c_str(),4.0f);
	(*ppStroke)->SetMiterLimit(miterLimit);
}


HRESULT parseStyle(IXMLDOMElementPtr element,Gdiplus::Brush **ppFill,Gdiplus::Pen **ppStroke,CAtlString* pFamily=NULL,Gdiplus::FontStyle* pStyle=NULL,float *pFontSize=NULL)
{
	ASSERT(ppFill);
	ASSERT(ppStroke);

	std::map<std::wstring,std::wstring> propDict;

	recursiveMakePropDict(element,propDict);

	//fill
	parseStyleFill(element,propDict,ppFill);
	//stroke
	parseStyleStroke(propDict,ppStroke);

	//font settings[optional]
	if(pFamily){
		*pFamily=propDict[_T("font-family")].c_str();
	}
	if(pStyle){
		int style=Gdiplus::FontStyleRegular;
		if(propDict[_T("font-weight")]==_T("bold")){
			style|=Gdiplus::FontStyleBold;
		}
		if(propDict[_T("font-style")]==_T("italic")){
			style|=Gdiplus::FontStyleItalic;
		}

		*pStyle=(Gdiplus::FontStyle)style;
	}
	if(pFontSize){
		*pFontSize=parseDistance(propDict[_T("font-size")].c_str());
	}

	return S_OK;
}


HRESULT parseRect(CSVGImage& image,IXMLDOMNodePtr& node)
{
	IXMLDOMElementPtr element=node;
	float x,y,width,height,rx,ry;
	x=		domGetDistance(element,_T("x"));
	y=		domGetDistance(element,_T("y"));
	width=	domGetDistance(element,_T("width"));
	height=	domGetDistance(element,_T("height"));
	rx=		domGetDistance(element,_T("rx"));
	ry=		domGetDistance(element,_T("ry"));


	Gdiplus::Brush *lpFill=NULL;
	Gdiplus::Pen *lpStroke=NULL;
	parseStyle(element,&lpFill,&lpStroke);

	CSVGElementRect* pRect=image.addRect(x,y,width,height,rx,ry);
	pRect->setFill(lpFill);
	pRect->setStroke(lpStroke);
	parseTransform(*pRect,element);

	return S_OK;
}

HRESULT parseCircle(CSVGImage& image,IXMLDOMNodePtr& node)
{
	IXMLDOMElementPtr element=node;
	float x,y,r;
	x=domGetDistance(element,_T("cx"));
	y=domGetDistance(element,_T("cy"));
	r=domGetDistance(element,_T("r"));

	Gdiplus::Brush *lpFill=NULL;
	Gdiplus::Pen *lpStroke=NULL;
	parseStyle(element,&lpFill,&lpStroke);

	CSVGElementCircle* pCircle=image.addCircle(x,y,r);
	pCircle->setFill(lpFill);
	pCircle->setStroke(lpStroke);
	parseTransform(*pCircle,element);

	return S_OK;
}

HRESULT parseLine(CSVGImage& image,IXMLDOMNodePtr& node)
{
	IXMLDOMElementPtr element=node;
	float x1=domGetDistance(element,_T("x1"));
	float y1=domGetDistance(element,_T("y1"));
	float x2=domGetDistance(element,_T("x2"));
	float y2=domGetDistance(element,_T("y2"));

	Gdiplus::Brush *lpFill=NULL;
	Gdiplus::Pen *lpStroke=NULL;
	parseStyle(element,&lpFill,&lpStroke);

	CSVGElementPath* pPath=image.addPath();
	pPath->moveTo(x1,y1,true);
	pPath->lineTo(x2,y2,true);

	pPath->setFill(lpFill);
	pPath->setStroke(lpStroke);
	parseTransform(*pPath,element);

	return S_OK;
}

HRESULT parsePolyLine(CSVGImage& image,IXMLDOMNodePtr& node)
{
	IXMLDOMElementPtr element=node;
	CAtlString strPath=domGetString(element,_T("points"));
	strPath.Remove(_T('\r'));
	strPath.Remove(_T('\n'));
	std::vector<CAtlString> pathData;
	suzume::splitString(strPath,_T(" ,"),pathData);

	if(!pathData.empty()){
		CSVGElementPath* pPath=image.addPath();

		for(size_t i=0;i<pathData.size();i+=2){
			if(i==0){
				pPath->moveTo(parseDistance(pathData[i]),parseDistance(pathData[i+1]),true);
			}else{
				pPath->lineTo(parseDistance(pathData[i]),parseDistance(pathData[i+1]),true);
			}
		}

		Gdiplus::Brush *lpFill=NULL;
		Gdiplus::Pen *lpStroke=NULL;
		parseStyle(element,&lpFill,&lpStroke);

		pPath->setFill(lpFill);
		pPath->setStroke(lpStroke);
		parseTransform(*pPath,element);
	}

	return S_OK;
}

HRESULT parsePolygon(CSVGImage& image,IXMLDOMNodePtr& node)
{
	IXMLDOMElementPtr element=node;
	CAtlString strPath=domGetString(element,_T("points"));
	std::vector<CAtlString> pathData;
	strPath.Remove(_T('\r'));
	strPath.Remove(_T('\n'));
	suzume::splitString(strPath,_T(" ,"),pathData);

	if(!pathData.empty()){
		CSVGElementPath* pPath=image.addPath();

		for(size_t i=0;i<pathData.size();i+=2){
			if(i==0){
				pPath->moveTo(parseDistance(pathData[i]),parseDistance(pathData[i+1]),true);
			}else{
				pPath->lineTo(parseDistance(pathData[i]),parseDistance(pathData[i+1]),true);
			}
		}
		pPath->closePath();

		Gdiplus::Brush *lpFill=NULL;
		Gdiplus::Pen *lpStroke=NULL;
		parseStyle(element,&lpFill,&lpStroke);

		pPath->setFill(lpFill);
		pPath->setStroke(lpStroke);
		parseTransform(*pPath,element);
	}

	return S_OK;
}


HRESULT parseText(CSVGImage& image,IXMLDOMNodePtr& node,float *pLastLeft=NULL,float *pLastTop=NULL)
{
	//texts are stored directly, or contained in some <tspan>s.
	float _tmpX=0.0f,_tmpY=0.0f;
	if(!pLastLeft){
		pLastLeft=&_tmpX;
		pLastTop=&_tmpY;
	}

	IXMLDOMElementPtr parent=node;

	IXMLDOMNodeListPtr resultList;
	if(SUCCEEDED(parent->get_childNodes(&resultList)) && resultList){
		//updating text coordinate
		{
			float x=domGetDistance(parent,_T("x"),*pLastLeft)+domGetDistance(parent,_T("dx"));
			float y=domGetDistance(parent,_T("y"),*pLastTop)+domGetDistance(parent,_T("dy"));
			*pLastLeft=x;
			*pLastTop=y;
		}

		while(true){
			IXMLDOMNodePtr child;
			if(FAILED(resultList->nextNode(&child)) || !child)break;

			DOMNodeType type;
			if(SUCCEEDED(child->get_nodeType(&type))){
				if(type==NODE_TEXT){
					//text found
					CComVariant var;
					if(SUCCEEDED(child->get_nodeValue(&var))){
						var.ChangeType(VT_BSTR);
						CAtlString strText;
						strText=var.bstrVal;

						Gdiplus::Brush *lpFill=NULL;
						Gdiplus::Pen *lpStroke=NULL;

						Gdiplus::FontStyle style;
						float fontSize;
						//Gdiplus::FontFamily::GenericSansSerif();
						CAtlString strFontFamily;
						parseStyle(parent,&lpFill,&lpStroke,&strFontFamily,&style,&fontSize);

						Gdiplus::FontFamily *lpFontFamily=NULL;
						if(!strFontFamily.IsEmpty()){
							lpFontFamily=new Gdiplus::FontFamily(strFontFamily);
							if(!lpFontFamily->IsAvailable()){
								//font not found.
								delete lpFontFamily;
								lpFontFamily=NULL;
							}
						}
						//if lpFontFamily==NULL, use default font
						const Gdiplus::FontFamily *lpFontToUse=(lpFontFamily ? lpFontFamily : Gdiplus::FontFamily::GenericSansSerif());

						//add text as a path
						CSVGElementPath* pPath=image.addPath();
						float dy=(lpFontToUse->GetCellAscent(style)) * (fontSize / lpFontToUse->GetEmHeight(style));

						pPath->addString(strText,lpFontToUse,style,fontSize,*pLastLeft,*pLastTop-dy);

						//get text width to update the coordinate of the next text.
						Gdiplus::RectF rect;
						pPath->getBounds(rect);
						*pLastLeft+=rect.Width;

						delete lpFontFamily;
						lpFontFamily=NULL;

						pPath->setFill(lpFill);
						pPath->setStroke(lpStroke);
						parseTransform(*pPath,parent);
					}
				}else{
					//recursive processing
					parseText(image,child,pLastLeft,pLastTop);
				}
			}
		}
	}

	return S_OK;
}

HRESULT parsePath(CSVGImage& image,IXMLDOMNodePtr node)
{
	using namespace suzume;
	IXMLDOMElementPtr element=node;
	CAtlString strPath=domGetString(element,_T("d"));
	strPath.Remove(_T('\r'));
	strPath.Remove(_T('\n'));

	//separate command and coordinates
	LPCTSTR lpCommands=_T("MmLlHhVvCcSsZzQqTtAa");
	for(LPCTSTR p=lpCommands;*p;p++){
		CAtlString oldValue=*p;
		CAtlString newValue=*p;
		newValue+=_T(' ');
		strPath.Replace(oldValue,newValue);
	}

	std::vector<CAtlString> pathData;
	suzume::splitString(strPath,_T(" ,"),pathData);
	if(!pathData.empty()){
		CSVGElementPath* pPath=image.addPath();

		TCHAR cmd=_T('\0');
		for(size_t i=0;i<pathData.size();){
			TCHAR head=pathData[i][0];
			for(LPCTSTR p=lpCommands;*p;p++){
				if(head==*p){
					cmd=head;
					i++;
					break;
				}
			}

			switch(cmd){
			case _T('M'):	//move to
			case _T('m'):
				if(!pPath->isClosed()){
					pPath->startPath();
				}
				pPath->moveTo(parseDistance(pathData[i]),parseDistance(pathData[i+1]),cmd==_T('M'));
				i+=2;
				cmd= (cmd == _T('M')) ? _T('L') : _T('l');
				break;
			case _T('L'):	//line to
			case _T('l'):
				pPath->lineTo(parseDistance(pathData[i]),parseDistance(pathData[i+1]),cmd==_T('L'));
				i+=2;
				break;
			case _T('H'):	//horizontal line to
			case _T('h'):
				pPath->horizontalLineTo(parseDistance(pathData[i]),cmd==_T('H'));
				i+=1;
				break;
			case _T('V'):	//vertical line to
			case _T('v'):
				pPath->verticalLineTo(parseDistance(pathData[i]),cmd==_T('V'));
				i+=1;
				break;
			case _T('C'):	//cubic bezier curve to
			case _T('c'):
				pPath->bezier(parseDistance(pathData[i]),parseDistance(pathData[i+1]),parseDistance(pathData[i+2]),parseDistance(pathData[i+3]),parseDistance(pathData[i+4]),parseDistance(pathData[i+5]),cmd==_T('C'));
				i+=6;
				break;
			case _T('S'):	//smooth cubic bezier curve to
			case _T('s'):
				pPath->smoothCubicBezier(parseDistance(pathData[i]),parseDistance(pathData[i+1]),parseDistance(pathData[i+2]),parseDistance(pathData[i+3]),cmd==_T('S'));
				i+=4;
				break;
			case _T('Z'):	//close path
			case _T('z'):
				pPath->closePath();
				break;
			case _T('Q'):	//quadratic bezier
			case _T('q'):
				pPath->quadraticBezier(parseDistance(pathData[i]),parseDistance(pathData[i+1]),parseDistance(pathData[i+2]),parseDistance(pathData[i+3]),cmd==_T('Q'));
				i+=4;
				break;
			case _T('T'):	//smooth quadratic bezier
			case _T('t'):
				pPath->smoothQuadraticBezier(parseDistance(pathData[i]),parseDistance(pathData[i+1]),cmd==_T('T'));
				i+=2;
				break;
			case _T('A'):	//arcto
			case _T('a'):
				pPath->arcTo(parseDistance(pathData[i]),parseDistance(pathData[i+1]),toFloat(pathData[i+2]),toInt(pathData[i+3]),toInt(pathData[i+4]),parseDistance(pathData[i+5]),parseDistance(pathData[i+6]),cmd==_T('A'));
				i+=7;
				break;
			}
		}


		Gdiplus::Brush *lpFill=NULL;
		Gdiplus::Pen *lpStroke=NULL;
		parseStyle(element,&lpFill,&lpStroke);

		pPath->setFill(lpFill);
		pPath->setStroke(lpStroke);
		parseTransform(*pPath,element);
	}

	return S_OK;
}


HRESULT recursiveBuildVectorImage(CSVGImage &image,IXMLDOMNodePtr node)
{
	IXMLDOMNodeListPtr children;
	node->get_childNodes(&children);

	while(children){
		IXMLDOMNodePtr child;
		children->nextNode(&child);
		if(!child)break;
		//check if this element is visible
		DOMNodeType type;
		if(SUCCEEDED(child->get_nodeType(&type))){
			if(type==NODE_ELEMENT){
				CAtlString strDisplay=domGetString(IXMLDOMElementPtr(child),_T("display"));
				if(strDisplay==_T("none"))continue;
			}
		}

		CComBSTR nodeName;
		child->get_nodeName(&nodeName);
		if(nodeName==_T("defs")){
			//do not recursive process
			continue;
		}else if(nodeName==_T("g")){
			//nothing to do
		}else if(nodeName==_T("rect")){
			parseRect(image,child);
		}else if(nodeName==_T("circle")){
			parseCircle(image,child);
		}else if(nodeName==_T("line")){
			parseLine(image,child);
		}else if(nodeName==_T("polyline")){
			parsePolyLine(image,child);
		}else if(nodeName==_T("polygon")){
			parsePolygon(image,child);
		}else if(nodeName==_T("text")){
			parseText(image,child);
		}else if(nodeName==_T("path")){
			parsePath(image,child);
		}
		recursiveBuildVectorImage(image,child);
	}

	return S_OK;
}

};

//----------

CSVGElementBase::CSVGElementBase():_fill(NULL),_stroke(NULL)
{
}

CSVGElementBase::~CSVGElementBase()
{
	if(_fill){delete _fill;_fill=NULL;}
	if(_stroke){delete _stroke;_stroke=NULL;}
}

void CSVGElementBase::setFill(Gdiplus::Brush* lpBrush)
{
	if(_fill){delete _fill;_fill=NULL;}
	_fill=lpBrush;
}

void CSVGElementBase::setStroke(Gdiplus::Pen* lpStroke)
{
	if(_stroke){delete _stroke;_stroke=NULL;}
	_stroke=lpStroke;
}

void CSVGElementBase::pushRotation(float angle)
{
	_matrix.Rotate(angle);
}

void CSVGElementBase::pushRotation(float angle,float cx,float cy)
{
	Gdiplus::PointF center(cx,cy);
	_matrix.RotateAt(angle,center);
}

void CSVGElementBase::pushScale(float sx,float sy)
{
	_matrix.Scale(sx,sy);
}

void CSVGElementBase::pushTranslation(float tx,float ty)
{
	_matrix.Translate(tx,ty);
}

void CSVGElementBase::pushSkewX(float angle)
{
	Gdiplus::Matrix mat(1.0f,0.0f,tan(angle),1.0f,0.0f,0.0f);
	_matrix.Multiply(&mat);
}

void CSVGElementBase::pushSkewY(float angle)
{
	Gdiplus::Matrix mat(1.0f,tan(angle),0.0f,1.0f,0.0f,0.0f);
	_matrix.Multiply(&mat);
}

void CSVGElementBase::pushMatrix(float a,float b,float c,float d,float e,float f)
{
	Gdiplus::Matrix mat(a,b,c,d,e,f);
	_matrix.Multiply(&mat);
}

void CSVGElementBase::setTransform(const Gdiplus::Matrix &mat)
{
	_matrix.Reset();
	_matrix.Multiply(&mat);
}


//----------

CSVGElementRect::CSVGElementRect():_x(0.0f),_y(0.0f),_width(0.0f),_height(0.0f),_rx(0.0f),_ry(0.0f)
{
}

CSVGElementRect::~CSVGElementRect()
{
}

void CSVGElementRect::setParams(float x,float y,float width,float height,float rx,float ry)
{
	_x=x;
	_y=y;
	_width=width;
	_height=height;
	_rx=rx;
	_ry=ry;
}

void CSVGElementRect::render(Gdiplus::Graphics& graphics)
{
	//copy & multiply : representing matrix stack
	Gdiplus::Matrix matOld;
	graphics.GetTransform(&matOld);
	graphics.MultiplyTransform(&_matrix);

	float rx=_rx,ry=_ry;
	if(rx>0.0 || ry>0.0){
		//drawing a rounded rectangle with path as combination of arcs
		if(rx==0.0){
			rx=ry;
		}else if(ry==0.0){
			ry=rx;
		}
		Gdiplus::GraphicsPath path;
		path.SetFillMode(Gdiplus::FillModeWinding);
		path.AddArc(_x + _width - rx,	_y,					rx, ry, 270,90);
		path.AddArc(_x + _width - rx,	_y + _height - ry,	rx, ry, 0,	90);
		path.AddArc(_x,					_y + _height - ry,	rx, ry, 90,	90);
		path.AddArc(_x,					_y,					rx, ry, 180,90);
		path.CloseFigure();

		if(_fill){
			graphics.FillPath(_fill, &path);
		}
		if(_stroke){
			graphics.DrawPath(_stroke, &path);
		}
	}else{
		//draw a pure rectangle
		if(_fill){
			graphics.FillRectangle(_fill, _x,_y,_width,_height);
		}
		if(_stroke){
			graphics.DrawRectangle(_stroke,_x,_y,_width,_height);
		}
	}
	graphics.SetTransform(&matOld);
}

//----------

CSVGElementCircle::CSVGElementCircle():_x(0.0f),_y(0.0f),_r(0.0f)
{
}

CSVGElementCircle::~CSVGElementCircle()
{
}

void CSVGElementCircle::setParams(float x,float y,float r)
{
	_x=x;
	_y=y;
	_r=r;
}

void CSVGElementCircle::render(Gdiplus::Graphics& graphics)
{
	//copy & multiply : representing matrix stack
	Gdiplus::Matrix matOld;
	graphics.GetTransform(&matOld);
	float e[6];
	matOld.GetElements(e);
	Gdiplus::Matrix mat;
	mat.SetElements(e[0],e[1],e[2],e[3],e[4],e[5]);
	mat.Multiply(&_matrix);

	graphics.SetTransform(&mat);

	//draw a circle
	if(_fill){
		graphics.FillEllipse(_fill, _x-_r,_y-_r,_r*2.0f,_r*2.0f);
	}
	if(_stroke){
		graphics.DrawEllipse(_stroke, _x-_r,_y-_r,_r*2.0f,_r*2.0f);
	}

	graphics.SetTransform(&matOld);
}


//----------


float CSVGElementPath::getAngle(const Gdiplus::PointF& a,const Gdiplus::PointF& b)
{
	using namespace suzume;
	//should be atan2?
	float prod=a.X*b.X + a.Y*b.Y;
	float div=sqrt( (pow2(a.X)+pow2(a.Y)) * (pow2(b.X)+pow2(b.Y)) );
	float sign=(a.X*b.Y - a.Y*b.X) > 0.0f ? 1.0f : -1.0f;
	return acos(prod/div)*sign *180.0f/(float)M_PI;
}

CSVGElementPath::CSVGElementPath():_bFirst(true)
{
	_path.SetFillMode(Gdiplus::FillModeAlternate);
}

CSVGElementPath::~CSVGElementPath()
{
}

void CSVGElementPath::addString(LPCWSTR lpText,const Gdiplus::FontFamily* lpFontFamily,Gdiplus::FontStyle style,float fontSize,float x,float y)
{
	_path.AddString(lpText,-1,lpFontFamily,style,fontSize,Gdiplus::PointF(x,y),Gdiplus::StringFormat::GenericTypographic());
}

void CSVGElementPath::lineTo(float x,float y,bool bAbsolute)
{
	Gdiplus::PointF currentPos(x,y);
	if(!bAbsolute){
		currentPos=currentPos+_lastPos;
	}
	if(_bFirst){
		_firstPos=currentPos;
	}
	_bFirst=false;

	_path.AddLine(_lastPos,currentPos);

	_lastPos=currentPos;
	_lastC2=_lastPos;
}

void CSVGElementPath::horizontalLineTo(float x,bool bAbsolute)
{
	Gdiplus::PointF currentPos(x,_lastPos.Y);
	if(!bAbsolute){
		currentPos.X+=_lastPos.X;
	}
	if(_bFirst){
		_firstPos=currentPos;
	}
	_bFirst=false;

	_path.AddLine(_lastPos,currentPos);

	_lastPos=currentPos;
	_lastC2=_lastPos;
}

void CSVGElementPath::verticalLineTo(float y,bool bAbsolute)
{
	Gdiplus::PointF currentPos(_lastPos.X,y);
	if(!bAbsolute){
		currentPos.Y+=_lastPos.Y;
	}
	if(_bFirst){
		_firstPos=currentPos;
	}
	_bFirst=false;

	_path.AddLine(_lastPos,currentPos);

	_lastPos=currentPos;
	_lastC2=_lastPos;
}

void CSVGElementPath::moveTo(float x,float y,bool bAbsolute)
{
	Gdiplus::PointF currentPos(x,y);
	if(!bAbsolute){
		currentPos=currentPos+_lastPos;
	}
	if(_bFirst){
		_firstPos=currentPos;
	}
	_bFirst=false;

	_lastPos=currentPos;
	_lastC2=_lastPos;
}

void CSVGElementPath::bezier(float x1,float y1,float x2,float y2,float endX,float endY,bool bAbsolute)
{
	Gdiplus::PointF c1(x1,y1);
	Gdiplus::PointF c2(x2,y2);
	Gdiplus::PointF end(endX,endY);
	if(!_bFirst && !bAbsolute){
		c1=c1+_lastPos;
		c2=c2+_lastPos;
		end=end+_lastPos;
	}
	if(_bFirst){
		_firstPos=_lastPos;
	}
	_bFirst=false;

	_path.AddBezier(_lastPos,c1,c2,end);

	_lastC2=c2;
	_lastPos=end;
}

void CSVGElementPath::smoothCubicBezier(float x2,float y2,float endX,float endY,bool bAbsolute)
{
	Gdiplus::PointF c2(x2,y2);
	Gdiplus::PointF end(endX,endY);
	if(!_bFirst && !bAbsolute){
		c2=c2+_lastPos;
		end=end+_lastPos;
	}
	if(_bFirst){
		_firstPos=_lastPos;
	}
	_bFirst=false;
	Gdiplus::PointF c1(_lastPos.X*2.0f-_lastC2.X,_lastPos.Y*2.0f-_lastC2.Y);	//reflection of the second control point of the previous curve

	_path.AddBezier(_lastPos,c1,c2,end);

	_lastC2=c2;
	_lastPos=end;
}

void CSVGElementPath::quadraticBezier(float x1,float y1,float endX,float endY,bool bAbsolute)
{
	Gdiplus::PointF c1(x1,y1);
	Gdiplus::PointF end(endX,endY);
	if(!_bFirst && !bAbsolute){
		c1=c1+_lastPos;
		end=end+_lastPos;
	}
	if(_bFirst){
		_firstPos=_lastPos;
	}
	_bFirst=false;

	_path.AddBezier(_lastPos,c1,c1,end);

	_lastC2=c1;
	_lastPos=end;
}

void CSVGElementPath::smoothQuadraticBezier(float endX,float endY,bool bAbsolute)
{
	Gdiplus::PointF end(endX,endY);
	if(!_bFirst && !bAbsolute){
		end=end+_lastPos;
	}
	if(_bFirst){
		_firstPos=_lastPos;
	}
	_bFirst=false;
	Gdiplus::PointF c1(_lastPos.X*2.0f-_lastC2.X,_lastPos.Y*2.0f-_lastC2.Y);	//reflection of the control point of the previous curve

	_path.AddBezier(_lastPos,c1,c1,end);

	_lastC2=c1;
	_lastPos=end;
}


void CSVGElementPath::arcTo(float rx,float ry,float x_axis_rotation,int large_arc_flag,int sweep_flag,float x2,float y2,bool bAbsolute)
{
	using namespace suzume;
	rx=fabs(rx);
	ry=fabs(ry);
	if(rx<1.0e-5f || ry<1.0e-5f){
		//Ensure radii are non-zero
		lineTo(x2,y2,bAbsolute);
		return;
	}

	if(!_bFirst && !bAbsolute){
		x2+=_lastPos.X;
		y2+=_lastPos.Y;
	}
	if(_bFirst){
		_firstPos=_lastPos;
	}
	_bFirst=false;

	float x1=_lastPos.X;
	float y1=_lastPos.Y;

	float cphi=cos(x_axis_rotation);
	float sphi=sin(x_axis_rotation);

	//step1
	float dx=x1-x2;
	float dy=y1-y2;
	float x1_=( cphi*dx + sphi*dy)*0.5f;
	float y1_=(-sphi*dx + cphi*dy)*0.5f;

	//Ensure radii are large enough
	float lambda=pow2(x1_/rx)+pow2(y1_/ry);
	if(lambda>1.0f){
		rx*=sqrt(lambda);
		ry*=sqrt(lambda);
	}

	//step2
	float sign=(large_arc_flag==sweep_flag) ? -1.0f : 1.0f;
	float scale=sqrt( (pow2(rx*ry)-pow2(rx*y1_)-pow2(ry*x1_)) / (pow2(rx*y1_) + pow2(ry*x1_)));
	float cx_= sign * scale * rx * y1_ / ry;
	float cy_=-sign * scale * ry * x1_ / rx;

	//step3
	float cx=( cphi*cx_ - sphi*cy_) + (x1+x2)*0.5f;
	float cy=( sphi*cx_ + cphi*cy_) + (y1+y2)*0.5f;

	//step4
	Gdiplus::PointF u( ( x1_-cx_)/rx, ( y1_-cy_)/ry );
	Gdiplus::PointF v( (-x1_-cx_)/rx, (-y1_-cy_)/ry );
	float theta=getAngle(Gdiplus::PointF(1.0f,0.0f),u);
	float deltatheta=getAngle(u,v);

	if(sweep_flag==0 && deltatheta>0.0f)deltatheta-=360.0f;
	else if(sweep_flag==1 && deltatheta<0.0f)deltatheta+=360.0f;

	//add a path
	_path.AddArc(cx-rx,cy-ry,rx*2.0f,ry*2.0f,theta,deltatheta);

	_lastPos.X=x2;
	_lastPos.Y=y2;
	_lastC2=_lastPos;
}

void CSVGElementPath::startPath()
{
	_path.StartFigure();

	_bFirst=true;
	//_lastPos=_firstPos;
	//_lastC2=_lastPos;
}

void CSVGElementPath::closePath()
{
	_path.CloseFigure();

	_bFirst=true;
	_lastPos=_firstPos;
	_lastC2=_lastPos;
}

bool CSVGElementPath::isClosed()const
{
	return _bFirst;
}

const Gdiplus::PointF& CSVGElementPath::getLastPos()const
{
	return _lastPos;
}

void CSVGElementPath::render(Gdiplus::Graphics& graphics)
{
	//copy & multiply : representing matrix stack
	Gdiplus::Matrix matOld;
	graphics.GetTransform(&matOld);
	float e[6];
	matOld.GetElements(e);
	Gdiplus::Matrix mat;
	mat.SetElements(e[0],e[1],e[2],e[3],e[4],e[5]);
	mat.Multiply(&_matrix);

	graphics.SetTransform(&mat);

	if(_fill){
		graphics.FillPath(_fill, &_path);
	}
	if(_stroke){
		graphics.DrawPath(_stroke, &_path);
	}

	graphics.SetTransform(&matOld);
}

void CSVGElementPath::getBounds(Gdiplus::RectF& bounds)const
{
	_path.GetBounds(&bounds,NULL,NULL);
}

//----------

CSVGImage::CSVGImage():_canvasWidth(0.0f),_canvasHeight(0.0f),_lpLastError(NULL)
{
}

CSVGImage::~CSVGImage()
{
	clear();
}

void CSVGImage::setLastError(LPCTSTR lpLastError)
{
	if(_lpLastError){
		delete [] _lpLastError;
		_lpLastError=NULL;
	}
	if(lpLastError){
		_lpLastError=new TCHAR[_tcslen(lpLastError)+1];
		_tcsncpy(_lpLastError,lpLastError,_tcslen(lpLastError));
	}
}

LPCTSTR CSVGImage::getLastError()const
{
	return _lpLastError;
}

void CSVGImage::clear()
{
	for(std::vector<CSVGElementBase*>::iterator ite=_elements.begin();ite!=_elements.end();++ite){
		delete *ite;
	}
	_elements.clear();
	_canvasWidth=0.0f;
	_canvasHeight=0.0f;

	if(_lpLastError){
		delete [] _lpLastError;
		_lpLastError=NULL;
	}
}

void CSVGImage::setCanvasSize(float width,float height)
{
	_canvasWidth=width;
	_canvasHeight=height;
}

float CSVGImage::getCanvasWidth()const
{
	return _canvasWidth;
}

float CSVGImage::getCanvasHeight()const
{
	return _canvasHeight;
}

CSVGElementRect* CSVGImage::addRect(float x,float y,float width,float height,float rx,float ry)
{
	CSVGElementRect* pRect=new CSVGElementRect;
	pRect->setParams(x,y,width,height,rx,ry);
	_elements.push_back(pRect);
	return pRect;
}

CSVGElementCircle* CSVGImage::addCircle(float x,float y,float r)
{
	CSVGElementCircle* pCircle=new CSVGElementCircle;
	pCircle->setParams(x,y,r);
	_elements.push_back(pCircle);
	return pCircle;
}

CSVGElementPath* CSVGImage::addPath()
{
	CSVGElementPath* pPath=new CSVGElementPath;
	_elements.push_back(pPath);
	return pPath;
}

void CSVGImage::render(Gdiplus::Graphics &graphics,float left,float top,float scaleX,float scaleY)
{
	Gdiplus::Matrix matOld,matTrans;
	matTrans.Translate(left,top);
	matTrans.Scale(scaleX,scaleY);
	graphics.GetTransform(&matOld);
	graphics.SetTransform(&matTrans);

	Gdiplus::SmoothingMode sm = graphics.GetSmoothingMode();
	Gdiplus::PixelOffsetMode pom = graphics.GetPixelOffsetMode();
	Gdiplus::CompositingMode comm=graphics.GetCompositingMode();
	Gdiplus::CompositingQuality comq=graphics.GetCompositingQuality();

	graphics.SetSmoothingMode(Gdiplus::SmoothingModeHighQuality);
	graphics.SetPixelOffsetMode(Gdiplus::PixelOffsetModeHighQuality);
	graphics.SetCompositingMode(Gdiplus::CompositingModeSourceOver);
	graphics.SetCompositingQuality(Gdiplus::CompositingQualityDefault);	//changing the CompositionQualityMode may affect the composited color

	for(std::vector<CSVGElementBase*>::iterator ite=_elements.begin();ite!=_elements.end();++ite){
		(*ite)->render(graphics);
	}

	graphics.SetSmoothingMode(sm);
	graphics.SetPixelOffsetMode(pom);
	graphics.SetCompositingMode(comm);
	graphics.SetCompositingQuality(comq);

	graphics.SetTransform(&matOld);
}

HRESULT CSVGImage::load(LPCTSTR lpszFilename,BOOL bValidate)
{
	if(!lpszFilename){
		setLastError(_T("Invalid filename pointer"));
		return E_POINTER;
	}

	CAtlString strErr;
	if(!PathFileExists(lpszFilename)){
		strErr.Format(_T("The file '%s' does not exist"),lpszFilename);
		setLastError(strErr);
		return E_INVALIDARG;
	}

	// initialize XML Parser
	IXMLDOMDocumentPtr document;
	HRESULT hResult = document.CreateInstance(CLSID_DOMDocument);
	if(FAILED(hResult)){
		setLastError(_T("Failed to init create document instance"));
		return hResult;
	}

	document->put_resolveExternals(bValidate ? VARIANT_TRUE : VARIANT_FALSE);	// DTD?
	document->put_validateOnParse(bValidate ? VARIANT_TRUE : VARIANT_FALSE);	// validate?
	document->put_async(VARIANT_FALSE);		// set flag to wait for parser

	// load SVG document
	VARIANT_BOOL verbResult;
	//hResult=document->loadXML(CComBSTR((char*)&rawXML[0]),&verbResult);
	CComVariant fname(lpszFilename);
	hResult=document->load(fname,&verbResult);
	if(FAILED(hResult) || !verbResult){
		// error detected
		IXMLDOMParseErrorPtr error;
		document->get_parseError(&error);
		long hErrorCode;
		error->get_errorCode(&hErrorCode);
		if(hErrorCode != 0){
			long line,linePos;
			CComBSTR reason;
			if( SUCCEEDED(error->get_line(&line)) && SUCCEEDED(error->get_linepos(&linePos)) && SUCCEEDED(error->get_reason(&reason))){
				strErr.Format(_T("Error 0x%x at line %d (%d) : %s\n"),hErrorCode,line,linePos,(LPCTSTR)reason);
				setLastError(strErr);
			}
			return hErrorCode;
		}
		return hErrorCode;
	}

	//get root element
	IXMLDOMElementPtr root;
	document->get_documentElement(&root);

	IXMLDOMNodeListPtr resultList;
	if(SUCCEEDED(document->getElementsByTagName(CComBSTR(_T("svg")),&resultList)) && resultList){
		IXMLDOMNodePtr nodeSVG;
		if(SUCCEEDED(resultList->get_item(0,&nodeSVG))){
			IXMLDOMElementPtr element=nodeSVG;
			float width=suzume::domGetDistance(element,_T("width"));
			float height=suzume::domGetDistance(element,_T("height"));
			//Canvas size
			setCanvasSize(width,height);
			suzume::recursiveBuildVectorImage(*this,nodeSVG);
		}
	}
	return S_OK;
}
