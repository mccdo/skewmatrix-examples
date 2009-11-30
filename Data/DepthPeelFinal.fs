
uniform sampler2D depthPeelLayerMap;

varying vec2 oTC;

void main( void )
{
    gl_FragColor = texture2D( depthPeelLayerMap, oTC );
}
