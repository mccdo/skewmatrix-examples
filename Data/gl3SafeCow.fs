
uniform sampler2D sphereTex;

varying vec3 color;
varying vec2 tc;

void main()
{
    vec4 texColor = texture2D( sphereTex, tc );
    gl_FragColor = texColor * vec4( color, 1.0 );
}