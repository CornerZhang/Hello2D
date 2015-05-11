/*!
 * Hello2D Game Engine
 * yoyo 2015 ShenZhen China
 * repo:https://github.com/play175/Hello2D
 * website:http://yoyo.play175.com
 * MIT Licensed
 */

#define WINVER 0x500
#define WIN32_LEAN_AND_MEAN
#include <windows.h>
#include <stdbool.h>
#include <time.h>
#include <shlobj.h>//for dropfiles

#include "window.h"
#include "h2d.h"

#define CLASSNAME L"hello2d"
#define WINDOWSTYLE (WS_OVERLAPPEDWINDOW)// & ~WS_THICKFRAME) & ~WS_MAXIMIZEBOX)
#define WINDOWSTYLE_NOBORDER (WS_POPUP | WS_SYSMENU | WS_MINIMIZEBOX)
#define WINDOWSTYLE_EXLAYERED (WS_EX_LAYERED)

HWND _hwnd = NULL;
static HDC _mem_hdc = NULL;

void set_memdc(int w,int h, void **bits) {
	HDC hdc = (G->alpha) ? GetDC(NULL) : GetDC(_hwnd);

	if (_mem_hdc == NULL)_mem_hdc = CreateCompatibleDC(hdc);

	BITMAPINFO bminfo;
	ZeroMemory(&bminfo, sizeof(bminfo));
	bminfo.bmiHeader.biSize = sizeof(BITMAPINFOHEADER);

	bminfo.bmiHeader.biWidth = w;
	bminfo.bmiHeader.biHeight = -((LONG) h);

	bminfo.bmiHeader.biPlanes = 1;
	bminfo.bmiHeader.biBitCount = 32;
	bminfo.bmiHeader.biCompression = BI_RGB;

	HBITMAP hbmp = CreateDIBSection(hdc, &bminfo, DIB_RGB_COLORS, bits, NULL, 0x0);
	if (hbmp == NULL)perror("CreateDIBSection failed");

	HGDIOBJ  oldobj = SelectObject(_mem_hdc,hbmp);
	if (oldobj != NULL)DeleteObject(oldobj);


	ReleaseDC((G->alpha) ? NULL : _hwnd, hdc);
}

static void window_init(HWND hwnd) {
	_hwnd = hwnd;

	set_memdc(G->w,G->h,&(G->bits));
}

void resize_window(int w,int h) {
	PostMessage(_hwnd, WM_SIZE, 0, w | (h << 16));
}

void window_resize(int w, int h) {
	if (w == G->w && h == G->h)return;
	if (w == 0) w = 1;
	if (h == 0) h = 1;

	game_resizing(w, h, G->w, G->h);

	G->w = w;
	G->h = h;

	set_memdc(G->w,G->h,&(G->bits));

	game_resized(w,h);
}

static void window_render(HWND hwnd) {

	if (G->alpha) {
		BLENDFUNCTION blendFunc = { AC_SRC_OVER, 0, 255, AC_SRC_ALPHA };
		RECT rc;
		GetWindowRect(hwnd,&rc);
		POINT ptWinPos = {rc.left, rc.top};
		SIZE sizeWindow= {rc.right - rc.left, rc.bottom - rc.top};
		POINT ptSrc={0,0};

		HDC screenhdc = GetDC(NULL);

		UpdateLayeredWindow(
		    hwnd//HWND hwnd,             // layered���ھ�������ڱ�����WS_EX_LAYERED��ʽ
		    ,screenhdc//HDC hdcDst,            // ��ĻDC,����ͨ��GetDC(NULL)ȡ��
		    ,&ptWinPos//POINT *pptDst,         // layered����Ҫ�ƶ�����λ��
		    ,&sizeWindow//SIZE *psize,           // ����layered���ڵĴ�С
		    ,_mem_hdc//HDC hdcSrc,            // ԴDC,����BitBlt��ԴDC
		    ,&ptSrc//POINT *pptSrc,         // Ҫ����layered�����ϵ�ԴDC�ϵ����
		    ,0//COLORREF crKey,        // Ҫ��Ϊ͸������ɫ������������λͼ����Ч
		    ,&blendFunc//BLENDFUNCTION *pblend, // ���ģʽ��ָ��alphaFormatΪAC_SRC_ALPHA����ʹ��alpha��Ϸ�ʽ��
		    ,ULW_ALPHA//DWORD dwFlags          // ��־ʹ�����ֻ��ģʽ��������ULW_ALPHA ��ʾʹ��alpha��ϣ�ULW_OPAQUE��ʾ��ʹ��alpha��ϣ�ULW_COLORKEY��ʾָ������ɫΪ͸��ɫ��
		);

		ReleaseDC(NULL,screenhdc);
	} else {
		HDC hdc = GetDC(hwnd);
		BitBlt(hdc,0,0,G->w,G->h,_mem_hdc,0,0,SRCCOPY);
		ReleaseDC(hwnd, hdc);
	}
}

void minimize_window() {
	if (! is_windowminimize())ShowWindow(_hwnd,SW_MINIMIZE);
}
void restore_window() {
	ShowWindow(_hwnd,SW_RESTORE);
}

bool is_windowminimize() {
	if (IsIconic(_hwnd))
		return true;
	else
		return false;
}

//�ƶ�����
void begin_movewindow() {
	UpdateWindow(_hwnd);
	ReleaseCapture();
	SendMessage(_hwnd,WM_NCLBUTTONDOWN,HTCAPTION,0);
	G->mousedown = false;
}

void window_dropfile(HDROP hdrop) {
	int size = DragQueryFile(hdrop, 0xFFFFFFFF, NULL, 0);
	char str[1000];
	int i;
	for (i=0;i<size;i++) {
		DragQueryFile(hdrop, i, str,1000);
		game_dropfile(str,i,size);
	}
	SetForegroundWindow(_hwnd);
}

bool send_dropfile(HWND hwnd,char *file) {
	long processid;
	//long sz = sizeof(DROPFILES) + strlen(file) + 1;
	long sz = sizeof(DROPFILES) + strlen(file) + 1;
	GetWindowThreadProcessId(hwnd,(LPDWORD )&processid);
	if (processid == 0)return false;
	HANDLE hprocess = OpenProcess(PROCESS_VM_OPERATION | PROCESS_VM_WRITE, FALSE, processid);
	if (hprocess == NULL)return false;
	void *addr = VirtualAllocEx(hprocess, NULL, sz, MEM_COMMIT, PAGE_READWRITE); //ע��ʵ�ʷ�����ڴ��С��ҳ�ڴ��С��������
	if (addr == NULL) {
		CloseHandle(hprocess);
		return false;
	}
	//void *buf = malloc(sz);//������ջ���ڴ棬ԭ����
	//DROPFILES *dp = (DROPFILES *)buf;
	//dp->pFiles = sizeof(*dp);
	//memcpy(dp + dp->pFiles,file,strlen(file) + 1);
	//if(!WriteProcessMemory(hprocess, addr, buf, sz, 0)) {
	if (!WriteProcessMemory(hprocess, addr, file, strlen(file) + 1, 0)) {
		VirtualFreeEx(hprocess, addr,0, MEM_RELEASE);
		CloseHandle(hprocess);
		//free(buf);
		return false;
	}
	//SendMessage(hwnd, WM_DROPFILES, (WPARAM)addr, 0);
	SendMessage(hwnd, WM_USER + 0x100, (WPARAM)addr, 0);
	VirtualFreeEx(hprocess, addr,0, MEM_RELEASE);
	CloseHandle(hprocess);
	//free(buf);
	return true;
}

static void inline get_xy(LPARAM lParam, int *x, int *y) {
	*x = (short)(lParam & 0xffff);
	*y = (short)((lParam>>16) & 0xffff);
}

LRESULT CALLBACK WndProc(HWND hwnd, UINT message, WPARAM wParam, LPARAM lParam) {
	static int mouse_state = 0;
	static uint32_t last_frame = 0;
	//static int x = 0,y = 0;
	static bool ctrl,alt,shift;
	POINT p;

	switch (message) {
	case WM_CREATE:
		window_init(hwnd);
		last_frame = G->now;
		SetTimer(hwnd,0,G->fixeddelta,NULL);
		break;
	case WM_PAINT: {
		if (GetUpdateRect(hwnd, NULL, FALSE)) {
			G->now = clock();
			++G->frame;
			game_frame();
			window_render(hwnd);
			ValidateRect(hwnd, NULL);//ʹ������Ч   �����Ϣ�����е�WM_PAINT��Ϣ
		}
		return 0;
	}
	case WM_TIMER : {
		G->now = clock();
		G->delta = 1.0f * (G->now - last_frame) / 1000.0f;
		game_update();
		last_frame = G->now;
		InvalidateRect(hwnd, NULL , FALSE);//ʹ������Ч   ������ϢWM_PAINT
		break;
	}
	case WM_MOUSEWHEEL://wParam��4λ��ʶdelta����4λ����Щʶ���Ƽ�����;lParam��ʶ:y|x
		p.x = (short)(lParam & 0xffff);
		p.y = (short)((lParam >> 16) & 0xffff);
		ScreenToClient(hwnd,&p);
		game_wheel((short)((wParam >> 16) & 0xffff),p.x,p.y);
		break;
	case WM_USER + 0x100:
		game_dropfile((char *)wParam,0,1);
		break;
	case WM_DROPFILES:
		//debug("WM_DROPFILES,%d,%d",(int)wParam ,(int)lParam);
		window_dropfile((HDROP)wParam);
		break;
	case WM_KILLFOCUS:
		ctrl = shift = alt = false;
		break;
	case WM_KEYDOWN:
		switch ((int)wParam) {
		case VK_CONTROL:
			ctrl = true;
			break;
		case VK_SHIFT:
			shift = true;
			break;
		}
		game_keydown((int)wParam, ctrl, alt, shift,lParam & 0xffff);
		//debug("WM_KEYDOWN:%d,%d",(int)wParam ,(int)lParam & 0xffff);
		break;
	case WM_KEYUP:
		switch ((int)wParam) {
		case VK_CONTROL:
			ctrl = false;
			break;
		case VK_SHIFT:
			shift = false;
			break;
		}
		game_keyup((int)wParam, ctrl, alt, shift,lParam & 0xffff);
		//debug("WM_KEYUP:%d,%d",(int)wParam ,(int)lParam & 0xffff);
		break;
	case WM_SYSKEYDOWN:
		switch ((int)wParam) {
		case VK_MENU:
			alt = true;
			break;
		}
		//debug("WM_SYSKEYDOWN:%d,%d",(int)wParam ,(int)lParam);
		break;
	case WM_SYSKEYUP:
		switch ((int)wParam) {
		case VK_MENU:
			alt = false;
			break;
		}
		//debug("WM_SYSKEYUP:%d,%d",(int)wParam ,(int)lParam);
		break;
	case WM_SIZE:
		window_resize((short)(lParam & 0xffff),(short)((lParam >> 16) & 0xffff));
		break;
	case WM_DESTROY:
		PostQuitMessage(0);
		return 0;
	case WM_QUERYENDSESSION:
		//�ػ�����ע��
		printf("�ػ�ing\n");
		PostQuitMessage(0);
		break;
	case WM_LBUTTONUP: {
		ReleaseCapture();
		get_xy(lParam, &G->mousex, &G->mousey);
		game_touch(G->mousex,G->mousey,TOUCH_END);
		G->mousedown = false;
		break;
	}
	/*case WM_KILLFOCUS: {
	    		ReleaseCapture();
	  if(touch_phase == TOUCH_BEGIN) {
	    touch_phase = TOUCH_END;
	    game_touch(lx,ly,TOUCH_END);
	  }
	}*/
	case WM_LBUTTONDOWN: {
		get_xy(lParam, &G->mousex, &G->mousey);
		SetCapture(hwnd);
		G->mousedown = true;
		game_touch(G->mousex,G->mousey,TOUCH_BEGIN);
		break;
	}
	/*case WM_MOUSEHOVER: {
	  game_touch(-1,-1,TOUCH_OVER);
	  break;
	}*/
	case WM_MOUSELEAVE: {
		mouse_state = 0;
		game_touch(G->mousex,G->mousey,TOUCH_OUT);
		break;
	}
	case WM_MOUSEMOVE: {
		get_xy(lParam, &G->mousex, &G->mousey);
		if (mouse_state == 0) {
			TRACKMOUSEEVENT tme = { sizeof(TRACKMOUSEEVENT) };
			tme.hwndTrack       = hwnd;
			tme.dwFlags         = TME_LEAVE;

			TrackMouseEvent(&tme);

			mouse_state = 1;

			game_touch(G->mousex,G->mousey,TOUCH_OVER);
		}
		game_touch(G->mousex,G->mousey,TOUCH_MOVE);
		break;
	}
	}
	return DefWindowProcW(hwnd, message, wParam, lParam);
}

static void register_class() {
	WNDCLASSW wndclass;

	wndclass.style = CS_HREDRAW | CS_VREDRAW;
	wndclass.lpfnWndProc = WndProc;
	wndclass.cbClsExtra = 0;
	wndclass.cbWndExtra = 0;
	wndclass.hInstance = GetModuleHandleW(0);
	wndclass.hIcon = 0;//LoadIcon (wndclass.hInstance, (LPCTSTR )IDC_MAIN);
	wndclass.hCursor = LoadCursor (NULL, IDC_ARROW);
	wndclass.hbrBackground = 0;
	wndclass.lpszMenuName = 0;
	wndclass.lpszClassName = CLASSNAME;

	RegisterClassW(&wndclass);
}

static void unregister_class() {
	UnregisterClassW(CLASSNAME,GetModuleHandleW(0));
}

static HWND create_window(int w, int h,uint32_t style,uint32_t exstyle) {
	RECT rect;

	int sw = GetSystemMetrics(SM_CXFULLSCREEN);
	int sh = GetSystemMetrics(SM_CYFULLSCREEN) - GetSystemMetrics(SM_CYCAPTION) + GetSystemMetrics(SM_CYDLGFRAME);
	if (w > sw)w = sw;
	if (h > sh)h = sh;
	int x = (sw - w)*0.5;
	int y = (sh - h)*0.45;
	if (x < 0)x = 0;
	if (y < 0)y = 0;

	rect.left=0;
	rect.right=w;
	rect.top=0;
	rect.bottom=h;

	AdjustWindowRect(&rect,style,0);

	HWND hwnd = CreateWindowExW(exstyle,CLASSNAME,WINDOWNAME,style, x,y,/*CW_USEDEFAULT,0,*/
	                            rect.right-rect.left,rect.bottom-rect.top,
	                            0,0,
	                            GetModuleHandleW(0),
	                            0);

	return hwnd;
}

void set_exe_workdir() {
	char exedir[512];
	GetModuleFileName(NULL,exedir,512);
	int len = strlen(exedir);
	while(len-->0) {
		if(len>0 && exedir[len-1] == '\\') {
			exedir[len-1] = '\0';
			break;
		}
	}
	SetCurrentDirectory(exedir);
}

int main(int argc, char **argv) {
	HWND hwnd;

	set_exe_workdir();

	game_init();

	if (!game_create(argc, argv)) {
		goto exit;
	}

	if (G->singleton) {
		//�ж��Ƿ��Ѿ�������
		hwnd = FindWindowW(CLASSNAME,NULL);
		if (hwnd != NULL) {
			//����Ѿ����У��򼤻�
			if (IsIconic(hwnd))ShowWindow(hwnd,SW_RESTORE);
			SetForegroundWindow(hwnd);
			//���Ұ��������ļ�����ȥ
			if (argc>1)send_dropfile(hwnd,argv[1]);
			goto destroy;
		}
	}

	uint32_t winstyle = WINDOWSTYLE;
	uint32_t winexstyle = 0;
	if (G->alpha) {
		winstyle = WINDOWSTYLE_NOBORDER;
		winexstyle = WINDOWSTYLE_EXLAYERED;
	} else if (G->noborder) {
		winstyle = WINDOWSTYLE_NOBORDER;
		winexstyle = 0;
	}

	register_class();
	hwnd = create_window(G->w,G->h,winstyle,winexstyle);

	ShowWindow(hwnd, SW_SHOWDEFAULT);
	UpdateWindow(hwnd);

	//���ô��ڿ��Խ����ļ���ק
	DragAcceptFiles(hwnd,TRUE);

	MSG msg;
	while (G->running && GetMessage(&msg, NULL, 0, 0)) {
		TranslateMessage(&msg);
		DispatchMessage(&msg);
	}

	unregister_class();

destroy:
	game_destroy();

exit:
	game_free();
	return 0;
}
