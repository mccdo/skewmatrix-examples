// Copyright (c) 2010 Skew Matrix Software LLC. All rights reserved.

uniform vec2 sizes;
uniform float totalInstances;
uniform sampler2D texPos;

uniform float osg_SimulationTime;
uniform mat4 osg_ViewMatrix;


// Total traces to draw on this streamline.
// E.g., 1, 2, 3, etc.
uniform int numTraces;

// Time interval between traces. Total time for
// a trace to run the length of all samples points:
//     ( traceInterval * numTraces ) / osg_SimulationTime
uniform float traceInterval;

// Trace length, in number of sample points. Alpha fades
// linearly over the traceLength.
uniform int traceLength;


mat3
makeOrientMat( const in vec3 dir )
{
    // Compute a vector at a right angle to the direction.
    // First try projection direction into xy rotated -90 degrees.
    // If that gives us a very short vector,
    // then project into yz instead, rotated -90 degrees.
    vec3 c = vec3( dir.y, -dir.x, 0.0 );
    if( dot( c, c ) < 0.1 )
        c = vec3( 0.0, dir.z, -dir.y );
        
    // Appears to be a bug in normalize when z==0
    //normalize( c.xyz );
    float l = length( c );
    c /= l;

    vec3 up = normalize( cross( dir, c ) );

    // Orientation uses the cross product vector as x,
    // the up vector as y, and the direction vector as z.
    return( mat3( c, up, dir ) );
}


void main()
{
    // Pass texture coords of tri pair to fragment processing.
    gl_TexCoord[ 1 ] = gl_MultiTexCoord1;

    // Using the instance ID, generate "texture coords" for this instance.
    float fInstanceID = gl_InstanceID;
    float r = fInstanceID / sizes.x;
    vec2 tC;
    tC.s = fract( r ); tC.t = floor( r ) / sizes.y;

    // Get position from the texture.
    vec4 instancePos = texture2D( texPos, tC );

    // Compute orientation
    vec4 eye = gl_ModelViewMatrixInverse * vec4( 0., 0., 0., 1. );
    vec3 direction = normalize( eye.xyz - instancePos.xyz );
    mat3 orient = makeOrientMat( direction );
    // Orient the incoming vertices and translate by the instance position.
    vec4 modelPos = vec4( orient * gl_Vertex.xyz, 0. ) + instancePos;
    // Transform into clip coordinates.
    gl_Position = gl_ProjectionMatrix * gl_ModelViewMatrix * modelPos;


    // Compute the length of a trace segment, in points.
    float segLength = totalInstances / numTraces;
    // Use current time to compute an offset in points for the animation.
    float time = mod( osg_SimulationTime, traceInterval );
    float pointOffset = ( time / traceInterval ) * segLength;

    // Find the segment tail for this point's relavant segment.
    float segTail = floor( (fInstanceID - pointOffset) / segLength ) * segLength + pointOffset;
    // ...and the head, which will have full intensity alpha.
    float segHead = floor( segTail + segLength );

#if 1
    // Use smoothstep to fade from the head to the traceLength.
    float alpha = smoothstep( segHead-traceLength, segHead, fInstanceID );
#else
    // Alternative: Use step() instead for no fade.
    float alpha = step( segHead-traceLength, fInstanceID );
#endif

    vec4 color = gl_Color;
    color.a *= alpha;
    gl_FrontColor = color;
}
