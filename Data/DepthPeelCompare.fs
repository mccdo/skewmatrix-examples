
// Depth peel compare fragment globals
uniform sampler2DShadow depthPeelDepthMap;
varying vec4 depthPeelTC;

varying vec4 oColor;

void main( void )
{
    // Depth peel compare fragment code
    vec4 depthResult = shadow2DProj( depthPeelDepthMap, depthPeelTC );
    if( depthResult.a == 0.0 )
        discard;
        
    gl_FragColor = oColor;
}
