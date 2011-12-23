// Copyright (c) 2011 Skew Matrix Software LLC. All rights reserved.

#ifndef __CHARACTER_H__
#define __CHARACTER_H__ 1


#include <osg/Group>
#include <osg/ref_ptr>
#include <string>


class Character
{
public:
    Character();
    ~Character();

    /**
    \param transform If true, model is assumed to be oriented +y up / +z forward,
    and requiring transformation to +z up / +y forward. Pass false if the model is
    already oriented +z up / +y forward.
    */
    osg::Group* setModel( const std::string& fileName, bool transform=true );

    void setModelVisible( bool visible=true ) { _modelVisible = visible; }
    bool getModelVisible() const { return( _modelVisible ); }

    void setCapsuleVisible( bool visible=true ) { _capsuleVisible = visible; }
    bool getCapsuleVisible() const { return( _capsuleVisible ); }

    void setHeight( double height );
    double getHeight() const;

    void setMatrix( const osg::Matrix m );

protected:
    osg::ref_ptr< osg::MatrixTransform > _root;
    osg::Node* _model;
    osg::Node* _capsule;

    bool _modelVisible;
    bool _capsuleVisible;

    double _capsuleRadius;
    double _capsuleHeight;

    void generateCapsule();
};


//  __CHARACTER_H__
#endif
