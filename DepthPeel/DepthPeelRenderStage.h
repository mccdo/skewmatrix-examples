// Copyright (c) 2008 Skew Matrix Software LLC. All rights reserved.

#ifndef __DEPTH_PEEL_RENDER_STAGE_H__
#define __DEPTH_PEEL_RENDER_STAGE_H__ 1


#include <osg/Camera>
#include <osg/buffered_value>
#include <osgUtil/RenderStage>

#include <string>


// Define "OSG297" only if the OSG verion is at least v2.9.7.
// This code will also compile on 2.8.2.
#if defined( OSG_MIN_VERSION_REQUIRED )
#  if OSG_MIN_VERSION_REQUIRED( 2,9,7 )
#    define OSG297
#  else
     // OSG_MIN_VERSION_REQUIRED didn't exist until svn head
     // following the 2.9.6 release
#    define OSG297
#  endif
#else
#  undef OSG297
#endif



class DepthPeelGroup;

class DepthPeelRenderStage : public osgUtil::RenderStage
{
public:
    DepthPeelRenderStage();
    DepthPeelRenderStage( const osgUtil::RenderStage& rhs, const osg::CopyOp& copyop=osg::CopyOp::SHALLOW_COPY );
    DepthPeelRenderStage( const DepthPeelRenderStage& rhs, const osg::CopyOp& copyop=osg::CopyOp::SHALLOW_COPY );

    virtual osg::Object* cloneType() const { return new DepthPeelRenderStage(); }
    virtual osg::Object* clone(const osg::CopyOp& copyop) const { return new DepthPeelRenderStage( *this, copyop ); } // note only implements a clone of type.
    virtual bool isSameKindAs(const osg::Object* obj) const { return dynamic_cast<const DepthPeelRenderStage*>(obj)!=0L; }
    virtual const char* className() const { return "DepthPeelRenderStage"; }

    virtual void draw( osg::RenderInfo& renderInfo, osgUtil::RenderLeaf*& previous );

    void setDepthPeelGroup( DepthPeelGroup* dpg ) { _dpg = dpg; }
    DepthPeelGroup* getDepthPeelGroup() const { return _dpg; }

protected:
    ~DepthPeelRenderStage();

    void internalInit();
    void errorCheck( const std::string& msg );
    void checkFBOStatus( osg::FBOExtensions* fboExt );

    struct PerContextInfo; // forward
    void peelBegin( unsigned int pass, PerContextInfo& ctxInfo, osg::State& state, osg::FBOExtensions* fboExt, osg::GL2Extensions* gl2Ext );
    void peelEnd( unsigned int pass );

    osg::ref_ptr< DepthPeelGroup > _dpg;


    struct PerLayerInfo
    {
        PerLayerInfo();

        void init( unsigned int pass, const osg::State& state, const osg::Viewport* vp, const PerContextInfo& ctxInfo );
        bool _init;

        // TBD must be deleted.
        GLuint _fbo;
        GLuint _colorTex;
        GLuint _depthTex;
    };
    typedef std::vector< PerLayerInfo > PerLayerList;

    typedef void ( APIENTRY * ActiveTextureProc )( GLenum texture );

    typedef void ( APIENTRY * GenQueriesProc )( GLsizei n, GLuint *ids );
    typedef void ( APIENTRY * DeleteQueriesProc )( GLsizei n, const GLuint *ids );
    typedef GLboolean ( APIENTRY * IsQueryProc )( GLuint id );
    typedef void ( APIENTRY * BeginQueryProc )( GLenum target, GLuint id );
    typedef void ( APIENTRY * EndQueryProc )( GLenum target );
    typedef void ( APIENTRY * GetQueryivProc )( GLenum target, GLenum pname, GLint *params );
    typedef void ( APIENTRY * GetQueryObjectivProc )( GLuint id, GLenum pname, GLint *params );
    typedef void ( APIENTRY * GetQueryObjectuivProc )( GLuint id, GLenum pname, GLuint *params );
    typedef void ( APIENTRY * GetQueryObjectui64vProc )( GLuint id, GLenum pname, GLuint64EXT *params );

    struct PerContextInfo
    {
        PerContextInfo();

        void init( GLuint unit, const osg::Viewport* vp );
        bool _init;

        PerLayerList _layersList;

        unsigned int _maximumPasses;

        // TBD must be deleted.
        GLuint _queryID;
        GLuint _depthTex[ 3 ];

        ActiveTextureProc _glActiveTexture;

        GenQueriesProc _glGenQueries;
        DeleteQueriesProc _glDeleteQueries;
        BeginQueryProc _glBeginQuery;
        EndQueryProc _glEndQuery;
        GetQueryObjectivProc _glGetQueryObjectiv;
    };
    osg::buffered_object< PerContextInfo > _contextInfo;
};

// __DEPTH_PEEL_RENDER_STAGE_H__
#endif
