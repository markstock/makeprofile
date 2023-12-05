//
// makeprofile - read a png dem/dsm and write a side profile
//
// (c)2023 Mark J. Stock <markjstock@gmail.com>
//

#include "memory.h"
#include "inout.h"
#include "CLI11.hpp"

#include <cassert>
#include <iostream>
#include <string>
#include <cmath>

constexpr double pi() { return std::atan(1)*4; }

// begin execution here

int main(int argc, char const *argv[]) {

  std::cout << "makeprofile v0.1\n";

  // process command line args
  CLI::App app{"Generate profile image from input dem/dsm"};

  // load a dem from a png file - check command line for file name
  std::string demfile = "in.png";
  app.add_option("-i,--input", demfile, "png DEM for elevations");

  // set output file name and size
  std::string outfile = "out.png";
  app.add_option("-o,--output", outfile, "png profile output");
  size_t ox = 1000;
  app.add_option("-x,--ox", ox, "number of pixels in horizontal direction (if no dem png is given)");
  size_t oy = 1000;
  app.add_option("-y,--oy", oy, "number of pixels in vertical direction (if no dem png is given)");

  // line definition
  float px = 0.5;
  app.add_option("--px", px, "horizontal position of line datum, 0..1, default 0.5 (center)");
  float py = 0.5;
  app.add_option("--py", py, "vertical position of line datum, 0..1, default 0.5 (center)");
  float alpha = 0.0;
  app.add_option("-a,--angle", alpha, "angle of line, degrees, 0=180=horizontal=default");

  // finally parse
  try {
    app.parse(argc, argv);
  } catch (const CLI::ParseError &e) {
    return app.exit(e);
  }


  //
  // read a png of elevations
  //

  std::cout << "Reading elevations from file (" << demfile << ")\n";

  // check the resolution first
  size_t nx, ny;
  {
  int hgt, wdt;
  (void) read_png_res (demfile.c_str(), &hgt, &wdt);
  if (wdt > 0) nx = wdt;
  if (hgt > 0) ny = hgt;
  }

  // allocate the space
  float** dem = allocate_2d_array_f((int)nx, (int)ny);

  // read the first channel into the elevation array, scaled as 0..vscale
  (void) read_png (demfile.c_str(), (int)nx, (int)ny, 0, 0, 0.0, 0,
                   dem, 0.0, 1.0, nullptr, 0.0, 1.0, nullptr, 0.0, 1.0);


  //
  // generate the profile
  //

  // find start and finish pixel positions
  // convert angle to radians and keep within 0..2pi range
  float alpharad = alpha/360.0f;
  alpharad = alpharad - floor(alpharad);
  alpharad = alpharad * pi() / 180.f;

  // where does a ray from the given datum position leave the volume (which of 4 sides)?
  const float topright = std::atan2(ny*(1.0f-py), nx*(1.0f-px));
  const float topleft  = std::atan2(ny*(1.0f-py), nx*(-px));
  const float botleft  = std::atan2(ny*(-py),     nx*(-px));
  const float botright = std::atan2(ny*(-py),     nx*(1.0f-px));
  printf("Angles to corners, CCW from +x: %g %g %g %g\n", topright, topleft, botleft, botright);

  // default is straight across the image
  float sx = 0.0;
  float sy = ny/2.0f;
  float fx = nx;
  float fy = ny/2.0f;

  // TODO: support arbitrary plane through the volume
  if (std::tan(alpharad) < ny*(1.0f-py)/(nx*(1.0f-px))) {
    // intersects the right edge
    fx = nx;
    fy = ny * (py + (1.f-px)*std::sin(alpharad));
  } else if (std::tan(alpharad) < ny*(1.f-py)/(nx*(1.f-px))) {
  } else {
    // intersects the right edge
  }
  
  // march along the line, setting elevation values
  float* profile = allocate_1d_array_f((int)ox);
  for (size_t i=0; i<ox; ++i) {
    const float wgt = (i+0.5f)/ox;
    const float tx = sx*(1.0f-wgt) + fx*wgt;
    const float ty = sy*(1.0f-wgt) + fy*wgt;
    // closest
    const size_t ix = std::max((size_t)0, std::min((size_t)(nx-1), (size_t)(tx+0.5f)));
    const size_t iy = std::max((size_t)0, std::min((size_t)(ny-1), (size_t)(ty+0.5f)));
    profile[i] = dem[ix][iy];
    // TODO: bi-linear interpolation
  }

  // free the dem
  free_2d_array_f(dem);

  //
  // generate the profile image
  //
  float** profimg = allocate_2d_array_f((int)ox, (int)oy);

  for (size_t i=0; i<ox; ++i) {
    const float yval = profile[i] * oy;
    const size_t yidx = yval+0.5;
    // TODO: linear interpolation
    for (size_t j=0; j<yidx; ++j) profimg[i][j] = 0.0;
    for (size_t j=yidx; j<oy; ++j) profimg[i][j] = 1.0;
  }

  // free the profile
  free_1d_array_f(profile);

  //
  // write the profile image
  //

  std::cout << "Writing dem to " << outfile << std::endl;

  float** data = allocate_2d_array_f((int)nx, (int)ny);

  (void) write_png (outfile.c_str(), (int)ox, (int)oy, FALSE, TRUE,
                    profimg, 0.0, 1.0, nullptr, 0.0, 1.0, nullptr, 0.0, 1.0);

  free_2d_array_f(data);

}
