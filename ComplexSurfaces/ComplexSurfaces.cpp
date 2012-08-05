/* OpenSceneGraph example, osgshaders.
*
*  Permission is hereby granted, free of charge, to any person obtaining a copy
*  of this software and associated documentation files (the "Software"), to deal
*  in the Software without restriction, including without limitation the rights
*  to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
*  copies of the Software, and to permit persons to whom the Software is
*  furnished to do so, subject to the following conditions:
*
*  THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
*  IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
*  FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
*  AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
*  LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
*  OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN
*  THE SOFTWARE.
*/

/* file:        examples/osgglsl/osgshaders.cpp
 * author:        Mike Weiblen 2005-04-05
 *
 * A demo of the OpenGL Shading Language shaders using core OSG.
 *
 * See http://www.3dlabs.com/opengl2/ for more information regarding
 * the OpenGL Shading Language.
*/

#include <osg/Notify>
#include <osgGA/GUIEventAdapter>
#include <osgGA/GUIActionAdapter>
#include <osgDB/ReadFile>
#include <osgDB/WriteFile>
#include <osgUtil/Optimizer>
#include <osgViewer/Viewer>

#include "GL2Scene.h"
#include "LightManipulator.h"

using namespace osg;

///////////////////////////////////////////////////////////////////////////

class KeyHandler: public osgGA::GUIEventHandler
{
    public:
        KeyHandler( GL2ScenePtr gl2Scene ) :
                _gl2Scene(gl2Scene)
        {}

        bool handle( const osgGA::GUIEventAdapter& ea, osgGA::GUIActionAdapter& )
        {
            if( ea.getEventType() != osgGA::GUIEventAdapter::KEYDOWN )
                return false;

            switch( ea.getKey() )
            {
                case 'x':
                    _gl2Scene->reloadShaderSource();
                    return true;
                case 'y':
                    return true;
            }
            return false;
        }

    private:
        GL2ScenePtr _gl2Scene;
};

///////////////////////////////////////////////////////////////////////////

int main(int, char **)
{
    // construct the viewer.
    osgViewer::Viewer viewer;
    viewer.setUpViewInWindow( 20, 30, 500, 500 );

    // create the scene
    GL2ScenePtr gl2Scene = new GL2Scene( DIRT );
    viewer.addEventHandler( new KeyHandler(gl2Scene) );

    osg::Node* model = gl2Scene->getRootNode().get();
    osgDB::writeNodeFile( *model, "surface.ive" );

    osg::Group* root = new osg::Group;
    root->addChild( model );

    osg::ref_ptr< LightManipulator > lightManip( new LightManipulator( .25f ) );
    root->addChild( lightManip->getLightSubgraph() );
    viewer.addEventHandler( lightManip.get() );

    viewer.setSceneData( root );

    return viewer.run();
}

/*EOF*/

