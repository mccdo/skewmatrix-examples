
uniform mat4 viewMat, projMat;
uniform mat3 normalMat;

attribute vec3 vertex;
attribute vec3 normal;
varying vec3 color;
varying vec2 tc;

void main()
{
    vec3 normalPrime = normalMat * normal;
    normalize( normalPrime );

    // sphere map
    tc = (normalPrime.st + vec2( 1.0, 1.0 )) * .5;

    // Simple diffuse and ambient
    const vec3 lightDir = vec3( 0.0, 0.0, 1.0 );
    float diffuseDot = max( dot( lightDir, normalPrime ), 0.0 );
    float diffAmb = diffuseDot * 0.75 + 0.25;
    color = vec3( diffAmb, diffAmb, diffAmb );

    gl_Position = projMat * viewMat * vec4( vertex, 1.0 );
}
