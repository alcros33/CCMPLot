#include "plot.hpp"

#include <iostream>

namespace ccm
{

std::unordered_map<GLFWwindow*, Plot*>& Plot::window_to_instance()
{
    static std::unordered_map<GLFWwindow*, Plot*> _window_to_instance;
    return _window_to_instance;
}

void Plot::mouse_pressed_cb(GLFWwindow* window, int button, int action, int mods)
{
    auto p = Plot::window_to_instance()[window];
    if (button == GLFW_MOUSE_BUTTON_LEFT && action == GLFW_PRESS)
    {
        glfwGetCursorPos(window, &p->m_pressedX, &p->m_pressedY);
        if (p->m_pressedX > p->width - p->m_margin[1] || p->m_pressedX < p->m_margin[3] ||
            p->m_pressedY > p->height - p->m_margin[2] || p->m_pressedY < p->m_margin[0])
            return;
        p->m_selection_square = true;
    }
    else if (button == GLFW_MOUSE_BUTTON_LEFT && action == GLFW_RELEASE)
    {
        if (!p->m_selection_square)
            return;
        glfwGetCursorPos(window, &p->m_posX, &p->m_posY);
        p->m_selection_square = false;
        double tmin, tmax;
        if (p->m_posX < p->m_pressedX)
            tmin = p->m_posX, tmax = p->m_pressedX;
        else
            tmin = p->m_pressedX, tmax = p->m_posX;
        tmin = map(tmin, p->m_margin[3], p->width - (p->m_margin[1]), p->m_minx, p->m_maxx);
        tmax = map(tmax, p->m_margin[3], p->width - (p->m_margin[1]), p->m_minx, p->m_maxx);
        p->m_minx = tmin;
        p->m_maxx = tmax;
        if (p->m_posY > p->m_pressedY)
            tmin = p->m_posY, tmax = p->m_pressedY;
        else
            tmin = p->m_pressedY, tmax = p->m_posY;
        tmin = map(-tmin, -p->height + (p->m_margin[2]), -(p->m_margin[0]), p->m_miny, p->m_maxy);
        tmax = map(-tmax, -p->height + (p->m_margin[2]), -(p->m_margin[0]), p->m_miny, p->m_maxy);
        p->m_miny = tmin;
        p->m_maxy = tmax;
        p->m_one_time_render = true;
    }
}

void Plot::show(const std::string& title)
{
    if (!glfwInit())
        return;

    glfwWindowHint(GLFW_CLIENT_API, GLFW_NO_API);
    GLFWwindow* window = glfwCreateWindow(width, height, title.c_str(), nullptr, nullptr);
    Plot::window_to_instance()[window] = this;
    m_cursor = glfwCreateStandardCursor(GLFW_CROSSHAIR_CURSOR);
    glfwSetCursor(window, m_cursor);
    glfwSetMouseButtonCallback(window, mouse_pressed_cb);

    if (!window)
    {
        glfwTerminate();
        return;
    }
    glfwSwapInterval(1);

    glfwGetFramebufferSize(window, &width, &height);
#ifdef _WIN32
    auto win32_window = glfwGetWin32Window(window);
    auto surface = Cairo::Win32Surface::create(GetDC(win32_window));
#elif defined(__linux__)
    Display* x11_display = glfwGetX11Display();
    Window x11_window = glfwGetX11Window(window);
    Visual* vis = DefaultVisual(x11_display, DefaultScreen(x11_display));
    auto surface = Cairo::XlibSurface::create(x11_display, x11_window, vis, width, height);
#endif
    int newwitdh, newheight;
    auto ctx = Cairo::Context::create(surface);
    render_to(ctx, true);
    while (!glfwWindowShouldClose(window))
    {
        // on framebuffer size change, resize the surface
        glfwGetFramebufferSize(window, &newwitdh, &newheight);
        if (width != newwitdh || height != newheight)
        {
            width = newwitdh, height = newheight;
#ifdef _WIN32
            surface = Cairo::Win32Surface::create(GetDC(win32_window));
#endif
            ctx = Cairo::Context::create(surface);
            m_one_time_render = true;
        }
        render_to(ctx, m_one_time_render);
        if (m_one_time_render)
            m_one_time_render = false;
        glfwGetCursorPos(window, &m_posX, &m_posY);
        if (m_selection_square)
        {
            m_posX = clamp(m_posX, m_margin[3], width - m_margin[1]);
            m_posY = clamp(m_posY, m_margin[0], height - m_margin[2]);
            render_to(ctx, false);
            ctx->save();
            ctx->set_source_rgba(0, 0, 0, 0.5);
            ctx->move_to(m_pressedX, m_pressedY);
            ctx->line_to(m_pressedX, m_posY);
            ctx->line_to(m_posX, m_posY);
            ctx->line_to(m_posX, m_pressedY);
            ctx->close_path();
            ctx->fill();
            ctx->restore();
        }
        surface->flush();
        glfwSwapBuffers(window);
        glfwPollEvents();
    }
    Plot::window_to_instance().erase(window);
    surface.clear();
    ctx.clear();
    glfwDestroyCursor(m_cursor);
    glfwDestroyWindow(window);
    glfwTerminate();
}
} // namespace ccm