#define FP highp

uniform vec4 handleColor;

void main()
{
    // Set the Z value of the fragment so that it'll always get drawn on top of everything else
    gl_FragDepth = 0.0;
    gl_FragColor = handleColor;
}
