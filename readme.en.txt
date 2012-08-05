+--------------------------+
|  The Suzume SVG Library  |
+--------------------------+

[What is this?]
This is a tiny SVG loader for Windows.
It's written in C++, and depends on few other libraries.

The library is released under the zlib license.

This library depends on GDI+ and MSXML(+ATL).
It means that most Windows developpers do not need any additional library.


[Known Issues]
This version does not support the full SVG features.
The following is a part of unsupported features.
 - CSS
 - gradation
 - relative size specification (%)
 - viewbox
 - pattern
 - text alignment along path
 - embedded image
 - path caps


[History]
- Ver.0.02 Aug. 5, 2012
  fixed: lost return in CSVGImage::load()
  added: location and scaling parameter in CSVGImage::render()
- Ver.0.01 Aug. 4, 2012
  Initial Release

