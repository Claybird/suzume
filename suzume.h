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
#include <vector>
#include <gdiplus.h>

class CSVGElementBase{
protected:
	Gdiplus::Brush* _fill;
	Gdiplus::Pen* _stroke;

	Gdiplus::Matrix _matrix;
public:
	CSVGElementBase();
	virtual ~CSVGElementBase();
	void setFill(Gdiplus::Brush* lpBrush);
	void setStroke(Gdiplus::Pen* lpStroke);
	void pushRotation(float angle);
	void pushRotation(float angle,float cx,float cy);
	void pushScale(float sx,float sy);
	void pushTranslation(float tx,float ty);
	void pushSkewX(float angle);
	void pushSkewY(float angle);
	void pushMatrix(float a,float b,float c,float d,float e,float f);
	void setTransform(const Gdiplus::Matrix &mat);
	virtual void render(Gdiplus::Graphics&)=0;
};

class CSVGElementRect:public CSVGElementBase{
protected:
	float _x,_y;
	float _width,_height;
	float _rx,_ry;
public:
	CSVGElementRect();
	virtual ~CSVGElementRect();
	void setParams(float x,float y,float width,float height,float rx,float ry);
	void render(Gdiplus::Graphics& graphics);
};

class CSVGElementCircle:public CSVGElementBase{
protected:
	float _x,_y,_r;
public:
	CSVGElementCircle();
	virtual ~CSVGElementCircle();
	void setParams(float x,float y,float r);
	void render(Gdiplus::Graphics& graphics);
};

class CSVGElementPath:public CSVGElementBase{
protected:
	Gdiplus::GraphicsPath _path;
	Gdiplus::PointF _lastPos;
	Gdiplus::PointF _lastC2;	//last position of the bezier control point
	Gdiplus::PointF _firstPos;
	bool _bFirst;
protected:
	static float getAngle(const Gdiplus::PointF& a,const Gdiplus::PointF& b);
public:
	CSVGElementPath();
	virtual ~CSVGElementPath();
	void addString(LPCWSTR lpText,const Gdiplus::FontFamily* lpFontFamily,Gdiplus::FontStyle style,float fontSize,float x,float y);
	void lineTo(float x,float y,bool bAbsolute=false);
	void horizontalLineTo(float x,bool bAbsolute=false);
	void verticalLineTo(float y,bool bAbsolute=false);
	void moveTo(float x,float y,bool bAbsolute=false);
	void bezier(float x1,float y1,float x2,float y2,float endX,float endY,bool bAbsolute=false);
	void smoothCubicBezier(float x2,float y2,float endX,float endY,bool bAbsolute=false);
	void quadraticBezier(float x1,float y1,float endX,float endY,bool bAbsolute=false);
	void smoothQuadraticBezier(float endX,float endY,bool bAbsolute=false);
	void arcTo(float rx,float ry,float x_axis_rotation,int large_arc_flag,int sweep_flag,float x2,float y2,bool bAbsolute=false);
	void startPath();
	void closePath();
	bool isClosed()const;
	const Gdiplus::PointF& getLastPos()const;
	void render(Gdiplus::Graphics& graphics);
	void getBounds(Gdiplus::RectF& bounds)const;
};


class CSVGImage{
protected:
	std::vector<CSVGElementBase*> _elements;
	float _canvasWidth;
	float _canvasHeight;
	TCHAR* _lpLastError;
protected:
	void setLastError(LPCTSTR);
public:
	CSVGImage();
	virtual ~CSVGImage();
	void clear();
	void setCanvasSize(float width,float height);
	float getCanvasWidth()const;
	float getCanvasHeight()const;
	CSVGElementRect* addRect(float x,float y,float width,float height,float rx,float ry);
	CSVGElementCircle* addCircle(float x,float y,float r);
	CSVGElementPath* addPath();
	void render(Gdiplus::Graphics &graphics,float left,float top,float scaleX=1.0f,float scaleY=1.0f);
	LPCTSTR getLastError()const;
	HRESULT load(LPCTSTR lpszFilename,BOOL bValidate=FALSE);
};

