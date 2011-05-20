vesuite.org\skewmatrix\examples\TextureSubload
Chris Hanson
2011-05-20

This is a basic example of how to update a subregion of a texture given an offset. It can be used to work with UI compositing.

Development environment (no idea on this for Chris):
- Microsoft Windows 7 Professional
- Microsoft Visual C++ 9/2008 Express + SP1
- OpenSceneGraph 2.8.3
- ATI MOBILITY RADEON 9600/9700 Series
- OpenGL 2.1
- GLSL 1.20

From Chris:
See attached. You can run with or without the -slow command line option to switch from
the fast subload path to the slow dirty() and reload it all path.

John cranked the base texture size up to 8k*8k to make a noticeable lag in the
non-subload texture dirty/reload. On my laptop, the 8k texture caused my machine to run
out of display memory (I have a lot fo things going). If you run into the problem, just
change the defines starting on line 28.

// vim: set sw=4 ts=8 et ic ai:
