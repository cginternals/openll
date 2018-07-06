# OpenLL

Open Label Library is an Open Source, MIT licensed API specification for high performance label (glyphs, text, etc) rendering in 2D and 3D graphics environments. 


# Overview

1. [Terminology](#terminology)
2. [Reference Implementations](#reference-implementations)
3. [Tool Support](#tool-support)


## Terminology

This section introduces and partially extends common terminology from computer graphics and typography within the context of texture-based glyph rendering, as it is the basis of *OpenLL* and required for comprehending its API. Please note that these are not intended as general definitions but instead used to clarify their use within *OpenLL*.

### Basic Computer Graphics

<dl>
  <dt>Bitmask</dt><dd>A rectangular, high-resolution bitfield that approximates the shape of a grapheme by means of 'inside' and 'outside' bits.</dd>
  <dt>Distance Map</dt><dd>A discrete, low-resolution scalar field that specifies the minimum distance to a shape's contour approximated by a bitmask. The distance may be signed to distinguish between the inside and outside of the shape.</dd>
  <dt>Texture</dt><dd>A rectangular container storing data in a GPU friendly format for efficient processing.</dd>
  <dt>Texture Atlas</dt><dd>A large, rectangular container comprising multiple textures that can be efficiently addressed as single texture.</dd>
</dl>

For efficient label rendering *OpenLL* uses texture atlases of distance maps of graphemes. *OpenLL* further allows the computation of distance maps based on bitmasks at run-time.

### Reference Systems and Units

<dl>
  <dt>Pixel</dt><dd>Base element of a digital image in computer graphics; a physical point in a raster image and the smallest addressable element of a picture represented on the screen.</dd>
  <dt>Point</dt><dd>0-dimensional object, i.e. a position in a geometric space, used as a unit in World Space or Local Space.</dd>
  <dt>Dots per Inch</dt><dd> Measure of spatial printing or video dot density; number of individual dots that can be placed in a line within the span of 1 inch (2.54cm). Regarding screens, there are no dots but pixels, and the measure concept is thus called PPI (pixel per inch).</dd>
  <dt>Screen Space</dt><dd>defined by the screen, thus a 2-dimensional space addressable via a positive integer coordinate system.</dd>
  <dt>World Space</dt><dd> space that contains the geometric objects, thus a potentially unlimited 2- or 3-dimensional space addressable via a floating-point coordinate system.
  The reference space of each object is called Object Space, which is part of the World Space. Glyph Sequences can be placed in Object Space or World Space, this should be up to the user.</dd>
  <dt>Local Space</dt><dd> also called Typesetting Space; reference space of a Glyph Sequence. All typesetting transformations (kerning, glyph sizes, etc.) are applied within this space. Usually, it is a 2-dimensional space, as long as the baseline is not a spline that extends in 3-dimensional space.</dd>
</dl>

### Typography

<dl>
  <dt>Character</dt><dd>Abstract idea of a smallest readable component with semantic value in written language.</dd>
  <dt>Grapheme</dt><dd>Term intended to designate a unit of a writing system, parallel to phoneme and morpheme, but in practice used as a synonym for letter, diacritic, character, or sign. (taken from: https://de.wikipedia.org/wiki/Graphem#cite_note-DanielsBright1995:Glossar-11)</dd>
  <dt>Glyph</dt><dd>An elemental symbol (graphical unit) intended to represent a readable character for the purposes of written communication.
  It is defined by a sub ... ToDo</dd>
  <dt>Ligature</dt><dd>Two or more graphemes or glyphs are joined as a single glyph.</dd>
  <dt>Glyph Sequence</dt><dd>Sequence of glyphs placed along the baseline.</dd>
  <dt>Glyph Catalogue</dt><dd>ToDo</dd>
  <dt>Font Face</dt><dd>A collection of glyph-based  described by, e.g., font-size, line spacing, a glyph catalogue, as well as kerning information... ToDo</dd>
</dl>

### Typesetting
<dl>
  <dt>Kerning</dt><dd>process of adjusting the spacing between two glyphs to achieve a visually pleasing result. The kerning value depends on each glyph pair and can be positive (larger spacing) or negative (smaller spacing). As opposed to kerning, the term tracking (letter-spacing) means adjusting the spacing uniformly and independently of the affected glyphs.</dd>
  <dt>Baseline</dt><dd>Line or spine along which glyphs are placed in a glyph sequence. Descending glyphs (such as 'p') extend below the baseline.</dd>
  <dt>Advance</dt><dd>ToDo</dd>
  <dt>Anchor</dt><dd>ToDo</dd>
  <dt>Alignment</dt><dd>Setting of text flow along the maximum extent of the baseline, e.g. centered, left, right, block.</dd>
  <dt>...</dt><dd>ToDo</dd>
  <dt>Typesetting</dt><dd>composition of text; arranging glyphs according to a font</dd>
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
  <dt>Transformation</dt><dd>ToDo Additional glyph and glpyh sequence...</dd>
</dl>

### Stylization

<dl>
  <dt>ToDo</dt><dd>ToDo</dd>
</dl>

### Advanced Computer Graphics

<dl>
  <dt>Renderer</dt><dd>ToDo</dd>
  <dt>Attributed Vertex Cloud</dt><dd>ToDo</dd>
  <dt>Super Sampling</dt><dd>spatial anti-aliasing method to make jagged glyph edges appear smoother. For each output pixel, multiple samples at higher resolution are processed to calculate the average value for that pixel. The number of samples influences the quality.</dd>
  <dt>Back-face Culling</dt><dd>When activated, the back-faces are not rendered, i.e. Glyphs are not visible when the viewer would see them from behind. If culling is not active, the viewer would see mirrored glyphs.</dd>
  <dt>Texture Array</dt><dd>ToDo page... Texture Array <-> OpenGL Array Texture (mipmap)?</dd>
  <dt>Sub Texture</dt><dd>ToDo</dd>
</dl>


## Reference Implementations

openLL is currently implemented in C++ using OpenGL and TypeScript using WebGL.
The corresponding projects are [openll-cpp](https://github.com/cginternals/openll-cpp) for C++ and [webgl-operate](https://github.com/cginternals/webgl-operate) for TypeScript and web browsers. Further, [gloperate](https://github.com/cginternals/gloperate) provides an integration into a rendering system.

## Tool Support

To create font assets from font files required by openll, we suggest to use the command line tool [llassetgen](https://github.com/cginternals/openll-asset-generator).
