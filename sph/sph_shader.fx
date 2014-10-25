////////////////////////////////////////////////////////////////////////////////////////////////
//Global Variable
////////////////////////////////////////////////////////////////////////////////////////////////
float4x4 g_mWorldViewProjection;    // World * View * Projection matrix


struct VS_OUTPUT
{
  float4 position	: POSITION;
  float4 color		: COLOR;
};


VS_OUTPUT MainVS(	float3 pos		: POSITION,
					float4 color	: COLOR0)
{	
	VS_OUTPUT OUT;
	
	OUT.position = mul(float4(pos,1), g_mWorldViewProjection);
	OUT.color = color; //float4(1.0, 0.0, 1.0, 1.0);

	return OUT;	
}

struct PS_OUTPUT
{
    float4 rgbaColor : COLOR0;  // Pixel color    
};

PS_OUTPUT MainPS(VS_OUTPUT In) 
{ 
	PS_OUTPUT OUT;
	OUT.rgbaColor =  In.color;
	return OUT;
}

technique MainTech
{
    pass P0
    {          
        VertexShader = compile vs_2_0 MainVS();
        PixelShader  = compile ps_2_0 MainPS();
        
//        CullMode = None;
  //      FillMode = WireFrame;
    }
}

