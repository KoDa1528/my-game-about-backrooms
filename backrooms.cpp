#include <SDL2/SDL.h>
#include <cmath>
#include <unordered_map>
#include <cstdlib>
#include <ctime>
#include <SDL2/SDL_image.h>
float playerAngle = 0.0;
float rotSpeed = 0.05;
float fov = 1.65;
int corridor[10][10] = {
	{1, 1, 1, 1, 1, 0, 1, 1, 1, 1},
	{1, 1, 1, 1, 1, 0, 1, 1, 1, 1},
	{1, 1, 1, 1, 1, 0, 1, 1, 1, 1},
	{1, 1, 1, 1, 1, 0, 1, 1, 1, 1},
	{1, 1, 1, 1, 1, 0, 0, 0, 0, 0},
	{0, 0, 0, 0, 0, 1, 1, 1, 1, 1},
	{1, 1, 1, 1, 0, 1, 1, 1, 1, 1},
	{1, 1, 1, 1, 0, 1, 1, 1, 1, 1},
	{1, 1, 1, 1, 0, 1, 1, 1, 1, 1},
	{1, 1, 1, 1, 0, 1, 1, 1, 1, 1}};
struct Chunk
{
	int tiles[10][10];
	int type;
};
Chunk generatorChunk()
{
	Chunk chunk;
	chunk.type = rand() % 3;
	for (int y = 0; y < 10; y++)
	{
		for (int x = 0; x < 10; x++)
		{
			if (chunk.type == 0)
				chunk.tiles[y][x] = (rand() % 100 < 45) ? 1 : 0;
			else if (chunk.type == 1)
				chunk.tiles[y][x] = (rand() % 100 < 15) ? 1 : 0;
			else if (chunk.type == 2)
			{
				chunk.tiles[y][x] = corridor[y][x];
			}
		}
	}
	return chunk;
}
std::unordered_map<int, Chunk> mir;
int getChunkKey(int cx, int cy)
{
	return (cx << 16) ^ cy;
}
bool isWall(float mirX, float mirY)
{
	int cx = (int)std::floor(mirX / 10.0f);
	int cy = (int)std::floor(mirY / 10.0f);
	int key = getChunkKey(cx, cy);
	if (mir.find(key) == mir.end())
	{
		mir[key] = generatorChunk();
	}
	int lx = (int)mirX % 10;
	int ly = (int)mirY % 10;
	if (lx < 0)
		lx += 10;
	if (ly < 0)
		ly += 10;
	return mir[key].tiles[ly][lx] == 1;
}
bool upPressed = false;
bool downPressed = false;
bool leftPressed = false;
bool rightPressed = false;
bool canMove(float x, float y)
{
	return !isWall(x, y);
}
void updateWorld(float playerX, float playerY)
{
	int cx = (int)std::floor(playerX / 10.0f);
	int cy = (int)std::floor(playerY / 10.0f);

	for (int dy = -1; dy <= 1; dy++)
	{
		for (int dx = -1; dx <= 1; dx++)
		{
			int key = getChunkKey(cx + dx, cy + dy);
			if (mir.find(key) == mir.end())
			{
				mir[key] = generatorChunk();
			}
		}
	}
}
void updateButtons(SDL_Rect &up, SDL_Rect &down,
				   SDL_Rect &left, SDL_Rect &right,
				   int screenW, int screenH)
{
	int size = 100;
	int margin = 20;

	// Левые кнопки
	up.x = margin;
	up.y = screenH - size - margin;

	down.x = margin + size + 10;
	down.y = screenH - size - margin;

	// Правые кнопки
	left.x = screenW - size - margin;
	left.y = screenH - size * 2 - 10 - margin;

	right.x = screenW - size - margin;
	right.y = screenH - size - margin;

	left.w = right.w = up.w = down.w = size;
	left.h = right.h = up.h = down.h = size;
}
void raycast(SDL_Renderer *m, float playerX, float playerY, float angle, int screenW, int screenH, Uint32 *texturePixels, int texWidth, int texHeight)
{
	int rays = screenW;

	for (float x = 0.0; x < rays; x++)
	{
		float rayAngle = angle - fov / 2 + fov * x / rays;

		float rayX = playerX;
		float rayY = playerY;
		float distance = 0;
		float prevX, prevY;

		while (distance < 5)
		{
			prevX = rayX; // ✅ Сохраняем до изменения
			prevY = rayY; // ✅ Сохраняем до изменения
			rayX += cos(rayAngle) * 0.06;
			rayY += sin(rayAngle) * 0.06;
			distance += 0.06;

			if (isWall(rayX, rayY))
			{
				break;
			}
		}

		int wallHeight = 300 / distance;
		SDL_Rect wall;
		wall.x = x;
		wall.y = screenH / 2 - wallHeight / 2;
		wall.w = 1;
		wall.h = wallHeight;

		float wallX = 0.0;
		if (floor(rayX) != floor(prevX))
		{
			wallX = rayY - floor(rayY);
		}
		else
		{
			wallX = rayX - floor(rayX);
		}
		if (wallX < 0)
			wallX = 0;
		if (wallX > 1)
			wallX = 1;

		int texX = (int)(wallX * texWidth);
		int texY = (int)((float)(wall.y - screenH / 2 + wallHeight / 2) / wallHeight * texHeight);

		if (texX < 0)
			texX = 0;
		if (texX >= texWidth)
			texX = texWidth - 1;
		if (texY < 0)
			texY = 0;
		if (texY >= texHeight)
			texY = texHeight - 1;

		Uint32 color = texturePixels[texY * texWidth + texX];
		Uint8 r = (color >> 16) & 0xFF;
		Uint8 g = (color >> 8) & 0xFF;
		Uint8 b = color & 0xFF;
		SDL_SetRenderDrawColor(m, r, g, b, 255);
		//SDL_SetRenderDrawColor(m, (color >> 16) & 0xFF, (color >> 8) & 0xFF, color & 0xFF, 255);
		int shade = 255 - distance * 45;
		SDL_SetRenderDrawColor(m, shade, shade * 0.8, 50, 255);
		SDL_RenderFillRect(m, &wall);
	}
}

void touchMove(
	SDL_Event &sobitie,
	float &playerX,
	float &playerY,
	SDL_Rect &up,
	SDL_Rect &down,
	SDL_Rect &left,
	SDL_Rect &right)
{
	if (sobitie.type == SDL_MOUSEBUTTONDOWN)
	{
		float x = sobitie.button.x;
		float y = sobitie.button.y;

		if (x >= up.x && x <= up.x + up.w &&
			y >= up.y && y <= up.y + up.h)
			upPressed = true;

		if (x >= down.x && x <= down.x + down.w &&
			y >= down.y && y <= down.y + down.h)
			downPressed = true;

		if (x >= left.x && x <= left.x + left.w &&
			y >= left.y && y <= left.y + left.h)
			leftPressed = true;

		if (x >= right.x && x <= right.x + right.w &&
			y >= right.y && y <= right.y + right.h)
			rightPressed = true;
	}
	if (sobitie.type == SDL_KEYDOWN)
	{
		switch (sobitie.key.keysym.sym)
		{
		case SDLK_w:
		case SDLK_UP:
			leftPressed = true;
			break;
		case SDLK_s:
		case SDLK_DOWN:
			rightPressed = true;
			break;
		case SDLK_a:
		case SDLK_LEFT:
			upPressed = true;
			break;
		case SDLK_d:
		case SDLK_RIGHT:
			downPressed = true;
			break;
		}
	}
	if (sobitie.type == SDL_KEYUP)
	{
		switch (sobitie.key.keysym.sym)
		{
		case SDLK_w:
		case SDLK_UP:
			upPressed = false;
			break;
		case SDLK_s:
		case SDLK_DOWN:
			downPressed = false;
			break;
		case SDLK_a:
		case SDLK_LEFT:
			leftPressed = false;
			break;
		case SDLK_d:
		case SDLK_RIGHT:
			rightPressed = false;
			break;
		}
	}
	float speed = 0.025;
	if (downPressed)
	{
		playerAngle += rotSpeed;
	}
	if (upPressed)
	{
		playerAngle -= rotSpeed;
	}

	if (leftPressed)
	{
		float newX = playerX + cos(playerAngle) * speed;
		float newY = playerY + sin(playerAngle) * speed;

		if (canMove(newX, newY))
		{
			playerX = newX;
			playerY = newY;
		}
	}

	if (rightPressed)
	{
		float newX = playerX - cos(playerAngle) * speed;
		float newY = playerY - sin(playerAngle) * speed;

		if (canMove(newX, newY))
		{
			playerX = newX;
			playerY = newY;
		}
	}
}
void draw(SDL_Renderer *m, SDL_Rect *n)
{
	SDL_RenderFillRect(m, n);
}
int main()
{
	float playerX = 2.5, playerY = 2.5;
	/*int map [10][10] = {
    	{1,1,1,1,1,1,1,1,1,1},
    	{1,0,0,0,0,0,0,0,0,1},
    	{1,0,0,0,0,0,0,1,0,1},
    	{1,0,0,0,1,1,0,0,0,1},
    	{1,0,0,0,1,0,0,0,0,1},
    	{1,0,1,0,0,0,0,0,0,1},
    	{1,0,0,0,1,1,0,0,0,1},
    	{1,0,1,1,0,0,0,1,1,1},
    	{1,0,0,0,0,0,0,0,0,1},
    	{1,1,1,1,1,1,1,1,1,1}
    	};*/
	int screenW, screenH;
	bool run = true;
	SDL_Event sobitie;
	SDL_Init(SDL_INIT_VIDEO);
	SDL_Surface *textureSurface = IMG_Load("/storage/emulated/0/wall.jpg");
	if (!textureSurface)
	{
		printf("Ошибка загрузки текстуры\n");
		return 1;
	}

	SDL_Surface *converted = SDL_ConvertSurfaceFormat(textureSurface, SDL_PIXELFORMAT_ABGR8888, 0);
	SDL_FreeSurface(textureSurface);
	textureSurface = converted;

	Uint32 *texturePixels = (Uint32 *)textureSurface->pixels;
	int texWidth = textureSurface->w;
	int texHeight = textureSurface->h;
	SDL_Window *okno;
	okno = SDL_CreateWindow(
		"MemEngine",
		SDL_WINDOWPOS_CENTERED,
		SDL_WINDOWPOS_CENTERED,
		640,
		480,
		SDL_WINDOW_SHOWN | SDL_WINDOW_RESIZABLE);
	SDL_Rect up;
	SDL_Rect down;
	SDL_Rect left;
	SDL_Rect right;
	SDL_Renderer *m;
	m = SDL_CreateRenderer(okno, -1, 1);
	SDL_GetRendererOutputSize(m, &screenW, &screenH);
	updateButtons(up, down, left, right, screenW, screenH);
	while (run)
	{
		SDL_SetRenderDrawColor(m, 0, 0, 0, 255);
		SDL_RenderClear(m);
		updateWorld(playerX, playerY);
		SDL_SetRenderDrawColor(m, 30, 30, 30, 255);
		SDL_Rect floor = {0, screenH / 2, screenW, screenH / 2};
		SDL_RenderFillRect(m, &floor);

		SDL_SetRenderDrawColor(m, 100, 100, 100, 255);
		SDL_Rect ceiling = {0, 0, screenW, screenH / 2};
		SDL_RenderFillRect(m, &ceiling);
		/* Временно закомментируй raycast и вместо него нарисуй текстуру на весь экран
SDL_Rect dst = {0, 0, screenW, screenH};
SDL_Texture* tex = SDL_CreateTextureFromSurface(m, textureSurface);
SDL_RenderCopy(m, tex, NULL, &dst);
SDL_DestroyTexture(tex);*/
		raycast(m, playerX, playerY, playerAngle, screenW, screenH, texturePixels, texWidth, texHeight);
		SDL_SetRenderDrawColor(m, 255, 0, 0, 255);
		SDL_RenderFillRect(m, &up);
		SDL_SetRenderDrawColor(m, 0, 255, 0, 255);
		SDL_RenderFillRect(m, &down);
		SDL_SetRenderDrawColor(m, 0, 0, 255, 255);
		SDL_RenderFillRect(m, &right);
		SDL_SetRenderDrawColor(m, 0, 255, 255, 255);
		SDL_RenderFillRect(m, &left);
		if (SDL_PollEvent(&sobitie))
		{
			if (sobitie.type == SDL_WINDOWEVENT)
			{
				if (sobitie.window.event == SDL_WINDOWEVENT_SIZE_CHANGED)
				{
					SDL_GetRendererOutputSize(m, &screenW, &screenH);
					updateButtons(up, down, left, right, screenW, screenH);
				}
			}
			if (sobitie.type == SDL_QUIT)
			{
				run = false;
			}
		}
		if (sobitie.type == SDL_MOUSEBUTTONUP)
		{
			upPressed = false;
			downPressed = false;
			leftPressed = false;
			rightPressed = false;
		}
		touchMove(sobitie, playerX, playerY, up, down, left, right);
		SDL_Delay(16);
		SDL_RenderPresent(m);
	}
	SDL_DestroyRenderer(m);
	SDL_DestroyWindow(okno);
	SDL_Quit();
}