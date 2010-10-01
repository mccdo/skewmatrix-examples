/*************** <auto-copyright.rb BEGIN do not edit this line> **************
 *
 * VE-Suite is (C) Copyright 1998-2010 by Iowa State University
 *
 * Original Development Team:
 *   - ISU's Thermal Systems Virtual Engineering Group,
 *     Headed by Kenneth Mark Bryden, Ph.D., www.vrac.iastate.edu/~kmbryden
 *   - Reaction Engineering International, www.reaction-eng.com
 *
 * This library is free software; you can redistribute it and/or
 * modify it under the terms of the GNU Library General Public
 * License as published by the Free Software Foundation; either
 * version 2 of the License, or (at your option) any later version.
 *
 * This library is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * Library General Public License for more details.
 *
 * You should have received a copy of the GNU Library General Public
 * License along with this library; if not, write to the
 * Free Software Foundation, Inc., 59 Temple Place - Suite 330,
 * Boston, MA 02111-1307, USA.
 *
 * -----------------------------------------------------------------
 * Date modified: $Date: 2010-09-29 20:49:01 -0500 (Wed, 29 Sep 2010) $
 * Version:       $Rev: 14907 $
 * Author:        $Author: mccdo $
 * Id:            $Id: CameraImageCaptureCallback.cxx 14907 2010-09-30 01:49:01Z mccdo $
 * -----------------------------------------------------------------
 *
 *************** <auto-copyright.rb END do not edit this line> ***************/

#include "CameraImageCaptureCallback.h"

// --- OSG Includes --- //
#include <osg/Image>
#include <osg/io_utils>
#include <osg/Texture2D>

#include <osgDB/WriteFile>
#include <osgDB/FileUtils>

#include <iostream>


////////////////////////////////////////////////////////////////////////////////
CameraImageCaptureCallback::CameraImageCaptureCallback(
    const std::string& filename, int w, int h, osg::Texture2D* tex )
    :
    m_filename( filename ),
    width( w ),
    height( h ),
    m_tex( tex )
{
    ;
}
////////////////////////////////////////////////////////////////////////////////
void CameraImageCaptureCallback::operator()( osg::RenderInfo& ri ) const
{
    //std::cout << ri.getCurrentCamera() << std::endl;
    //m_tex->copyTexImage2D( *ri.getState(), 0, 0, 1024, 1024 );

    osg::Image* image = m_tex->getImage();
    std::string fName( m_filename );

    osg::notify( osg::ALWAYS ) << "Reading image for file " 
        << fName << " ... " << std::endl;
    //const osg::Viewport* vp = ri.getState()->getCurrentViewport();    
    //image->readPixels( 0, 0, width, height, GL_RGBA, GL_UNSIGNED_BYTE );
    
    //image->readPixels( 0, 0, width, height, GL_RGBA, GL_UNSIGNED_BYTE );
    image->readImageFromCurrentTexture( ri.getContextID(), false, GL_UNSIGNED_BYTE );
    osg::notify( osg::ALWAYS ) << "Writing file " 
        << fName << " ... " << std::endl;
    osgDB::writeImageFile( *image, fName );
    osg::notify( osg::ALWAYS ) << "Capture complete." << std::endl;
}
////////////////////////////////////////////////////////////////////////////////