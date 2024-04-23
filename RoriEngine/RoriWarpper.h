#pragma once

namespace RoriDef {

	const int TargetTPS = 30;
	const int TargetFPS = 5000;

	const int WindowW = 768, WindowH = 768;

	struct Color {
		unsigned __int8 r, g, b, a;//a:transparency
		Color(unsigned __int8 r2 = 0, unsigned __int8 g2 = 0, unsigned __int8 b2 = 0, unsigned __int8 a2 = 0):r(r2), g(g2), b(b2), a(a2){}
		const Color operator*(const Color *that) const {//see https://blog.csdn.net/mkr67n/article/details/117026093
			unsigned __int16	r1 = this->r, g1 = this->g, b1 = this->b, a1 = this->a,
								r2 = that->r, g2 = that->g, b2 = that->b, a2 = that->a,
								rr, gr, br, ar;
			ar = a1 + a2 - a1 * a2;
			rr = (r1 * a1 + (r2 * a2) * (1 - a1)) / ar;
			gr = (g1 * a1 + (g2 * a2) * (1 - a1)) / ar;
			br = (b1 * a1 + (b2 * a2) * (1 - a1)) / ar;
			return Color((unsigned __int8)rr, (unsigned __int8)gr, (unsigned __int8)br, (unsigned __int8)ar);
		}
	};



	struct Image {
		int width = 0, height = 0;
		Color* data=nullptr;
	};
	Image EmptyImage;
}
#include<easyx.h>

namespace PlatformAPI {
	namespace Graphical {
		typedef HWND WindowType;

		WindowType EmptyWindow = (WindowType)nullptr;

		inline int GetScreenWidth(void) { return GetSystemMetrics(SM_CXFULLSCREEN); }
		inline int GetScreenHeight(void) { return GetSystemMetrics(SM_CYFULLSCREEN); }

		inline COLORREF ColorCov(RoriDef::Color color) { return RGB(color.r, color.g, color.b); }//conv RoriColor to PlatformColor
		
		WindowType InitGraph(int width, int height) {return (WindowType)initgraph(width,height, EX_SHOWCONSOLE);}

		void BeginDraw(WindowType Window) { BeginBatchDraw(); }
		void FlushDraw(WindowType Window) { FlushBatchDraw(); }
		void EndDraw(WindowType Window) { EndBatchDraw(); }

		unsigned __int32 tmpp = 0;

		inline void SyncToScreen(int width, int height, RoriDef::Image img) {
			DWORD* Buffer = GetImageBuffer();
			RoriDef::Color col;
			for (int py = 0; py < height; py++) {
				tmpp++;
				for (int px = 0; px < width; px++) {
					col = img.data[px + py * width];
					Buffer[px + py * width] = BGR(RGB(col.r,col.g,col.b));
					//RoriUsefuls::Log(std::to_string(px) + " " + std::to_string(py) + " " + std::to_string(px + py * WindowW));
				}
			}
		}
		//inline void DrawPixel(int x, int y, RoriDef::Color color) {putpixel(x, y, ColorCov(color));}

	}

	namespace FileAccess {
		
		struct RGBTRIPLE_MEM {
			BYTE rgbtBlue;
			BYTE rgbtGreen;
			BYTE rgbtRed;
			BYTE empty;
		};

		#pragma warning(push)
		#pragma warning(disable:6386)

		void LoadImage(RoriDef::Image& img, std::wstring loc) {
			if (img.data != nullptr) delete[] img.data;

			IMAGE rawimg;
			union { RGBTRIPLE_MEM* tri; DWORD* dword; }bufferConv;
			loadimage(&rawimg, (LPCTSTR)(loc.c_str()));
			bufferConv.dword = GetImageBuffer(&rawimg);

			int w = rawimg.getwidth(), h = rawimg.getheight();
			std::cout<<"loading img with"<<w<<' '<<h;
			img.height = h; img.width = w;
			img.data = new RoriDef::Color[w * h];
			for (int j = 0; j < h; j++) {
				for (int i = 0; i < w; i++) {
					img.data[i + j * w].r = bufferConv.tri[i + j * w].rgbtRed;
					img.data[i + j * w].g = bufferConv.tri[i + j * w].rgbtGreen;
					img.data[i + j * w].b = bufferConv.tri[i + j * w].rgbtBlue;
					img.data[i + j * w].a = 255;
				}
			}
		}
		#pragma warning(pop)
	}
}