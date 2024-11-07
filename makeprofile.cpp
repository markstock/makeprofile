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

// finding intersection points of a point-angle combination with the box boundaries

void findIntersection(const float px, const float py, const float alpha,
                      const float nx, const float ny,
                      float& x_intersect, float& y_intersect) {

  const float alpharad = std::fmod(alpha, 360.f) * M_PI / 180.0;

  // Check intersection with the right boundary
  if (alpharad < M_PI / 2 || alpharad > 3 * M_PI / 2) {
    x_intersect = nx;
    y_intersect = py + std::tan(alpharad) * (nx - px);
    //printf("  right bdry, testing %g %g\n", x_intersect, y_intersect);
    if (y_intersect >= 0 && y_intersect <= ny) {
      return;
    }
  } 

  // Check intersection with the left boundary
  if (alpharad > M_PI / 2 && alpharad < 3 * M_PI / 2) {
    x_intersect = 0;
    y_intersect = py - std::tan(alpharad) * px;
    //printf("  left bdry, testing %g %g\n", x_intersect, y_intersect);
    if (y_intersect >= 0 && y_intersect <= ny) {
      return;
    }
  }

  // Check intersection with the top boundary
  if (alpharad > 0 && alpharad < M_PI) {
    y_intersect = ny;
    x_intersect = px + (ny - py) / std::tan(alpharad);
    //printf("  top bdry, testing %g %g\n", x_intersect, y_intersect);
    if (x_intersect >= 0 && x_intersect <= nx) {
      return;
    }
  }

  // Check intersection with the bottom boundary
  if (alpharad > M_PI && alpharad < 2 * M_PI) {
    y_intersect = 0;
    x_intersect = px - py / std::tan(alpharad);
    //printf("  bottom bdry, testing %g %g\n", x_intersect, y_intersect);
    if (x_intersect >= 0 && x_intersect <= nx) {
      return;
    }
  }

  // If no intersection is found, return the original point (this should not happen with valid inputs)
  x_intersect = px;
  y_intersect = py;
  return;
}


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

  // default is straight across the image
  float sx = 0.0;
  float sy = ny/2.0f;
  float fx = nx;
  float fy = ny/2.0f;

  // we go left-to-right, which means alpha+180 first
  findIntersection(px*nx, py*ny, alpha+180.f, nx, ny, sx, sy);
  findIntersection(px*nx, py*ny, alpha, nx, ny, fx, fy);
  printf("  start and end points: %g %g %g %g\n", sx, sy, fx, fy);

  // march along the line, setting elevation values
  float* profile = allocate_1d_array_f((int)ox);
  for (size_t i=0; i<ox; ++i) {
    const float wgt = (i+0.5f)/ox;
    const float tx = sx*(1.0f-wgt) + fx*wgt;
    const float ty = sy*(1.0f-wgt) + fy*wgt;

    // closest
    //const size_t ix = std::max((size_t)0, std::min((size_t)(nx-1), (size_t)(tx+0.5f)));
    //const size_t iy = std::max((size_t)0, std::min((size_t)(ny-1), (size_t)(ty+0.5f)));
    //profile[i] = dem[ix][iy];

    // Bilinear interpolation
    const size_t x1 = std::max((size_t)0, std::min((size_t)(nx - 1), (size_t)tx));
    const size_t y1 = std::max((size_t)0, std::min((size_t)(ny - 1), (size_t)ty));
    const size_t x2 = std::min(nx - 1, x1 + 1);
    const size_t y2 = std::min(ny - 1, y1 + 1);

    const float x_diff = tx - x1;
    const float y_diff = ty - y1;

    profile[i] = dem[x1][y1] * (1 - x_diff) * (1 - y_diff) +
                 dem[x2][y1] *      x_diff  * (1 - y_diff) +
                 dem[x1][y2] * (1 - x_diff) *      y_diff +
                 dem[x2][y2] *      x_diff  *      y_diff;
  }

  // free the dem
  free_2d_array_f(dem);

  //
  // generate the profile image
  //
  float** profimg = allocate_2d_array_f((int)ox, (int)oy);

  for (size_t i=0; i<ox; ++i) {
    const float yval = profile[i] * oy;

    // nearest
    //const size_t yidx = yval+0.5;
    //for (size_t j=0; j<yidx; ++j) profimg[i][j] = 0.0;
    //for (size_t j=yidx; j<oy; ++j) profimg[i][j] = 1.0;

    // linear interpolation
    const float lefty = oy * ((i==0) ? (2.f*profile[0]-profile[1]) : profile[i-1]);
    const float righty = oy * ((i==ox-1) ? (2.f*profile[ox-1]-profile[ox-2]) : profile[i+1]);
    for (size_t j=0; j<oy; ++j) {
      // half of the value comes from where we are between left and middle y values
      const float ldist = ((j+0.5f)-(0.5f*(yval+lefty))) / (std::abs(yval-lefty) + 1.f);
      profimg[i][j] = (ldist > 0.5f) ? 1.f : ((ldist < -0.5f) ? 0.f : (0.5f+ldist));
      // other half the value comes from where we are between right and middle y values
      const float rdist = ((j+0.5f)-(0.5f*(yval+righty))) / (std::abs(yval-righty) + 1.f);
      profimg[i][j] += (rdist > 0.5f) ? 1.f : ((rdist < -0.5f) ? 0.f : (0.5f+rdist));
      profimg[i][j] *= 0.5f;
    }
  }

  // free the profile
  free_1d_array_f(profile);

  //
  // write the profile image
  //

  std::cout << "Writing dem to " << outfile << std::endl;

  (void) write_png (outfile.c_str(), (int)ox, (int)oy, FALSE, TRUE,
                    profimg, 0.0, 1.0, nullptr, 0.0, 1.0, nullptr, 0.0, 1.0);

  free_2d_array_f(profimg);

}
