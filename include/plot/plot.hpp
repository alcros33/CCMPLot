#include <GLFW/glfw3.h>
#include <cairomm/context.h>
#include <cairomm/matrix.h>
#include <cairomm/surface.h>
#include <cmath>
#include <functional>
#include <map>
#include <optional>
#include <string>
#include <vector>
#ifdef _WIN32
#include <cairomm/win32_surface.h>
#define GLFW_EXPOSE_NATIVE_WIN32
#elif defined(__linux__)
#define GLFW_EXPOSE_NATIVE_X11
#endif
#include <GLFW/glfw3native.h>

namespace ccm
{

using Float = double;
using Vector = std::vector<Float>;
using Func = std::function<Float(Float)>;

const long double PI = 3.141592653589793238462643383279502884;

std::string to_string_truncate(Float f, int prec);
Float map(Float x, Float imin, Float imax, Float omin, Float omax);
Vector linspace(Float from, Float to, int points);

Float min(Float a, Float b);
Float max(Float a, Float b);
Float clamp(Float x, Float a, Float b);

struct Color
{
    Float r, g, b, a;
};

class Plot
{
public:
    Plot(int width, int height);

    Plot& plot(const Vector& x,
               const Vector& y,
               Color color = {0, 0, 1, 1},
               char marker = ' ',
               Float linewidth = 2.0);
    Plot& plot(Func F,
               Float minx,
               Float maxx,
               int points,
               Color color = {0, 0, 1, 1},
               char marker = ' ',
               Float linewidth = 2.0);
    Plot& setXLim(Float minx, Float maxx);
    Plot& setYLim(Float miny, Float maxy);
    Plot& grid(bool);

    void save(const std::string& fname);
    void show(const std::string& title);
    int width, height;

private:
    void render_to(Cairo::RefPtr<Cairo::Context> ctx, bool with_ticks);
    void render_grid(Cairo::RefPtr<Cairo::Context> ctx);
    void render_marker(Cairo::RefPtr<Cairo::Context> ctx, Float x, Float y, char marker);
    void render_ticks(Cairo::RefPtr<Cairo::Context> ctx);

    struct PlotDataSrcFunc
    {
        Func f;
        Float min, max;
        int points;
    };
    class PlotData
    {
    public:
        PlotData(
          Color color, Float linewidth, char marker, bool line, const Vector& x, const Vector& y);
        PlotData(Color color,
                 Float linewidth,
                 char marker,
                 bool line,
                 Func f,
                 Float minx,
                 Float maxx,
                 int points);
        Color m_color;
        Float m_linewidth;
        char m_marker;
        bool m_line;
        std::optional<PlotDataSrcFunc> m_f_data;
        Vector m_X, m_Y;
    };
    std::vector<PlotData> m_pd;

    bool m_grid{false};
    Float m_minx{INFINITY}, m_maxx{-INFINITY}, m_miny{INFINITY}, m_maxy{-INFINITY};
    double m_margin[4] = {10, 10, 50, 50};

    static void mouse_pressed_cb(GLFWwindow* window, int button, int action, int mods);
    static std::unordered_map<GLFWwindow*, Plot*>& window_to_instance();
    double m_pressedX, m_pressedY;
    double m_posX, m_posY;
    GLFWcursor* m_cursor;
    bool m_selection_square{false};
    bool m_one_time_render{true};
};
} // namespace ccm