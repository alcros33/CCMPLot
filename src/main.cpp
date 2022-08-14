#include "plot.hpp"
#include <algorithm>
#include <cairomm/context.h>
#include <cairomm/surface.h>
#include <cmath>
#include <iostream>
#include <muParser.h>
#include <string>

ccm::Float somefunc(ccm::Float x) { return 1 / (1 + x); }

int main(int argc, char** argv)
{
    if (argc < 2)
    {
        std::cout << "Please provide a function" << std::endl;
        return -1;
    }
    std::vector<std::string> args(argv, argv + argc);
    mu::Parser parser;
    double var = 1;
    parser.DefineVar("x", &var);
    parser.SetExpr(args[1]);

    auto F = [&parser, &var](ccm::Float x) -> ccm::Float {
        var = x;
        try
        {
            return parser.Eval();
        }
        catch (mu::Parser::exception_type& e)
        {
            std::cout << e.GetMsg() << std::endl;
            return NAN;
        }
    };

    double minX, maxX;
    char* end;
    size_t pos;
    if (args.size() >= 3)
    {
        minX = std::stod(args[2], &pos);
        maxX = std::strtod(args[2].c_str() + pos + 1, &end);
    }
    else
    {
        minX = -10, maxX = 10;
    }

    int steps = args.size() >= 5 ? std::stod(args[4]) : 10000;

    ccm::Plot P(700, 500);
    P.grid(true).plot(F, minX, maxX, steps);
    if (args.size() >= 4)
    {
        double minY = std::stod(args[3], &pos);
        double maxY = std::strtod(args[3].c_str() + pos + 1, &end);
        P.setYLim(minY, maxY);
    }
    std::string title("CCM Plot of ");
    P.show(title.append(args[1]));

    return 0;
}