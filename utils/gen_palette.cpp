#include<SDL2/SDL.h>
#include<SDL2/SDL_image.h>
#include<fstream>
#include<cstdlib>

#undef main
int main()
{
	SDL_Init(SDL_INIT_VIDEO);
	IMG_Init(IMG_INIT_PNG);

	SDL_Window* wnd;
	SDL_Renderer* rend;
	SDL_CreateWindowAndRenderer(320, 240, SDL_WINDOW_HIDDEN, &wnd, &rend);

	std::ofstream f("mode13h_palette.txt");

	auto img = IMG_Load("palette.png");
	img = SDL_ConvertSurfaceFormat(img, SDL_PIXELFORMAT_RGBA8888, 0);
	printf("IMG bpp: %d\n", img->format->BitsPerPixel);

	for(unsigned y = 0; y < img->h; y++)
	{
		for(unsigned x = 0; x < img->w; x++)
		{
			Uint8 cr, cg, cb;
			Uint32 pixel = ((Uint32*)img->pixels)[y * img->w + x];
			SDL_GetRGB(pixel, img->format, &cr, &cg, &cb);

			char tmp[64];
			sprintf(tmp, "FF%02x%02x%02x\n", cr, cg, cb);
			f << tmp;
		}
	}

	f.close();
	IMG_Quit();
	SDL_Quit();
	return 0;
}