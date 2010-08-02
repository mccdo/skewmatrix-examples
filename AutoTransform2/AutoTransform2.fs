// AutoTransform2.fs - fragment shader for AutoTransform2
// mike.weiblen@gmail.com 2010-08-02

varying vec4 v_Color;

void main(void)
{
    float fragZ = gl_FragCoord.z;
    fragZ = fragZ * 1.5;

    vec4 color = vec4( fragZ, 0., 0., 1. );
    if( fragZ > 1.0 ) color.g = 1.0;
    if( fragZ < 0.1 ) color.b = 1.0;

color = v_Color;
    gl_FragColor = clamp( color, 0., 1. );
}

