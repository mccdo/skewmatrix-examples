vesuite.org\skewmatrix\examples\AutoTransform2
Mike Weiblen mike.weiblen@gmail.com
2010-08-09

AutoTransform2 is a variation of the OpenSceneGraph "osg/AutoTransform"
node that replaces the CPU-based functionality with a GLSL shader-based
implementation.

Development environment:
- Microsoft Windows 7 Professional
- Microsoft Visual C++ 9/2008 Express + SP1
- OpenSceneGraph 2.8.3
- ATI MOBILITY RADEON 9600/9700 Series
- OpenGL 2.1
- GLSL 1.20

In this implementation, the functionality of the AutoTransform2 is not
yet encapsulated into a class; rather the osg::Program and associated
uniforms are left exposed.

The "at2demo" loads a model, and instances it twice in the scenegraph:
once under the AT2 GLSL program, and the other without.  Moving about
the scene will demonstrate the AT2 program:
1) rotating the model to maintain a fixed orientation
2) compensating to preserve a fixed screen size.

// vim: set sw=4 ts=8 et ic ai:
