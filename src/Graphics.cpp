#include "Graphics.hpp"
#include "Cleanup.hpp"

#if PLATFORM_WINDOWS
#include <windows.h>
#else
#include <X11/Xlib.h>
#include <X11/Xutil.h>
#include <X11/keysym.h>
#endif

using std::string;
using std::vector;
using std::map;

class GraphicsShared
{
public:
    static std::shared_ptr<GraphicsShared> initialize(string* pMsg = nullptr);
    ~GraphicsShared();

    bool m_init;
#if PLATFORM_WINDOWS
    HINSTANCE m_hInstance;
#else
    Display* m_dis;
    int m_screen;
    Atom m_wmDeleteWindow;
    Atom m_wmProtocols;
    unsigned long m_blackPixel;
    unsigned long m_whitePixel;
    Colormap m_colormap;
    map<uint32_t, unsigned long> m_pixelMap;
    vector<unsigned long> m_pixels;
    map<Window, GraphicsImpl*> m_windowMap;
#endif
};

#if PLATFORM_WINDOWS
LRESULT CALLBACK GraphicsWndProc(
    HWND hwnd,
    UINT msg,
    WPARAM wParam,
    LPARAM lParam);
#endif

std::shared_ptr<GraphicsShared> GraphicsShared::initialize(string* pMsg)
{
    auto pShared = std::make_shared<GraphicsShared>();
    pShared->m_init = false;

#if PLATFORM_WINDOWS
    pShared->m_hInstance = GetModuleHandle(NULL);

    WNDCLASSEX wc;
    wc.cbSize = sizeof(WNDCLASSEX);
    wc.style = 0;
    wc.lpfnWndProc = GraphicsWndProc;
    wc.cbClsExtra = 0;
    wc.cbWndExtra = 0;
    wc.hInstance = pShared->m_hInstance;
    wc.hIcon = LoadIcon(NULL, IDI_APPLICATION);
    wc.hCursor = LoadCursor(NULL, IDC_ARROW);
    wc.hbrBackground = NULL;
    wc.lpszMenuName = NULL;
    wc.lpszClassName = "Graphics";
    wc.hIconSm = LoadIcon(NULL, IDI_APPLICATION);

    ATOM wcAtom = RegisterClassEx(&wc);
    if (!wcAtom)
    {
        if (pMsg)
        {
            *pMsg = "RegisterClassEx failed";
        }
        return nullptr;
    }
#else
    pShared->m_dis = XOpenDisplay(nullptr);
    if (!pShared->m_dis)
    {
        if (pMsg)
        {
            *pMsg = "XOpenDisplay failed";
        }
        return nullptr;
    }
    Cleanup displayCleanup([&]{ XCloseDisplay(pShared->m_dis); });

    pShared->m_screen = DefaultScreen(pShared->m_dis);

    pShared->m_wmDeleteWindow =
        XInternAtom(pShared->m_dis, "WM_DELETE_WINDOW", False);
    pShared->m_wmProtocols =
        XInternAtom(pShared->m_dis, "WM_PROTOCOLS", False);

    pShared->m_blackPixel = BlackPixel(pShared->m_dis, pShared->m_screen);
    pShared->m_whitePixel = WhitePixel(pShared->m_dis, pShared->m_screen);

    pShared->m_colormap = DefaultColormap(pShared->m_dis, pShared->m_screen);

    displayCleanup.reset();
#endif

    pShared->m_init = true;
    return pShared;
}

GraphicsShared::~GraphicsShared()
{
    if (!m_init)
    {
        return;
    }

#if PLATFORM_WINDOWS
    UnregisterClass("Graphics", m_hInstance);
#else
    if (!m_pixels.empty())
    {
        XFreeColors(m_dis, m_colormap, &m_pixels[0],
                    (int)m_pixels.size(), 0);
        m_pixels.clear();
    }
    m_pixelMap.clear();

    XCloseDisplay(m_dis);
    m_dis = nullptr;
#endif

    m_init = false;
}

class GraphicsImpl
{
public:
    GraphicsImpl() :
        m_pShared(),
        m_pGraphics(),
        m_init(false),
        m_onDestroy(),
        m_onPaint(),
        m_onKey(),
        m_onClick(),
        m_width(),
        m_height(),
        m_painting(false),
        m_buffered(false),
        m_lineWidth(0),
        m_lineStyle(LineStyle::Solid),
        m_lineCap(LineCap::Round),
        m_lineJoin(LineJoin::Round),
        m_fillMode(FillMode::Alternate)
#if PLATFORM_WINDOWS
        ,
        m_hwnd(),
        m_hdc(),
        m_hdcPaint(),
        m_hdcOld(),
        m_hbmBuf(),
        m_hbmOld(),
        m_hfont(),
        m_textMetrics()
#else
        ,
        m_win(),
        m_gc(),
        m_pixmap(),
        m_drawable(),
        m_paintRequested(false),
        m_destroyed(false),
        m_pFontStruct(nullptr)
#endif
    {
    }

    static std::weak_ptr<GraphicsShared> s_pShared;
    std::shared_ptr<GraphicsShared> m_pShared;
    Graphics* m_pGraphics;
    bool m_init;
    std::function<void()> m_onDestroy;
    std::function<void()> m_onPaint;
    std::function<void(Key)> m_onKey;
    std::function<void(int32_t, int32_t)> m_onClick;
    uint32_t m_width;
    uint32_t m_height;
    bool m_painting;
    bool m_buffered;
    uint32_t m_lineWidth;
    LineStyle m_lineStyle;
    LineCap m_lineCap;
    LineJoin m_lineJoin;
    FillMode m_fillMode;
#if PLATFORM_WINDOWS
    HWND m_hwnd;
    HDC m_hdc;
    HDC m_hdcPaint;
    HDC m_hdcOld;
    HBITMAP m_hbmBuf;
    HBITMAP m_hbmOld;
    HFONT m_hfont;
    TEXTMETRIC m_textMetrics;
#else
    Window m_win;
    GC m_gc;
    Pixmap m_pixmap;
    Drawable m_drawable;
    bool m_paintRequested;
    bool m_destroyed;
    XFontStruct* m_pFontStruct;
#endif
};

std::weak_ptr<GraphicsShared> GraphicsImpl::s_pShared;

Graphics::Graphics() :
    m_pImpl(new GraphicsImpl)
{
    m_pImpl->m_pGraphics = this;
}

Graphics::Graphics(Graphics&& other) :
    m_pImpl(std::move(other.m_pImpl))
{
    m_pImpl->m_pGraphics = this;
    other.m_pImpl.reset(new GraphicsImpl);
    other.m_pImpl->m_pGraphics = &other;
}

Graphics::~Graphics()
{
}

Graphics& Graphics::operator=(Graphics&& other)
{
    if (this != &other)
    {
        m_pImpl = std::move(other.m_pImpl);
        m_pImpl->m_pGraphics = this;
        other.m_pImpl.reset(new GraphicsImpl);
        other.m_pImpl->m_pGraphics = &other;
    }
    return *this;
}

void Graphics::setOnDestroy(std::function<void()> f)
{
    m_pImpl->m_onDestroy = f;
}

void Graphics::setOnPaint(std::function<void()> f)
{
    m_pImpl->m_onPaint = f;
}

void Graphics::setOnKey(std::function<void(Key)> f)
{
    m_pImpl->m_onKey = f;
}

void Graphics::setOnClick(std::function<void(int32_t, int32_t)> f)
{
    m_pImpl->m_onClick = f;
}

#if PLATFORM_WINDOWS
LRESULT CALLBACK GraphicsWndProc(
    HWND hwnd,
    UINT msg,
    WPARAM wParam,
    LPARAM lParam)
{
    auto pShared = GraphicsImpl::s_pShared.lock();
    if (!pShared)
    {
        return DefWindowProc(hwnd, msg, wParam, lParam);
    }

    GraphicsImpl* pImpl = (GraphicsImpl*)GetWindowLongPtr(hwnd, GWLP_USERDATA);
    if (!pImpl)
    {
        if (msg == WM_NCCREATE)
        {
            CREATESTRUCT* pCS = (CREATESTRUCT*)lParam;
            pImpl = (GraphicsImpl*)(pCS->lpCreateParams);
            SetWindowLongPtr(hwnd, GWLP_USERDATA, (LONG_PTR)pImpl);
        }
        if (!pImpl)
        {
            return DefWindowProc(hwnd, msg, wParam, lParam);
        }
    }

    switch (msg)
    {
    case WM_NCCREATE:
    {
        RECT rect;
        if (GetClientRect(hwnd, &rect))
        {
            pImpl->m_width = (uint32_t)(rect.right - rect.left);
            pImpl->m_height = (uint32_t)(rect.bottom - rect.top);
        }
        //return TRUE; // This prevents the window title from appearing
        return DefWindowProc(hwnd, msg, wParam, lParam);
    }
    case WM_CREATE:
    {
        RECT rect;
        if (GetClientRect(hwnd, &rect))
        {
            pImpl->m_width = (uint32_t)(rect.right - rect.left);
            pImpl->m_height = (uint32_t)(rect.bottom - rect.top);
        }
        return 0;
    }
    case WM_DESTROY:
    {
        if (pImpl->m_onDestroy)
        {
            pImpl->m_onDestroy();
        }
        pImpl->m_pShared.reset();
        pImpl->m_init = false;
        return 0;
    }
    case WM_SIZE:
    {
        RECT rect;
        if (GetClientRect(hwnd, &rect))
        {
            pImpl->m_width = (uint32_t)(rect.right - rect.left);
            pImpl->m_height = (uint32_t)(rect.bottom - rect.top);
        }
        pImpl->m_pGraphics->requestPaint();
        return 0;
    }
    case WM_ERASEBKGND:
    {
        return 0;
    }
    case WM_PAINT:
    {
        PAINTSTRUCT ps;
        pImpl->m_hdcPaint = BeginPaint(hwnd, &ps);
        if (pImpl->m_onPaint)
        {
            pImpl->m_onPaint();
        }
        EndPaint(hwnd, &ps);
        pImpl->m_hdcPaint = NULL;
        return 0;
    }
    case WM_KEYDOWN:
    case WM_SYSKEYDOWN:
    {
        uint32_t repeatCount = lParam & 0xffff;
        bool bShift = ((GetKeyState(VK_SHIFT) & 0x8000) != 0);
        bool bCtrl = ((GetKeyState(VK_CONTROL) & 0x8000) != 0);
        bool bAlt = ((GetKeyState(VK_MENU) & 0x8000) != 0);
        bool bCaps = ((GetKeyState(VK_CAPITAL) & 0x0001) != 0);
        Key key = Key::Invalid;

        switch (wParam)
        {
        case VK_BACK: key = Key::Backspace; break;
        case VK_TAB: key = Key::Tab; break;
        case VK_CLEAR: key = Key::Center; break;
        case VK_RETURN: key = Key::Enter; break;
        case VK_ESCAPE: key = Key::Esc; break;
        case VK_SPACE: key = Key::Space; break;
        case VK_PRIOR: key = Key::PgUp; break;
        case VK_NEXT: key = Key::PgDn; break;
        case VK_END: key = Key::End; break;
        case VK_HOME: key = Key::Home; break;
        case VK_LEFT: key = Key::Left; break;
        case VK_UP: key = Key::Up; break;
        case VK_RIGHT: key = Key::Right; break;
        case VK_DOWN: key = Key::Down; break;
        case VK_INSERT: key = Key::Insert; break;
        case VK_DELETE: key = Key::Delete; break;
        case '0': key = Key::Key0; break;
        case '1': key = Key::Key1; break;
        case '2': key = Key::Key2; break;
        case '3': key = Key::Key3; break;
        case '4': key = Key::Key4; break;
        case '5': key = Key::Key5; break;
        case '6': key = Key::Key6; break;
        case '7': key = Key::Key7; break;
        case '8': key = Key::Key8; break;
        case '9': key = Key::Key9; break;
        case 'A': key = Key::A; break;
        case 'B': key = Key::B; break;
        case 'C': key = Key::C; break;
        case 'D': key = Key::D; break;
        case 'E': key = Key::E; break;
        case 'F': key = Key::F; break;
        case 'G': key = Key::G; break;
        case 'H': key = Key::H; break;
        case 'I': key = Key::I; break;
        case 'J': key = Key::J; break;
        case 'K': key = Key::K; break;
        case 'L': key = Key::L; break;
        case 'M': key = Key::M; break;
        case 'N': key = Key::N; break;
        case 'O': key = Key::O; break;
        case 'P': key = Key::P; break;
        case 'Q': key = Key::Q; break;
        case 'R': key = Key::R; break;
        case 'S': key = Key::S; break;
        case 'T': key = Key::T; break;
        case 'U': key = Key::U; break;
        case 'V': key = Key::V; break;
        case 'W': key = Key::W; break;
        case 'X': key = Key::X; break;
        case 'Y': key = Key::Y; break;
        case 'Z': key = Key::Z; break;
        case VK_F1: key = Key::F1; break;
        case VK_F2: key = Key::F2; break;
        case VK_F3: key = Key::F3; break;
        case VK_F4: key = Key::F4; break;
        case VK_F5: key = Key::F5; break;
        case VK_F6: key = Key::F6; break;
        case VK_F7: key = Key::F7; break;
        case VK_F8: key = Key::F8; break;
        case VK_F9: key = Key::F9; break;
        case VK_F10: key = Key::F10; break;
        case VK_F11: key = Key::F11; break;
        case VK_F12: key = Key::F12; break;
        case VK_OEM_1: key = Key::Semicolon; break;
        case VK_OEM_PLUS: key = Key::Equals; break;
        case VK_OEM_COMMA: key = Key::Comma; break;
        case VK_OEM_MINUS: key = Key::Minus; break;
        case VK_OEM_PERIOD: key = Key::Period; break;
        case VK_OEM_2: key = Key::Slash; break;
        case VK_OEM_3: key = Key::Backtick; break;
        case VK_OEM_4: key = Key::LeftBracket; break;
        case VK_OEM_5: key = Key::Backslash; break;
        case VK_OEM_6: key = Key::RightBracket; break;
        case VK_OEM_7: key = Key::Apostrophe; break;
        default: ;
        }

        if (key == Key::Invalid)
        {
            break;
        }

        if (msg == WM_SYSKEYDOWN && key == Key::Space)
        {
            break;
        }

        bool bAdjustedShift = bShift;
        if (bCaps && (key >= Key::A && key <= Key::Z))
        {
            bAdjustedShift = !bAdjustedShift;
        }

        if (bAdjustedShift)
        {
            key = (Key)((uint32_t)key | (uint32_t)Key::Shift);
        }
        if (bCtrl)
        {
            key = (Key)((uint32_t)key | (uint32_t)Key::Ctrl);
        }
        if (bAlt)
        {
            key = (Key)((uint32_t)key | (uint32_t)Key::Alt);
        }

        for (uint32_t i = 0; i < repeatCount; i++)
        {
            if (pImpl->m_onKey)
            {
                pImpl->m_onKey(key);
            }
        }

        return 0;
    }
    case WM_CHAR:
    {
        uint32_t repeatCount = lParam & 0xffff;
        Key key = Key::Invalid;

        switch (wParam)
        {
        case ' ': key = Key::Space; break;
        case '!': key = Key::Exclamation; break;
        case '"': key = Key::Quote; break;
        case '#': key = Key::Pound; break;
        case '$': key = Key::Dollar; break;
        case '%': key = Key::Percent; break;
        case '&': key = Key::Ampersand; break;
        case '\'': key = Key::Apostrophe; break;
        case '(': key = Key::LeftParen; break;
        case ')': key = Key::RightParen; break;
        case '*': key = Key::Asterisk; break;
        case '+': key = Key::Plus; break;
        case ',': key = Key::Comma; break;
        case '-': key = Key::Minus; break;
        case '.': key = Key::Period; break;
        case '/': key = Key::Slash; break;
        case '0': key = Key::Key0; break;
        case '1': key = Key::Key1; break;
        case '2': key = Key::Key2; break;
        case '3': key = Key::Key3; break;
        case '4': key = Key::Key4; break;
        case '5': key = Key::Key5; break;
        case '6': key = Key::Key6; break;
        case '7': key = Key::Key7; break;
        case '8': key = Key::Key8; break;
        case '9': key = Key::Key9; break;
        case ':': key = Key::Colon; break;
        case ';': key = Key::Semicolon; break;
        case '<': key = Key::Less; break;
        case '=': key = Key::Equals; break;
        case '>': key = Key::Greater; break;
        case '?': key = Key::Question; break;
        case '@': key = Key::At; break;
        case 'A': key = Key::ShiftA; break;
        case 'B': key = Key::ShiftB; break;
        case 'C': key = Key::ShiftC; break;
        case 'D': key = Key::ShiftD; break;
        case 'E': key = Key::ShiftE; break;
        case 'F': key = Key::ShiftF; break;
        case 'G': key = Key::ShiftG; break;
        case 'H': key = Key::ShiftH; break;
        case 'I': key = Key::ShiftI; break;
        case 'J': key = Key::ShiftJ; break;
        case 'K': key = Key::ShiftK; break;
        case 'L': key = Key::ShiftL; break;
        case 'M': key = Key::ShiftM; break;
        case 'N': key = Key::ShiftN; break;
        case 'O': key = Key::ShiftO; break;
        case 'P': key = Key::ShiftP; break;
        case 'Q': key = Key::ShiftQ; break;
        case 'R': key = Key::ShiftR; break;
        case 'S': key = Key::ShiftS; break;
        case 'T': key = Key::ShiftT; break;
        case 'U': key = Key::ShiftU; break;
        case 'V': key = Key::ShiftV; break;
        case 'W': key = Key::ShiftW; break;
        case 'X': key = Key::ShiftX; break;
        case 'Y': key = Key::ShiftY; break;
        case 'Z': key = Key::ShiftZ; break;
        case '[': key = Key::LeftBracket; break;
        case '\\': key = Key::Backslash; break;
        case ']': key = Key::RightBracket; break;
        case '^': key = Key::Caret; break;
        case '_': key = Key::Underscore; break;
        case '`': key = Key::Backtick; break;
        case 'a': key = Key::A; break;
        case 'b': key = Key::B; break;
        case 'c': key = Key::C; break;
        case 'd': key = Key::D; break;
        case 'e': key = Key::E; break;
        case 'f': key = Key::F; break;
        case 'g': key = Key::G; break;
        case 'h': key = Key::H; break;
        case 'i': key = Key::I; break;
        case 'j': key = Key::J; break;
        case 'k': key = Key::K; break;
        case 'l': key = Key::L; break;
        case 'm': key = Key::M; break;
        case 'n': key = Key::N; break;
        case 'o': key = Key::O; break;
        case 'p': key = Key::P; break;
        case 'q': key = Key::Q; break;
        case 'r': key = Key::R; break;
        case 's': key = Key::S; break;
        case 't': key = Key::T; break;
        case 'u': key = Key::U; break;
        case 'v': key = Key::V; break;
        case 'w': key = Key::W; break;
        case 'x': key = Key::X; break;
        case 'y': key = Key::Y; break;
        case 'z': key = Key::Z; break;
        case '{': key = Key::LeftBrace; break;
        case '|': key = Key::VerticalBar; break;
        case '}': key = Key::RightBrace; break;
        case '~': key = Key::Tilde; break;
        default: ;
        }

        if (key == Key::Invalid)
        {
            break;
        }

        for (uint32_t i = 0; i < repeatCount; i++)
        {
            if (pImpl->m_onKey)
            {
                pImpl->m_onKey(key);
            }
        }

        return 0;
    }
    default: ;
    }

    return DefWindowProc(hwnd, msg, wParam, lParam);
}
#endif

bool Graphics::create(uint32_t width, uint32_t height,
                      const char* title,
                      string* pMsg)
{
    if (m_pImpl->m_init)
    {
        if (pMsg)
        {
            *pMsg = "Already initialized";
        }
        return false;
    }

    std::shared_ptr<GraphicsShared> pShared = GraphicsImpl::s_pShared.lock();
    if (!pShared)
    {
        pShared = GraphicsShared::initialize(pMsg);
        if (!pShared)
        {
            return false;
        }
        GraphicsImpl::s_pShared = pShared;
    }

#if PLATFORM_WINDOWS
    DWORD dwStyle = WS_OVERLAPPEDWINDOW;
    DWORD dwExStyle = 0;
    int createWidth = (int)width;
    int createHeight = (int)height;

    RECT rect;
    rect.left = 0;
    rect.top = 0;
    rect.right = (LONG)width;
    rect.bottom = (LONG)height;

    if (AdjustWindowRectEx(&rect, dwStyle, FALSE, dwExStyle))
    {
        createWidth = (int)(rect.right - rect.left);
        createHeight = (int)(rect.bottom - rect.top);
    }

    m_pImpl->m_hwnd = CreateWindowEx(
        dwExStyle,
        "Graphics",
        title ? title : "",
        dwStyle,
        CW_USEDEFAULT,
        CW_USEDEFAULT,
        createWidth,
        createHeight,
        NULL,
        NULL,
        pShared->m_hInstance,
        (LPVOID)m_pImpl.get()
        );
    if (!m_pImpl->m_hwnd)
    {
        if (pMsg)
        {
            *pMsg = "CreateWindowEx failed";
        }
        return false;
    }

    ShowWindow(m_pImpl->m_hwnd, SW_SHOW);
    UpdateWindow(m_pImpl->m_hwnd);
#else
    m_pImpl->m_paintRequested = false;
    m_pImpl->m_destroyed = false;

    m_pImpl->m_win =
        XCreateSimpleWindow(pShared->m_dis,
                            RootWindow(pShared->m_dis, pShared->m_screen),
                            0, 0,
                            (unsigned int)width, (unsigned int)height,
                            5,
                            pShared->m_whitePixel, pShared->m_blackPixel);
    Cleanup createCleanup([&]{ XDestroyWindow(pShared->m_dis, m_pImpl->m_win); });

    pShared->m_windowMap[m_pImpl->m_win] = m_pImpl.get();

    XTextProperty nameProperty;
    if (!title)
    {
        title = "";
    }
    if (XStringListToTextProperty((char**)&title, 1, &nameProperty) != 0)
    {
        XSetWMName(pShared->m_dis, m_pImpl->m_win, &nameProperty);
        XFree(nameProperty.value);
    }

    XSetWindowBackgroundPixmap(pShared->m_dis, m_pImpl->m_win, None);

    XSelectInput(pShared->m_dis, m_pImpl->m_win,
                 ExposureMask | KeyPressMask | ButtonPressMask | StructureNotifyMask);

    XSetWMProtocols(pShared->m_dis, m_pImpl->m_win,
                    &pShared->m_wmDeleteWindow, 1);

    m_pImpl->m_gc = XCreateGC(pShared->m_dis, m_pImpl->m_win, 0, nullptr);
    Cleanup gcCleanup([&]{ XFreeGC(pShared->m_dis, m_pImpl->m_gc); });

    XSetForeground(pShared->m_dis, m_pImpl->m_gc, pShared->m_whitePixel);
    XSetBackground(pShared->m_dis, m_pImpl->m_gc, pShared->m_blackPixel);

    XMapRaised(pShared->m_dis, m_pImpl->m_win);
    XFlush(pShared->m_dis);

    createCleanup.reset();
    gcCleanup.reset();
#endif

    m_pImpl->m_pShared = std::move(pShared);
    m_pImpl->m_init = true;

    return true;
}

bool Graphics::destroy()
{
    if (!m_pImpl->m_init)
    {
        return false;
    }

#if PLATFORM_WINDOWS
    DestroyWindow(m_pImpl->m_hwnd);
#else
    XFreeGC(m_pImpl->m_pShared->m_dis, m_pImpl->m_gc);
    m_pImpl->m_gc = GC();
    XDestroyWindow(m_pImpl->m_pShared->m_dis, m_pImpl->m_win);
    m_pImpl->m_win = Window();
    XFlush(m_pImpl->m_pShared->m_dis);
    m_pImpl->m_destroyed = true;
#endif

    return true;
}

uint32_t Graphics::getWidth() const
{
    return m_pImpl->m_width;
}

uint32_t Graphics::getHeight() const
{
    return m_pImpl->m_height;
}

bool Graphics::update(bool blocking)
{
    auto pShared = GraphicsImpl::s_pShared.lock();
    if (!pShared)
    {
        return false;
    }

#if PLATFORM_WINDOWS
    MSG msg;
    bool gotMessage =
        blocking ?
        (GetMessage(&msg, NULL, 0, 0) > 0) :
        (PeekMessage(&msg, NULL, 0, 0, PM_REMOVE) != 0);
    if (gotMessage)
    {
        if ((msg.message == WM_KEYDOWN &&
             (msg.wParam == VK_PROCESSKEY ||
              msg.wParam == VK_PACKET)) ||
            (msg.message == WM_SYSKEYDOWN &&
             msg.wParam == VK_SPACE))
        {
            TranslateMessage(&msg);
        }
        DispatchMessage(&msg);
    }
    return gotMessage;
#else
    if (XEventsQueued(pShared->m_dis, QueuedAfterFlush) <= 0)
    {
        for (auto& item : pShared->m_windowMap)
        {
            GraphicsImpl* pImpl = item.second;
            if (pImpl->m_paintRequested)
            {
                if (!pImpl->m_destroyed)
                {
                    if (pImpl->m_onPaint)
                    {
                        pImpl->m_onPaint();
                    }
                }
                pImpl->m_paintRequested = false;
                return true;
            }
        }

        if (!blocking)
        {
            return false;
        }
    }

    XEvent event;
    XNextEvent(pShared->m_dis, &event);

    GraphicsImpl* pImpl = nullptr;
    {
        auto it = pShared->m_windowMap.find(event.xany.window);
        if (it != pShared->m_windowMap.end())
        {
            pImpl = it->second;
        }
    }
    if (!pImpl)
    {
        return true;
    }

    switch (event.type)
    {
    case ConfigureNotify:
    {
        pImpl->m_width = (uint32_t)event.xconfigure.width;
        pImpl->m_height = (uint32_t)event.xconfigure.height;
        pImpl->m_pGraphics->requestPaint();
        break;
    }
    case DestroyNotify:
    {
        if (pImpl->m_onDestroy)
        {
            pImpl->m_onDestroy();
        }
        XFlush(pShared->m_dis);
        pShared->m_windowMap.erase(event.xany.window);
        pImpl->m_pShared.reset();
        pImpl->m_init = false;
        break;
    }
    case ClientMessage:
    {
        if (event.xclient.message_type == pShared->m_wmProtocols &&
            event.xclient.format == 32 &&
            (Atom)event.xclient.data.l[0] == pShared->m_wmDeleteWindow)
        {
            pImpl->m_pGraphics->destroy();
        }
        break;
    }
    case Expose:
    {
        pImpl->m_pGraphics->requestPaint();
        break;
    }
    case KeyPress:
    {
        KeySym keySym = XLookupKeysym(&event.xkey, 0);
        unsigned int state = event.xkey.state;

        bool bShift = ((state & ShiftMask) != 0);
        bool bCtrl = ((state & ControlMask) != 0);
        bool bAlt = ((state & Mod1Mask) != 0);
        bool bCapsLock = ((state & LockMask) != 0);
        bool bNumLock = ((state & Mod2Mask) != 0);
        Key key = Key::Invalid;

        switch (keySym)
        {
        case XK_BackSpace: key = Key::Backspace; break;
        case XK_Tab: key = Key::Tab; break;
        case XK_Return: key = Key::Enter; break;
        case XK_Escape: key = Key::Esc; break;
        case XK_space: key = Key::Space; break;
        case XK_Prior: key = Key::PgUp; break;
        case XK_Next: key = Key::PgDn; break;
        case XK_End: key = Key::End; break;
        case XK_Home: key = Key::Home; break;
        case XK_Left: key = Key::Left; break;
        case XK_Up: key = Key::Up; break;
        case XK_Right: key = Key::Right; break;
        case XK_Down: key = Key::Down; break;
        case XK_Insert: key = Key::Insert; break;
        case XK_Delete: key = Key::Delete; break;
        case XK_0: key = Key::Key0; break;
        case XK_1: key = Key::Key1; break;
        case XK_2: key = Key::Key2; break;
        case XK_3: key = Key::Key3; break;
        case XK_4: key = Key::Key4; break;
        case XK_5: key = Key::Key5; break;
        case XK_6: key = Key::Key6; break;
        case XK_7: key = Key::Key7; break;
        case XK_8: key = Key::Key8; break;
        case XK_9: key = Key::Key9; break;
        case XK_a: key = Key::A; break;
        case XK_b: key = Key::B; break;
        case XK_c: key = Key::C; break;
        case XK_d: key = Key::D; break;
        case XK_e: key = Key::E; break;
        case XK_f: key = Key::F; break;
        case XK_g: key = Key::G; break;
        case XK_h: key = Key::H; break;
        case XK_i: key = Key::I; break;
        case XK_j: key = Key::J; break;
        case XK_k: key = Key::K; break;
        case XK_l: key = Key::L; break;
        case XK_m: key = Key::M; break;
        case XK_n: key = Key::N; break;
        case XK_o: key = Key::O; break;
        case XK_p: key = Key::P; break;
        case XK_q: key = Key::Q; break;
        case XK_r: key = Key::R; break;
        case XK_s: key = Key::S; break;
        case XK_t: key = Key::T; break;
        case XK_u: key = Key::U; break;
        case XK_v: key = Key::V; break;
        case XK_w: key = Key::W; break;
        case XK_x: key = Key::X; break;
        case XK_y: key = Key::Y; break;
        case XK_z: key = Key::Z; break;
        case XK_F1: key = Key::F1; break;
        case XK_F2: key = Key::F2; break;
        case XK_F3: key = Key::F3; break;
        case XK_F4: key = Key::F4; break;
        case XK_F5: key = Key::F5; break;
        case XK_F6: key = Key::F6; break;
        case XK_F7: key = Key::F7; break;
        case XK_F8: key = Key::F8; break;
        case XK_F9: key = Key::F9; break;
        case XK_F10: key = Key::F10; break;
        case XK_F11: key = Key::F11; break;
        case XK_F12: key = Key::F12; break;
        case XK_semicolon: key = Key::Semicolon; break;
        case XK_equal: key = Key::Equals; break;
        case XK_comma: key = Key::Comma; break;
        case XK_minus: key = Key::Minus; break;
        case XK_period: key = Key::Period; break;
        case XK_slash: key = Key::Slash; break;
        case XK_quoteleft: key = Key::Backtick; break;
        case XK_bracketleft: key = Key::LeftBracket; break;
        case XK_backslash: key = Key::Backslash; break;
        case XK_bracketright: key = Key::RightBracket; break;
        case XK_apostrophe: key = Key::Apostrophe; break;

        case XK_KP_Prior:
            if (bNumLock) { key = Key::Key9; bShift = false; }
            else { key = Key::PgUp; } break;
        case XK_KP_Next:
            if (bNumLock) { key = Key::Key3; bShift = false; }
            else { key = Key::PgDn; } break;
        case XK_KP_End:
            if (bNumLock) { key = Key::Key1; bShift = false; }
            else { key = Key::End; } break;
        case XK_KP_Home:
            if (bNumLock) { key = Key::Key7; bShift = false; }
            else { key = Key::Home; } break;
        case XK_KP_Left:
            if (bNumLock) { key = Key::Key4; bShift = false; }
            else { key = Key::Left; } break;
        case XK_KP_Up:
            if (bNumLock) { key = Key::Key8; bShift = false; }
            else { key = Key::Up; } break;
        case XK_KP_Right:
            if (bNumLock) { key = Key::Key6; bShift = false; }
            else { key = Key::Right; } break;
        case XK_KP_Down:
            if (bNumLock) { key = Key::Key2; bShift = false; }
            else { key = Key::Down; } break;
        case XK_KP_Begin:
            if (bNumLock) { key = Key::Key5; bShift = false; }
            else { key = Key::Center; } break;
        case XK_KP_Insert:
            if (bNumLock) { key = Key::Key0; bShift = false; }
            else { key = Key::Insert; } break;
        case XK_KP_Delete:
            if (bNumLock) { key = Key::Period; bShift = false; }
            else { key = Key::Delete; } break;
        case XK_KP_Divide: key = Key::Slash; bShift = false; break;
        case XK_KP_Multiply: key = Key::Asterisk; bShift = false; break;
        case XK_KP_Subtract: key = Key::Minus; bShift = false; break;
        case XK_KP_Add: key = Key::Plus; bShift = false; break;
        case XK_KP_Enter: key = Key::Enter; break;

        default: ;
        }

        if (key == Key::Invalid)
        {
            break;
        }

        bool bAdjustedShift = bShift;
        if (bCapsLock && (key >= Key::A && key <= Key::Z))
        {
            bAdjustedShift = !bAdjustedShift;
        }

        if (bAdjustedShift)
        {
            key = (Key)((uint32_t)key | (uint32_t)Key::Shift);
        }

        if (bCtrl)
        {
            key = (Key)((uint32_t)key | (uint32_t)Key::Ctrl);
        }
        if (bAlt)
        {
            key = (Key)((uint32_t)key | (uint32_t)Key::Alt);
        }

        if (pImpl->m_onKey)
        {
            pImpl->m_onKey(key);
        }

        break;
    }
    case ButtonPress:
    {
        int32_t x = (int32_t)event.xbutton.x;
        int32_t y = (int32_t)event.xbutton.y;

        if (pImpl->m_onClick)
        {
            pImpl->m_onClick(x, y);
        }

        break;
    }
    default: ;
    }

    return true;
#endif
}

void Graphics::requestPaint()
{
#if PLATFORM_WINDOWS
    InvalidateRect(m_pImpl->m_hwnd, NULL, FALSE);
#else
    m_pImpl->m_paintRequested = true;
#endif
}

void Graphics::beginPaint(bool buffered)
{
    if (m_pImpl->m_painting)
    {
        return;
    }

#if PLATFORM_WINDOWS
    if (m_pImpl->m_hdcPaint)
    {
        m_pImpl->m_hdc = m_pImpl->m_hdcPaint;
    }
    else
    {
        m_pImpl->m_hdc = GetDC(m_pImpl->m_hwnd);
    }

    m_pImpl->m_buffered = false;
    if (buffered && m_pImpl->m_width > 0 && m_pImpl->m_height > 0)
    {
        HDC hdcBuf = CreateCompatibleDC(m_pImpl->m_hdc);
        if (hdcBuf)
        {
            HBITMAP hbmBuf = CreateCompatibleBitmap(m_pImpl->m_hdc,
                                                    (int)m_pImpl->m_width,
                                                    (int)m_pImpl->m_height);
            if (hbmBuf)
            {
                m_pImpl->m_hbmOld = (HBITMAP)SelectObject(hdcBuf, hbmBuf);
                m_pImpl->m_hbmBuf = hbmBuf;
                m_pImpl->m_hdcOld = m_pImpl->m_hdc;
                m_pImpl->m_hdc = hdcBuf;
                m_pImpl->m_buffered = true;
            }
            else
            {
                DeleteDC(hdcBuf);
            }
        }
    }
#else
    if (m_pImpl->m_destroyed)
    {
        return;
    }

    if (buffered && m_pImpl->m_width > 0 && m_pImpl->m_height > 0)
    {
        int depth = DefaultDepth(m_pImpl->m_pShared->m_dis,
                                 m_pImpl->m_pShared->m_screen);
        m_pImpl->m_pixmap = XCreatePixmap(m_pImpl->m_pShared->m_dis,
                                          m_pImpl->m_win,
                                          m_pImpl->m_width,
                                          m_pImpl->m_height,
                                          depth);
        m_pImpl->m_drawable = m_pImpl->m_pixmap;
        m_pImpl->m_buffered = true;
    }
    else
    {
        m_pImpl->m_drawable = m_pImpl->m_win;
        m_pImpl->m_buffered = false;
    }
#endif

    m_pImpl->m_painting = true;
}

void Graphics::endPaint()
{
    if (!m_pImpl->m_painting)
    {
        return;
    }

    m_pImpl->m_lineWidth = 0;
    m_pImpl->m_lineStyle = LineStyle::Solid;
    m_pImpl->m_lineCap = LineCap::Round;
    m_pImpl->m_lineJoin = LineJoin::Round;

    m_pImpl->m_fillMode = FillMode::Alternate;

    clearClip();

#if PLATFORM_WINDOWS
    if (m_pImpl->m_hfont)
    {
        DeleteObject(m_pImpl->m_hfont);
        m_pImpl->m_hfont = NULL;
    }

    if (m_pImpl->m_buffered)
    {
        BitBlt(m_pImpl->m_hdcOld,
               0, 0,
               (int)m_pImpl->m_width, (int)m_pImpl->m_height,
               m_pImpl->m_hdc,
               0, 0,
               SRCCOPY);
        SelectObject(m_pImpl->m_hdc, m_pImpl->m_hbmOld);
        m_pImpl->m_hbmOld = NULL;
        DeleteObject(m_pImpl->m_hbmBuf);
        m_pImpl->m_hbmBuf = NULL;
        DeleteDC(m_pImpl->m_hdc);
        m_pImpl->m_hdc = m_pImpl->m_hdcOld;
        m_pImpl->m_hdcOld = NULL;
    }

    if (m_pImpl->m_hdcPaint)
    {
        m_pImpl->m_hdc = NULL;
    }
    else
    {
        ReleaseDC(m_pImpl->m_hwnd, m_pImpl->m_hdc);
        m_pImpl->m_hdc = NULL;
    }
#else
    if (m_pImpl->m_pFontStruct)
    {
        XFreeFont(m_pImpl->m_pShared->m_dis, m_pImpl->m_pFontStruct);
        m_pImpl->m_pFontStruct = nullptr;
    }

    if (m_pImpl->m_buffered)
    {
        XCopyArea(m_pImpl->m_pShared->m_dis,
                  m_pImpl->m_pixmap,
                  m_pImpl->m_win,
                  m_pImpl->m_gc,
                  0, 0,
                  m_pImpl->m_width, m_pImpl->m_height,
                  0, 0);
        XFreePixmap(m_pImpl->m_pShared->m_dis,
                    m_pImpl->m_pixmap);
        m_pImpl->m_pixmap = Pixmap();
    }
    m_pImpl->m_drawable = Drawable();
    XFlush(m_pImpl->m_pShared->m_dis);
#endif

    m_pImpl->m_painting = false;
    m_pImpl->m_buffered = false;
}

namespace
{
#if PLATFORM_WINDOWS
    COLORREF toColorRef(uint32_t color)
    {
        return RGB((color >> 16) & 0xff,
                   (color >> 8) & 0xff,
                   color & 0xff);
    }

    HPEN createPen(GraphicsImpl* pImpl, COLORREF cr)
    {
        DWORD dwPenStyle = 0;
        DWORD dwWidth = 0;
        LOGBRUSH logBrush;
        memset(&logBrush, 0, sizeof(LOGBRUSH));

        if (pImpl->m_lineWidth == 0)
        {
            dwPenStyle |= PS_COSMETIC;

            dwWidth = 1;
        }
        else
        {
            dwPenStyle |= PS_GEOMETRIC;

            switch (pImpl->m_lineCap)
            {
            case LineCap::Round: dwPenStyle |= PS_ENDCAP_ROUND; break;
            case LineCap::Flat: dwPenStyle |= PS_ENDCAP_FLAT; break;
            case LineCap::Projecting: dwPenStyle |= PS_ENDCAP_SQUARE; break;
            }

            switch (pImpl->m_lineJoin)
            {
            case LineJoin::Round: dwPenStyle |= PS_JOIN_ROUND; break;
            case LineJoin::Miter: dwPenStyle |= PS_JOIN_MITER; break;
            case LineJoin::Bevel: dwPenStyle |= PS_JOIN_BEVEL; break;
            }

            dwWidth = (DWORD)pImpl->m_lineWidth;
        }

        switch (pImpl->m_lineStyle)
        {
        case LineStyle::Solid: dwPenStyle |= PS_SOLID; break;
        case LineStyle::Dash: dwPenStyle |= PS_DASH; break;
        }

        logBrush.lbStyle = BS_SOLID;
        logBrush.lbColor = cr;
        logBrush.lbHatch = 0;

        return ExtCreatePen(dwPenStyle,
                            dwWidth,
                            &logBrush,
                            0,
                            nullptr);
    }
#else
    unsigned long allocColor(GraphicsImpl* pImpl, uint32_t color)
    {
        auto it = pImpl->m_pShared->m_pixelMap.find(color);
        if (it != pImpl->m_pShared->m_pixelMap.end())
        {
            return it->second;
        }

        XColor xcolor;
        xcolor.pixel = 0;
        xcolor.red = ((color >> 16) & 0xff) * 0x101;
        xcolor.green = ((color >> 8) & 0xff) * 0x101;
        xcolor.blue = (color & 0xff) * 0x101;
        xcolor.flags = 0;
        if (XAllocColor(pImpl->m_pShared->m_dis,
                        pImpl->m_pShared->m_colormap,
                        &xcolor))
        {
            pImpl->m_pShared->m_pixels.push_back(xcolor.pixel);
        }
        else
        {
            xcolor.pixel = (color == 0x000000) ?
                pImpl->m_pShared->m_blackPixel :
                pImpl->m_pShared->m_whitePixel;
        }
        pImpl->m_pShared->m_pixelMap[color] = xcolor.pixel;
        return xcolor.pixel;
    }

    void setLineAttributes(GraphicsImpl* pImpl)
    {
        unsigned int lineWidth = (unsigned int)pImpl->m_lineWidth;
        int lineStyle = LineSolid;
        int capStyle = CapRound;
        int joinStyle = JoinRound;

        switch (pImpl->m_lineStyle)
        {
        case LineStyle::Solid: lineStyle = LineSolid; break;
        case LineStyle::Dash: lineStyle = LineOnOffDash; break;
        }

        switch (pImpl->m_lineCap)
        {
        case LineCap::Round: capStyle = CapRound; break;
        case LineCap::Flat: capStyle = CapButt; break;
        case LineCap::Projecting: capStyle = CapProjecting; break;
        }

        switch (pImpl->m_lineJoin)
        {
        case LineJoin::Round: joinStyle = JoinRound; break;
        case LineJoin::Miter: joinStyle = JoinMiter; break;
        case LineJoin::Bevel: joinStyle = JoinBevel; break;
        }

        XSetLineAttributes(pImpl->m_pShared->m_dis,
                           pImpl->m_gc,
                           lineWidth,
                           lineStyle,
                           capStyle,
                           joinStyle);

        if (pImpl->m_lineStyle == LineStyle::Dash)
        {
            uint32_t dashSize = pImpl->m_lineWidth;
            if (dashSize <= 0) { dashSize = 2; }
            if (dashSize > 32) { dashSize = 32; }
            dashSize *= 2;
            char dashList[2];
            dashList[0] = dashList[1] = (char)dashSize;

            XSetDashes(pImpl->m_pShared->m_dis,
                       pImpl->m_gc,
                       0,
                       dashList,
                       2);
        }
    }

    void setDefaultLineAttributes(GraphicsImpl* pImpl)
    {
        XSetLineAttributes(pImpl->m_pShared->m_dis,
                           pImpl->m_gc,
                           0,
                           LineSolid,
                           CapButt,
                           JoinMiter);

        if (pImpl->m_lineStyle == LineStyle::Dash)
        {
            char dashList[2] = { 4, 4 };
            XSetDashes(pImpl->m_pShared->m_dis,
                       pImpl->m_gc,
                       0,
                       dashList,
                       2);
        }
    }
#endif
}

void Graphics::drawPoint(int32_t x, int32_t y, uint32_t color)
{
    if (!m_pImpl->m_painting) { return; }

#if PLATFORM_WINDOWS
    COLORREF cr = toColorRef(color);
    SetPixel(m_pImpl->m_hdc, (int)x, (int)y, cr);
#else
    unsigned long pixel = allocColor(m_pImpl.get(), color);
    XSetForeground(m_pImpl->m_pShared->m_dis, m_pImpl->m_gc, pixel);
    XDrawPoint(m_pImpl->m_pShared->m_dis,
               m_pImpl->m_drawable,
               m_pImpl->m_gc,
               (int)x, (int)y);
    XSetForeground(m_pImpl->m_pShared->m_dis, m_pImpl->m_gc,
                   m_pImpl->m_pShared->m_whitePixel);
#endif
}

void Graphics::setLineWidth(uint32_t lineWidth)
{
    if (!m_pImpl->m_painting) { return; }

    m_pImpl->m_lineWidth = lineWidth;
}

void Graphics::setLineStyle(LineStyle lineStyle)
{
    if (!m_pImpl->m_painting) { return; }

    m_pImpl->m_lineStyle = lineStyle;
}

void Graphics::setLineCap(LineCap lineCap)
{
    if (!m_pImpl->m_painting) { return; }

    m_pImpl->m_lineCap = lineCap;
}

void Graphics::setLineJoin(LineJoin lineJoin)
{
    if (!m_pImpl->m_painting) { return; }

    m_pImpl->m_lineJoin = lineJoin;
}

void Graphics::drawLine(int32_t x1, int32_t y1, int32_t x2, int32_t y2,
                        uint32_t color)
{
    if (!m_pImpl->m_painting) { return; }

#if PLATFORM_WINDOWS
    COLORREF cr = toColorRef(color);
    HPEN hPen = createPen(m_pImpl.get(), cr);
    HPEN hPenOld = (HPEN)SelectObject(m_pImpl->m_hdc, hPen);

    MoveToEx(m_pImpl->m_hdc, (int)x1, (int)y1, nullptr);
    LineTo(m_pImpl->m_hdc, (int)x2, (int)y2);
    if (m_pImpl->m_lineWidth == 0)
    {
        SetPixel(m_pImpl->m_hdc, (int)x2, (int)y2, cr);
    }

    SelectObject(m_pImpl->m_hdc, hPenOld);
    DeleteObject(hPen);
#else
    setLineAttributes(m_pImpl.get());
    unsigned long pixel = allocColor(m_pImpl.get(), color);
    XSetForeground(m_pImpl->m_pShared->m_dis, m_pImpl->m_gc, pixel);
    XDrawLine(m_pImpl->m_pShared->m_dis,
              m_pImpl->m_drawable,
              m_pImpl->m_gc,
              (int)x1, (int)y1,
              (int)x2, (int)y2);
    XSetForeground(m_pImpl->m_pShared->m_dis, m_pImpl->m_gc,
                   m_pImpl->m_pShared->m_whitePixel);
    setDefaultLineAttributes(m_pImpl.get());
#endif
}

void Graphics::drawPolyline(const Point* points, uint32_t numPoints,
                            uint32_t color)
{
    if (!m_pImpl->m_painting) { return; }

    if (numPoints <= 0)
    {
        return;
    }

    if (numPoints == 1)
    {
        drawLine(points[0].x, points[0].y,
                 points[0].x, points[0].y,
                 color);
        return;
    }

    if (numPoints == 2)
    {
        drawLine(points[0].x, points[0].y,
                 points[1].x, points[1].y,
                 color);
        return;
    }

#if PLATFORM_WINDOWS
    std::unique_ptr<POINT[]> copiedPoints(new POINT[numPoints]);
    for (uint32_t iPoint = 0; iPoint < numPoints; iPoint++)
    {
        copiedPoints[iPoint].x = (LONG)points[iPoint].x;
        copiedPoints[iPoint].y = (LONG)points[iPoint].y;
    }

    COLORREF cr = toColorRef(color);
    HPEN hPen = createPen(m_pImpl.get(), cr);
    HPEN hPenOld = (HPEN)SelectObject(m_pImpl->m_hdc, hPen);

    Polyline(m_pImpl->m_hdc, &copiedPoints[0], (int)numPoints);

    SelectObject(m_pImpl->m_hdc, hPenOld);
    DeleteObject(hPen);
#else
    std::unique_ptr<XPoint[]> copiedPoints(new XPoint[numPoints]);
    for (uint32_t iPoint = 0; iPoint < numPoints; iPoint++)
    {
        copiedPoints[iPoint].x = (short)points[iPoint].x;
        copiedPoints[iPoint].y = (short)points[iPoint].y;
    }

    setLineAttributes(m_pImpl.get());
    unsigned long pixel = allocColor(m_pImpl.get(), color);
    XSetForeground(m_pImpl->m_pShared->m_dis, m_pImpl->m_gc, pixel);
    XDrawLines(m_pImpl->m_pShared->m_dis,
               m_pImpl->m_drawable,
               m_pImpl->m_gc,
               &copiedPoints[0],
               (int)numPoints,
               CoordModeOrigin);
    XSetForeground(m_pImpl->m_pShared->m_dis, m_pImpl->m_gc,
                   m_pImpl->m_pShared->m_whitePixel);
#endif
}

void Graphics::drawPolygon(const Point* points, uint32_t numPoints,
                           uint32_t color)
{
    if (!m_pImpl->m_painting) { return; }

    if (numPoints <= 0)
    {
        return;
    }

    if (numPoints == 1)
    {
        drawLine(points[0].x, points[0].y,
                 points[0].x, points[0].y,
                 color);
        return;
    }

    if (numPoints == 2)
    {
        drawLine(points[0].x, points[0].y,
                 points[1].x, points[1].y,
                 color);
        return;
    }

#if PLATFORM_WINDOWS
    std::unique_ptr<POINT[]> copiedPoints(new POINT[numPoints]);
    for (uint32_t iPoint = 0; iPoint < numPoints; iPoint++)
    {
        copiedPoints[iPoint].x = (LONG)points[iPoint].x;
        copiedPoints[iPoint].y = (LONG)points[iPoint].y;
    }

    COLORREF cr = toColorRef(color);
    HPEN hPen = createPen(m_pImpl.get(), cr);
    HPEN hPenOld = (HPEN)SelectObject(m_pImpl->m_hdc, hPen);
    HBRUSH hBrushOld =
        (HBRUSH)SelectObject(m_pImpl->m_hdc, GetStockObject(NULL_BRUSH));

    Polygon(m_pImpl->m_hdc, &copiedPoints[0], (int)numPoints);

    SelectObject(m_pImpl->m_hdc, hBrushOld);
    SelectObject(m_pImpl->m_hdc, hPenOld);
    DeleteObject(hPen);
#else
    std::unique_ptr<XPoint[]> copiedPoints(new XPoint[numPoints + 1]);
    for (uint32_t iPoint = 0; iPoint < numPoints; iPoint++)
    {
        copiedPoints[iPoint].x = (short)points[iPoint].x;
        copiedPoints[iPoint].y = (short)points[iPoint].y;
    }
    copiedPoints[numPoints].x = (short)points[0].x;
    copiedPoints[numPoints].y = (short)points[0].y;

    setLineAttributes(m_pImpl.get());
    unsigned long pixel = allocColor(m_pImpl.get(), color);
    XSetForeground(m_pImpl->m_pShared->m_dis, m_pImpl->m_gc, pixel);
    XDrawLines(m_pImpl->m_pShared->m_dis,
               m_pImpl->m_drawable,
               m_pImpl->m_gc,
               &copiedPoints[0],
               (int)(numPoints + 1),
               CoordModeOrigin);
    XSetForeground(m_pImpl->m_pShared->m_dis, m_pImpl->m_gc,
                   m_pImpl->m_pShared->m_whitePixel);
    setDefaultLineAttributes(m_pImpl.get());
#endif
}

void Graphics::drawRect(int32_t x, int32_t y, uint32_t width, uint32_t height,
                        uint32_t color)
{
    if (!m_pImpl->m_painting) { return; }

#if PLATFORM_WINDOWS
    COLORREF cr = toColorRef(color);
    HPEN hPen = createPen(m_pImpl.get(), cr);
    HPEN hPenOld = (HPEN)SelectObject(m_pImpl->m_hdc, hPen);
    HBRUSH hBrushOld =
        (HBRUSH)SelectObject(m_pImpl->m_hdc, GetStockObject(NULL_BRUSH));

    Rectangle(m_pImpl->m_hdc,
              (int)x, (int)y,
              (int)x + (int)width, (int)y + (int)height);

    SelectObject(m_pImpl->m_hdc, hBrushOld);
    SelectObject(m_pImpl->m_hdc, hPenOld);
    DeleteObject(hPen);
#else
    if (width == 0 || height == 0) return;
    if (width > 0) width--;
    if (height > 0) height--;

    setLineAttributes(m_pImpl.get());
    unsigned long pixel = allocColor(m_pImpl.get(), color);
    XSetForeground(m_pImpl->m_pShared->m_dis, m_pImpl->m_gc, pixel);
    XDrawRectangle(m_pImpl->m_pShared->m_dis,
                   m_pImpl->m_drawable,
                   m_pImpl->m_gc,
                   (int)x, (int)y,
                   (unsigned int)width, (unsigned int)height);
    XSetForeground(m_pImpl->m_pShared->m_dis, m_pImpl->m_gc,
                   m_pImpl->m_pShared->m_whitePixel);
    setDefaultLineAttributes(m_pImpl.get());
#endif
}

void Graphics::drawCircle(int32_t x, int32_t y,
                          uint32_t width, uint32_t height,
                          uint32_t color)
{
    if (!m_pImpl->m_painting) { return; }

#if PLATFORM_WINDOWS
    COLORREF cr = toColorRef(color);
    HPEN hPen = createPen(m_pImpl.get(), cr);
    HPEN hPenOld = (HPEN)SelectObject(m_pImpl->m_hdc, hPen);
    HBRUSH hBrushOld =
        (HBRUSH)SelectObject(m_pImpl->m_hdc, GetStockObject(NULL_BRUSH));

    Ellipse(m_pImpl->m_hdc,
            (int)x, (int)y,
            (int)x + (int)width, (int)y + (int)height);

    SelectObject(m_pImpl->m_hdc, hBrushOld);
    SelectObject(m_pImpl->m_hdc, hPenOld);
    DeleteObject(hPen);
#else
    if (width == 0 || height == 0) return;
    if (width > 0) width--;
    if (height > 0) height--;

    setLineAttributes(m_pImpl.get());
    unsigned long pixel = allocColor(m_pImpl.get(), color);
    XSetForeground(m_pImpl->m_pShared->m_dis, m_pImpl->m_gc, pixel);
    XDrawArc(m_pImpl->m_pShared->m_dis,
             m_pImpl->m_drawable,
             m_pImpl->m_gc,
             (int)x, (int)y,
             (unsigned int)width, (unsigned int)height,
             0, 64*360);
    XSetForeground(m_pImpl->m_pShared->m_dis, m_pImpl->m_gc,
                   m_pImpl->m_pShared->m_whitePixel);
    setDefaultLineAttributes(m_pImpl.get());
#endif
}

void Graphics::setFillMode(FillMode fillMode)
{
    if (!m_pImpl->m_painting) { return; }

    m_pImpl->m_fillMode = fillMode;
}

void Graphics::fillPolygon(const Point* points, uint32_t numPoints,
                           uint32_t color)
{
    if (!m_pImpl->m_painting) { return; }

    if (numPoints <= 0)
    {
        return;
    }

    if (numPoints == 1)
    {
        drawLine(points[0].x, points[0].y,
                 points[0].x, points[0].y,
                 color);
        return;
    }

    if (numPoints == 2)
    {
        drawLine(points[0].x, points[0].y,
                 points[1].x, points[1].y,
                 color);
        return;
    }

#if PLATFORM_WINDOWS
    std::unique_ptr<POINT[]> copiedPoints(new POINT[numPoints]);
    for (uint32_t iPoint = 0; iPoint < numPoints; iPoint++)
    {
        copiedPoints[iPoint].x = (LONG)points[iPoint].x;
        copiedPoints[iPoint].y = (LONG)points[iPoint].y;
    }

    COLORREF cr = toColorRef(color);
    HPEN hPenOld =
        (HPEN)SelectObject(m_pImpl->m_hdc, GetStockObject(DC_PEN));
    HBRUSH hBrushOld =
        (HBRUSH)SelectObject(m_pImpl->m_hdc, GetStockObject(DC_BRUSH));
    COLORREF crPenOld = SetDCPenColor(m_pImpl->m_hdc, cr);
    COLORREF crBrushOld = SetDCBrushColor(m_pImpl->m_hdc, cr);
    int fillMode =
        m_pImpl->m_fillMode == FillMode::Winding ? WINDING : ALTERNATE;
    int fillModeOld = SetPolyFillMode(m_pImpl->m_hdc, fillMode);

    Polygon(m_pImpl->m_hdc, &copiedPoints[0], (int)numPoints);

    SetPolyFillMode(m_pImpl->m_hdc, fillModeOld);
    SetDCBrushColor(m_pImpl->m_hdc, crBrushOld);
    SetDCPenColor(m_pImpl->m_hdc, crPenOld);
    SelectObject(m_pImpl->m_hdc, hBrushOld);
    SelectObject(m_pImpl->m_hdc, hPenOld);
#else
    std::unique_ptr<XPoint[]> copiedPoints(new XPoint[numPoints + 1]);
    for (uint32_t iPoint = 0; iPoint < numPoints; iPoint++)
    {
        copiedPoints[iPoint].x = (short)points[iPoint].x;
        copiedPoints[iPoint].y = (short)points[iPoint].y;
    }
    copiedPoints[numPoints].x = (short)points[0].x;
    copiedPoints[numPoints].y = (short)points[0].y;

    unsigned long pixel = allocColor(m_pImpl.get(), color);
    XSetForeground(m_pImpl->m_pShared->m_dis, m_pImpl->m_gc, pixel);
    if (m_pImpl->m_fillMode == FillMode::Winding)
    {
        XSetFillRule(m_pImpl->m_pShared->m_dis, m_pImpl->m_gc, WindingRule);
    }
    XFillPolygon(m_pImpl->m_pShared->m_dis,
                 m_pImpl->m_drawable,
                 m_pImpl->m_gc,
                 &copiedPoints[0],
                 (int)numPoints,
                 Complex,
                 CoordModeOrigin);
    XDrawLines(m_pImpl->m_pShared->m_dis,
               m_pImpl->m_drawable,
               m_pImpl->m_gc,
               &copiedPoints[0],
               (int)(numPoints + 1),
               CoordModeOrigin);
    if (m_pImpl->m_fillMode == FillMode::Winding)
    {
        XSetFillRule(m_pImpl->m_pShared->m_dis, m_pImpl->m_gc, EvenOddRule);
    }
    XSetForeground(m_pImpl->m_pShared->m_dis, m_pImpl->m_gc,
                   m_pImpl->m_pShared->m_whitePixel);
#endif
}

void Graphics::fillRect(int32_t x, int32_t y, uint32_t width, uint32_t height,
                        uint32_t color)
{
    if (!m_pImpl->m_painting) { return; }

#if PLATFORM_WINDOWS
    COLORREF cr = toColorRef(color);
    COLORREF crBrushOld = SetDCBrushColor(m_pImpl->m_hdc, cr);

    RECT rect;
    rect.left = (LONG)x;
    rect.top = (LONG)y;
    rect.right = (LONG)x + (LONG)width;
    rect.bottom = (LONG)y + (LONG)height;

    FillRect(m_pImpl->m_hdc, &rect, (HBRUSH)GetStockObject(DC_BRUSH));

    SetDCBrushColor(m_pImpl->m_hdc, crBrushOld);
#else
    if (width == 0 || height == 0) return;

    unsigned long pixel = allocColor(m_pImpl.get(), color);
    XSetForeground(m_pImpl->m_pShared->m_dis, m_pImpl->m_gc, pixel);
    XFillRectangle(m_pImpl->m_pShared->m_dis,
                   m_pImpl->m_drawable,
                   m_pImpl->m_gc,
                   (int)x, (int)y,
                   (unsigned int)width, (unsigned int)height);
    XSetForeground(m_pImpl->m_pShared->m_dis, m_pImpl->m_gc,
                   m_pImpl->m_pShared->m_whitePixel);
#endif
}

void Graphics::fillCircle(int32_t x, int32_t y,
                          uint32_t width, uint32_t height,
                          uint32_t color)
{
    if (!m_pImpl->m_painting) { return; }

#if PLATFORM_WINDOWS
    COLORREF cr = toColorRef(color);
    HPEN hPenOld =
        (HPEN)SelectObject(m_pImpl->m_hdc, GetStockObject(DC_PEN));
    HBRUSH hBrushOld =
        (HBRUSH)SelectObject(m_pImpl->m_hdc, GetStockObject(DC_BRUSH));
    COLORREF crPenOld = SetDCPenColor(m_pImpl->m_hdc, cr);
    COLORREF crBrushOld = SetDCBrushColor(m_pImpl->m_hdc, cr);

    Ellipse(m_pImpl->m_hdc,
            (int)x, (int)y,
            (int)x + (int)width, (int)y + (int)height);

    SetDCBrushColor(m_pImpl->m_hdc, crBrushOld);
    SetDCPenColor(m_pImpl->m_hdc, crPenOld);
    SelectObject(m_pImpl->m_hdc, hBrushOld);
    SelectObject(m_pImpl->m_hdc, hPenOld);
#else
    if (width == 0 || height == 0) return;
    if (width > 0) width--;
    if (height > 0) height--;

    unsigned long pixel = allocColor(m_pImpl.get(), color);
    XSetForeground(m_pImpl->m_pShared->m_dis, m_pImpl->m_gc, pixel);
    XFillArc(m_pImpl->m_pShared->m_dis,
             m_pImpl->m_drawable,
             m_pImpl->m_gc,
             (int)x, (int)y,
             (unsigned int)width, (unsigned int)height,
             0, 64*360);
    XDrawArc(m_pImpl->m_pShared->m_dis,
             m_pImpl->m_drawable,
             m_pImpl->m_gc,
             (int)x, (int)y,
             (unsigned int)width, (unsigned int)height,
             0, 64*360);
    XSetForeground(m_pImpl->m_pShared->m_dis, m_pImpl->m_gc,
                   m_pImpl->m_pShared->m_whitePixel);
#endif
}

void Graphics::drawBitmap(int32_t destX, int32_t destY,
                          const uint32_t* data,
                          uint32_t dataWidth, uint32_t dataHeight)
{
    if (!m_pImpl->m_painting) { return; }

    drawBitmap(destX, destY, data, dataWidth, dataHeight,
               0, 0, dataWidth, dataHeight);
}

void Graphics::drawBitmap(int32_t destX, int32_t destY,
                          const uint32_t* data,
                          uint32_t dataWidth, uint32_t dataHeight,
                          int32_t srcX, int32_t srcY,
                          uint32_t srcWidth, uint32_t srcHeight)
{
    if (!m_pImpl->m_painting) { return; }

    if (destX >= (int32_t)m_pImpl->m_width ||
        destX + (int32_t)srcWidth <= 0 ||
        destY >= (int32_t)m_pImpl->m_height ||
        destY + (int32_t)srcHeight <= 0 ||
        srcWidth <= 0 || srcHeight <= 0)
    {
        return;
    }

    if (destX < 0)
    {
        srcX += (-destX);
        srcWidth -= (uint32_t)(-destX);
        destX = 0;
    }
    if (srcWidth > m_pImpl->m_width)
    {
        srcWidth = m_pImpl->m_width;
    }
    if (destY < 0)
    {
        srcY += (-destY);
        srcHeight -= (uint32_t)(-destY);
        destY = 0;
    }
    if (srcHeight > m_pImpl->m_height)
    {
        srcHeight = m_pImpl->m_height;
    }

#if PLATFORM_WINDOWS
    BITMAPINFO bmi;
    BITMAPINFOHEADER& bi = bmi.bmiHeader;;
    bi.biSize = sizeof(BITMAPINFOHEADER);
    bi.biWidth = (LONG)dataWidth;
    bi.biHeight = -(LONG)dataHeight;
    bi.biPlanes = 1;
    bi.biBitCount = 32;
    bi.biCompression = BI_RGB;
    bi.biSizeImage = 0;
    bi.biXPelsPerMeter = 0;
    bi.biYPelsPerMeter = 0;
    bi.biClrUsed = 0;
    bi.biClrImportant = 0;

    SetDIBitsToDevice(m_pImpl->m_hdc,
                      (int)destX, (int)destY,
                      (int)srcWidth, (int)srcHeight,
                      (int)srcX, (int)dataHeight - (int)srcHeight - (int)srcY,
                      0,
                      (UINT)dataHeight,
                      data,
                      &bmi,
                      DIB_RGB_COLORS);
#else
    Visual* vis = DefaultVisual(m_pImpl->m_pShared->m_dis,
                                m_pImpl->m_pShared->m_screen);
    XImage* img = XCreateImage(m_pImpl->m_pShared->m_dis,
                               vis, 24, ZPixmap, 0,
                               (char*)(data + dataWidth * srcY + srcX),
                               (unsigned int)srcWidth, (unsigned int)srcHeight,
                               32, (int)dataWidth * 4);
    img->byte_order = LSBFirst;

    XPutImage(m_pImpl->m_pShared->m_dis,
              m_pImpl->m_drawable,
              m_pImpl->m_gc,
              img,
              0, 0,
              (int)destX, (int)destY,
              (unsigned int)srcWidth, (unsigned int)srcHeight);

    img->data = nullptr;
    XDestroyImage(img);
#endif
}

void Graphics::stretchBitmap(int32_t destX, int32_t destY,
                             uint32_t destWidth, uint32_t destHeight,
                             const uint32_t* data,
                             uint32_t dataWidth, uint32_t dataHeight)
{
    if (!m_pImpl->m_painting) { return; }

    stretchBitmap(destX, destY, destWidth, destHeight,
                  data, dataWidth, dataHeight,
                  0, 0, dataWidth, dataHeight);
}

void Graphics::stretchBitmap(int32_t destX, int32_t destY,
                             uint32_t destWidth, uint32_t destHeight,
                             const uint32_t* data,
                             uint32_t dataWidth, uint32_t dataHeight,
                             int32_t srcX, int32_t srcY,
                             uint32_t srcWidth, uint32_t srcHeight)
{
    if (!m_pImpl->m_painting) { return; }

    if (destWidth == srcWidth && destHeight == srcHeight)
    {
        drawBitmap(destX, destY,
                   data, dataWidth, dataHeight,
                   srcX, srcY, srcWidth, srcHeight);
        return;
    }

    if (destX >= (int32_t)m_pImpl->m_width ||
        destX + (int32_t)destWidth <= 0 ||
        destY >= (int32_t)m_pImpl->m_height ||
        destY + (int32_t)destHeight <= 0 ||
        destWidth <= 0 || destHeight <= 0)
    {
        return;
    }

#if PLATFORM_WINDOWS
    BITMAPINFO bmi;
    BITMAPINFOHEADER& bi = bmi.bmiHeader;;
    bi.biSize = sizeof(BITMAPINFOHEADER);
    bi.biWidth = (LONG)dataWidth;
    bi.biHeight = -(LONG)dataHeight;
    bi.biPlanes = 1;
    bi.biBitCount = 32;
    bi.biCompression = BI_RGB;
    bi.biSizeImage = 0;
    bi.biXPelsPerMeter = 0;
    bi.biYPelsPerMeter = 0;
    bi.biClrUsed = 0;
    bi.biClrImportant = 0;

    int oldStretchMode = SetStretchBltMode(m_pImpl->m_hdc, COLORONCOLOR);

    StretchDIBits(m_pImpl->m_hdc,
                  (int)destX, (int)destY,
                  (int)destWidth, (int)destHeight,
                  (int)srcX, (int)dataHeight - (int)srcHeight - (int)srcY,
                  (int)srcWidth, (int)srcHeight,
                  data,
                  &bmi,
                  DIB_RGB_COLORS,
                  SRCCOPY);

    SetStretchBltMode(m_pImpl->m_hdc, oldStretchMode);
#else
    uint32_t minX = (uint32_t)std::max(destX, 0);
    uint32_t maxX = (uint32_t)std::min(destX + (int32_t)destWidth,
                                       (int32_t)m_pImpl->m_width);
    uint32_t minY = (uint32_t)std::max(destY, 0);
    uint32_t maxY = (uint32_t)std::min(destY + (int32_t)destHeight,
                                       (int32_t)m_pImpl->m_height);
    uint32_t sizeX = maxX - minX;
    uint32_t sizeY = maxY - minY;
    uint32_t offsetX = (uint32_t)((int32_t)minX - (int32_t)destX);
    uint32_t offsetY = (uint32_t)((int32_t)minY - (int32_t)destY);

    std::unique_ptr<uint32_t[]> stretched(new uint32_t[sizeX * sizeY]);

    for (uint32_t stretchedY = 0; stretchedY < sizeY; stretchedY++)
    {
        uint32_t unstretchedY = (uint32_t)
            (((uint64_t)(stretchedY + offsetY) * (uint64_t)srcHeight) /
             (uint64_t)destHeight) + srcY;

        for (uint32_t stretchedX = 0; stretchedX < sizeX; stretchedX++)
        {
            uint32_t unstretchedX = (uint32_t)
                (((uint64_t)(stretchedX + offsetX) * (uint64_t)srcWidth) /
                 (uint64_t)destWidth) + srcX;

            stretched[stretchedY * sizeX + stretchedX] =
                data[unstretchedY * dataWidth + unstretchedX];
        }
    }

    drawBitmap((int32_t)minX, (int32_t)minY, stretched.get(), sizeX, sizeY);
#endif
}

void Graphics::setFont(FontSpacing spacing,
                       FontFace face,
                       uint32_t height,
                       bool bold,
                       bool italic)
{
    if (!m_pImpl->m_painting) { return; }

    if (height <= 0)
    {
        return;
    }

#if PLATFORM_WINDOWS
    if (m_pImpl->m_hfont)
    {
        DeleteObject(m_pImpl->m_hfont);
        m_pImpl->m_hfont = NULL;
    }

    LOGFONT logFont;
    memset(&logFont, 0, sizeof(LOGFONT));

    if (spacing == FontSpacing::Proportional)
    {
        if (face == FontFace::SansSerif)
        {
            strcpy(logFont.lfFaceName, "Arial");
        }
        else
        {
            strcpy(logFont.lfFaceName, "Georgia");
        }
    }
    else
    {
        if (face == FontFace::SansSerif)
        {
            strcpy(logFont.lfFaceName, "Lucida Console");
        }
        else
        {
            strcpy(logFont.lfFaceName, "Courier New");
        }
    }

    logFont.lfHeight = (LONG)height;
    logFont.lfWeight = bold ? FW_BOLD : FW_NORMAL;
    logFont.lfItalic = italic ? TRUE : FALSE;

    m_pImpl->m_hfont = CreateFontIndirect(&logFont);
    if (!m_pImpl->m_hfont)
    {
        return;
    }

    HFONT hFontOld = (HFONT)SelectObject(m_pImpl->m_hdc, m_pImpl->m_hfont);
    memset(&m_pImpl->m_textMetrics, 0, sizeof(TEXTMETRIC));
    GetTextMetrics(m_pImpl->m_hdc, &m_pImpl->m_textMetrics);
    SelectObject(m_pImpl->m_hdc, hFontOld);
#else
    if (m_pImpl->m_pFontStruct)
    {
        XFreeFont(m_pImpl->m_pShared->m_dis, m_pImpl->m_pFontStruct);
        m_pImpl->m_pFontStruct = nullptr;
    }

    char fontName[256];
    if (spacing == FontSpacing::Proportional)
    {
        snprintf(fontName, sizeof(fontName),
                 "-bitstream-bitstream charter-%s-%s-normal-*-%" PRIu32 "-*-*-*-*-*-iso8859-1",
                 bold ? "bold" : "medium",
                 italic ? "i" : "r",
                 height);
    }
    else
    {
        snprintf(fontName, sizeof(fontName),
                 "-bitstream-courier 10 pitch-%s-%s-normal-*-%" PRIu32 "-*-*-*-*-*-iso8859-1",
                 bold ? "bold" : "medium",
                 italic ? "i" : "r",
                 height);
    }
    m_pImpl->m_pFontStruct = XLoadQueryFont(m_pImpl->m_pShared->m_dis,
                                            fontName);
    if (!m_pImpl->m_pFontStruct)
    {
        return;
    }

    XSetFont(m_pImpl->m_pShared->m_dis,
             m_pImpl->m_gc,
             m_pImpl->m_pFontStruct->fid);
#endif
}

uint32_t Graphics::getFontHeight()
{
    if (!m_pImpl->m_painting) { return 0; }

#if PLATFORM_WINDOWS
    if (!m_pImpl->m_hfont)
    {
        return 0;
    }

    return (uint32_t)(m_pImpl->m_textMetrics.tmHeight +
                      m_pImpl->m_textMetrics.tmExternalLeading);
#else
    if (!m_pImpl->m_pFontStruct)
    {
        return 0;
    }

    return (uint32_t)(m_pImpl->m_pFontStruct->ascent +
                      m_pImpl->m_pFontStruct->descent);
#endif
}

uint32_t Graphics::getFontAscent()
{
    if (!m_pImpl->m_painting) { return 0; }

#if PLATFORM_WINDOWS
    if (!m_pImpl->m_hfont)
    {
        return 0;
    }

    return (uint32_t)(m_pImpl->m_textMetrics.tmAscent);
#else
    if (!m_pImpl->m_pFontStruct)
    {
        return 0;
    }

    return (uint32_t)(m_pImpl->m_pFontStruct->ascent);
#endif
}

uint32_t Graphics::getFontDescent()
{
    if (!m_pImpl->m_painting) { return 0; }

#if PLATFORM_WINDOWS
    if (!m_pImpl->m_hfont)
    {
        return 0;
    }

    return (uint32_t)(m_pImpl->m_textMetrics.tmDescent +
                      m_pImpl->m_textMetrics.tmExternalLeading);
#else
    if (!m_pImpl->m_pFontStruct)
    {
        return 0;
    }

    return (uint32_t)(m_pImpl->m_pFontStruct->descent);
#endif
}

uint32_t Graphics::getTextWidth(const char* str)
{
    if (!m_pImpl->m_painting) { return 0; }

    if (!str || !*str)
    {
        return 0;
    }

#if PLATFORM_WINDOWS
    if (!m_pImpl->m_hfont)
    {
        return 0;
    }

    SIZE size;
    memset(&size, 0, sizeof(SIZE));
    GetTextExtentPoint32(m_pImpl->m_hdc, str, (int)strlen(str), &size);
    return (uint32_t)size.cx;
#else
    if (!m_pImpl->m_pFontStruct)
    {
        return 0;
    }

    int width = XTextWidth(m_pImpl->m_pFontStruct, str, (int)strlen(str));
    return (uint32_t)width;
#endif
}

void Graphics::drawText(int32_t x, int32_t y,
                        const char* str, uint32_t color,
                        HAlign halign, VAlign valign)
{
    if (!m_pImpl->m_painting) { return; }

    if (!str || !*str)
    {
        return;
    }

#if PLATFORM_WINDOWS
    if (!m_pImpl->m_hfont)
    {
        return;
    }

    UINT textAlign = 0;

    if (halign == HAlign::Right)
    {
        textAlign |= TA_RIGHT;
    }
    else if (halign == HAlign::Center)
    {
        textAlign |= TA_CENTER;
    }
    else //if (halign == HAlign::Left)
    {
        textAlign |= TA_LEFT;
    }

    if (valign == VAlign::Bottom)
    {
        textAlign |= TA_BOTTOM;
    }
    else if (valign == VAlign::Baseline)
    {
        textAlign |= TA_BASELINE;
    }
    else //if (valign == VAlign::Top)
    {
        textAlign |= TA_TOP;
    }

    COLORREF cr = toColorRef(color);
    COLORREF crTextOld = SetTextColor(m_pImpl->m_hdc, cr);
    int oldBkMode = SetBkMode(m_pImpl->m_hdc, TRANSPARENT);
    HFONT hFontOld = (HFONT)SelectObject(m_pImpl->m_hdc, m_pImpl->m_hfont);
    UINT oldTextAlign = SetTextAlign(m_pImpl->m_hdc, textAlign);

    TextOut(m_pImpl->m_hdc, (int)x, (int)y, str, (int)strlen(str));

    SetTextAlign(m_pImpl->m_hdc, oldTextAlign);
    SelectObject(m_pImpl->m_hdc, hFontOld);
    SetBkMode(m_pImpl->m_hdc, oldBkMode);
    SetTextColor(m_pImpl->m_hdc, crTextOld);
#else
    if (!m_pImpl->m_pFontStruct)
    {
        return;
    }

    int len = (int)strlen(str);
    int drawX = (int)x;
    int drawY = (int)y;

    if (halign == HAlign::Right)
    {
        int width = XTextWidth(m_pImpl->m_pFontStruct, str, len);
        drawX -= width;
    }
    else if (halign == HAlign::Center)
    {
        int width = XTextWidth(m_pImpl->m_pFontStruct, str, len);
        drawX -= width / 2;
    }
    else //if (halign == HAlign::Left)
    {
    }

    if (valign == VAlign::Bottom)
    {
        drawY -= m_pImpl->m_pFontStruct->descent;
    }
    else if (valign == VAlign::Baseline)
    {
    }
    else //if (valign == VAlign::Top)
    {
        drawY += m_pImpl->m_pFontStruct->ascent;
    }

    unsigned long pixel = allocColor(m_pImpl.get(), color);
    XSetForeground(m_pImpl->m_pShared->m_dis, m_pImpl->m_gc, pixel);
    XDrawString(m_pImpl->m_pShared->m_dis,
                m_pImpl->m_drawable,
                m_pImpl->m_gc,
                drawX, drawY,
                str, len);
    XSetForeground(m_pImpl->m_pShared->m_dis, m_pImpl->m_gc,
                   m_pImpl->m_pShared->m_whitePixel);
#endif
}

void Graphics::setClipRect(int32_t x, int32_t y,
                           uint32_t width, uint32_t height)
{
    if (!m_pImpl->m_painting) { return; }

#if PLATFORM_WINDOWS
    HRGN hrgn = CreateRectRgn((int)x,
                              (int)y,
                              (int)x + (int)width,
                              (int)y + (int)height);
    if (hrgn)
    {
        SelectClipRgn(m_pImpl->m_hdc, hrgn);
        DeleteObject(hrgn);
    }
#else
    XRectangle rect;
    rect.x = (short)x;
    rect.y = (short)y;
    rect.width = (unsigned short)width;
    rect.height = (unsigned short)height;
    XSetClipRectangles(m_pImpl->m_pShared->m_dis,
                       m_pImpl->m_gc,
                       0, 0,
                       &rect, 1, YXBanded);
#endif
}

void Graphics::clearClip()
{
    if (!m_pImpl->m_painting) { return; }

#if PLATFORM_WINDOWS
    SelectClipRgn(m_pImpl->m_hdc, NULL);
#else
    XSetClipMask(m_pImpl->m_pShared->m_dis,
                 m_pImpl->m_gc,
                 None);
#endif
}
