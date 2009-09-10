uniform vec3 sizes;
uniform sampler3D texPos;
uniform sampler3D texDir;
uniform sampler3D texCross;
uniform sampler3D scalar;
uniform sampler1D texCS;

uniform float modulo;

uniform vec4 plane0;
uniform vec4 plane1;


// There is no straightforward way to "discard" a vertex in a vertex shader,
// (unlike discard in a fragment shader). So we use an aspect of clipping to
// "clip away" unwanted vertices and vectors. Here's how it works:
// The gl_Position output of the vertex shader is an xyzw value in clip coordinates,
// with -w < xyz < w. All xyz outside the range -w to w are clipped by hardware
// (they are outside the view volume). So, to discard an entire vector, we set all
// its gl_Positions to (1,1,1,0). All vertices are clipped because -0 < 1 < 0 is false.
// If all vertices for a given instance have this value, the entire instance is
// effectively discarded.
bool
discardInstance( in float fiid )
{
    bool discardInstance = ( mod( fiid, modulo ) > 0.0 );
    if( discardInstance )
        gl_Position = vec4( 1.0, 1.0, 1.0, 0.0 );
    return( discardInstance );
}

vec3
generateTexCoord( in const float fiid )
{
    // Using the instance ID, generate stp texture coords for this instance.
    float p1 = fiid / (sizes.x*sizes.y);
    float t1 = fract( p1 ) * sizes.y;
    
    vec3 tC;
    tC.s = fract( t1 );
    tC.t = floor( t1 ) / sizes.y;
    tC.p = floor( p1 ) / sizes.z;
    
    return( tC );
}

bool
clipInstance( in vec3 pos )
{
    // Is the entire arrow clipped?
    float dot0 = dot( plane0, vec4( pos.xyz, 1.0 ) );
    float dot1 = dot( plane1, vec4( pos.xyz, 1.0 ) );
    bool clip = ( (dot0<0.0) || (dot1<0.0) );
    if( clip )
        // Clipped -- discard the whole arror.
        gl_Position = vec4( 1.0, 1.0, 1.0, 0.0 );
    return( clip );
}

mat3
makeOrientMat( in const vec3 tC )
{
    vec4 dir = texture3D( texDir, tC );
    vec4 c = texture3D( texCross, tC );
    vec3 up = cross( c.xyz, dir.xyz );
    
    mat3 m = mat3( c.x, up.x, dir.x,
                   c.y, up.y, dir.y,
                   c.z, up.z, dir.z );
                   
    return( m );
}

vec4
simpleLighting( in const vec4 color, in const vec3 normal, in const float diffCont, in const float ambCont )
{
    vec4 amb = color * ambCont;
    vec4 diff = color * dot( normal, vec3( 0.0, 0.0, 1.0 ) ) * diffCont;
    return( amb + diff );
}

void main()
{
    float fiid = gl_InstanceID;
    
    if( discardInstance( fiid ) )
        return;

    vec3 tC = generateTexCoord( fiid );

    // Sample (look up) position. Discard instance if clipped.
    vec4 pos = texture3D( texPos, tC );
    if( clipInstance( pos ) )
        return;

    // Sample (look up) orientation values and create a matrix.
    mat3 orientMat = makeOrientMat( tC );

    // Orient the arrow.
    vec3 oVec = orientMat * gl_Vertex.xyz;

    // Position the oriented arrow and convert to clip coords.
    vec4 hoVec = vec4( oVec + pos, 1.0 );
    gl_Position = gl_ModelViewProjectionMatrix * hoVec;

    // Orient the normal.
    vec3 oNorm = orientMat * gl_Normal;
    vec3 norm = normalize(gl_NormalMatrix * oNorm);

    // Diffuse lighting with light at the eyepoint.
    vec4 scalarV = texture3D( scalar, tC );
    vec4 oColor = texture1D( texCS, scalarV.a );
    gl_FrontColor = simpleLighting( oColor, norm, 0.7, 0.3 );
}
