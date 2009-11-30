
uniform sampler2D[ 16 ] layer; // TBD Hardcoded maximum.
uniform int numLayers;

varying vec2 oTC;

void main( void )
{
    // Sort of a HACK. Sampling inside a loop generates a compile error.
    // Unroll the sampling to get all the source colors.
    // Get the source colors.
    vec4[ 16 ] c;
    c[ 0 ] = texture2D( layer[ 0 ], oTC );
    c[ 1 ] = texture2D( layer[ 1 ], oTC );
    c[ 2 ] = texture2D( layer[ 2 ], oTC );
    c[ 3 ] = texture2D( layer[ 3 ], oTC );
    c[ 4 ] = texture2D( layer[ 4 ], oTC );
    c[ 5 ] = texture2D( layer[ 5 ], oTC );
    c[ 6 ] = texture2D( layer[ 6 ], oTC );
    c[ 7 ] = texture2D( layer[ 7 ], oTC );
    c[ 8 ] = texture2D( layer[ 8 ], oTC );
    c[ 9 ] = texture2D( layer[ 9 ], oTC );
    c[ 10 ] = texture2D( layer[ 10 ], oTC );
    c[ 11 ] = texture2D( layer[ 11 ], oTC );
    c[ 12 ] = texture2D( layer[ 12 ], oTC );
    c[ 13 ] = texture2D( layer[ 13 ], oTC );
    c[ 14 ] = texture2D( layer[ 14 ], oTC );
    c[ 15 ] = texture2D( layer[ 15 ], oTC );

    // TBD need the clear color here.
    // First layer is the initial destination color.
    vec4 dstColor = vec4( 0.0, 0.0, 0.0, 0.0 );

    int idx;
    for( idx=numLayers-1; idx >= 0; idx-- )
    {
        vec4 srcColor = c[ idx ];
        float oneMinusA = 1.0 - srcColor.a;
        vec3 color = (srcColor.rgb * srcColor.a) +
            (dstColor.rgb * oneMinusA);
        dstColor.rgb = color;
    }

    // Debug
#if 0
    if( oTC.t > 0.5 )
    {
        if( oTC.s < 0.5 )
            dstColor = texture2D( layer[ 0 ], vec2( oTC.s*2.0 + 1.0, oTC.t*2.0 ) );
        else
            dstColor = texture2D( layer[ 1 ], oTC*2.0 );
    }
    else
    {
        if( oTC.s < 0.5 )
            dstColor = texture2D( layer[ 2 ], oTC*2.0 + 1.0 );
        else
            dstColor = texture2D( layer[ 3 ], vec2( oTC.s*2.0, oTC.t*2.0 + 1.0 ) );
    }
#endif

    gl_FragColor = dstColor;
}
