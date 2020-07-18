#ifndef GRID_HPP
#define GRID_HPP

#include "Common.hpp"

#define FAST_GRID 0

template<typename T>
class Grid
{
public:
    Grid() :
        m_data(),
        m_width(0),
        m_height(0)
#if FAST_GRID
        ,m_shift(0)
#endif
    {
    }

    Grid(size_t width, size_t height) :
        m_data(),
        m_width(0),
        m_height(0)
#if FAST_GRID
        ,m_shift(0)
#endif
    {
        resize(width, height);
    }

    Grid(const Grid& other) :
        m_data(),
        m_width(0),
        m_height(0)
#if FAST_GRID
        ,m_shift(0)
#endif
    {
        *this = other;
    }

    Grid(Grid&& other) :
        m_data(std::move(other.m_data)),
        m_width(other.m_width),
        m_height(other.m_height)
#if FAST_GRID
        ,m_shift(other.m_shift)
#endif
    {
        other.m_width = 0;
        other.m_height = 0;
#if FAST_GRID
        other.m_shift = 0;
#endif
    }

    ~Grid() {}

    Grid& operator=(const Grid& other)
    {
        if (this != &other)
        {
            size_t width = other.m_width;
            size_t height = other.m_height;
#if FAST_GRID
            size_t shift = other.m_shift;
#endif

            m_width = width;
            m_height = height;
#if FAST_GRID
            m_shift = shift;
#endif

            if (width == 0 || height == 0)
            {
                m_data.reset(nullptr);
            }
            else
            {
#if FAST_GRID
                m_data.reset(new T[((size_t)1 << shift) * height]);
                const T* otherData = other.m_data.get();
                T* data = m_data.get();
                for (size_t y = 0; y < height; y++)
                {
                    for (size_t x = 0; x < width; x++)
                    {
                        data[(y << shift) + x] = otherData[(y << shift) + x];
                    }
                }
#else
                m_data.reset(new T[width * height]);
                size_t size = width * height;
                const T* otherData = other.m_data.get();
                T* data = m_data.get();
                for (size_t i = 0; i < size; i++)
                {
                    data[i] = otherData[i];
                }
#endif
            }
        }
        return *this;
    }
    Grid& operator=(Grid&& other)
    {
        if (this != &other)
        {
            m_data = std::move(other.m_data);
            m_width = other.m_width;
            m_height = other.m_height;
#if FAST_GRID
            m_shift = other.m_shift;
#endif

            other.m_width = 0;
            other.m_height = 0;
#if FAST_GRID
            other.m_shift = 0;
#endif
        }
        return *this;
    }

    size_t getWidth() const { return m_width; }
    size_t getHeight() const { return m_height; }

    void resize(size_t width, size_t height)
    {
        m_width = width;
        m_height = height;

#if FAST_GRID
        size_t shift = 0;
        while (((size_t)1 << shift) < width)
        {
            shift++;
        }
        m_shift = shift;
#endif

        if (width == 0 || height == 0)
        {
            m_data.reset(nullptr);
        }
        else
        {
#if FAST_GRID
            m_data.reset(new T[((size_t)1 << shift) * height]());
#else
            m_data.reset(new T[width * height]());
#endif
        }
    }

    void clear()
    {
        m_data.reset(nullptr);
        m_width = 0;
        m_height = 0;
#if FAST_GRID
        m_shift = 0;
#endif
    }

    const T& operator()(size_t x, size_t y) const
    {
#if FAST_GRID
        return m_data[(y << m_shift) + x];
#else
        return m_data[y * m_width + x];
#endif
    }

    T& operator()(size_t x, size_t y)
    {
#if FAST_GRID
        return m_data[(y << m_shift) + x];
#else
        return m_data[y * m_width + x];
#endif
    }

    template<typename Pos>
    const T& operator[](const Pos& pos) const
    { using std::get; return operator()(get<0>(pos), get<1>(pos)); }

    template<typename Pos>
    T& operator[](const Pos& pos)
    { using std::get; return operator()(get<0>(pos), get<1>(pos)); }

    void swap(Grid& other)
    {
        using std::swap;
        swap(m_data, other.m_data);
#if FAST_GRID
        swap(m_shift, other.m_shift);
#endif
        swap(m_width, other.m_width);
        swap(m_height, other.m_height);
    }

private:
    std::unique_ptr<T[]> m_data;
    size_t m_width;
    size_t m_height;
#if FAST_GRID
    size_t m_shift;
#endif
};

template<typename T>
void swap(Grid<T>& x, Grid<T>& y)
{
    x.swap(y);
}

#endif
