#version 330 core
layout (location = 0) in vec3 a_Pos;

uniform mat4 u_View;
uniform mat4 u_Model;
uniform mat4 u_Projection;
uniform float u_TileSize;
uniform float u_TextureTiling;

out vec3 v_Normal;
out vec3 v_FragPos;
out vec2 v_TexCoord;

void main()
{
    v_FragPos = vec3(u_Model * vec4(a_Pos, 1.0));
    v_Normal = vec3(0.0, 1.0, 0.0);

    float texScale = u_TextureTiling / u_TileSize;
    v_TexCoord = v_FragPos.xz * texScale;

    gl_Position = u_Projection * u_View * vec4(v_FragPos, 1.0);
}
