/*==============================================================================

   シェーダー [shader.h]
														 Author : Youhei Sato
														 Date   : 2025/05/15
--------------------------------------------------------------------------------

==============================================================================*/
#ifndef SHADER_H
#define	SHADER_H

//================================================================
//	インクルード
//================================================================
#include<d3d11.h>
#include<DirectXMath.h>
#define MAX_BONES 100
#include "model.h"
#include"direct3d.h"//<<<<<<<<<<<<<<
void Shader_SetBones(MODEL* model);
bool Shader_Initialize(ID3D11Device* pDevice, ID3D11DeviceContext* pContext);
void Shader_Finalize();

void Shader_SetMatrix(const DirectX::XMMATRIX& matrix);

void Shader_SetWorldMatrix(const DirectX::XMMATRIX& matrix);//<<<<<<<<
void Shader_SetLight(LIGHT light);//<<<<<<<<<<<<<<<<<<<


void Shader_Begin();

#endif // SHADER_H