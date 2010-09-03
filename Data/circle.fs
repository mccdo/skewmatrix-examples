// Copyright (c) 2010 Skew Matrix Software LLC. All rights reserved.

uniform int circleEnableText;
uniform sampler2D circleTextSampler;

void main(void)
{
    if( circleEnableText > 0 )
    {
        vec4 sample = texture2D( circleTextSampler, gl_TexCoord[0].st );
        gl_FragColor = vec4( gl_Color.rgb, sample.a );
    }
    else
        gl_FragColor = gl_Color;
}
