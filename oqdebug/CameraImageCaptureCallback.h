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

// --- OSG Includes --- //
#include <osg/Camera>
#include <osg/Texture2D>

#include <string>


struct CameraImageCaptureCallback : public osg::Camera::DrawCallback
{
    CameraImageCaptureCallback( const std::string& filename, int w, int h, osg::Texture2D* tex );

    virtual void operator()( osg::RenderInfo& ri ) const;

protected:
    const std::string& m_filename;
    int width;
    int height;
    osg::ref_ptr< osg::Texture2D > m_tex;
};
