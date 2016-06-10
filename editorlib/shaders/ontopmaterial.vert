attribute highp vec3 vertexPosition;

uniform highp mat4 modelViewProjection;

void main()
{
    gl_Position = modelViewProjection * vec4( vertexPosition, 1.0 );
    // Set the Z value of the vertex so that it'll always get drawn on top of everything else
    gl_Position.z = 0.0;
}
