attribute vec3 vertexPosition;

uniform mat4 modelViewProjection;

void main()
{
    // TODO
    gl_Position = modelViewProjection * vec4( vertexPosition, 1.0 );
}
