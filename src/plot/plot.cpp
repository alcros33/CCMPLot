#include "plot.hpp"
#include <iomanip>
#include <iostream>

namespace ccm
{

Float min(Float a, Float b) { return a < b ? a : b; }
Float max(Float a, Float b) { return a > b ? a : b; }
Float clamp(Float x, Float a, Float b) { return max(min(x, b), a); }

std::string to_string_truncate(Float f, int prec)
{
    std::stringstream ss;
    ss << std::fixed << std::setprecision(prec) << f;
    return ss.str();
}

Float map(Float x, Float imin, Float imax, Float omin, Float omax)
{
    return (x - imin) / (imax - imin) * (omax - omin) + omin;
}

Vector linspace(Float from, Float to, int points)
{
    Vector res(points);
    Float step = (to - from) / points;
    for (int i = 0; i < points; i++)
    {
        res[i] = from;
        from += step;
    }
    return res;
}

Plot::PlotData::PlotData(
  Color color, Float linewidth, char marker, bool line, Func f, Float minx, Float maxx, int points)
    : m_color(color)
    , m_linewidth(linewidth)
    , m_marker(marker)
    , m_line(line)
    , m_f_data(PlotDataSrcFunc{f, minx, maxx, points})
    , m_X(points)
    , m_Y(points)
{}

Plot::PlotData::PlotData(
  Color color, Float linewidth, char marker, bool line, const Vector& x, const Vector& y)
    : m_color(color)
    , m_linewidth(linewidth)
    , m_marker(marker)
    , m_line(line)
    , m_f_data()
    , m_X(x)
    , m_Y(y)
{}

Plot::Plot(int width, int height) : width(width), height(height) {}

Plot& Plot::grid(bool st)
{
    m_grid = st;
    return *this;
}

Plot& Plot::plot(const Vector& x, const Vector& y, Color color, char marker, Float linewidth)
{
    // maybe throw error
    if (x.size() != y.size() || x.empty())
        return *this;

    if (marker == ' ' && x.size() == 1)
        marker = 'o';

    m_pd.emplace_back(color, linewidth, marker, true, x, y);

    return *this;
}

Plot& Plot::plot(
  Func F, Float minx, Float maxx, int points, Color color, char marker, Float linewidth)
{
    m_pd.emplace_back(color, linewidth, marker, true, F, minx, maxx, points);
    return *this;
}

Plot& Plot::setXLim(Float minx, Float maxx)
{
    m_minx = minx, m_maxx = maxx;
    return *this;
}
Plot& Plot::setYLim(Float miny, Float maxy)
{
    m_miny = miny, m_maxy = maxy;
    return *this;
}

namespace inner
{
    bool is_invalid_value(Float x) { return x == NAN || x == INFINITY || x == -INFINITY; }
    bool asymptote_case(const Vector& Y, int idx, Float size)
    {
        if (idx == 0)
            return false;
        if (Y[idx] * Y[idx - 1] > 0)
            return false;
        return abs(Y[idx] - Y[idx - 1]) > size;
    }
} // namespace inner

void Plot::render_to(Cairo::RefPtr<Cairo::Context> ctx, bool with_ticks)
{
    // generate points for functions for the first time
    if (m_minx == INFINITY || m_miny == INFINITY)
    {
        for (auto& pd : m_pd)
        {
            if (!pd.m_f_data.has_value())
                continue;
            pd.m_X = linspace(
              pd.m_f_data.value().min, pd.m_f_data.value().max, pd.m_f_data.value().points);
            std::transform(pd.m_X.begin(), pd.m_X.end(), pd.m_Y.begin(), pd.m_f_data.value().f);
        }
    }
    // automatically detect bounds if not set
    if (m_minx == INFINITY)
    {
        for (auto& pd : m_pd)
        {
            for (auto& x : pd.m_X)
            {
                m_minx = min(x, m_minx);
                m_maxx = max(x, m_maxx);
            }
        }
    }
    if (m_miny == INFINITY)
    {
        for (auto& pd : m_pd)
        {
            for (auto& y : pd.m_Y)
            {
                m_miny = min(y, m_miny);
                m_maxy = max(y, m_maxy);
            }
        }
    }
    // white background
    if (with_ticks)
    {
        // generate points for functions
        for (auto& pd : m_pd)
        {
            if (!pd.m_f_data.has_value())
                continue;
            pd.m_X = linspace(max(pd.m_f_data.value().min, m_minx),
                              min(pd.m_f_data.value().max, m_maxx),
                              pd.m_f_data.value().points);
            std::transform(pd.m_X.begin(), pd.m_X.end(), pd.m_Y.begin(), pd.m_f_data.value().f);
        }
        ctx->save();
        ctx->set_source_rgb(1, 1, 1);
        ctx->paint();
        ctx->restore();
        render_ticks(ctx);
    }
    render_grid(ctx);

    Float x, y;
    ctx->save();
    double buffsizex = width - m_margin[1] - m_margin[3],
           buffsizey = height - m_margin[0] - m_margin[2];
    auto buff_surf =
      Cairo::ImageSurface::create(Cairo::Format::FORMAT_ARGB32, buffsizex, buffsizey);
    auto buff_ctx = Cairo::Context::create(buff_surf);
    Float sizey = m_maxy - m_miny;
    for (const auto& pd : m_pd)
    {
        buff_ctx->set_line_width(pd.m_linewidth);
        buff_ctx->set_source_rgba(pd.m_color.r, pd.m_color.g, pd.m_color.b, pd.m_color.a);
        if (pd.m_marker != ' ')
            for (size_t i = 0; i < pd.m_X.size(); i++)
            {
                if (inner::is_invalid_value(pd.m_X[i]) || inner::is_invalid_value(pd.m_Y[i]))
                    continue;
                x = map(pd.m_X[i], m_minx, m_maxx, 0, buffsizex);
                y = -map(pd.m_Y[i], m_miny, m_maxy, -buffsizey, 0);
                render_marker(buff_ctx, x, y, pd.m_marker);
            }
        if (pd.m_line)
        {
            for (size_t i = 0; i < pd.m_X.size(); i++)
            {
                if (inner::is_invalid_value(pd.m_X[i]) || inner::is_invalid_value(pd.m_Y[i]) ||
                    inner::asymptote_case(pd.m_Y, i, sizey))
                {
                    buff_ctx->stroke();
                    continue;
                }
                x = map(pd.m_X[i], m_minx, m_maxx, 0, buffsizex);
                y = -map(pd.m_Y[i], m_miny, m_maxy, -buffsizey, 0);
                buff_ctx->line_to(x, y);
            }
            buff_ctx->stroke();
        }
    }
    ctx->set_source(buff_surf, m_margin[3], m_margin[0]);
    ctx->paint();
    ctx->restore();
}

void Plot::render_grid(Cairo::RefPtr<Cairo::Context> ctx)
{
    ctx->save();
    ctx->set_line_width(1.0);
    // canvas square
    ctx->set_source_rgb(0, 0, 0);
    ctx->move_to(m_margin[3], m_margin[0]);
    ctx->line_to(width - m_margin[1], m_margin[0]);
    ctx->line_to(width - m_margin[1], height - m_margin[2]);
    ctx->line_to(m_margin[3], height - m_margin[2]);
    ctx->close_path();
    ctx->stroke();
    // canvas fill
    ctx->set_source_rgb(1, 1, 1);
    ctx->move_to(m_margin[3], m_margin[0]);
    ctx->line_to(width - m_margin[1], m_margin[0]);
    ctx->line_to(width - m_margin[1], height - m_margin[2]);
    ctx->line_to(m_margin[3], height - m_margin[2]);
    ctx->close_path();
    ctx->fill();
    ctx->restore();

    if (!m_grid)
        return;
    int n_ticks = 8;
    Float stepsizey = (height - (m_margin[0] + m_margin[2])) / (n_ticks - 1);
    Float stepsizex = (width - (m_margin[1] + m_margin[3])) / (n_ticks - 1);
    ctx->set_source_rgb(0, 0, 0);
    for (int i = 0; i < n_ticks; i++)
    {
        ctx->set_line_width(0.5);
        ctx->move_to(m_margin[3], m_margin[0] + i * stepsizey);
        ctx->line_to(width - m_margin[1], m_margin[0] + i * stepsizey);
        ctx->stroke();

        ctx->move_to(m_margin[3] + i * stepsizex, height - m_margin[2]);
        ctx->line_to(m_margin[3] + i * stepsizex, m_margin[0]);
        ctx->stroke();
    }
}

void Plot::render_ticks(Cairo::RefPtr<Cairo::Context> ctx)
{
    int n_ticks = 8;
    ctx->save();
    ctx->set_source_rgb(0, 0, 0);
    ctx->select_font_face(
      "serif", Cairo::FontSlant::FONT_SLANT_NORMAL, Cairo::FontWeight::FONT_WEIGHT_NORMAL);
    ctx->set_font_size(12);
    Float stepsizey = (height - (m_margin[0] + m_margin[2])) / (n_ticks - 1);
    Float stepsizex = (width - (m_margin[1] + m_margin[3])) / (n_ticks - 1);
    Float mark;
    for (int i = 0; i < n_ticks; i++)
    {
        ctx->set_line_width(1.0);
        ctx->move_to(m_margin[3], m_margin[0] + i * stepsizey);
        ctx->line_to(m_margin[3] - 10, m_margin[0] + i * stepsizey);
        ctx->stroke();

        ctx->move_to(m_margin[3] + i * stepsizex, height - m_margin[2]);
        ctx->line_to(m_margin[3] + i * stepsizex, height - m_margin[2] + 10);
        ctx->stroke();

        ctx->move_to(m_margin[3] - 45, m_margin[0] + i * stepsizey + 5);
        mark = map(m_margin[0] + (n_ticks - i - 1) * stepsizey,
                   m_margin[0],
                   height - m_margin[2],
                   m_miny,
                   m_maxy);
        ctx->show_text(to_string_truncate(mark, 2));

        ctx->move_to(m_margin[3] + i * stepsizex - 10, height - m_margin[2] + 30);
        ctx->show_text(to_string_truncate(
          map(m_margin[3] + i * stepsizex, m_margin[3], width - m_margin[1], m_minx, m_maxx), 2));
    }
    ctx->begin_new_path();
    ctx->restore();
}

void Plot::render_marker(Cairo::RefPtr<Cairo::Context> ctx, Float x, Float y, char marker)
{
    if (marker == ' ')
        return;
    switch (marker)
    {
    case 'o':
        ctx->arc(x, y, 3, 0, 2 * PI);
        ctx->fill();
        break;
    // TODO add other markers like *, ^
    default:
        break;
    }
}

void Plot::save(const std::string& fname)
{
    auto surface = Cairo::ImageSurface::create(Cairo::Format::FORMAT_ARGB32, width, height);
    auto ctx = Cairo::Context::create(surface);
    render_to(ctx, true);
    surface->write_to_png(fname);
}

} // namespace ccm