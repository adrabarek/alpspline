A library for generation of arc-length parametrized splines.

# Background

Most spline representations' parameters do not correspond to the length of the spline. For example,
a point travelling along the spline from `t = 0.1` to `t = 0.2` will NOT travel for 0.1 units of distance for most splines.

In many applications it is useful to have a way of parametrizing a spline so that a change in parameter corresponds
 directly to the distance travelled along the spline (i. e. arc length). For example, whenever a character in a video game needs to travel along a spline with constant speed, having the spline be arc-length parametrized makes the problem trivial - we can calculate the new position of the character by adding the distance it's supposed to travel in current frame to the spline parameter.. 

This is problem is known as arc-length parametrization. Most practical spline representations are not arc-length parametrized by default, and it's a problem that can have a significant impact on performance. It generally requires integration to get the relationship between arc length and the spline parameter. The solution is usually to perform the arc-length parametrization in an offline step.

This repository is a small library that performs the above, using method described in paper "Arc-length parametrized spline curves for real-time simulation" by Hongling Wang, Joseph K. Kearney and Kendall E. Atkinson. The gist of
it is that if we divide the spline into shorter splines, the shorter splines will generally be "more straight" (have smaller curvature), and therefore closer to being arc-parametrized. Knowing their lengths (that we need to use integration to calculate) we can trivially extrapolate using arc length. We can control the precision by changing the length of sub-splines.

# Building

The library itself (`src/alpspline`) has no dependencies, doesn't need building and can be directly compiled into your project. I only
tested in on GNU Linux (Debian), but it should work on other operating systems.

Building the demo requires Clang, git and make, and can only be done on GNU Linux.
This can be done by:
1. `./build_external.sh` - this will clone and build [Raylib](https://github.com/raysan5/raylib).
2. `./build_demo.sh` - builds and runs the demo.
This only works on GNU Linux and requires Clang to be installed on the system.

Demo looks like this:
![](resources/demo.gif)


