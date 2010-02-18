// Copyright (c) 2010 Skew Matrix Software LLC. All rights reserved.

#ifndef __REMOVE_SOUND_VISITOR_H__
#define __REMOVE_SOUND_VISITOR_H__ 1


#include <osg/NodeVisitor>

#include <osgAudio/SoundNode.h>
#include <osgAudio/SoundUpdateCB.h>

#include <osgwTools/InsertRemove.h>

#include "SoundUtilities.h"


class RemoveSoundVisitor : public osg::NodeVisitor
{
public:
	RemoveSoundVisitor()
      : osg::NodeVisitor( osg::NodeVisitor::TRAVERSE_ALL_CHILDREN )
	{}

	virtual void apply( osg::Node& node )
    {
        osgAudio::SoundNode* sn( dynamic_cast< osgAudio::SoundNode* >( &node ) );
        if( sn != NULL )
        {
            // TBD If this is a SoundNode, remove it.
            osg::notify( osg::WARN ) << "RemoveSoundVisitor: Removing SoundNode is not yet implemented." << std::endl;
        }
        else
        {
            // Remove any attached SoundUpdateCB.
            SoundUtilities::instance()->removeSound( &node );
        }

        traverse( node );
	}

protected:
};


// __REMOVE_SOUND_VISITOR_H__
#endif
