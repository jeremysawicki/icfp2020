#ifndef GRAPHICS_HPP
#define GRAPHICS_HPP

#include "Common.hpp"
#include "Key.hpp"

enum class LineStyle : uint32_t
{
    Solid,
    Dash
};

enum class LineCap : uint32_t
{
    Round,
    Flat,
    Projecting
};

enum class LineJoin : uint32_t
{
    Round,
    Miter,
    Bevel
};

enum class FillMode : uint32_t
{
    Alternate,
    Winding
};

enum class FontSpacing : uint32_t
{
    Proportional,
    Monospace
};

enum class FontFace : uint32_t
{
    SansSerif,
    Serif
};

enum class HAlign
{
    Left,
    Center,
    Right
};

enum class VAlign
{
    Top,
    Baseline,
    Bottom
};

class Point
{
public:
    Point() : x(0), y(0) {}
    Point(int32_t xIn, int32_t yIn) : x(xIn), y(yIn) {}

public:
    int32_t x;
    int32_t y;
};

class GraphicsImpl;

class Graphics
{
public:
    Graphics();
    Graphics(Graphics&& other);
    ~Graphics();
    Graphics& operator=(Graphics&& other);

    void setOnDestroy(std::function<void()> f = nullptr);
    void setOnPaint(std::function<void()> f = nullptr);
    void setOnKey(std::function<void(Key)> f = nullptr);
    void setOnClick(std::function<void(int32_t, int32_t)> f = nullptr);

    bool create(uint32_t width, uint32_t height,
                const char* title,
                std::string* pMsg = nullptr);
    bool destroy();

    uint32_t getWidth() const;
    uint32_t getHeight() const;

    static bool update(bool blocking);
    void requestPaint();

    void beginPaint(bool buffered);
    void endPaint();

    // Points

    void drawPoint(int32_t x, int32_t y, uint32_t color);

    // Lines and curves

    void setLineWidth(uint32_t lineWidth);
    void setLineStyle(LineStyle lineStyle);
    void setLineCap(LineCap lineCap);
    void setLineJoin(LineJoin lineJoin);

    void drawLine(int32_t x1, int32_t y1, int32_t x2, int32_t y2,
                  uint32_t color);
    void drawPolyline(const Point* points, uint32_t numPoints,
                      uint32_t color);
    void drawPolygon(const Point* points, uint32_t numPoints,
                     uint32_t color);
    void drawRect(int32_t x, int32_t y,
                  uint32_t width, uint32_t height,
                  uint32_t color);
    void drawCircle(int32_t x, int32_t y, // upper-left corner of rectangle
                    uint32_t width, uint32_t height, // size of rectangle
                    uint32_t color);

    // Filled regions

    void setFillMode(FillMode fillMode);

    void fillPolygon(const Point* points, uint32_t numPoints,
                     uint32_t color);
    void fillRect(int32_t x, int32_t y,
                  uint32_t width, uint32_t height,
                  uint32_t color);
    void fillCircle(int32_t x, int32_t y, // upper-left corner of rectangle
                    uint32_t width, uint32_t height, // size of rectangle
                    uint32_t color);

    // Bitmaps

    void drawBitmap(int32_t destX, int32_t destY,
                    const uint32_t* data,
                    uint32_t dataWidth, uint32_t dataHeight);
    void drawBitmap(int32_t destX, int32_t destY,
                    const uint32_t* data,
                    uint32_t dataWidth, uint32_t dataHeight,
                    int32_t srcX, int32_t srcY,
                    uint32_t srcWidth, uint32_t srcHeight);
    void stretchBitmap(int32_t destX, int32_t destY,
                       uint32_t destWidth, uint32_t destHeight,
                       const uint32_t* data,
                       uint32_t dataWidth, uint32_t dataHeight);
    void stretchBitmap(int32_t destX, int32_t destY,
                       uint32_t destWidth, uint32_t destHeight,
                       const uint32_t* data,
                       uint32_t dataWidth, uint32_t dataHeight,
                       int32_t srcX, int32_t srcY,
                       uint32_t srcWidth, uint32_t srcHeight);

    // Text

    void setFont(FontSpacing spacing, FontFace face,
                 uint32_t height,
                 bool bold, bool italic);
    uint32_t getFontHeight();
    uint32_t getFontAscent();
    uint32_t getFontDescent();
    uint32_t getTextWidth(const char* str);
    void drawText(int32_t x, int32_t y,
                  const char* str, uint32_t color,
                  HAlign halign, VAlign valign);

    // Clipping

    void setClipRect(int32_t x, int32_t y,
                     uint32_t width, uint32_t height);
    void clearClip();

private:
    std::unique_ptr<GraphicsImpl> m_pImpl;

private:
    Graphics(const Graphics& other) = delete;
    Graphics& operator=(const Graphics& other) = delete;
};

#endif
