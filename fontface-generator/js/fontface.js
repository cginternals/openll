---
layout: compress
---


var font = null;

function showErrorMessage(message) {
    var el = document.getElementById('message');
    if (!message || message.trim().length === 0) {
        el.style.display = 'none';
    } else {
        el.style.display = 'block';
    }
    el.innerHTML = message;
}

function escapeHtml(unsafe) {
    return unsafe
         .replace(/&/g, '&amp;')
         .replace(/</g, '&lt;')
         .replace(/>/g, '&gt;')
         .replace(/\u0022/g, '&quot;')
         .replace(/\u0027/g, '&#039;');
}

function sortKeys(dict) {
    var keys = [];
    for (var key in dict) {
        keys.push(key);
    }
    keys.sort();
    return keys;
}

function displayNames(names) {
    var html = '';
    properties = sortKeys(names);
    for (var i = 0; i < properties.length; i++) {
        var property = properties[i];
        html += '<dt>'+escapeHtml(property)+'</dt><dd>';
        var translations = names[property];
        var langs = sortKeys(translations);
        for (var j = 0; j < langs.length; j++) {
            var lang = langs[j];
            var esclang = escapeHtml(lang);
            html += '<span class="langtag">' + esclang
                + '</span> <span class="langname" lang=' + esclang + '>'
                + escapeHtml(translations[lang]) + '</span> ';
        }
        html += '</dd>';
    }

    document.getElementById('name-table').innerHTML = html;
}

function displayFontData() {
    var html, tablename, table, property, value;
    for (tablename in font.tables) {
        table = font.tables[tablename];
        if (tablename == 'name') {
            displayNames(table);
            continue;
        }

        html = '';
        for (property in table) {
            value = table[property];
            html += '<dt>'+property+'</dt><dd>';
            if (Array.isArray(value) && typeof value[0] === 'object') {
                html += value.map(function(item) {
                    return JSON.stringify(item);
                }).join('<br>');
            } else if (typeof value === 'object') {
              html += JSON.stringify(value);
            } else if (['created', 'modified'].indexOf(property) > -1) {
                var date = new Date(value * 1000);
                html += date;
            }
            else {
                html += value;
            }
            html += '</dd>';
        }
        var element = document.getElementById(tablename+"-table");
        if (element) {
            element.innerHTML = html;
        }
    }
}


function getWordInformation(text) {
    var fontSize = 18;
    var ascent = 0;
    var descent = 0;
    var width = 0;
    var scale = 1 / font.unitsPerEm * fontSize;
    var glyphs = font.stringToGlyphs(text);

    for (var i = 0; i < glyphs.length; i++) {
        var glyph = glyphs[i];
        if (glyph.advanceWidth) {
            width += glyph.advanceWidth * scale;
        }
        if (i < glyphs.length - 1) {
            kerningValue = font.getKerningValue(glyph, glyphs[i + 1]);
            width += kerningValue * scale;
        }
        ascent = Math.max(ascent, glyph.yMax);
        descent = Math.min(descent, glyph.yMin);
    }

    return {
        width: width,
        actualBoundingBoxAscent: ascent * scale,
        actualBoundingBoxDescent: descent * scale,
        fontBoundingBoxAscent: font.ascender * scale,
        fontBoundingBoxDescent: font.descender * scale
    };
};


function onFontLoaded(font) {
    var glyphsDiv, i, x, y, fontSize;
    window.font = font;

    // Show the first 100 glyphs.
    //glyphsDiv = document.getElementById('glyphs');
    //glyphsDiv.innerHTML = '';

    amount = Math.min(100, font.glyphs.length);
    x = 50;
    y = 120;
    fontSize = 72;
    /*for (i = 0; i < amount; i++) {
        glyph = font.glyphs.get(i);
        ctx = createGlyphCanvas(glyph, 150);
        glyph.draw(ctx, x, y, fontSize);
        glyph.drawPoints(ctx, x, y, fontSize);
        glyph.drawMetrics(ctx, x, y, fontSize);
    }*/

    //renderText();


    // display font information
    var fontInfo = '';
    for(var key in font.names){
        fontInfo += '<b>' + key + '</b>: ' + font.names[key]['en'] + ' <br>';
    }

    fontInfo += '\n<b>unitsPerEm: </b>' + font.unitsPerEm + '\n'
             + '<b>ascender: </b>' + font.ascender + '\n'
             + '<b>descender: </b>' + font.descender + '\n';


    document.getElementById('font-info').innerHTML = fontInfo;

    displayFontData();
}


function onReadFile(e) {
	console.log("read file...");
    //document.getElementById('font-name').innerHTML = '';
    var file = e.target.files[0];
    var reader = new FileReader();
    reader.onload = function(e) {
        try {
            font = opentype.parse(e.target.result);
            onFontLoaded(font);
            //showErrorMessage('');
        } catch (err) {
            //showErrorMessage(err.toString());
            console.error(err);
        }
    }
    reader.onerror = function(err) {
        //showErrorMessage(err.toString());
        console.error(err);
    }

    reader.readAsArrayBuffer(file);
}

//var fontFileName = 'fonts/Roboto-Black.ttf';
var fontFileName = '/ ';

//document.getElementById('font-name').innerHTML = fontFileName.split('/')[1];

var fileButton = document.getElementById('file');
fileButton.addEventListener('change', onReadFile, false);

var tableHeaders = document.getElementById('font-data').getElementsByTagName('h3');
for(var i = tableHeaders.length; i--; ) {
    tableHeaders[i].addEventListener('click', function(e) {
        e.target.className = (e.target.className === 'collapsed') ? 'expanded' : 'collapsed';
    }, false);
}
