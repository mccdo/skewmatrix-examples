
varying vec4 oColor;

// Depth peel compare vertex globals
varying vec4 depthPeelTC;

const vec3 lightDir = vec3( 0, 0, 1 );

vec3 computeDiffuse( vec3 nPrime )
{
    float dotProd = max( dot( nPrime, lightDir ), 0.0 ) * .75;
    return( gl_Color.rgb * dotProd );
}
vec3 computeSpecular( vec3 nPrime )
{
    const vec3 eyeVec = vec3( 0, 0, -1 );
    vec3 reflectVec = reflect( lightDir, nPrime );
    float dotProd = max( dot( reflectVec, eyeVec ), 0.0 );
    dotProd = pow( dotProd, 16.0 ) * .7;
    return( vec3( dotProd, dotProd, dotProd ) );
}


void main( void )
{
    vec3 nPrime = normalize( gl_NormalMatrix * gl_Normal );

    const vec3 ambient = vec3( .25, .25, .25 );
    vec3 diffuse = computeDiffuse( nPrime );
    vec3 spec = computeSpecular( nPrime );
    vec3 sum = ambient + diffuse + spec;
    oColor = vec4( sum, gl_Color.a );

    vec4 cc = ftransform();

    //    
    // Depth peel compare vertex code
    depthPeelTC = vec4( (cc.xyz + cc.w) * 0.5, cc.w );

    // Offset slightly for depth compare.
    // TBD this might suffer from accuracy. Actual offset
    // value should be based on depth range.
    depthPeelTC.z = depthPeelTC.z - 0.00001;
    // End depth peel compare vertex code
    //

    gl_Position = cc;
}
