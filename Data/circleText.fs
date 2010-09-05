// Copyright (c) 2010 Skew Matrix Software LLC. All rights reserved.

uniform sampler2D circleTextSampler;

void main(void)
{
    vec4 sample = texture2D( circleTextSampler, gl_TexCoord[0].st );
    gl_FragColor = vec4( gl_Color.rgb, sample.a );
}
