#include<SDL2/SDL.h>
#include<SDL2/SDL_image.h>
#include<fstream>
#include<cstdlib>
#include<string>

Uint8 palette[256][3];

Uint8 FindIndex(Uint8 r, Uint8 g, Uint8 b)
{
	for(unsigned a = 0; a < 256; a++)
	{
		if(palette[a][0] == r && palette[a][1] == g && palette[a][2] == b)
			return a;
	}

	return 0;
}

void Convert(char* fileIn, char* fileOut)
{
	auto img_tmp = IMG_Load(fileIn);
	auto img = SDL_ConvertSurfaceFormat(img_tmp, SDL_PIXELFORMAT_RGBA8888, 0);
	SDL_FreeSurface(img_tmp);

	std::ofstream f(fileOut, std::ios::binary);

	Uint32 w = img->w;
	Uint32 h = img->h;
	f.write((char*)&w, sizeof(w));
	f.write((char*)&h, sizeof(h));

	for(unsigned y = 0; y < h; y++)
	{
		for(unsigned x = 0; x < w; x++)
		{
			Uint32* pixels = (Uint32*)img->pixels;
			Uint8 cr, cg, cb;
			SDL_GetRGB(pixels[y * w + x], img->format, &cr, &cg, &cb);

			Uint8 paletteIndex = FindIndex(cr, cg, cb);
			f.write((char*)&paletteIndex, 1);
		}
	}

	f.close();
	SDL_FreeSurface(img);
}

#undef main
int main(int argc, char* argv[])
{
	SDL_Init(SDL_INIT_VIDEO);
	IMG_Init(IMG_INIT_PNG);

	if(argc != 3)
	{
		printf("Usage: %s input output\n", argv[0]);
		return 1;
	}

	SDL_Window* wnd;
	SDL_Renderer* rend;
	SDL_CreateWindowAndRenderer(320, 240, SDL_WINDOW_HIDDEN, &wnd, &rend);

	std::ifstream f("mode13h_palette.txt");
	for(unsigned a = 0; a < 256; a++)
	{
		std::string tmp;
		f >> tmp;

		Uint32 v;
		v = strtoul(tmp.c_str(), NULL, 16);
		Uint8 cr, cg, cb;
		cr = (v >> 16) & 0xff;
		cg = (v >> 8) & 0xff;
		cb = (v >> 0) & 0xff;

		palette[a][0] = cr;
		palette[a][1] = cg;
		palette[a][2] = cb;

		// printf("Color: %d %d %d\n", cr, cg, cb);
	}

	if(f.fail())
		return 1;

	Convert(argv[1], argv[2]);

	printf("Ok~!\n");
	f.close();
	IMG_Quit();
	SDL_Quit();
	return 0;
}