struct VOut
{
    float4 position : SV_POSITION;
    float4 color : COLOR;
};

struct SpriteGShaderInput
{
	float2 topLeft : ANCHOR;
	float2 dimensions : DIMENSIONS;
	float opacity : OPACITY;
};

struct SpritePShaderInput
{
	float4 p : SV_POSITION;
	float2 t : TEXCOORD;
	float opacity : OPACITY;
};

VOut VShader(float4 position : POSITION, float4 color : COLOR)
{
    VOut output;

    output.position = position;
    output.color = color;

    return output;
};

float4 PShader(float4 position : SV_POSITION, float4 color : COLOR) : SV_TARGET
{
    return color;
}


[maxvertexcount(4)]
void SpriteGShader(point SpriteGShaderInput particles[1], inout TriangleStream<SpritePShaderInput> OutputStream)
{	
}
