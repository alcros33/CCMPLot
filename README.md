# CCM Plot 
A plot library for c++.

## Dependencies
* muparser
* cairomm
* cairo

### Installation on Debians
`# apt-get install libcairomm-1.0-dev libglfw3-dev libmuparser-dev`

### Instalation on Archs
`# pacman -S cairomm glfw-x11 muparser`

### Instalation on Msys2
`$ pacman -S mingw-w64-x86_64-cairomm mingw-w64-x86_64-glfw mingw-w64-x86_64-muparser`

### Compilation
`$ make release`

or 

`$ make debug`

### Usage
`$ plot expression[ minX,maxX[ minY,maxY[ numsteps]]]`

## Examples
`$ plot "sin(x)"`

`$ plot "x^2" -10,10`

`$ plot "1/(x+1)" -10,10 -100,100`