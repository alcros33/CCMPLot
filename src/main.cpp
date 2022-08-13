#define CCM_PLOT_WITH_WINDOW
#include "plot.hpp"
#include <algorithm>
#include <cairomm/context.h>
#include <cairomm/surface.h>
#include <cmath>
#include <iostream>
#include <numeric>
#include <string>

ccm::Float somefunc(ccm::Float x) { return x * x; }

int main()
{
    ccm::Plot P(700, 500);
    P.grid(true).plot(somefunc, -50, 50, 100, {0, 0, 1, 1});

    auto X = ccm::linspace(-35, -33, 1000);
    ccm::Vector Y(1000, 350);
    P.plot(X, Y, {1, 0, 0, 1}, 'o');

    P.show("Im a title");

    return 0;
}