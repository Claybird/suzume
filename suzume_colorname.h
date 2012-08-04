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

struct COLORTABLE{
	LPCTSTR name;
	COLORREF color;
};

const COLORTABLE s_Colortable[]={
{_T("aliceblue"),		RGB(240, 248, 255)},
{_T("antiquewhite"),	RGB(250, 235, 215)},
{_T("aqua"),			RGB( 0, 255, 255)},
{_T("aquamarine"),		RGB(127, 255, 212)},
{_T("azure"),			RGB(240, 255, 255)},
{_T("beige"),			RGB(245, 245, 220)},
{_T("bisque"),			RGB(255, 228, 196)},
{_T("black"),			RGB( 0, 0, 0)},
{_T("blanchedalmond"),	RGB(255, 235, 205)},
{_T("blue"),			RGB( 0, 0, 255)},
{_T("blueviolet"),		RGB(138, 43, 226)},
{_T("brown"),			RGB(165, 42, 42)},
{_T("burlywood"),		RGB(222, 184, 135)},
{_T("cadetblue"),		RGB( 95, 158, 160)},
{_T("chartreuse"),		RGB(127, 255, 0)},
{_T("chocolate"),		RGB(210, 105, 30)},
{_T("coral"),			RGB(255, 127, 80)},
{_T("cornflowerblue"),	RGB(100, 149, 237)},
{_T("cornsilk"),		RGB(255, 248, 220)},
{_T("crimson"),			RGB(220, 20, 60)},
{_T("cyan"),			RGB( 0, 255, 255)},
{_T("darkblue"),		RGB( 0, 0, 139)},
{_T("darkcyan"),		RGB( 0, 139, 139)},
{_T("darkgoldenrod"),	RGB(184, 134, 11)},
{_T("darkgray"),		RGB(169, 169, 169)},
{_T("darkgreen"),		RGB( 0, 100, 0)},
{_T("darkgrey"),		RGB(169, 169, 169)},
{_T("darkkhaki"),		RGB(189, 183, 107)},
{_T("darkmagenta"),		RGB(139, 0, 139)},
{_T("darkolivegreen"),	RGB( 85, 107, 47)},
{_T("darkorange"),		RGB(255, 140, 0)},
{_T("darkorchid"),		RGB(153, 50, 204)},
{_T("darkred"),			RGB(139, 0, 0)},
{_T("darksalmon"),		RGB(233, 150, 122)},
{_T("darkseagreen"),	RGB(143, 188, 143)},
{_T("darkslateblue"),	RGB( 72, 61, 139)},
{_T("darkslategray"),	RGB( 47, 79, 79)},
{_T("darkslategrey"),	RGB( 47, 79, 79)},
{_T("darkturquoise"),	RGB( 0, 206, 209)},
{_T("darkviolet"),		RGB(148, 0, 211)},
{_T("deeppink"),		RGB(255, 20, 147)},
{_T("deepskyblue"),		RGB( 0, 191, 255)},
{_T("dimgray"),			RGB(105, 105, 105)},
{_T("dimgrey"),			RGB(105, 105, 105)},
{_T("dodgerblue"),		RGB( 30, 144, 255)},
{_T("firebrick"),		RGB(178, 34, 34)},
{_T("floralwhite"),		RGB(255, 250, 240)},
{_T("forestgreen"),		RGB( 34, 139, 34)},
{_T("fuchsia"),			RGB(255, 0, 255)},
{_T("gainsboro"),		RGB(220, 220, 220)},
{_T("ghostwhite"),		RGB(248, 248, 255)},
{_T("gold"),			RGB(255, 215, 0)},
{_T("goldenrod"),		RGB(218, 165, 32)},
{_T("gray"),			RGB(128, 128, 128)},
{_T("grey"),			RGB(128, 128, 128)},
{_T("green"),			RGB( 0, 128, 0)},
{_T("greenyellow"),		RGB(173, 255, 47)},
{_T("honeydew"),		RGB(240, 255, 240)},
{_T("hotpink"),			RGB(255, 105, 180)},
{_T("indianred"),		RGB(205, 92, 92)},
{_T("indigo"),			RGB( 75, 0, 130)},
{_T("ivory"),			RGB(255, 255, 240)},
{_T("khaki"),			RGB(240, 230, 140)},
{_T("lavender"),		RGB(230, 230, 250)},
{_T("lavenderblush"),	RGB(255, 240, 245)},
{_T("lawngreen"),		RGB(124, 252, 0)},
{_T("lemonchiffon"),	RGB(255, 250, 205)},
{_T("lightblue"),		RGB(173, 216, 230)},
{_T("lightcoral"),		RGB(240, 128, 128)},
{_T("lightcyan"),		RGB(224, 255, 255)},
{_T("lightgoldenrodyellow"),	RGB(250, 250, 210)},
{_T("lightgray"),		RGB(211, 211, 211)},
{_T("lightgreen"),		RGB(144, 238, 144)},
{_T("lightgrey"),		RGB(211, 211, 211)},

{_T("lightpink"),		RGB(255, 182, 193)},
{_T("lightsalmon"),		RGB(255, 160, 122)},
{_T("lightseagreen"),	RGB( 32, 178, 170)},
{_T("lightskyblue"),	RGB(135, 206, 250)},
{_T("lightslategray"),	RGB(119, 136, 153)},
{_T("lightslategrey"),	RGB(119, 136, 153)},
{_T("lightsteelblue"),	RGB(176, 196, 222)},
{_T("lightyellow"),		RGB(255, 255, 224)},
{_T("lime"),			RGB( 0, 255, 0)},
{_T("limegreen"),		RGB( 50, 205, 50)},
{_T("linen"),			RGB(250, 240, 230)},
{_T("magenta"),			RGB(255, 0, 255)},
{_T("maroon"),			RGB(128, 0, 0)},
{_T("mediumaquamarine"),RGB(102, 205, 170)},
{_T("mediumblue"),		RGB( 0, 0, 205)},
{_T("mediumorchid"),	RGB(186, 85, 211)},
{_T("mediumpurple"),	RGB(147, 112, 219)},
{_T("mediumseagreen"),	RGB( 60, 179, 113)},
{_T("mediumslateblue"),	RGB(123, 104, 238)},
{_T("mediumspringgreen"),RGB( 0, 250, 154)},
{_T("mediumturquoise"),	RGB( 72, 209, 204)},
{_T("mediumvioletred"),	RGB(199, 21, 133)},
{_T("midnightblue"),	RGB( 25, 25, 112)},
{_T("mintcream"),		RGB(245, 255, 250)},
{_T("mistyrose"),		RGB(255, 228, 225)},
{_T("moccasin"),		RGB(255, 228, 181)},
{_T("navajowhite"),		RGB(255, 222, 173)},
{_T("navy"),			RGB( 0, 0, 128)},
{_T("oldlace"),			RGB(253, 245, 230)},
{_T("olive"),			RGB(128, 128, 0)},
{_T("olivedrab"),		RGB(107, 142, 35)},
{_T("orange"),			RGB(255, 165, 0)},
{_T("orangered"),		RGB(255, 69, 0)},
{_T("orchid"),			RGB(218, 112, 214)},
{_T("palegoldenrod"),	RGB(238, 232, 170)},
{_T("palegreen"),		RGB(152, 251, 152)},
{_T("paleturquoise"),	RGB(175, 238, 238)},
{_T("palevioletred"),	RGB(219, 112, 147)},
{_T("papayawhip"),		RGB(255, 239, 213)},
{_T("peachpuff"),		RGB(255, 218, 185)},
{_T("peru"),			RGB(205, 133, 63)},
{_T("pink"),			RGB(255, 192, 203)},
{_T("plum"),			RGB(221, 160, 221)},
{_T("powderblue"),		RGB(176, 224, 230)},
{_T("purple"),			RGB(128, 0, 128)},
{_T("red"),				RGB(255, 0, 0)},
{_T("rosybrown"),		RGB(188, 143, 143)},
{_T("royalblue"),		RGB( 65, 105, 225)},
{_T("saddlebrown"),		RGB(139, 69, 19)},
{_T("salmon"),			RGB(250, 128, 114)},
{_T("sandybrown"),		RGB(244, 164, 96)},
{_T("seagreen"),		RGB( 46, 139, 87)},
{_T("seashell"),		RGB(255, 245, 238)},
{_T("sienna"),			RGB(160, 82, 45)},
{_T("silver"),			RGB(192, 192, 192)},
{_T("skyblue"),			RGB(135, 206, 235)},
{_T("slateblue"),		RGB(106, 90, 205)},
{_T("slategray"),		RGB(112, 128, 144)},
{_T("slategrey"),		RGB(112, 128, 144)},
{_T("snow"),			RGB(255, 250, 250)},
{_T("springgreen"),		RGB( 0, 255, 127)},
{_T("steelblue"),		RGB( 70, 130, 180)},
{_T("tan"),				RGB(210, 180, 140)},
{_T("teal"),			RGB( 0, 128, 128)},
{_T("thistle"),			RGB(216, 191, 216)},
{_T("tomato"),			RGB(255, 99, 71)},
{_T("turquoise"),		RGB( 64, 224, 208)},
{_T("violet"),			RGB(238, 130, 238)},
{_T("wheat"),			RGB(245, 222, 179)},
{_T("white"),			RGB(255, 255, 255)},
{_T("whitesmoke"),		RGB(245, 245, 245)},
{_T("yellow"),			RGB(255, 255, 0)},
{_T("yellowgreen"),		RGB(154, 205, 50)},
};

};
