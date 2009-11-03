// Copyright (c) 2008 Skew Matrix Software LLC. All rights reserved.

#include <osgDB/ReadFile>
#include <osgDB/WriteFile>
#include <osgDB/FileUtils>
#include <osgViewer/Viewer>
#include <osgViewer/ViewerEventHandlers>

#include <osg/Geometry>
#include <osg/Texture3D>
#include <osg/Texture1D>
#include <osg/Uniform>
#include <osg/ClipPlane>

#include <osg/io_utils>


// (Some of the) GL 3 enums not defined by OSG.
#define GL_CLIP_DISTANCE0                 0x3000
#define GL_MAX_CLIP_DISTANCES             0x0D32


class KeyHandler : public osgGA::GUIEventHandler
{
public:
    KeyHandler( osg::Uniform* modulo, osg::Uniform* planeOn )
      : _modulo( modulo ),
        _planeOn( planeOn )
    {}

    virtual bool handle( const osgGA::GUIEventAdapter& ea, osgGA::GUIActionAdapter& aa )
    {
        bool handled = false;
        switch( ea.getEventType() )
        {
            case ( osgGA::GUIEventAdapter::KEYDOWN ):
            {
                const bool ctrl( ( ea.getModKeyMask() & osgGA::GUIEventAdapter::MODKEY_CTRL ) != 0 );
                const int key = ea.getKey();
                const int keyv( key - '0' );
                if( (keyv > 0) && (keyv < 10) )
                {
                    if( ctrl )
                    {
                        // ctrl-1 thru ctrl-6 toggle enable of clip planes.
                        int value;
                        _planeOn->getElement( keyv-1, value );
                        _planeOn->setElement( keyv-1, (value==1) ? 0 : 1 );
                        handled = true;
                    }
                    else
                    {
                        // Keys '1' through '9': draw every Nth vector.
                        // Key '1' draws all vectors.
                        _modulo->set( (float)keyv );
                        handled = true;
                    }
                }
            }
        }
        return( handled );
    }

private:
    osg::ref_ptr< osg::Uniform > _modulo;
    osg::ref_ptr< osg::Uniform > _planeOn;
};


class FindVectorDataVisitor : public osg::NodeVisitor
{
public:
    std::string _texSizeName;

    FindVectorDataVisitor()
      : osg::NodeVisitor( osg::NodeVisitor::TRAVERSE_ALL_CHILDREN ),
        _dataSize( 0 )
    {
        _texSizeName = std::string( "sizes" );
    }

    void apply( osg::Node& node )
    {
        parse( node.getStateSet() );
        traverse( node );
    }
    void apply( osg::Geode& node )
    {
        parse( node.getStateSet() );
        parse( node.getDrawableList() );
        traverse( node );
    }

    osg::ref_ptr< osg::Texture3D > _tex0;
    osg::ref_ptr< osg::Texture3D > _tex1;
    osg::ref_ptr< osg::Texture3D > _tex2;
    unsigned int _dataSize;
    osg::Vec3 _texSizes;
    osg::BoundingBox _bb;

protected:
    void parse( osg::StateSet* ss )
    {
        if( ss == NULL )
            return;

        if( _tex0 == NULL )
            _tex0 = dynamic_cast< osg::Texture3D* >(
                ss->getTextureAttribute( 0, osg::StateAttribute::TEXTURE ) );
        if( _tex1 == NULL )
            _tex1 = dynamic_cast< osg::Texture3D* >(
                ss->getTextureAttribute( 1, osg::StateAttribute::TEXTURE ) );
        if( _tex2 == NULL )
            _tex2 = dynamic_cast< osg::Texture3D* >(
                ss->getTextureAttribute( 2, osg::StateAttribute::TEXTURE ) );

        osg::Uniform* uniform = ss->getUniform( _texSizeName );
        if( uniform != NULL )
            uniform->get( _texSizes );
    }
    void parse( const osg::Geode::DrawableList& dl )
    {
        if( _dataSize > 1 )
            return;

        osg::Geode::DrawableList::const_iterator dlItr;
        for( dlItr = dl.begin(); dlItr != dl.end(); dlItr++ )
        {
            const osg::Geometry* geom = dynamic_cast< const osg::Geometry* >( (*dlItr).get() );
            if( geom == NULL )
                continue;

            parse( const_cast< osg::StateSet* >( geom->getStateSet() ) );

            _bb = geom->getBound();

            const osg::PrimitiveSet* ps = geom->getPrimitiveSet( 0 );
            if( ps == NULL )
                return;

            _dataSize = ps->getNumInstances();
            if( _dataSize > 1 )
                break;
        }
    }
};

// Base class for abstracting vector field data storage
class VectorFieldData : public osg::Referenced
{
public:
    VectorFieldData()
      : _pos( NULL ),
        _dir( NULL ),
        _scalar( NULL )
    {}

    void loadData()
    {
        internalLoad();
    }

    osg::Texture3D* getPositionTexture()
    {
        return( _texPos.get() );
    }
    osg::Texture3D* getDirectionTexture()
    {
        return( _texDir.get() );
    }
    osg::Texture3D* getScalarTexture()
    {
        return( _texScalar.get() );
    }

    osg::Vec3s getTextureSizes()
    {
        return( _texSizes );
    }
    unsigned int getDataCount()
    {
        return( _dataSize );
    }

    // Make sure you set the bounding box when you load your data.
    osg::BoundingBox getBoundingBox()
    {
        return( _bb );
    }

    // Call this to restore from file, OR call loadData to
    // generate or load raw data.
    void restoreData( osg::Node* node )
    {
        FindVectorDataVisitor fvdv;
        node->accept( fvdv );
        _texPos = fvdv._tex0;
        osg::notify( osg::ALWAYS ) << "  " << std::hex << _texPos << std::endl;
        _texDir = fvdv._tex1;
        osg::notify( osg::ALWAYS ) << "  " << _texDir << std::endl;
        _texScalar = fvdv._tex2;
        osg::notify( osg::ALWAYS ) << "  " << _texScalar << std::endl;
        _dataSize = fvdv._dataSize;
        osg::notify( osg::ALWAYS ) << "  " << std::dec << _dataSize << std::endl;
        _texSizes = osg::Vec3s( fvdv._texSizes.x(), fvdv._texSizes.y(), fvdv._texSizes.z() );
        osg::notify( osg::ALWAYS ) << "  " << _texSizes << std::endl;
        _bb = fvdv._bb;
        osg::notify( osg::ALWAYS ) << "  " << _bb._min << std::endl;
        osg::notify( osg::ALWAYS ) << "  " << _bb._max << std::endl;
    }

protected:
    osg::ref_ptr< osg::Texture3D > _texPos, _texDir, _texScalar;
    osg::Vec3s _texSizes;
    unsigned int _dataSize;

    float* _pos;
    float* _dir;
    float* _scalar;

    osg::BoundingBox _bb;

    virtual ~VectorFieldData()
    {
        //Let osg handle the memory for the textures with the NEW DELETE
        //setting on the image
        ;
    }

    // You must override this to load your data and create textures from that data.
    // The code below is intended only as a template/example. See also
    // MyVectorFieldData::internalLoad().
    virtual void internalLoad()
    {
        // TBD Actual data size would come from file.
        _dataSize = 0;

        // Determine optimal 3D texture dimensions.
        int s, t, p;
        compute3DTextureSize( getDataCount(), s, t, p );
        _texSizes = osg::Vec3s( s, t, p );

        // Allocate memory for data.
        unsigned int size( s*t*p );
        _pos = new float[ size * 3 ];
        _dir = new float[ size * 3 ];
        _scalar = new float[ size ];

        // TBD You would replace this line with code to load the data from file.
        // In this example, we just generate test data.

        _texPos = makeFloatTexture( (unsigned char*)_pos, 3, osg::Texture2D::NEAREST );
        _texDir = makeFloatTexture( (unsigned char*)_dir, 3, osg::Texture2D::NEAREST );
        _texScalar = makeFloatTexture( (unsigned char*)_scalar, 1, osg::Texture2D::NEAREST );
    }


    //
    // The following set of protected member functions exist in support
    // of your probable implementation of internalLoad. You can use them
    // if you wish, but you are not required to do so.
    //

    //
    // ceilPower2 - Originally in OpenGL Distilled example source code.
    //
    // Return next highest power of 2 greater than x
    // if x is a power of 2, return x.
    static unsigned short ceilPower2( unsigned short x )
    {
        if( x == 0 )
            return( 1 );

        if( (x & (x-1)) == 0 )
            // x is a power of 2.
            return( x );

        x |= x >> 1;
        x |= x >> 2;
        x |= x >> 4;
        x |= x >> 8;
        return( x+1 );
    }

    // Given we need to store 'dataCount' values in a texture, compute
    // optimal 3D texture dimensions large enough to hold those values.
    // (This might not be the best implementation. Cubed root would
    // come in handy here.)
    static void compute3DTextureSize( unsigned int dataCount, int& s, int& t, int& p )
    {
        // GL_MAX_3D_TEXTURE_SIZE min value is 16.
        // NVIDIA seems to support 2048.
        s = 256;
        while( dataCount / s == 0 )
            s >>= 1;

        float sliceSize( (float) dataCount / (float) s );
        float tDim = sqrtf( sliceSize );
        if( tDim > 65535.f )
            osg::notify( osg::FATAL ) << "compute3DTextureSize: Value too large " << tDim << std::endl;
        if( tDim == (int)tDim )
            t = ceilPower2( (unsigned short)( tDim ) );
        else
            t = ceilPower2( (unsigned short)( tDim ) + 1 );

        float pDim = sliceSize / ((float)t);
        if( pDim == ((int)sliceSize) / t)
            p = ceilPower2( (unsigned short)( pDim ) );
        else
            p = ceilPower2( (unsigned short)( pDim ) + 1 );
        osg::notify( osg::ALWAYS ) << "dataCount " << dataCount <<
            " produces tex size (" << s << "," << t << "," << p <<
            "), total storage: " << s*t*p << std::endl;
    }

    // Creates a 3D texture containing floating point somponents.
    osg::Texture3D* makeFloatTexture( unsigned char* data, int numComponents, osg::Texture::FilterMode filter )
    {
        int s( _texSizes.x() );
        int t( _texSizes.y() );
        int p( _texSizes.z() );

        GLenum intFormat, pixFormat;
        if( numComponents == 1 )
        {
            intFormat = GL_ALPHA32F_ARB;
            pixFormat = GL_ALPHA;
        }
        else
        {
            // Must be 3 for now.
            intFormat = GL_RGB32F_ARB;
            pixFormat = GL_RGB;
        }
        osg::Image* image = new osg::Image;
        image->setImage( s, t, p, intFormat, pixFormat, GL_FLOAT,
            data, osg::Image::USE_NEW_DELETE );
        osg::Texture3D* texture = new osg::Texture3D( image );
        texture->setFilter( osg::Texture::MIN_FILTER, filter );
        texture->setFilter( osg::Texture::MAG_FILTER, filter );
        return( texture );
    }
};

// Derived class for testing purposes. generates data at runtime.
class MyVectorFieldData : public VectorFieldData
{
public:
    MyVectorFieldData()
      : VectorFieldData()
    {
        /*
        // For testing
        int idx, s, t, p;
        for( idx=1; idx<17; idx++)
            compute3DTextureSize( idx, s, t, p );
        */
    }

protected:
    osg::Vec3 _sizes;

    virtual ~MyVectorFieldData()
    {}

    virtual void internalLoad()
    {
        // Actual data size would come from file.
        // NOTE: Crash in NVIDIA friver if total _dataSize
        // is > 32768.
        _sizes = osg::Vec3( 30, 37, 26 );
        _dataSize = _sizes.x() * _sizes.y() * _sizes.z();

        // Determine optimal 3D texture dimensions.
        int s, t, p;
        compute3DTextureSize( getDataCount(), s, t, p );
        _texSizes = osg::Vec3s( s, t, p );

        // Allocate memory for data.
        unsigned int size( s*t*p );
        _pos = new float[ size * 3 ];
        _dir = new float[ size * 3 ];
        _scalar = new float[ size ];

        // TBD You would replace this line with code to load the data from file.
        // In this example, we just generate test data.
        createDataArrays( _pos, _dir, _scalar );

        _texPos = makeFloatTexture( (unsigned char*)_pos, 3, osg::Texture2D::NEAREST );
        _texDir = makeFloatTexture( (unsigned char*)_dir, 3, osg::Texture2D::NEAREST );
        _texScalar = makeFloatTexture( (unsigned char*)_scalar, 1, osg::Texture2D::NEAREST );

        // Must set the bounding box.
        {
            float x0, y0, z0;
            getPosition( 0, 0, 0, x0, y0, z0 );
            float x1, y1, z1;
            getPosition( _sizes.x(), _sizes.y(), _sizes.z(), x1, y1, z1 );
            _bb = osg::BoundingBox( x0, y0, z0, x1, y1, z1 );
        }

    }

    void getPosition( int m, int n, int o, float& x, float& y, float& z )
    {
        const float center( 15.5f );
        x = ( m - center );
        y = ( n - center );
        z = ( o - center );
    }

    void createDataArrays( float* pos, float* dir, float* scalar )
    {
        float* posI = pos;
        float* dirI = dir;
        float* scalarI = scalar;

        int mIdx, nIdx, oIdx;
        for( mIdx = 0; mIdx < _sizes.x(); mIdx++ )
        {
            for( nIdx = 0; nIdx < _sizes.y(); nIdx++ )
            {
                for( oIdx = 0; oIdx < _sizes.z(); oIdx++ )
                {
                    float x, y, z;
                    getPosition( mIdx, nIdx, oIdx, x, y, z );
                    *posI++ = x;
                    *posI++ = y;
                    *posI++ = z;

                    float yzLen( sqrtf( y*y + z*z ) );
                    *scalarI++ = yzLen / 21.9f;

                    float xD;
                    if( yzLen < 1.f )
                        xD = 25.f;
                    else
                        xD = 3.f/yzLen;
                    float yD = y * -0.1f;
                    float zD = z * -0.1f;
                    float len( sqrtf( xD*xD + yD*yD + zD*zD ) );
                    *dirI++ = xD/len;
                    *dirI++ = yD/len;
                    *dirI++ = zD/len;
                }
            }
        }
    }
};
class DebugVectorFieldData : public VectorFieldData
{
public:
    DebugVectorFieldData()
      : VectorFieldData()
    {}

protected:
    virtual ~DebugVectorFieldData()
    {}

    virtual void internalLoad()
    {
        _dataSize = 12;

        int s, t, p;
        compute3DTextureSize( getDataCount(), s, t, p );
        _texSizes = osg::Vec3s( s, t, p );

        // Allocate memory for data.
        unsigned int size( 16 );
        _pos = new float[ size * 3 ];
        _dir = new float[ size * 3 ];
        _scalar = new float[ size ];

        createDataArrays( _pos, _dir, _scalar );

        _texPos = makeFloatTexture( (unsigned char*)_pos, 3, osg::Texture2D::NEAREST );
        _texDir = makeFloatTexture( (unsigned char*)_dir, 3, osg::Texture2D::NEAREST );
        _texScalar = makeFloatTexture( (unsigned char*)_scalar, 1, osg::Texture2D::NEAREST );

        // Must set the bounding box.
        {
            _bb = osg::BoundingBox( -6, -6, -6, 6, 6, 6 );
        }

    }

    void createDataArrays( float* pos, float* dir, float* scalar )
    {
        float* posI = pos;
        float* dirI = dir;
        float* scalarI = scalar;

        int idx;
        for( idx=0; idx<12; idx++ )
        {
            float bias( idx>=6 ? 3.f : -3.f );
            *posI++ = 0.f;
            *posI++ = bias;
            *posI++ = 0.f;

            *scalarI++ = (float)idx / 11.f;
        }

        for( idx=0; idx<2; idx++ )
        {
            float len( idx==0 ? 1.f : 2.f );

            *dirI++ = 0.436436f * len; // x
            *dirI++ = 0.218218f * len;
            *dirI++ = 0.872872f * len;
            *dirI++ = -1.f * len; // -x
            *dirI++ = 0.f * len;
            *dirI++ = 0.f * len;
            *dirI++ = 0.f * len; // y
            *dirI++ = 1.f * len;
            *dirI++ = 0.f * len;
            *dirI++ = 0.f * len; // -y
            *dirI++ = -1.f * len;
            *dirI++ = 0.f * len;
            *dirI++ = 0.f * len; // z
            *dirI++ = 0.f * len;
            *dirI++ = 1.f * len;
            *dirI++ = 0.f * len; // -z
            *dirI++ = 0.f * len;
            *dirI++ = -1.f * len;
        }
    }
};
//osg::ref_ptr< MyVectorFieldData > _vectorField;
osg::ref_ptr< DebugVectorFieldData > _vectorField;


// Number of vertices in arrow
const int nVerts( 22 );

void
createArrow( osg::Geometry& geom, int nInstances=1 )
{
    // Create an arrow pointing in the +z direction.
    const float sD( .05 ); // shaft diameter
    const float hD( .075 ); // head diameter
    const float len( 1. ); // length
    const float sh( .65 ); // length from base to start of head

    osg::Vec3Array* v = new osg::Vec3Array;
    v->resize( 22 );
    geom.setVertexArray( v );

    osg::Vec3Array* n = new osg::Vec3Array;
    n->resize( 22 );
    geom.setNormalArray( n );
    geom.setNormalBinding( osg::Geometry::BIND_PER_VERTEX );

    // Shaft
    (*v)[ 0 ] = osg::Vec3( sD, 0., 0. );
    (*v)[ 1 ] = osg::Vec3( sD, 0., sh );
    (*v)[ 2 ] = osg::Vec3( 0., -sD, 0. );
    (*v)[ 3 ] = osg::Vec3( 0., -sD, sh );
    (*v)[ 4 ] = osg::Vec3( -sD, 0., 0. );
    (*v)[ 5 ] = osg::Vec3( -sD, 0., sh );
    (*v)[ 6 ] = osg::Vec3( 0., sD, 0. );
    (*v)[ 7 ] = osg::Vec3( 0., sD, sh );
    (*v)[ 8 ] = osg::Vec3( sD, 0., 0. );
    (*v)[ 9 ] = osg::Vec3( sD, 0., sh );

    (*n)[ 0 ] = osg::Vec3( 1., 0., 0. );
    (*n)[ 1 ] = osg::Vec3( 1., 0., 0. );
    (*n)[ 2 ] = osg::Vec3( 0., -1., 0. );
    (*n)[ 3 ] = osg::Vec3( 0., -1., 0. );
    (*n)[ 4 ] = osg::Vec3( -1., 0., 0. );
    (*n)[ 5 ] = osg::Vec3( -1., 0., 0. );
    (*n)[ 6 ] = osg::Vec3( 0., 1., 0. );
    (*n)[ 7 ] = osg::Vec3( 0., 1., 0. );
    (*n)[ 8 ] = osg::Vec3( 1., 0., 0. );
    (*n)[ 9 ] = osg::Vec3( 1., 0., 0. );

    // TBD tri strip
    geom.addPrimitiveSet( new osg::DrawArrays( GL_QUAD_STRIP, 0, 10, nInstances ) );

    // Head
    (*v)[ 10 ] = osg::Vec3( hD, -hD, sh );
    (*v)[ 11 ] = osg::Vec3( hD, hD, sh );
    (*v)[ 12 ] = osg::Vec3( 0., 0., len );
    osg::Vec3 norm = ((*v)[ 11 ] - (*v)[ 10 ]) ^ ((*v)[ 12 ] - (*v)[ 10 ]);
    norm.normalize();
    (*n)[ 10 ] = norm;
    (*n)[ 11 ] = norm;
    (*n)[ 12 ] = norm;

    (*v)[ 13 ] = osg::Vec3( hD, hD, sh );
    (*v)[ 14 ] = osg::Vec3( -hD, hD, sh );
    (*v)[ 15 ] = osg::Vec3( 0., 0., len );
    norm = ((*v)[ 14 ] - (*v)[ 13 ]) ^ ((*v)[ 15 ] - (*v)[ 13 ]);
    norm.normalize();
    (*n)[ 13 ] = norm;
    (*n)[ 14 ] = norm;
    (*n)[ 15 ] = norm;

    (*v)[ 16 ] = osg::Vec3( -hD, hD, sh );
    (*v)[ 17 ] = osg::Vec3( -hD, -hD, sh );
    (*v)[ 18 ] = osg::Vec3( 0., 0., len );
    norm = ((*v)[ 17 ] - (*v)[ 16 ]) ^ ((*v)[ 18 ] - (*v)[ 16 ]);
    norm.normalize();
    (*n)[ 16 ] = norm;
    (*n)[ 17 ] = norm;
    (*n)[ 18 ] = norm;

    (*v)[ 19 ] = osg::Vec3( -hD, -hD, sh );
    (*v)[ 20 ] = osg::Vec3( hD, -hD, sh );
    (*v)[ 21 ] = osg::Vec3( 0., 0., len );
    norm = ((*v)[ 20 ] - (*v)[ 19 ]) ^ ((*v)[ 21 ] - (*v)[ 19 ]);
    norm.normalize();
    (*n)[ 19 ] = norm;
    (*n)[ 20 ] = norm;
    (*n)[ 21 ] = norm;

    geom.addPrimitiveSet( new osg::DrawArrays( GL_TRIANGLES, 10, 12, nInstances ) );
}

float colorScale[] = {
    1.0f, 1.0f, 1.0f, // white
    1.0f, 0.0f, 0.0f, // red
    1.0f, 0.5f, 0.0f, // orange
    0.8f, 0.8f, 0.0f, // yellow
    0.0f, 0.8f, 0.0f, // green
    0.0f, 0.8f, 1.0f, // turquise
    0.2f, 0.2f, 1.0f, // blue
    0.5f, 0.0f, 0.7f  // violet
};

osg::Node*
createInstanced( DebugVectorFieldData& vf )
{
    osg::Group* grp = new osg::Group;

    osg::Geode* geode = new osg::Geode;
    osg::Geometry* geom = new osg::Geometry;
    geom->setUseDisplayList( false );
    geom->setUseVertexBufferObjects( true );

    createArrow( *geom, vf.getDataCount() );
    geode->addDrawable( geom );
    grp->addChild( geode );

    geom->setInitialBound( vf.getBoundingBox() );



    osg::ref_ptr< osg::Shader > vertexShader = new osg::Shader( osg::Shader::VERTEX );
    vertexShader->loadShaderSourceFromFile( osgDB::findDataFile( "vectorfield.vs" ) );

    osg::ref_ptr< osg::Program > program = new osg::Program();
    program->addShader( vertexShader.get() );

    osg::StateSet* ss = geode->getOrCreateStateSet();
    ss->setAttribute( program.get(),
        osg::StateAttribute::ON | osg::StateAttribute::PROTECTED );



    // Position array.
    ss->setTextureAttribute( 0, vf.getPositionTexture() );
    osg::ref_ptr< osg::Uniform > texPosUniform =
        new osg::Uniform( "texPos", 0 );
    ss->addUniform( texPosUniform.get() );

    // Direction array.
    ss->setTextureAttribute( 1, vf.getDirectionTexture() );
    osg::ref_ptr< osg::Uniform > texDirUniform =
        new osg::Uniform( "texDir", 1 );
    ss->addUniform( texDirUniform.get() );

    // Scalar array.
    ss->setTextureAttribute( 2, vf.getScalarTexture() );
    osg::ref_ptr< osg::Uniform > texScalarUniform =
        new osg::Uniform( "scalar", 2 );
    ss->addUniform( texScalarUniform.get() );

    {
        // Pass the 3D texture dimensions to the shader as a "sizes" uniform.
        osg::Vec3s ts( vf.getTextureSizes() );
        osg::ref_ptr< osg::Uniform > sizesUniform =
            new osg::Uniform( "sizes", osg::Vec3( (float)ts.x(), (float)ts.y(), (float)ts.z() ) );
        ss->addUniform( sizesUniform.get() );
    }


    // Set up the color spectrum.
    osg::Image* iColorScale = new osg::Image;
    iColorScale->setImage( 8, 1, 1, GL_RGBA, GL_RGB, GL_FLOAT,
        (unsigned char*)colorScale, osg::Image::NO_DELETE );
    osg::Texture1D* texCS = new osg::Texture1D( iColorScale );
    texCS->setFilter( osg::Texture::MIN_FILTER, osg::Texture2D::LINEAR);
    texCS->setFilter( osg::Texture::MAG_FILTER, osg::Texture2D::LINEAR );
    texCS->setWrap( osg::Texture::WRAP_S, osg::Texture::CLAMP_TO_EDGE );

    ss->setTextureAttribute( 3, texCS );
    osg::ref_ptr< osg::Uniform > texCSUniform =
        new osg::Uniform( "texCS", 3 );
    ss->addUniform( texCSUniform.get() );


    //delete[] pos, dir, scalar;

    return grp;
}



osg::Vec4 planeEquations[] = {
    osg::Vec4( 1., 0., 0., 2. ),
    osg::Vec4( -1., 0., 0., 2. ),
    osg::Vec4( 0.707, 0.707, 0., 1. ),
    osg::Vec4( -0.707, -0.707, 0., 1. ),
    osg::Vec4( 0., 0., 1., 2. ),
    osg::Vec4( 0., 0., -1., 0. )
};

int
main( int argc,
      char ** argv )
{
    osg::ArgumentParser arguments(&argc,argv);

    std::string pathfile;
    arguments.read( "-p", pathfile );

    std::string outfile;
    arguments.read( "-o", outfile );

    osg::ref_ptr< osg::Node > root;
    _vectorField = new DebugVectorFieldData;

    {
        osg::ref_ptr< osg::Node > node( osgDB::readNodeFiles( arguments ) );
        if( node != NULL )
        {
            // Restore from file
            osg::notify( osg::ALWAYS ) << "Restoring..." << std::endl;
            _vectorField->restoreData( node.get() );
        }
        else
        {
            // generate data
            _vectorField->loadData();
        }

        root = createInstanced( *_vectorField );
    }

    if( !outfile.empty() )
        osgDB::writeNodeFile( *root, outfile );

    unsigned int totalData( _vectorField->getDataCount() );
    osg::notify( osg::ALWAYS ) << totalData << " instances." << std::endl;
    osg::notify( osg::ALWAYS ) << totalData * nVerts << " total vertices." << std::endl;


    osg::ref_ptr< osg::Uniform > uModulo( new osg::Uniform( "modulo", 1.0f ) );
    uModulo->setDataVariance( osg::Object::DYNAMIC );
    root->getOrCreateStateSet()->addUniform( uModulo.get() );

    int idx;
    osg::ref_ptr< osg::Uniform > uPlanes = new osg::Uniform( osg::Uniform::FLOAT_VEC4, "planes", 6 );
    for( idx=0; idx<6; idx++ )
        uPlanes->setElement( idx, planeEquations[ idx ] );
    root->getOrCreateStateSet()->addUniform( uPlanes.get() );

    int enables[] = { 0, 0, 0, 0, 0, 0 };
    osg::IntArray* iArray = new osg::IntArray( 6, enables );
    osg::ref_ptr< osg::Uniform > uPlaneOn = new osg::Uniform( osg::Uniform::INT, "planeOn", 6 );
    uPlaneOn->setArray( iArray );
    root->getOrCreateStateSet()->addUniform( uPlaneOn.get() );

    KeyHandler* kh = new KeyHandler( uModulo.get(), uPlaneOn.get() );

    osgViewer::Viewer viewer;
    viewer.setUpViewInWindow( 10, 30, 800, 600 );
    viewer.getCamera()->setClearColor( osg::Vec4(0,0,0,0) );
    viewer.addEventHandler( new osgViewer::StatsHandler );
    viewer.addEventHandler( kh );
    viewer.setSceneData( root.get() );

    if( !pathfile.empty() )
        viewer.setCameraManipulator( new osgGA::AnimationPathManipulator( pathfile ) );

    viewer.setThreadingModel( osgViewer::ViewerBase::SingleThreaded );
    viewer.realize();

    // Hm. Doesn't seem to work. How could we not have a current context at this point?
    // Anyhow, had this as a draw callback at one point and was getting back 2048 on
    // GeForce 9800M.
    GLint n( 0 );
    glGetIntegerv( GL_MAX_3D_TEXTURE_SIZE, &n );
    osg::notify( osg::ALWAYS ) << "GL_MAX_3D_TEXTURE_SIZE: " << n << std::endl;

    return( viewer.run() );
}

