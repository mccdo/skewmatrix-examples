
// Depth peel compare fragment globals
uniform sampler2DShadow depthPeelDepthMap;
varying vec4 depthPeelTC;

varying vec4 oColor;

void main( void )
{
    // Apply depth offset before depth compare
    vec4 depthOffsetTC = depthPeelTC;
    float z = depthPeelTC.z;
    float m = max( abs(dFdx(z)), abs(dFdy(z)) );
    // TBD how to obtain r? Should be an implementation constant.
    const float r = 0.0001;
    // TBD using -1 for factor, -2 for units. These could be passed in via uniforms.
    depthOffsetTC.z = z + (-1. * m) + (-2. * r);

    // Depth peel compare fragment code
    vec4 depthResult = shadow2DProj( depthPeelDepthMap, depthOffsetTC );
    if( depthResult.a == 0.0 )
        discard;
        
    gl_FragColor = oColor;
}
