# OpenLL

Open Label Library is an Open Source, MIT licensed API specification for high performance label (glyphs, text, etc) rendering in 2D and 3D graphics environments.

Until first approval, a [language-agnostic API](https://github.com/scheibel/openll-api) is externally maintained.

OpenLL was initially published on the [22nd International Conference on Information Visualisation](http://www.graphicslink.co.uk/) (iV 2018).

* Daniel Limberger, Anne Gropler, Stefan Buschmann, Jürgen Döllner, and Benjamin Wasty: "OpenLL: an API for Dynamic 2D and 3D Labeling". Proceedings of the International Conference on Information Visualization 2018.


## Overview

1. [Reference Implementations](#reference-implementations)
2. [Tool Support](#tool-support)
3. [Terminology](#terminology)
4. [Usage](#usage)
5. [Contributing](#contributing)


## Reference Implementations

Reference implementations of OpenLL are currently implemented in C++ using OpenGL and TypeScript using WebGL.
The corresponding projects are [openll-cpp](https://github.com/cginternals/openll-cpp) for C++ and [webgl-operate](https://github.com/cginternals/webgl-operate) for TypeScript and web browsers. Further, [gloperate](https://github.com/cginternals/gloperate) provides an integration into a rendering system.


## Tool Support

To create font assets from font files required by OpenLL, we suggest to use the command line tool [llassetgen](https://github.com/cginternals/openll-asset-generator).



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
  <dt>Glyph</dt><dd>An elemental symbol (graphical unit) intended to represent a readable character for the purposes of written communication. If a glyph should be renderable, a distance field representation has to be present in the font face texture atlas.</dd>
  <dt>Ligature</dt><dd>Two or more graphemes or glyphs are joined as a single glyph. A font face can declare combinations of glyphs that should be rendered using a different distance transform representation.</dd>
  <dt>Glyph Sequence</dt><dd>Sequence of glyphs placed along the baseline.</dd>
  <dt>Font Face</dt><dd>A collection of glyphs that form a coherent composition. It is typically a subset of the Unicode character set. Discerning a font family, a font face has fixed font size, font weight, line spacing, a glyph catalogue, as well as kerning information.</dd>
</dl>

### Typesetting
<dl>
  <dt>Typesetting</dt><dd>The process of text composition. This takes a glyph sequence, the associated font face, the baseline, alignment and all advance and kerning information into account. Additionally, multi-line typesetting is supported.</dd>
  <dt>Kerning</dt><dd>process of adjusting the spacing between two glyphs to achieve a visually pleasing result. The kerning value depends on each glyph pair and can be positive (larger spacing) or negative (smaller spacing). As opposed to kerning, the term tracking (letter-spacing) means adjusting the spacing uniformly and independently of the affected glyphs.</dd>
  <dt>Baseline</dt><dd>Line or spline along which glyphs are placed in a glyph sequence. Descending glyphs (such as 'p') extend below the baseline.</dd>
  <dt>Advance</dt><dd>The space along the baseline that is advanced before a next glyph is placed.</dd>
  <dt>Anchor</dt><dd>The reference anchor in typesetting space that is placed onto the reference point in view or world coordinates.</dd>
  <dt>Alignment</dt><dd>Setting of text flow along the maximum extent of the baseline, e.g. centered, left, right, block.</dd>
</dl>

### Advanced Computer Graphics

<dl>
  <dt>Renderer</dt><dd>A software system that provides (hardware-accelerated) functionality to synthesize an image containing the configured OpenLL labels. Renderer are vendor-specific but are intended to be implemented using rendering APIs such as OpenGL, Vulkan, OpenGL ES, WebGL, or Metal. Although unendorsed, software renderer are possible, too.</dd>
  <dt>Attributed Vertex Cloud</dt><dd>A specialized geometry encoding that relies on the programmable rendering pipeline of modern graphics hardware to instantiate geometry templates. For OpenLL, rectangles are used as templates for the glyph rendering.</dd>
  <dt>Super Sampling</dt><dd>spatial anti-aliasing method to make jagged glyph edges appear smoother. For each output pixel, multiple samples at higher resolution are processed to calculate the average value for that pixel. The number of samples influences the quality.</dd>
  <dt>Back-face Culling</dt><dd>When activated, the back-faces are not rendered, i.e., glyphs are not visible when the viewer would see them from behind. If culling is not active, the viewer would see mirrored glyphs.</dd>
  <dt>Texture Array</dt><dd>Multiple textures that are referenced using a single entry point and qualified using a texture index.</dd>
  <dt>Sub Texture</dt><dd>A virtual texture within a texture. A basic concept to use texture atlases.</dd>
</dl>


## Usage

If you want to use OpenLL within your project, feel free to [contact us](contact@cginternals.com).
For a quick-start, refer to our examples using OpenLL:

* [C++](https://github.com/cginternals/openll-cpp/tree/master/source/examples/openll-example)
* [TypeScript](https://github.com/cginternals/webgl-operate/pull/73)
* [Multi-configuration Labeling](https://github.com/cginternals/openll/tree/master/ll-opengl/source/examples/labeling-at-point)
* [Importance-based Labeling](https://github.com/cginternals/openll/tree/master/ll-opengl/source/examples/pointbasedlayouting)


## Contributing

OpenLL is a community-driven API and we rely on community effort and feedback to provide a versatile and usable API. The following tasks are highly appreciated:

* [Submit issue](https://github.com/cginternals/openll/issues/new) concerning the API
* [Submit issue](https://github.com/cginternals/openll-cpp/issues/new) concerning the C++ reference implementation
* [Submit issue](https://github.com/cginternals/webgl-operate/issues/new) concerning the Typescript reference implementation
* [Submit API extension](https://github.com/scheibel/openll-api/issues/new) to the actual API
* Provide additional documentation, tutorials, and using projects
