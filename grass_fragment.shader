#version 330 core

in vec2 TexCoords;

out vec4 color;

uniform float Ka;
uniform float Kd;
uniform float Ks;

uniform vec3 lightDir;
uniform sampler2D heightMap;
uniform sampler2D grassDist;
uniform sampler2D grassText;

void main() {
    vec3 lightColor = vec3(1.0, 1.0, 1.0);
    vec3 Ia = Ka * lightColor;
    //    vec3 lightDirection = -normalize(lightDir);
    //    float lambertian = max(dot(lightDirection, normalize(normal)), 0.0);
    //    vec3 Id = Kd * lambertian * lightColor;
    //    float spec = 0.0;
    //    vec3 Is = vec3(0.0);
    //    if(lambertian > 0.0) {
    //      vec3 reflection = reflect(-lightDir, interNormal);
    //      vec3 view = normalize(viewDirection - interPosition);
    //      float specAngle = max(dot(reflection, view), 0.0);
    //      Is = Ks * pow(specAngle, 120.f) * specularColor;
    //    }

        //vec4(normalize(vec4(Id , 1.0) *
    color = texture(grassText, TexCoords);    //+ Is  + Id
    //
    //color = texture(terrainTexture, texCoords);
}
