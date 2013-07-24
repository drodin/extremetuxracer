/* --------------------------------------------------------------------
Extreme Tuxracer

Copyright (c) 2001-2004 Henry Maddocks (FTGL)
Copyright (C) 2010 Extreme Tuxracer Team (modification)

The FTGL library from H. Maddocks is published under the terms
of the Lesser General Publish License (LGPL). You can find a
copy of the LGPL on the gnu website:
      http://www.gnu.org/copyleft/lesser.html

Hint: almost all comments are removed from the code to make it
shorter. So all modules could put together in a single module.
To read the comments of the author you should download the
original FTGL library. Most functions are the same as in this
module.
--------------------------------------------------------------------- */

#ifdef HAVE_CONFIG_H
#include <etr_config.h>
#endif

#include "ft_font.h"

// --------------------------------------------------------------------
//					FTFont
// --------------------------------------------------------------------

FTFont::FTFont (const char* fontFilePath)
: face (fontFilePath), glyphList(0) {
    err = face.Error();
    if (err == 0) glyphList = new FTGlyphContainer (&face);
}

FTFont::FTFont (const unsigned char *pBufferBytes, size_t bufferSizeInBytes)
: face (pBufferBytes, bufferSizeInBytes), glyphList(0) {
    err = face.Error();
    if (err == 0) glyphList = new FTGlyphContainer (&face);
}

FTFont::~FTFont() {delete glyphList;}

bool FTFont::Attach (const char* fontFilePath) {
    if (face.Attach (fontFilePath)) {
        err = 0;
        return true;
    } else {
        err = face.Error();
        return false;
    }
}

bool FTFont::Attach (const unsigned char *pBufferBytes, size_t bufferSizeInBytes) {
    if (face.Attach (pBufferBytes, bufferSizeInBytes)) {
        err = 0;
        return true;
    } else {
        err = face.Error();
        return false;
    }
}

bool FTFont::FaceSize (const unsigned int size, const unsigned int res) {
    charSize = face.Size (size, res);
    err = face.Error();

    if (err != 0) return false;
    if (glyphList != NULL) delete glyphList;
    glyphList = new FTGlyphContainer (&face);
    return true;
}

unsigned int FTFont::FaceSize() const {return charSize.CharSize();}

bool FTFont::CharMap (FT_Encoding encoding) {
    bool result = glyphList->CharMap (encoding);
    err = glyphList->Error();
    return result;
}

unsigned int FTFont::CharMapCount() {return face.CharMapCount();}
FT_Encoding *FTFont::CharMapList() {return face.CharMapList();}
float FTFont::Ascender() const {return charSize.Ascender();}
float FTFont::Descender() const {return charSize.Descender();}
float FTFont::LineHeight() const {return charSize.Height();}

void FTFont::BBox (const char* string, float& llx, float& lly, float& llz,
		float& urx, float& ury, float& urz) {
    FTBBox totalBBox;

    if((NULL != string) && ('\0' != *string)) {
        const unsigned char* c = (unsigned char*)string;
        float advance = 0;

        if(CheckGlyph (*c)) {
            totalBBox = glyphList->BBox (*c);
            advance = glyphList->Advance (*c, *(c + 1));
        }

        while (*++c) {
            if(CheckGlyph (*c)) {
                FTBBox tempBBox = glyphList->BBox (*c);
                tempBBox.Move (FTPoint (advance, 0.0f, 0.0f));
                totalBBox += tempBBox;
                advance += glyphList->Advance (*c, *(c + 1));
            }
        }
    }

    llx = totalBBox.lowerX;
    lly = totalBBox.lowerY;
    llz = totalBBox.lowerZ;
    urx = totalBBox.upperX;
    ury = totalBBox.upperY;
    urz = totalBBox.upperZ;
}

void FTFont::BBox (const wchar_t* string, float& llx, float& lly, float& llz,
		float& urx, float& ury, float& urz) {
    FTBBox totalBBox;

    if((NULL != string) && ('\0' != *string)) {
        const wchar_t* c = string;
        float advance = 0;

        if(CheckGlyph (*c)) {
            totalBBox = glyphList->BBox (*c);
            advance = glyphList->Advance (*c, *(c + 1));
        }

        while (*++c) {
            if(CheckGlyph (*c)) {
                FTBBox tempBBox = glyphList->BBox (*c);
                tempBBox.Move (FTPoint (advance, 0.0f, 0.0f));
                totalBBox += tempBBox;
                advance += glyphList->Advance (*c, *(c + 1));
            }
        }
    }
    llx = totalBBox.lowerX;
    lly = totalBBox.lowerY;
    llz = totalBBox.lowerZ;
    urx = totalBBox.upperX;
    ury = totalBBox.upperY;
    urz = totalBBox.upperZ;
}


float FTFont::Advance (const wchar_t* string) {
    const wchar_t* c = string;
    float width = 0.0f;

    while (*c) {
        if(CheckGlyph (*c)) width += glyphList->Advance (*c, *(c + 1));
        ++c;
    }
    return width;
}

float FTFont::Advance (const char* string) {
    const unsigned char* c = (unsigned char*)string;
    float width = 0.0f;

    while (*c) {
        if(CheckGlyph (*c)) width += glyphList->Advance (*c, *(c + 1));
        ++c;
    }
    return width;
}

void FTFont::Render (const char* string) {
    const unsigned char* c = (unsigned char*)string;
    pen.X(0); pen.Y(0);

    while (*c) {
        if(CheckGlyph (*c)) pen = glyphList->Render (*c, *(c + 1), pen);
        ++c;
    }
}

void FTFont::Render (const wchar_t* string) {
    const wchar_t* c = string;
    pen.X(0); pen.Y(0);

    while (*c) {
        if(CheckGlyph (*c)) pen = glyphList->Render (*c, *(c + 1), pen);
        ++c;
    }
}

bool FTFont::CheckGlyph (const unsigned int characterCode) {
    if (NULL == glyphList->Glyph (characterCode)) {
        unsigned int glyphIndex = glyphList->FontIndex (characterCode);
        FTGlyph* tempGlyph = MakeGlyph (glyphIndex);
        if (NULL == tempGlyph) {
            if (0 == err) err = 0x13;
            return false;
        }
        glyphList->Add (tempGlyph, characterCode);
    }
    return true;
}

// --------------------------------------------------------------------
//					FTLibrary
// --------------------------------------------------------------------

const FTLibrary &FTLibrary::Instance () {
    static FTLibrary ftlib;
    return ftlib;
}

FTLibrary::~FTLibrary () {
//	PrintStr ("desctructor FTLibrary");
	if (library != 0) {
        FT_Done_FreeType (*library);
        delete library;
        library= 0;
    }
}

FTLibrary::FTLibrary () :  library(0), err(0) {Initialise();}

bool FTLibrary::Initialise () {
    if (library != 0) return true;
    library = new FT_Library;
    err = FT_Init_FreeType (library);
    if (err) {
        delete library;
        library = 0;
        return false;
    }
    return true;
}

// --------------------------------------------------------------------
//					FTFace
// --------------------------------------------------------------------

FTFace::FTFace (const char* fontFilePath)
: numGlyphs(0), fontEncodingList(0), err(0) {
    const FT_Long DEFAULT_FACE_INDEX = 0;
    ftFace = new FT_Face;

    err = FT_New_Face (*FTLibrary::Instance().GetLibrary(), fontFilePath, DEFAULT_FACE_INDEX, ftFace);
    if (err) {
		Message ("error FT_New_Face");
		delete ftFace;
        ftFace = 0;
    } else {
		numGlyphs = (*ftFace)->num_glyphs;
        hasKerningTable = FT_HAS_KERNING((*ftFace));
    }
}

FTFace::FTFace (const unsigned char *pBufferBytes, size_t bufferSizeInBytes)
: numGlyphs(0), err(0) {
    const FT_Long DEFAULT_FACE_INDEX = 0;
    ftFace = new FT_Face;

    err = FT_New_Memory_Face (
			*FTLibrary::Instance().GetLibrary(),
			(FT_Byte *)pBufferBytes,
			bufferSizeInBytes, DEFAULT_FACE_INDEX, ftFace);

    if (err) {
        delete ftFace;
        ftFace = 0;
    } else numGlyphs = (*ftFace)->num_glyphs;
}

FTFace::~FTFace() {
	if (ftFace) {
		FT_Done_Face (*ftFace);
        delete ftFace;
        ftFace = 0;
    }
}

bool FTFace::Attach (const char* fontFilePath) {
    err = FT_Attach_File (*ftFace, fontFilePath);
    return !err;
}

bool FTFace::Attach (const unsigned char *pBufferBytes, size_t bufferSizeInBytes) {
    FT_Open_Args open;

    open.flags = FT_OPEN_MEMORY;
    open.memory_base = (FT_Byte *)pBufferBytes;
    open.memory_size = bufferSizeInBytes;

    err = FT_Attach_Stream (*ftFace, &open);
    return !err;
}

const FTSize& FTFace::Size (const unsigned int size, const unsigned int res) {
    charSize.CharSize (ftFace, size, res, res);
    err = charSize.Error();
    return charSize;
}

unsigned int FTFace::CharMapCount() {
    return (*ftFace)->num_charmaps;
}

FT_Encoding* FTFace::CharMapList() {
    if (0 == fontEncodingList) {
        fontEncodingList = new FT_Encoding[CharMapCount()];
        for (size_t encodingIndex = 0; encodingIndex < CharMapCount(); ++encodingIndex) {
            fontEncodingList[encodingIndex] = (*ftFace)->charmaps[encodingIndex]->encoding;
        }
    }
    return fontEncodingList;
}

FTPoint FTFace::KernAdvance (unsigned int index1, unsigned int index2) {
    float x, y;
    x = y = 0.0f;

    if (hasKerningTable && index1 && index2) {
        FT_Vector kernAdvance;
        kernAdvance.x = kernAdvance.y = 0;

        err = FT_Get_Kerning (*ftFace, index1, index2, ft_kerning_unfitted, &kernAdvance);
        if (!err) {
            x = static_cast<float> (kernAdvance.x) / 64.0f;
            y = static_cast<float> (kernAdvance.y) / 64.0f;
        }
    }
    return FTPoint (x, y, 0.0);
}

FT_GlyphSlot FTFace::Glyph (unsigned int index, FT_Int load_flags) {
    err = FT_Load_Glyph (*ftFace, index, load_flags);
    if (err) return NULL;
    return (*ftFace)->glyph;
}

// --------------------------------------------------------------------
//					FTPoint
// --------------------------------------------------------------------

bool operator == (const FTPoint &a, const FTPoint &b) {
    return((a.values[0] == b.values[0]) && (a.values[1] ==
	b.values[1]) && (a.values[2] == b.values[2]));
}

bool operator !=  (const FTPoint &a, const FTPoint &b) {
    return((a.values[0] != b.values[0]) || (a.values[1]
	!= b.values[1]) || (a.values[2] != b.values[2]));
}

FTPoint operator * (double multiplier, FTPoint& point) {
    return point * multiplier;
}

// --------------------------------------------------------------------
//					FTSize
// --------------------------------------------------------------------

FTSize::FTSize()
:   ftFace(0),
    ftSize(0),
    size(0),
    xResolution(0),
    yResolution(0),
    err(0)
{}

FTSize::~FTSize() {}

bool FTSize::CharSize (FT_Face* face, unsigned int pointSize,
		unsigned int xRes, unsigned int yRes) {
    if (size != pointSize || xResolution != xRes || yResolution != yRes) {
        err = FT_Set_Char_Size (*face, 0L, pointSize * 64, xResolution, yResolution);

        if (!err) {
            ftFace = face;
            size = pointSize;
            xResolution = xRes;
            yResolution = yRes;
            ftSize = (*ftFace)->size;
        } else {
            ftFace = 0;
            size = 0;
            xResolution = 0;
            yResolution = 0;
            ftSize = 0;
        }
    }
    return !err;
}

unsigned int FTSize::CharSize() const {
    return size;
}

float FTSize::Ascender() const {
    return ftSize == 0 ? 0.0f : static_cast<float> (ftSize->metrics.ascender) / 64.0f;
}

float FTSize::Descender() const {
    return ftSize == 0 ? 0.0f : static_cast<float> (ftSize->metrics.descender) / 64.0f;
}

float FTSize::Height() const {
    if (0 == ftSize) return 0.0f;
    if (FT_IS_SCALABLE((*ftFace))) {
        return  ((*ftFace)->bbox.yMax - (*ftFace)->bbox.yMin) *
		((float)ftSize->metrics.y_ppem / (float)(*ftFace)->units_per_EM);
    } else return static_cast<float> (ftSize->metrics.height) / 64.0f;
}

float FTSize::Width() const {
    if (0 == ftSize) return 0.0f;

    if (FT_IS_SCALABLE((*ftFace))) {
        return  ((*ftFace)->bbox.xMax - (*ftFace)->bbox.xMin) *
		(static_cast<float>(ftSize->metrics.x_ppem) /
		static_cast<float>((*ftFace)->units_per_EM));
    } else return static_cast<float> (ftSize->metrics.max_advance) / 64.0f;
}

float FTSize::Underline() const {return 0.0f;}

// --------------------------------------------------------------------
//					FTGlyph
// --------------------------------------------------------------------

FTGlyph::FTGlyph (FT_GlyphSlot glyph)
:   err(0) {
    if (glyph) {
        bBox = FTBBox (glyph);
        advance = FTPoint (glyph->advance.x / 64.0f, glyph->advance.y / 64.0f, 0.0f);
    }
}

FTGlyph::~FTGlyph() {}

FTGlyphContainer::FTGlyphContainer (FTFace* f)
:   face(f), err(0) {
    glyphs.push_back (NULL);
    charMap = new FTCharmap (face);
}

// --------------------------------------------------------------------
//					FTGlyphContainer
// --------------------------------------------------------------------

FTGlyphContainer::~FTGlyphContainer() {
    GlyphVector::iterator glyphIterator;
    for (glyphIterator = glyphs.begin(); glyphIterator != glyphs.end(); ++glyphIterator) {
        delete *glyphIterator;
    }
    glyphs.clear();
    delete charMap;
}

bool FTGlyphContainer::CharMap (FT_Encoding encoding) {
    bool result = charMap->CharMap (encoding);
    err = charMap->Error();
    return result;
}

unsigned int FTGlyphContainer::FontIndex (const unsigned int characterCode) const {
    return charMap->FontIndex (characterCode);
}

void FTGlyphContainer::Add (FTGlyph* tempGlyph, const unsigned int characterCode) {
    charMap->InsertIndex (characterCode, glyphs.size());
    glyphs.push_back (tempGlyph);
}

const FTGlyph* const FTGlyphContainer::Glyph (const unsigned int characterCode) const {
    signed int index = charMap->GlyphListIndex (characterCode);
    return glyphs[index];
}

FTBBox FTGlyphContainer::BBox (const unsigned int characterCode) const {
    return glyphs[charMap->GlyphListIndex (characterCode)]->BBox();
}

float FTGlyphContainer::Advance (const unsigned int characterCode,
		const unsigned int nextCharacterCode) {
    unsigned int left = charMap->FontIndex (characterCode);
    unsigned int right = charMap->FontIndex (nextCharacterCode);

    float width = face->KernAdvance (left, right).X();
    width += glyphs[charMap->GlyphListIndex (characterCode)]->Advance().X();

    return width;
}

FTPoint FTGlyphContainer::Render (const unsigned int characterCode,
		const unsigned int nextCharacterCode, FTPoint penPosition) {
    FTPoint kernAdvance, advance;

    unsigned int left = charMap->FontIndex (characterCode);
    unsigned int right = charMap->FontIndex (nextCharacterCode);

    kernAdvance = face->KernAdvance (left, right);
    if (!face->Error()) {
        advance = glyphs[charMap->GlyphListIndex (characterCode)]->Render (penPosition);
    }

    kernAdvance += advance;
    return kernAdvance;
}

// --------------------------------------------------------------------
//					FTTextureGlyph
// --------------------------------------------------------------------

GLint FTTextureGlyph::activeTextureID = 0;

FTTextureGlyph::FTTextureGlyph (FT_GlyphSlot glyph, int id, int xOffset,
		int yOffset, GLsizei width, GLsizei height)
: FTGlyph (glyph), destWidth(0), destHeight(0), glTextureID(id) {
    err = FT_Render_Glyph (glyph, FT_RENDER_MODE_NORMAL);
    if (err || glyph->format != ft_glyph_format_bitmap) return;

    FT_Bitmap bitmap = glyph->bitmap;
    destWidth  = bitmap.width;
    destHeight = bitmap.rows;

    if (destWidth && destHeight) {
        glPushClientAttrib (GL_CLIENT_PIXEL_STORE_BIT);
        glPixelStorei (GL_UNPACK_LSB_FIRST, GL_FALSE);
        glPixelStorei (GL_UNPACK_ROW_LENGTH, 0);
        glPixelStorei (GL_UNPACK_ALIGNMENT, 1);

        glBindTexture (GL_TEXTURE_2D, glTextureID);
        glTexSubImage2D (GL_TEXTURE_2D, 0, xOffset, yOffset,
			destWidth, destHeight, GL_ALPHA, GL_UNSIGNED_BYTE, bitmap.buffer);

        glPopClientAttrib();
    }

    uv[0].X (static_cast<float>(xOffset) / static_cast<float>(width));
    uv[0].Y (static_cast<float>(yOffset) / static_cast<float>(height));
    uv[1].X (static_cast<float> (xOffset + destWidth) / static_cast<float>(width));
    uv[1].Y (static_cast<float> (yOffset + destHeight) / static_cast<float>(height));

    pos.X (glyph->bitmap_left);
    pos.Y (glyph->bitmap_top);
}

FTTextureGlyph::~FTTextureGlyph() {}

const FTPoint& FTTextureGlyph::Render (const FTPoint& pen) {
    if (activeTextureID != glTextureID) {
        glBindTexture (GL_TEXTURE_2D, (GLuint)glTextureID);
        activeTextureID = glTextureID;
    }

    glTranslatef (pen.X(),  pen.Y(), 0.0f);

    glBegin (GL_QUADS);
        glTexCoord2f (uv[0].X(), uv[0].Y());
        glVertex2f (pos.X(), pos.Y());

        glTexCoord2f (uv[0].X(), uv[1].Y());
        glVertex2f (pos.X(), pos.Y() - destHeight);

        glTexCoord2f (uv[1].X(), uv[1].Y());
        glVertex2f (destWidth + pos.X(), pos.Y() - destHeight);

        glTexCoord2f (uv[1].X(), uv[0].Y());
        glVertex2f (destWidth + pos.X(), pos.Y());
    glEnd();
    return advance;
}

inline GLuint NextPowerOf2 (GLuint in) {
     in -= 1;

     in |= in >> 16;
     in |= in >> 8;
     in |= in >> 4;
     in |= in >> 2;
     in |= in >> 1;
     return in + 1;
}

// --------------------------------------------------------------------
//					FTGLTextureFont
// --------------------------------------------------------------------

FTGLTextureFont::FTGLTextureFont (const char* fontFilePath)
:   FTFont (fontFilePath),
    maximumGLTextureSize(0),
    textureWidth(0),
    textureHeight(0),
    glyphHeight(0),
    glyphWidth(0),
    padding(3),
    xOffset(0),
    yOffset(0)
{
    remGlyphs = numGlyphs = face.GlyphCount();
}

FTGLTextureFont::FTGLTextureFont (const unsigned char *pBufferBytes, size_t bufferSizeInBytes)
:   FTFont (pBufferBytes, bufferSizeInBytes),
    maximumGLTextureSize(0),
    textureWidth(0),
    textureHeight(0),
    glyphHeight(0),
    glyphWidth(0),
    padding(3),
    xOffset(0),
    yOffset(0)
{
    remGlyphs = numGlyphs = face.GlyphCount();
}

FTGLTextureFont::~FTGLTextureFont() {
    glDeleteTextures (textureIDList.size(), (const GLuint*)&textureIDList[0]);
}

FTGlyph* FTGLTextureFont::MakeGlyph (unsigned int glyphIndex) {
    FT_GlyphSlot ftGlyph = face.Glyph (glyphIndex, FT_LOAD_NO_HINTING);

    if (ftGlyph) {
        glyphHeight = static_cast<int> (charSize.Height());
        glyphWidth = static_cast<int> (charSize.Width());

        if (textureIDList.empty()) {
            textureIDList.push_back (CreateTexture());
            xOffset = yOffset = padding;
        }

        if (xOffset >  (textureWidth - glyphWidth)) {
            xOffset = padding;
            yOffset += glyphHeight;
            if (yOffset >  (textureHeight - glyphHeight)) {
                textureIDList.push_back (CreateTexture());
                yOffset = padding;
            }
        }

        FTTextureGlyph* tempGlyph =
			new FTTextureGlyph (ftGlyph, textureIDList[textureIDList.size() - 1],
            xOffset, yOffset, textureWidth, textureHeight);
        xOffset += static_cast<int> (tempGlyph->BBox().upperX - tempGlyph->BBox().lowerX + padding);

        --remGlyphs;
        return tempGlyph;
    }
    err = face.Error();
    return NULL;
}

void FTGLTextureFont::CalculateTextureSize() {
    if (!maximumGLTextureSize) {
        glGetIntegerv (GL_MAX_TEXTURE_SIZE, (GLint*)&maximumGLTextureSize);
    }

    textureWidth = NextPowerOf2 ((remGlyphs * glyphWidth) +  (padding * 2));
    textureWidth = textureWidth > maximumGLTextureSize ? maximumGLTextureSize : textureWidth;
    int h = static_cast<int> ((textureWidth -  (padding * 2)) / glyphWidth);
    textureHeight = NextPowerOf2 (( (numGlyphs / h) + 1) * glyphHeight);
    textureHeight = textureHeight > maximumGLTextureSize ? maximumGLTextureSize : textureHeight;
}

GLuint FTGLTextureFont::CreateTexture() {
    CalculateTextureSize();

    int totalMemory = textureWidth * textureHeight;
    unsigned char* textureMemory = new unsigned char[totalMemory];
    memset (textureMemory, 0, totalMemory);

    GLuint textID;
    glGenTextures (1, (GLuint*)&textID);

    glBindTexture (GL_TEXTURE_2D, textID);
    glTexParameteri (GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP);
    glTexParameteri (GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP);
    glTexParameteri (GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    glTexParameteri (GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);

    glTexImage2D (GL_TEXTURE_2D, 0, GL_ALPHA, textureWidth, textureHeight, 0,
		GL_ALPHA, GL_UNSIGNED_BYTE, textureMemory);

    delete [] textureMemory;
    return textID;
}

bool FTGLTextureFont::FaceSize (const unsigned int size, const unsigned int res) {
    if (!textureIDList.empty()) {
        glDeleteTextures (textureIDList.size(), (const GLuint*)&textureIDList[0]);
        textureIDList.clear();
        remGlyphs = numGlyphs = face.GlyphCount();
    }
    return FTFont::FaceSize (size, res);
}

void FTGLTextureFont::Render (const char* string) {
    glPushAttrib (GL_ENABLE_BIT | GL_COLOR_BUFFER_BIT);
    glEnable(GL_BLEND);
    glBlendFunc (GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA); // GL_ONE
    FTTextureGlyph::ResetActiveTexture();
    FTFont::Render (string);
    glPopAttrib();
}

void FTGLTextureFont::Render (const wchar_t* string) {
    glPushAttrib (GL_ENABLE_BIT | GL_COLOR_BUFFER_BIT);
    glEnable(GL_BLEND);
    glBlendFunc (GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA); // GL_ONE
    FTTextureGlyph::ResetActiveTexture();
    FTFont::Render (string);
    glPopAttrib();
}

// --------------------------------------------------------------------
//					FTCharmap (character map)
// --------------------------------------------------------------------

FTCharmap::FTCharmap (FTFace* face)
: ftFace (*(face->Face())), err(0) {
    if (!ftFace->charmap) err = FT_Set_Charmap (ftFace, ftFace->charmaps[0]);
    ftEncoding = ftFace->charmap->encoding;
}

FTCharmap::~FTCharmap() {charMap.clear();}

bool FTCharmap::CharMap (FT_Encoding encoding) {
    if (ftEncoding == encoding) return true;
    err = FT_Select_Charmap (ftFace, encoding);
    if (!err) ftEncoding = encoding;
    else ftEncoding = ft_encoding_none;
    charMap.clear();
    return !err;
}

unsigned int FTCharmap::GlyphListIndex (unsigned int characterCode) {
    return charMap.find (characterCode);
}

unsigned int FTCharmap::FontIndex (unsigned int characterCode) {
    return FT_Get_Char_Index (ftFace, characterCode);
}

void FTCharmap::InsertIndex (const unsigned int characterCode, const unsigned int containerIndex) {
    charMap.insert (characterCode, containerIndex);
}


// --------------------------------------------------------------------
//			FTPixmapGlyph
// --------------------------------------------------------------------

FTPixmapGlyph::FTPixmapGlyph( FT_GlyphSlot glyph)
:   FTGlyph( glyph),
    destWidth(0),
    destHeight(0),
    data(0)
{
    err = FT_Render_Glyph( glyph, FT_RENDER_MODE_NORMAL);
    if( err || ft_glyph_format_bitmap != glyph->format)  return;

    FT_Bitmap bitmap = glyph->bitmap;
    int srcWidth = bitmap.width;
    int srcHeight = bitmap.rows;

    destWidth = srcWidth;
    destHeight = srcHeight;

    if (destWidth && destHeight) {
        data = new unsigned char[destWidth * destHeight * 2];
        unsigned char* src = bitmap.buffer;

        unsigned char* dest = data + ((destHeight - 1) * destWidth * 2);
        size_t destStep = destWidth * 2 * 2;

        for( int y = 0; y < srcHeight; ++y) {
            for( int x = 0; x < srcWidth; ++x) {
                *dest++ = static_cast<unsigned char>(255);
                *dest++ = *src++;
            }
            dest -= destStep;
        }
        destHeight = srcHeight;
    }

    pos.X(glyph->bitmap_left);
    pos.Y(srcHeight - glyph->bitmap_top);
}

FTPixmapGlyph::~FTPixmapGlyph() {delete [] data;}

const FTPoint& FTPixmapGlyph::Render( const FTPoint& pen) {
    glBitmap( 0, 0, 0.0f, 0.0f, pen.X() + pos.X(), pen.Y() - pos.Y(), (const GLubyte*)0);

    if( data) {
        glPixelStorei( GL_UNPACK_ROW_LENGTH, 0);
        glPixelStorei( GL_UNPACK_ALIGNMENT, 2);

        glDrawPixels( destWidth, destHeight, GL_LUMINANCE_ALPHA, GL_UNSIGNED_BYTE, (const GLvoid*)data);
    }
    glBitmap( 0, 0, 0.0f, 0.0f, -pos.X(), pos.Y(), (const GLubyte*)0);
    return advance;
}


// --------------------------------------------------------------------
//			FTPixmapFont
// --------------------------------------------------------------------

FTGLPixmapFont::FTGLPixmapFont( const char* fontFilePath)
:   FTFont( fontFilePath)
{}


FTGLPixmapFont::FTGLPixmapFont( const unsigned char *pBufferBytes, size_t bufferSizeInBytes)
:   FTFont( pBufferBytes, bufferSizeInBytes)
{}


FTGLPixmapFont::~FTGLPixmapFont()
{}


FTGlyph* FTGLPixmapFont::MakeGlyph( unsigned int g) {
    FT_GlyphSlot ftGlyph = face.Glyph( g, FT_LOAD_NO_HINTING);

    if( ftGlyph) {
        FTPixmapGlyph* tempGlyph = new FTPixmapGlyph( ftGlyph);
        return tempGlyph;
    }

    err = face.Error();
    return NULL;
}


void FTGLPixmapFont::Render( const char* string) {
    glPushAttrib( GL_ENABLE_BIT | GL_PIXEL_MODE_BIT | GL_COLOR_BUFFER_BIT);
    glPushClientAttrib( GL_CLIENT_PIXEL_STORE_BIT);

    glEnable(GL_BLEND);
    glBlendFunc( GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

    glDisable( GL_TEXTURE_2D);

    GLfloat ftglColour[4];
    glGetFloatv( GL_CURRENT_RASTER_COLOR, ftglColour);

    glPixelTransferf(GL_RED_SCALE, ftglColour[0]);
    glPixelTransferf(GL_GREEN_SCALE, ftglColour[1]);
    glPixelTransferf(GL_BLUE_SCALE, ftglColour[2]);
    glPixelTransferf(GL_ALPHA_SCALE, ftglColour[3]);

    FTFont::Render( string);

    glPopClientAttrib();
    glPopAttrib();
}


void FTGLPixmapFont::Render( const wchar_t* string) {
    glPushAttrib( GL_ENABLE_BIT | GL_PIXEL_MODE_BIT | GL_COLOR_BUFFER_BIT);
    glPushClientAttrib( GL_CLIENT_PIXEL_STORE_BIT);

    glEnable(GL_BLEND);
    glBlendFunc( GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

    glDisable( GL_TEXTURE_2D);

    GLfloat ftglColour[4];
    glGetFloatv( GL_CURRENT_RASTER_COLOR, ftglColour);

    glPixelTransferf(GL_RED_SCALE, ftglColour[0]);
    glPixelTransferf(GL_GREEN_SCALE, ftglColour[1]);
    glPixelTransferf(GL_BLUE_SCALE, ftglColour[2]);
    glPixelTransferf(GL_ALPHA_SCALE, ftglColour[3]);

    FTFont::Render( string);

    glPopClientAttrib();
    glPopAttrib();
}








