+--------------------------+
|  The Suzume SVG Library  |
+--------------------------+

[概要]
Windows用の小さなSVG読み込み/表示ライブラリです。
C++で書かれており、また他のライブラリへの依存が少ないのが特徴です。

zlibライセンスで公開しています。

このライブラリは、他のライブラリへの依存をなるべく減らすことを念頭に開発しています。
現在のところ、GDI+とMSXML(+ATL)のみに依存しているため、ほとんどのWindows開発者は、
Suzume以外にライブラリを用意する必要がありません。


[既知の問題]
このバージョンではSVG規格に完全には対応していません。
以下は対応していない機能の一部です:
 - CSS
 - gradation
 - relative size specification (%)
 - viewbox
 - pattern
 - text alignment along path
 - embedded image
 - path caps


[開発履歴]
- Ver.0.02 Aug. 5, 2012
  CSVGImage::load()のreturn忘れを修正
  CSVGImage::render()に位置指定と拡大率指定を追加
- Ver.0.01 Aug. 4, 2012
  初版

