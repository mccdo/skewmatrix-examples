// Copyright (c) 2010 Skew Matrix Software LLC. All rights reserved.

uniform vec2 sizes;
uniform float totalInstances;
uniform sampler2D texPos;

uniform float osg_SimulationTime;


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


void main()
{
    // Using the instance ID, generate "texture coords" for this instance.
    float fInstanceID = gl_InstanceID;
    float r = fInstanceID / sizes.x;
    vec2 tC;
    tC.s = fract( r ); tC.t = floor( r ) / sizes.y;

    // Get position from the texture.
    vec4 pos = texture2D( texPos, tC );
    pos.w = 0.; // w is 1.0 after lookup; do NOT add 1.0 to gl_Vertex.w
    vec4 v = gl_ModelViewMatrix * ( gl_Vertex + pos );
    gl_Position = gl_ProjectionMatrix * v;

    // TBD. Need to make this configurable from a uniform.
    gl_PointSize = -2000. / v.z;


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
