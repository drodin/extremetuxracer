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

#ifndef FTFONT_H
#define FTFONT_H

#include "bh.h"

#include <ft2build.h>
#include FT_OUTLINE_H

typedef double FTGL_DOUBLE;

#ifndef FT_OPEN_MEMORY
    #define FT_OPEN_MEMORY (FT_Open_Flags)1
#endif

#ifndef FT_RENDER_MODE_MONO
    #define FT_RENDER_MODE_MONO ft_render_mode_mono
#endif

#ifndef FT_RENDER_MODE_NORMAL
    #define FT_RENDER_MODE_NORMAL ft_render_mode_normal
#endif

#ifdef _MSC_VER // MS Visual C++
    #pragma warning(disable : 4251)
    #pragma warning(disable : 4275)
    #pragma warning(disable : 4786)

    #ifdef FTGL_LIBRARY_STATIC      // static lib - no special export required
    #  define FTGL_EXPORT
    #elif FTGL_LIBRARY              // dynamic lib - must export/import symbols appropriately.
    #  define FTGL_EXPORT   __declspec(dllexport)
    #else
    #  define FTGL_EXPORT   __declspec(dllimport)
    #endif
#else
    #define FTGL_EXPORT
#endif
// --------------------------------------------------------------------
//			FTVector
// --------------------------------------------------------------------

template <typename FT_VECTOR_ITEM_TYPE>
class FTGL_EXPORT FTVector {
    public:
        typedef FT_VECTOR_ITEM_TYPE value_type;
        typedef value_type& reference;
        typedef const value_type& const_reference;
        typedef value_type* iterator;
        typedef const value_type* const_iterator;
        typedef size_t size_type;

        FTVector() {
            Capacity = Size = 0;
            Items = 0;
        }

        virtual ~FTVector() {clear();}

        FTVector &operator = (const FTVector& v) {
            reserve(v.capacity());
            iterator ptr = begin();
            const_iterator vbegin = v.begin();
            const_iterator vend = v.end();
            while (vbegin != vend) {*ptr++ = *vbegin++;}
            Size = v.size();
            return *this;
        }

        size_type size() const {return Size;}
        size_type capacity() const {return Capacity;}
        iterator begin() {return Items;}
        const_iterator begin() const {return Items;}
        iterator end() {return begin() + size();}
        const_iterator end() const {return begin() + size();}
        bool empty() const {return size() == 0;}
        reference operator [](size_type pos) {return(*(begin() + pos));}
        const_reference operator [](size_type pos) const {return(*(begin() + pos));}

        void clear() {
            if(Capacity) {
                delete [] Items;
                Capacity = Size = 0;
                Items = 0;
            }
        }

        void reserve(size_type n) {
            if(capacity() < n) {expand(n);}
        }

        void push_back(const value_type& x) {
            if(size() == capacity()) {expand();}
            (*this)[size()] = x;
            ++Size;
        }

        void resize(size_type n, value_type x) {
            if (n == size()) {return;}
            reserve(n);
            iterator begin, end;

            if(n >= Size) {
                begin = this->end();
                end = this->begin() + n;
            } else {
                begin = this->begin() + n;
                end = this->end();
            }

            while(begin != end) {*begin++ = x;}
            Size = n;
        }
	private:
        void expand(size_type capacity_hint = 0)
        {
            size_type new_capacity =(capacity() == 0) ? 256 : capacity()* 2;
            if(capacity_hint) {
                while(new_capacity < capacity_hint) {new_capacity *= 2;}
            }

            value_type *new_items = new value_type[new_capacity];
            iterator begin = this->begin();
            iterator end = this->end();
            value_type *ptr = new_items;
            while(begin != end) {*ptr++ = *begin++;}
            if(Capacity) {delete [] Items;}
            Items = new_items;
            Capacity = new_capacity;
        }
        size_type Capacity;
        size_type Size;
        value_type* Items;
};

// --------------------------------------------------------------------
//				FTList
// --------------------------------------------------------------------

template <typename FT_LIST_ITEM_TYPE>
class FTGL_EXPORT FTList {
		FTList(const FTList&);
		FTList& operator=(const FTList&);
    public:
        typedef FT_LIST_ITEM_TYPE value_type;
        typedef value_type& reference;
        typedef const value_type& const_reference;
        typedef size_t size_type;

        FTList() : listSize(0), tail(0) {
            tail = NULL;
            head = new Node;
        }

        ~FTList() {
            Node* next;
            for(Node *walk = head; walk; walk = next) {
                next = walk->next;
                delete walk;
            }
        }

        size_type size() const {return listSize;}
        void push_back(const value_type& item) {
            Node* node = new Node(item);
            if(head->next == NULL) {head->next = node;}
            if(tail) {tail->next = node;}
            tail = node;
            ++listSize;
        }

        reference front() const {return head->next->payload;}
        reference back() const {return tail->payload;}

    private:
        struct Node {
            Node() : next(NULL) {}
            Node(const value_type& item) : next(NULL) {payload = item;}
            Node* next;
            value_type payload;
        };

        size_type listSize;
        Node* head;
        Node* tail;
};

// --------------------------------------------------------------------
// 				FTCharToGlyphIndexMap
// --------------------------------------------------------------------

class FTGL_EXPORT FTCharToGlyphIndexMap {
    public:
        typedef unsigned long CharacterCode;
        typedef signed long GlyphIndex;

        enum {
            NumberOfBuckets = 256,
            BucketSize = 256,
            IndexNotFound = -1
        };
        FTCharToGlyphIndexMap() {this->Indices = 0;}
        virtual ~FTCharToGlyphIndexMap() {
            if(this->Indices) {
                this->clear();
                delete [] this->Indices;
                this->Indices = 0;
            }
        }

        void clear() {
            if (this->Indices) {
                for (int i = 0; i < FTCharToGlyphIndexMap::NumberOfBuckets; i++) {
                    if(this->Indices[i]) {
                        delete [] this->Indices[i];
                        this->Indices[i] = 0;
                    }
                }
            }
        }

        const GlyphIndex find(CharacterCode c) {
            if(!this->Indices) {return 0;}

            div_t pos = div(c, FTCharToGlyphIndexMap::BucketSize);
            if (!this->Indices[pos.quot]) {return 0;}
            const FTCharToGlyphIndexMap::GlyphIndex *ptr = &this->Indices[pos.quot][pos.rem];
            if (*ptr == FTCharToGlyphIndexMap::IndexNotFound) {return 0;}
            return *ptr;
        }

        void insert(CharacterCode c, GlyphIndex g) {
            if (!this->Indices) {
                this->Indices = new GlyphIndex* [FTCharToGlyphIndexMap::NumberOfBuckets];
                for(int i = 0; i < FTCharToGlyphIndexMap::NumberOfBuckets; i++)
                { this->Indices[i] = 0; }
            }
            div_t pos = div(c, FTCharToGlyphIndexMap::BucketSize);
            if (!this->Indices[pos.quot]) {
                this->Indices[pos.quot] = new GlyphIndex [FTCharToGlyphIndexMap::BucketSize];
                for(int i = 0; i < FTCharToGlyphIndexMap::BucketSize; i++)
                	{this->Indices[pos.quot][i] = FTCharToGlyphIndexMap::IndexNotFound;}
            }
            this->Indices[pos.quot][pos.rem] = g;
        }
    private:
        GlyphIndex** Indices;
};

// --------------------------------------------------------------------
//				FTLibrary
// --------------------------------------------------------------------

class FTGL_EXPORT FTLibrary {
public:
    static const FTLibrary &Instance();
    const FT_Library* const GetLibrary() const { return library;}
    FT_Error Error() const { return err;}
    ~FTLibrary();
private:
    FTLibrary();
    FTLibrary(const FT_Library&) {}
    FTLibrary& operator=(const FT_Library&) { return *this; }
    bool Initialise();
    FT_Library* library;
    FT_Error err;
};

// --------------------------------------------------------------------
//					FTPoint
// --------------------------------------------------------------------

class FTGL_EXPORT FTPoint {
    public:
        FTPoint() {
            values[0] = 0;
            values[1] = 0;
            values[2] = 0;
        }

        FTPoint(const FTGL_DOUBLE x, const FTGL_DOUBLE y, const FTGL_DOUBLE z) {
            values[0] = x;
            values[1] = y;
            values[2] = z;
        }

        FTPoint(const FT_Vector& ft_vector) {
            values[0] = ft_vector.x;
            values[1] = ft_vector.y;
            values[2] = 0;
        }

        FTPoint& operator += (const FTPoint& point) {
            values[0] += point.values[0];
            values[1] += point.values[1];
            values[2] += point.values[2];

            return *this;
        }

        FTPoint operator + (const FTPoint& point) {
            FTPoint temp;
            temp.values[0] = values[0] + point.values[0];
            temp.values[1] = values[1] + point.values[1];
            temp.values[2] = values[2] + point.values[2];

            return temp;
        }

        FTPoint operator * (double multiplier) {
            FTPoint temp;
            temp.values[0] = values[0] * multiplier;
            temp.values[1] = values[1] * multiplier;
            temp.values[2] = values[2] * multiplier;

            return temp;
        }

        friend FTPoint operator*(double multiplier, FTPoint& point);
        friend bool operator == (const FTPoint &a, const FTPoint &b);
        friend bool operator != (const FTPoint &a, const FTPoint &b);
        operator const FTGL_DOUBLE*() const {return values;}
        void X(FTGL_DOUBLE x) { values[0] = x;};
        void Y(FTGL_DOUBLE y) { values[1] = y;};
        void Z(FTGL_DOUBLE z) { values[2] = z;};

        FTGL_DOUBLE X() const { return values[0];};
        FTGL_DOUBLE Y() const { return values[1];};
        FTGL_DOUBLE Z() const { return values[2];};

    private:
        FTGL_DOUBLE values[3];
};

// --------------------------------------------------------------------
//				FTSize
// --------------------------------------------------------------------

class FTGL_EXPORT FTSize {
public:
	FTSize();
	virtual ~FTSize();

	bool CharSize(FT_Face* face, unsigned int point_size,
			unsigned int x_resolution, unsigned int y_resolution);

	unsigned int CharSize() const;
	float Ascender() const;
	float Descender() const;
	float Height() const;
	float Width() const;
	float Underline() const;
	FT_Error Error() const { return err; }
private:
	FT_Face* ftFace;
	FT_Size ftSize;
	unsigned int size;
	unsigned int xResolution;
	unsigned int yResolution;
	FT_Error err;
};

// --------------------------------------------------------------------
//					FTFace
// --------------------------------------------------------------------

class FTGL_EXPORT FTFace {
public:
    FTFace(const char* fontFilePath);
    FTFace(const unsigned char *pBufferBytes, size_t bufferSizeInBytes);
    virtual ~FTFace();
    bool Attach(const char* fontFilePath);
    bool Attach(const unsigned char *pBufferBytes, size_t bufferSizeInBytes);
    FT_Face* Face() const { return ftFace;}
    const FTSize& Size(const unsigned int size, const unsigned int res);
    unsigned int CharMapCount();
    FT_Encoding* CharMapList();
    FTPoint KernAdvance(unsigned int index1, unsigned int index2);
    FT_GlyphSlot Glyph(unsigned int index, FT_Int load_flags);
    unsigned int GlyphCount() const { return numGlyphs;}
    FT_Error Error() const { return err; }
private:
    FT_Face* ftFace;
    FTSize charSize;
    int numGlyphs;
    FT_Encoding* fontEncodingList;
    bool hasKerningTable;
    FT_Error err;
};

// --------------------------------------------------------------------
// 				FTBox
// --------------------------------------------------------------------

class FTGL_EXPORT FTBBox {
    public:
        FTBBox()
        :   lowerX(0.0f),
            lowerY(0.0f),
            lowerZ(0.0f),
            upperX(0.0f),
            upperY(0.0f),
            upperZ(0.0f)
        {}

        FTBBox(float lx, float ly, float lz, float ux, float uy, float uz)
        :   lowerX(lx),
            lowerY(ly),
            lowerZ(lz),
            upperX(ux),
            upperY(uy),
            upperZ(uz)
        {}

        FTBBox(FT_GlyphSlot glyph)
        :   lowerX(0.0f),
            lowerY(0.0f),
            lowerZ(0.0f),
            upperX(0.0f),
            upperY(0.0f),
            upperZ(0.0f)
        {
            FT_BBox bbox;
            FT_Outline_Get_CBox(&(glyph->outline), &bbox);
            lowerX = static_cast<float>(bbox.xMin) / 64.0f;
            lowerY = static_cast<float>(bbox.yMin) / 64.0f;
            lowerZ = 0.0f;
            upperX = static_cast<float>(bbox.xMax) / 64.0f;
            upperY = static_cast<float>(bbox.yMax) / 64.0f;
            upperZ = 0.0f;
        }
        FTBBox &Move (FTPoint distance)
        {
            lowerX += distance.X();
            lowerY += distance.Y();
            lowerZ += distance.Z();
            upperX += distance.X();
            upperY += distance.Y();
            upperZ += distance.Z();
            return *this;
        }

        ~FTBBox() {}

        FTBBox &operator += (const FTBBox& bbox)
        {
            lowerX = bbox.lowerX < lowerX? bbox.lowerX: lowerX;
            lowerY = bbox.lowerY < lowerY? bbox.lowerY: lowerY;
            lowerZ = bbox.lowerZ < lowerZ? bbox.lowerZ: lowerZ;
            upperX = bbox.upperX > upperX? bbox.upperX: upperX;
            upperY = bbox.upperY > upperY? bbox.upperY: upperY;
            upperZ = bbox.upperZ > upperZ? bbox.upperZ: upperZ;

            return *this;
        }

        void SetDepth(float depth) {upperZ = lowerZ + depth;}
        float lowerX, lowerY, lowerZ, upperX, upperY, upperZ;
    protected:
    private:
};

// --------------------------------------------------------------------
//			FTGlyph
// --------------------------------------------------------------------

class FTGL_EXPORT FTGlyph {
    public:
        FTGlyph(FT_GlyphSlot glyph);
        virtual ~FTGlyph();
        virtual const FTPoint& Render(const FTPoint& pen) = 0;
        const FTPoint& Advance() const { return advance;}
        const FTBBox& BBox() const { return bBox;}
        FT_Error Error() const { return err;}
    protected:
        FTPoint advance;
        FTBBox bBox;
        FT_Error err;
    private:
};

// --------------------------------------------------------------------
//			FTCharmap
// --------------------------------------------------------------------

class FTGL_EXPORT FTCharmap {
    public:
        FTCharmap(FTFace* face);
        virtual ~FTCharmap();
        FT_Encoding Encoding() const { return ftEncoding;}
        bool CharMap(FT_Encoding encoding);
        unsigned int GlyphListIndex(const unsigned int characterCode);
        unsigned int FontIndex(const unsigned int characterCode);
        void InsertIndex(const unsigned int characterCode, const unsigned int containerIndex);
        FT_Error Error() const { return err;}
    private:
        FT_Encoding ftEncoding;
        const FT_Face ftFace;
        typedef FTCharToGlyphIndexMap CharacterMap;
        CharacterMap charMap;
        FT_Error err;
};

// --------------------------------------------------------------------
//			FTGlyphContainer
// --------------------------------------------------------------------

class FTGL_EXPORT FTGlyphContainer {
    typedef FTVector<FTGlyph*> GlyphVector;
    public:
        FTGlyphContainer (FTFace* face);
        ~FTGlyphContainer();
        bool CharMap(FT_Encoding encoding);
        unsigned int FontIndex(const unsigned int characterCode) const;
        void Add(FTGlyph* glyph, const unsigned int characterCode);
        const FTGlyph* const Glyph(const unsigned int characterCode) const;
        FTBBox BBox(const unsigned int characterCode) const;
        float Advance(const unsigned int characterCode, const unsigned int nextCharacterCode);
        FTPoint Render(const unsigned int characterCode,
			const unsigned int nextCharacterCode, FTPoint penPosition);
        FT_Error Error() const { return err;}

    private:
        FTFace* face;
        FTCharmap* charMap;
        GlyphVector glyphs;
        FT_Error err;
};

// --------------------------------------------------------------------
//			FTTextureGlyph
// --------------------------------------------------------------------

class FTGL_EXPORT FTTextureGlyph : public FTGlyph {
    public:
        FTTextureGlyph(FT_GlyphSlot glyph, int id, int xOffset, int yOffset,
			GLsizei width, GLsizei height);
        virtual ~FTTextureGlyph();
        virtual const FTPoint& Render(const FTPoint& pen);
        static void ResetActiveTexture() { activeTextureID = 0;}
    private:
        int destWidth;
        int destHeight;
        FTPoint pos;
        FTPoint uv[2];
        int glTextureID;
        static GLint activeTextureID;
};

// --------------------------------------------------------------------
//			FTFont
// --------------------------------------------------------------------

class FTGL_EXPORT FTFont {
    public:
        FTFont (const char* fontFilePath);
        FTFont (const unsigned char *pBufferBytes, size_t bufferSizeInBytes);
        virtual ~FTFont();

        bool Attach (const char* fontFilePath);
        bool Attach (const unsigned char *pBufferBytes, size_t bufferSizeInBytes);
        bool CharMap (FT_Encoding encoding);
        unsigned int CharMapCount();
        FT_Encoding* CharMapList();
        virtual bool FaceSize(const unsigned int size, const unsigned int res = 72);
        unsigned int FaceSize() const;
        virtual void Depth(float depth) {}
        float Ascender() const;
        float Descender() const;
        float LineHeight() const;
        void BBox(const char* string, float& llx, float& lly,
			float& llz, float& urx, float& ury, float& urz);
        void BBox (const wchar_t* string, float& llx, float& lly,
			float& llz, float& urx, float& ury, float& urz);
        float Advance (const wchar_t* string);
        float Advance (const char* string);
        virtual void Render (const char* string);
        virtual void Render (const wchar_t* string);
        FT_Error Error() const { return err;}

    protected:
        virtual FTGlyph* MakeGlyph(unsigned int g) = 0;
        FTFace face;
        FTSize charSize;
        FT_Error err;
    private:
        inline bool CheckGlyph(const unsigned int chr);
        FTGlyphContainer* glyphList;
        FTPoint pen;
};

// --------------------------------------------------------------------
//			FTTextureFont
// --------------------------------------------------------------------

class FTGL_EXPORT FTGLTextureFont : public FTFont {
    public:
        FTGLTextureFont(const char* fontFilePath);
        FTGLTextureFont(const unsigned char *pBufferBytes, size_t bufferSizeInBytes);
        virtual ~FTGLTextureFont();
        virtual bool FaceSize(const unsigned int size, const unsigned int res = 72);
        virtual void Render(const char* string);
        virtual void Render(const wchar_t* string);
    private:
        inline virtual FTGlyph* MakeGlyph(unsigned int glyphIndex);
        inline void CalculateTextureSize();
        inline GLuint CreateTexture();
        GLsizei maximumGLTextureSize;
        GLsizei textureWidth;
        GLsizei textureHeight;
        FTVector<GLuint> textureIDList;
        int glyphHeight;
        int glyphWidth;
        unsigned int padding;
        unsigned int numGlyphs;
        unsigned int remGlyphs;
        int xOffset;
        int yOffset;
};

class FTGL_EXPORT FTGLPixmapFont : public FTFont
{
    public:
        FTGLPixmapFont( const char* fontFilePath);
        FTGLPixmapFont( const unsigned char *pBufferBytes, size_t bufferSizeInBytes);
        ~FTGLPixmapFont();
        void Render( const char* string);
        void Render( const wchar_t* string);

    private:
        inline virtual FTGlyph* MakeGlyph( unsigned int g);

};

class FTGL_EXPORT FTPixmapGlyph : public FTGlyph
{
    public:
        FTPixmapGlyph( FT_GlyphSlot glyph);
        virtual ~FTPixmapGlyph();
        virtual const FTPoint& Render( const FTPoint& pen);
    private:
        int destWidth;
        int destHeight;
        FTPoint pos;
        unsigned char* data;
};

#endif
