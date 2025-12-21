/*==============================================================================

   2D描画用頂点シェーダー [shader_vertex_2d.hlsl]
--------------------------------------------------------------------------------

==============================================================================*/

// 定数バッファ
cbuffer Buffer0 : register(b0)
{
    float4x4 mtx; //C言語から渡されたデータが入っている
}

cbuffer Buffer1 : register(b1)
{
    float4x4 World; //C言語から渡されたデータが入っている
}

struct LIGHT
{
    bool Enable;
	//bool3 dummy[3];
    bool3 dummy;
    float4 Dir;
    float4 Diffuse;
    float4 Ambient;
};

cbuffer Buffer2 : register(b2)
{
    LIGHT Light; //C言語から渡されたデータが入っている
}

cbuffer Buffer3 : register(b3)
{
    float4x4 Bones[100];
}

//入力用頂点構造体
struct VS_INPUT
{ //              V コロン！
    float4 posL : POSITION0; //頂点座標 オーでなくゼロ！
    float4 normal : NORMAL0;
    float4 color : COLOR0; //頂点カラー（R,G,B,A）
    float2 texcoord : TEXCOORD0;
    
    uint4 boneIndex : BONEINDEX0;
    float4 boneWeight : BONEWEIGHT0;
};

//出力用頂点構造体
struct VS_OUTPUT
{
    float4 posH : SV_POSITION; //変換済頂点座標
    float4 color : COLOR0; //頂点カラー
    float2 texcoord : TEXCOORD0;
    float3 posWorld : POSITION1; 
    float3 normal : NORMAL0; 
};

VS_OUTPUT main(VS_INPUT vs_in)
{

    VS_OUTPUT vs_out;
    vs_out.color = vs_in.color;
    vs_out.texcoord = vs_in.texcoord;

    float totalWeight = vs_in.boneWeight.x + vs_in.boneWeight.y +
                            vs_in.boneWeight.z + vs_in.boneWeight.w;
    if (Light.Enable == false )
    {
        vs_out.posH = mul(vs_in.posL, mtx);
        vs_out.posWorld = vs_in.posL.xyz;
        vs_out.normal = vs_in.normal.xyz;
    }
    
    else
    {
        float totalWeight = vs_in.boneWeight.x + vs_in.boneWeight.y +
                            vs_in.boneWeight.z + vs_in.boneWeight.w;

        float4 finalPos = float4(0, 0, 0, 0);
        float3 finalNormal = float3(0, 0, 0);

        if (totalWeight > 0.01f)
        {
            
            finalPos += mul(vs_in.posL, Bones[vs_in.boneIndex.x]) * vs_in.boneWeight.x;
            finalPos += mul(vs_in.posL, Bones[vs_in.boneIndex.y]) * vs_in.boneWeight.y;
            finalPos += mul(vs_in.posL, Bones[vs_in.boneIndex.z]) * vs_in.boneWeight.z;
            finalPos += mul(vs_in.posL, Bones[vs_in.boneIndex.w]) * vs_in.boneWeight.w;
            finalPos.w = 1.0f;

         
            finalNormal += mul(vs_in.normal.xyz, (float3x3) Bones[vs_in.boneIndex.x]) * vs_in.boneWeight.x;
            finalNormal += mul(vs_in.normal.xyz, (float3x3) Bones[vs_in.boneIndex.y]) * vs_in.boneWeight.y;
            finalNormal += mul(vs_in.normal.xyz, (float3x3) Bones[vs_in.boneIndex.z]) * vs_in.boneWeight.z;
            finalNormal += mul(vs_in.normal.xyz, (float3x3) Bones[vs_in.boneIndex.w]) * vs_in.boneWeight.w;
        }
        else
        {
            finalPos = vs_in.posL;
            finalNormal = vs_in.normal.xyz;
        }

   
        float4 worldPos = mul(finalPos, World);
        float3 worldNormal = normalize(mul(finalNormal, (float3x3) World));

        vs_out.posWorld = worldPos.xyz;
        vs_out.normal = worldNormal;
        vs_out.posH = mul(worldPos, mtx);

        
        float diffuse = saturate(dot(worldNormal, -Light.Dir.xyz));
        
    
        vs_out.color.rgb = vs_in.color.rgb * diffuse + Light.Ambient.rgb;
    }

    return vs_out;
}


////=============================================================================
//// 頂点シェーダ
////=============================================================================
//float4 main(in float4 posL : POSITION0 ) : SV_POSITION
//{
//	return mul(posL, mtx);//頂点座標＊mtx（変換行列）
//}
