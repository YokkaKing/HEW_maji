/*
* ファイル名	main.cpp
* タイトル	メイン
* 作成者		久保木幹太
* 作成日		12月02日
* 更新日		12月02日
*/

//================================================================
//	マクロ定義
//================================================================
#define		CLASS_NAME	"Duelists"
#define		WINDOW_CAPTION_P1	"Player1"
#define		WINDOW_CAPTION_P2	"Player2"
#define		SCREEN_WIDTH	(1920)
#define		SCREEN_HEIGHT	(1080)

//================================================================
//	インクルード
//================================================================
//ウィンドウの表示
#include<SDKDDKVer.h>	//利用できる最も上位の Windows プラットフォームが定義される
#define WIN32_LEAN_AND_MEAN	//32bitアプリには不要な情報を抑止してコンパイル時間を短縮
#include<windows.h>
#include"debug_ostream.h"	//デバッグ表示
#include<algorithm>			
#include"direct3d.h"			
#include"shader.h"
#include"field.h"
#include"sprite.h"
#include"keyboard.h"
#include"Player.h"
#include"Effect.h"
#include"Manager.h"
#include"Audio.h"
#include"Viewport.h"

//================================================================
//	グローバル変数
//================================================================
#ifdef _DEBUG	//デバッグビルドの時だけ変数が作られる
int		g_CountFPS;			//FPSカウンター
char	g_DebugStr[2048];	//FPS表示文字列
#endif

#pragma comment(lib, "winmm.lib")

//ウィンドウプロシージャ
//コールバック関数＝＞他人が呼び出してくれる関数
LRESULT	CALLBACK WndProc(HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam);

int APIENTRY WinMain(HINSTANCE hInstance,
	HINSTANCE hPrevInstance, LPSTR lpCmd, int nCmdShow)
{

	//乱数の初期化
	srand(timeGetTime());

	//フレームレート計測用変数
	DWORD	dwExecLastTime;
	DWORD	dwFPSLastTime;
	DWORD	dwCurrentTime;
	DWORD	dwFrameCount;

	HRESULT hr = CoInitializeEx(nullptr, COINITBASE_MULTITHREADED);


	//ウィンドウクラスの登録（ウィンドウの仕様的な物を決めてWindowsへセットする）
	WNDCLASS	wc;	//構造体を準備
	ZeroMemory(&wc, sizeof(WNDCLASS));//内容を０で初期化
	wc.lpfnWndProc = WndProc;	//コールバック関数のポインター
	wc.lpszClassName = CLASS_NAME;	//この仕様書の名前
	wc.hInstance = hInstance;	//このアプリケーションのこと
	wc.hCursor = LoadCursor(NULL, IDC_ARROW);//カーソルの種類
	wc.hbrBackground = (HBRUSH)(COLOR_BACKGROUND+1 );//ウィンドウの背景色は黒
	RegisterClass(&wc);	//構造体をWindowsへセット

	//クライアント領域のサイズを表す矩形 (左からleft, top, right, bottom)
	RECT window_rect = { 0, 0, SCREEN_WIDTH, SCREEN_HEIGHT };
	//ウィンドウのスタイル（ウィンドウ枠と最大化ボタンを削除）
	DWORD window_style = WS_OVERLAPPEDWINDOW ^ (WS_THICKFRAME | WS_MAXIMIZEBOX);
	//指定したクライアント領域を確保するために新たな矩形座標を計算
	AdjustWindowRect(&window_rect, window_style, FALSE);
	//調整された矩形の横と縦のサイズを計算
	int window_width = window_rect.right - window_rect.left;
	int window_height = window_rect.bottom - window_rect.top;

	//Player1ウィンドウの作成
	HWND	hWnd = CreateWindow(
		CLASS_NAME,	
		WINDOW_CAPTION_P1,
		window_style,
		CW_USEDEFAULT,
		CW_USEDEFAULT,
		window_width,
		window_height,
		NULL,
		NULL,
		hInstance,
		NULL
	);

	if (!hWnd) return 0;

	//1つ目のウィンドウの位置を取得
	RECT rect1;
	GetWindowRect(hWnd, &rect1);

	//2つ目のウィンドウを1つ目のウィンドウの右側に配置
	int window2_x = rect1.right;
	int window2_y = rect1.top;

	//画面の幅を取得して、画面外に出ないように調整
	int screenWidth = GetSystemMetrics(SM_CXSCREEN);
	if (window2_x + window_width > screenWidth)
	{
		//画面外に出る場合は、１つ目のウィンドウの下に配置
		window2_x = rect1.left;
		window2_y = rect1.bottom;
		//それでも外に出る場合は、画面の左上に配置
		if (window2_y + window_height > GetSystemMetrics(SM_CYSCREEN))
		{
			window2_x = 0;
			window2_y = 0;
		}
	}

	//Player2ウィンドウ作成(座標を右側にずらす)
	//メインモニターの横幅分右にずらすことで2枚目のモニターに表示される
	HWND	hWnd2 = CreateWindow(
		CLASS_NAME,
		WINDOW_CAPTION_P2,
		window_style,
		window2_x,
		window2_y,
		window_width,
		window_height,
		NULL,
		NULL,
		hInstance,
		NULL
	);

	if (!hWnd2) return 0;

	//作成したウィンドウを表示する
	ShowWindow(hWnd, nCmdShow);//引数に従って表示、または非表示
	//ウィンドウ内部の更新要求
	UpdateWindow(hWnd);
	//ウィンドウ2
	ShowWindow(hWnd2, nCmdShow);
	UpdateWindow(hWnd2);
	//Viewport_Initialize(hWnd);

	Direct3D_Initialize(hWnd, hWnd2);
	Keyboard_Initialize();
	Shader_Initialize(Direct3D_GetDevice(), Direct3D_GetDeviceContext()); // シェーダの初期化
	InitializeSprite();//スプライトの初期化

	InitAudio();	//サウンドの初期化



	Manager_Initialize();


	/////////////////////////////////

	//メッセージループ
	MSG	msg;
	ZeroMemory(&msg, sizeof(MSG));

	//フレームレート計測初期化
	timeBeginPeriod(1);	//タイマーの精度を設定
	dwExecLastTime = dwFPSLastTime = timeGetTime();//現在のタイマー値
	dwCurrentTime = dwFrameCount = 0;

	//終了メッセージが来るまでループする
	do {
		if (PeekMessage(&msg, nullptr, 0, 0, PM_REMOVE)) 
		{ // ウィンドウメッセージが来ていたら
			TranslateMessage(&msg);
			DispatchMessage(&msg);
		}
		else 
		{ // ゲームの処理
			dwCurrentTime = timeGetTime();
			if ((dwCurrentTime - dwFPSLastTime) >= 1000)
			{
#ifdef _DEBUG
				g_CountFPS = dwFrameCount;
#endif
				dwFPSLastTime = dwCurrentTime;//現在のタイマー値保存
				dwFrameCount = 0;
			}

			if ((dwCurrentTime - dwExecLastTime) >= ((float)1000 / 60))
			{// 1/60s 経過した
				dwExecLastTime = dwCurrentTime;//現在のタイマーと保存
#ifdef _DEBUG
				//ウィンドウキャプションへ現在のFPSを表示
				wsprintf(g_DebugStr, "Duelists");
				wsprintf(&g_DebugStr[strlen(g_DebugStr)],
									" FPS : %d", g_CountFPS);
				SetWindowText(hWnd, g_DebugStr);
#endif

				//更新処理
				Manager_Update();

				//描画処理:ウィンドウ1
				Direct3D_Clear(DX_WINDOW_ID_1);
				Direct3D_SetRenderTarget(DX_WINDOW_ID_1);
				Manager_Draw_Player1();
				Direct3D_Present(DX_WINDOW_ID_1);

				//描画処理:ウィンドウ2
				Direct3D_Clear(DX_WINDOW_ID_2);
				Direct3D_SetRenderTarget(DX_WINDOW_ID_2);
				Manager_Draw_Player2();
				Direct3D_Present(DX_WINDOW_ID_2);


				keycopy();

				dwFrameCount++;		//処理回数更新
			}

		}
	} while (msg.message != WM_QUIT);
	
	Manager_Finalize();


	//UninitAudio();		//サウンドの終了

	Shader_Finalize(); // シェーダの終了処理
	FinalizeSprite();	//スプライトの終了処理
	/////////////////////////////////////////
	Direct3D_Finalize();


	//終了する
	return (int)msg.wParam;

}

//=========================================
//ウィンドウプロシージャ
// メッセージループ内で呼び出される
//=========================================
LRESULT CALLBACK WndProc(HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam)
{
	HGDIOBJ hbrWhite, hbrGray;

	HDC		hdc;	//ウィンドウ画面を表す情報（デバイスコンテキスト 入出力先）
	PAINTSTRUCT	ps;	//ウィンドウ画面の大きさなど描画関連の情報

	switch (uMsg)
	{
		case WM_ACTIVATEAPP:
		case WM_SYSKEYDOWN:
		case WM_KEYUP:
		case WM_SYSKEYUP:
		     Keyboard_ProcessMessage(uMsg, wParam, lParam);
			 break;

		case WM_KEYDOWN:	//キーが押された
			if (wParam == VK_ESCAPE)//押されたのはESCキー
			{
				//ウィンドウを閉じたいリクエストをWindowsに送る
				SendMessage(hWnd, WM_CLOSE, 0, 0);
			}

			Keyboard_ProcessMessage(uMsg, wParam, lParam);
			break;
		case WM_CLOSE:	//ウィンドウを閉じなさい命令				
			//if (
			//	MessageBox(hWnd, "本当に終了してよろしいですか？",
			//		"確認", MB_OKCANCEL | MB_DEFBUTTON2) == IDOK
			//	)
			//{//OKが押されたとき
			//	DestroyWindow(hWnd);//終了する手続きをWindowsへリクエスト
			//}
			//else
			//{
			//	return 0;	//やっぱり終わらない
			//}

			//break;
		case WM_DESTROY:	//終了してOKですよ
			PostQuitMessage(0);		//自分のメッセージに０を送る
			break;

	}

	//必用の無いメッセージは適当に処理させて終了
	return DefWindowProc(hWnd, uMsg, wParam, lParam);

}
