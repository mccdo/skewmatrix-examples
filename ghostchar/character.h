// Copyright (c) 2011 Skew Matrix Software LLC. All rights reserved.

#ifndef __CHARACTER_H__
#define __CHARACTER_H__ 1


#include <osg/Group>
#include <osg/ref_ptr>
#include <string>


class btDynamicsWorld;
class btKinematicCharacterController;
class btPairCachingGhostObject;
class btConvexShape;


class Character
{
public:
    Character( btDynamicsWorld* bw );
    ~Character();

    /**
    \param transform If true, model is assumed to be oriented +y up / +z forward,
    and requiring transformation to +z up / +y forward. Pass false if the model is
    already oriented +z up / +y forward.
    */
    osg::Group* setModel( const std::string& fileName, bool transform=true );

    void setModelVisible( bool visible=true );
    bool getModelVisible() const;

    void setCapsuleVisible( bool visible=true );
    bool getCapsuleVisible() const;

    void setHeight( double height );
    double getHeight() const;

    void setPhysicsWorldTransform( const osg::Matrix& m );

    void setMatrix( const osg::Matrix& m );

    osg::Vec3 getPosition() const;

protected:
    osg::ref_ptr< osg::MatrixTransform > _root;
    osg::Node* _model;
    osg::Node* _capsule;

    double _capsuleRadius;
    double _capsuleHeight;

    void generateCapsule();


    osg::Vec3 _lastPosition;


    btDynamicsWorld* _bw;
    btKinematicCharacterController* _btChar;
    btPairCachingGhostObject* _btGhost;
    btConvexShape* _capsuleShape;
};


//  __CHARACTER_H__
#endif
