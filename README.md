# OpenLL

Open Label Library is an opensource, MIT licensed API specification for high performance label (glyphs, text, etc) rendering in 2D and 3D graphics environments. In addition there will be reference implementations for WebGL, Vulkan, and OpenGL.

## Project Health

| Service | System | Compiler | Status |
| ------- | ------ | -------- | -----: |
|  [Travis-CI](https://travis-ci.org/cginternals/openll) | Ubuntu 14.04 | GCC 4.8, Clang 3.5 | [![Build Status](https://travis-ci.org/cginternals/openll.svg?branch=master)](https://travis-ci.org/cginternals/openll) |
|  [Travis-CI](https://travis-ci.org/cginternals/openll) | OS X | Clang ? | upcoming |
| [Coverity](https://scan.coverity.com/projects/6829?tab=overview) | Ubuntu 14.04 | GCC 5.4 | [![Coverity Status](https://scan.coverity.com/projects/6829/badge.svg)](https://scan.coverity.com/projects/6829) |
| Jenkins <br><br><br><br> | Ubuntu 14.04 <br><br><br><br> | GCC 4.8 <br> GCC 4.9 <br> GCC 5.4 <br> Clang 3.8 <br> | [![Build Status](https://jenkins.hpi3d.de/buildStatus/icon?job=openll-linux-gcc4.8)](https://jenkins.hpi3d.de/job/openll-linux-gcc4.8) <br> [![Build Status](https://jenkins.hpi3d.de/buildStatus/icon?job=openll-linux-gcc4.9)](https://jenkins.hpi3d.de/job/openll-linux-gcc4.9) <br> [![Build Status](https://jenkins.hpi3d.de/buildStatus/icon?job=openll-linux-gcc5)](https://jenkins.hpi3d.de/job/openll-linux-gcc5) <br> [![Build Status](https://jenkins.hpi3d.de/buildStatus/icon?job=openll-linux-clang3.8)](https://jenkins.hpi3d.de/job/openll-linux-clang3.8) <br> |
| Jenkins <br><br> | Windows 10 <br><br> | MSVC 2013 Update 5 <br>  MSVC 2015 Update 1 <br> | [![Build Status](https://jenkins.hpi3d.de/buildStatus/icon?job=openll-windows-msvc2013)](https://jenkins.hpi3d.de/job/openll-windows-msvc2013) <br> [![Build Status](https://jenkins.hpi3d.de/buildStatus/icon?job=openll-windows-msvc2015)](https://jenkins.hpi3d.de/job/openll-windows-msvc2015) <br> |


## Terminology

This section introduces and partially extends common terminology from typography within the context of texture-based glyph rendering, as it is the basis of *OpenLL* and required for comprehending its API.

### Basic Computer Graphics

<dl>
  <dt>Bitmap</dt><dd>ToDo</dd>
  <dt>Bitmask</dt><dd>ToDo</dd>
  <dt>Distance Field</dt><dd>ToDo</dd>
  <dt>Texture</dt><dd>ToDo</dd>
  <dt>Texture Atlas</dt><dd>ToDo</dd>
</dl>

### Reference Systems and Units

<dl>
  <dt>Pixel</dt><dd>ToDo</dd>
  <dt>Point</dt><dd>ToDo</dd>
  <dt>Dots per Inch</dt><dd>ToDo</dd>
  <dt>Screen Space</dt><dd>ToDo</dd>
  <dt>World Space</dt><dd>ToDo also refer to Object Space here</dd>
  <dt>Local Space</dt><dd>ToDo Typesetting Space</dd>
</dl>

### Typography

<dl>
  <dt>Grapheme</dt><dd>ToDo</dd>
  <dt>Glyph</dt><dd>An elemental symbol intended to represent a readable character for the purposes of written communication.
  It is defined by a sub ... ToDo</dd>
  <dt>Ligature</dt><dd>ToDo</dd>
  <dt>Glyph Sequence</dt><dd>ToDo</dd>
  <dt>Glyph Catalogue</dt><dd>ToDo</dd>
  <dt>Font Face</dt><dd>A collection of glyph-based  described by, e.g., font-size, line spacing, a glyph catalogue, as well as kerning information... ToDo</dd>
</dl>

### Typesetting
<dl>
  <dt>Kerning</dt><dd>ToDo</dd>
  <dt>Base Line</dt><dd>ToDo</dd>
  <dt>Advance</dt><dd>ToDo</dd>
  <dt>Anchor</dt><dd>ToDo</dd>
  <dt>Alignment</dt><dd>ToDo</dd>
  <dt>...</dt><dd>ToDo</dd>
  <dt>Typesetting</dt><dd>ToDo</dd>
</dl>

### Positioning and Layouting

<dl>
  <dt>Point</dt><dd>ToDo 2D and 3D</dd>
  <dt>Line</dt><dd>ToDo 2D and 3D</dd>
  <dt>Spline</dt><dd>ToDo 2D and 3D</dd>
  <dt>Surface</dt><dd>ToDo 2D and 3D</dd>
  <dt>Visibility</dt><dd>ToDo</dd>
  <dt>Overlap</dt><dd>ToDo</dd>
  <dt>Precedence</dt><dd>ToDo</dd>
</dl>

### Stylization

<dl>
  <dt>ToDo</dt><dd>ToDo</dd>
</dl>

### Advanced Computer Graphics

<dl>
  <dt>Renderer</dt><dd>ToDo</dd>
  <dt>Attributed Vertex Cloud</dt><dd>ToDo</dd>
  <dt>Super Sampling</dt><dd>ToDo</dd>
  <dt>Back-face Culling</dt><dd>ToDo</dd>
  <dt>Texture Array</dt><dd>ToDo page...</dd>
  <dt>Sub Texture</dt><dd>ToDo</dd>
</dl>
