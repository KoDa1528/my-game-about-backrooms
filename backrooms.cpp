#include <SDL2/SDL.h>
#include <cmath>
#include <unordered_map>
#include <cstdlib>
#include <ctime>
#include <SDL2/SDL_mixer.h>
#include <SDL2/SDL_ttf.h>
//=объявление глобальных переменных=
TTF_Font *fontRec = NULL;
int recBlinkTime = 0;
bool recDotVisible = true;
float playerAngle = 0.0;
float rotSpeed = 0.05;
const float fov = 1.65;
int frame = 0;
Mix_Chunk *lampSound = NULL;
Mix_Chunk *stepSound = NULL;
int stepTimer = 0;
const int corridor[10][10] = {
	{1, 1, 1, 1, 1, 0, 1, 1, 1, 1},
	{1, 1, 1, 1, 1, 0, 1, 1, 1, 1},
	{1, 1, 1, 1, 1, 0, 1, 1, 1, 1},
	{1, 1, 1, 1, 1, 0, 1, 1, 1, 1},
	{1, 1, 1, 1, 0, 0, 0, 0, 0, 0},
	{0, 0, 0, 0, 0, 1, 1, 1, 1, 1},
	{1, 1, 1, 1, 0, 1, 1, 1, 1, 1},
	{1, 1, 1, 1, 0, 1, 1, 1, 1, 1},
	{1, 1, 1, 1, 0, 1, 1, 1, 1, 1},
	{1, 1, 1, 1, 0, 1, 1, 1, 1, 1}};
	const int nemap [10][10] = {
    	{0,0,0,0,0,0,0,0,0,0},
    	{1,0,1,0,1,0,1,0,1,0},
    	{0,0,0,0,0,0,0,0,0,0},
    	{0,1,0,1,0,1,0,1,0,1},
    	{0,0,0,0,0,0,0,0,0,0},
    	{1,0,1,0,1,0,1,0,1,0},
    	{0,0,0,0,0,0,0,0,0,0},
    	{0,1,0,1,0,1,0,1,0,1},
    	{0,0,0,0,0,0,0,0,0,0},
    	{1,0,1,0,1,0,1,0,1,0}
    	};
	float vhsShakeX = 0.0;
	float vhsShakeY = 0.0;
struct Chunk
{
	int tiles[10][10];
	int type;
};
//=========генерация мира===========
Chunk generatorChunk()
{
	Chunk chunk;
	chunk.type = rand() % 4;
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
			} else if(chunk.type == 3){
				chunk.tiles[y][x] = nemap[y][x];
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
//========булевы переменные========
bool rotatedLeft = false;
bool moveBack = false;
bool moveForwad = false;
bool rotatedRight = false;
bool canMove(float x, float y)
{
	return !isWall(x, y);
}
//=======создавание атмосферы=====
void horror(SDL_Renderer *m, int screenH, int screenW, int viewX, int viewWidht)
{
	frame++;
	//зернистность
	for (int i = 0; i < 175; i++)
	{
		int x = viewX + (rand() % viewWidht);
		int y = rand() % screenH;
		int size = rand() % 3 + 1;
		int yarkoct = rand() % 60;
		SDL_SetRenderDrawColor(m, yarkoct, yarkoct, yarkoct, 255);
		SDL_Rect pixel = {x, y, size, size};
		SDL_RenderFillRect(m, &pixel);
	}
	//сдвиги и разрыв кадров
		if(rand() % 100 < 5){
			int shiftY = rand() % 50 - 10;
			SDL_Rect react = {viewX, screenH * 4 / shiftY, viewWidht, 4};
			SDL_SetRenderDrawColor(m, 100, 100, 255, 40);
			SDL_RenderFillRect(m, &react);
			}
		if(rand() % 200 < 1){
			int shiftX = rand() % 6-3;
			SDL_SetRenderDrawColor(m, 0, 0, 50, 15);
			SDL_Rect react = {viewX + shiftX, 0, viewWidht, screenH};
			SDL_RenderFillRect(m, &react);
			}
		if(rand() % 500 < 4){
			int y = rand() % screenH;
			SDL_SetRenderDrawColor(m, 100, 255, 100, 200);
			SDL_Rect react = {viewX, y, viewWidht, rand() % 8 + 2};
			SDL_RenderFillRect(m, &react);
			if(rand() % 2){
    SDL_SetRenderDrawColor(m, 0, 0, 0, 255);
    SDL_Rect react_js = {viewX, y + 10, viewWidht, rand() % 30 + 20};
    SDL_RenderFillRect(m, &react_js);
    
    // === NO SIGNAL ===
    const char* msg[] = {
    	"ERROR 404",
    	"NO SIGNAL",
    	"REC END"
    	};
    	int idx = rand() % 3;
    SDL_Color colorK = {255, 255, 255, 255};  // Белый
    SDL_Surface* surfK = TTF_RenderText_Blended(fontRec, msg[idx], colorK);
    if (!surfK) return;
    
    SDL_Texture* texK = SDL_CreateTextureFromSurface(m, surfK);
    if (!texK) {
        SDL_FreeSurface(surfK);
        return;
    }
    
    // Центрируем на чёрной полосе
    SDL_Rect rectNoSignal = {
        viewX + viewWidht/2 - surfK->w/2,
        y + 10 + react_js.h/2 - surfK->h/2,
        surfK->w,
        surfK->h
    };
    SDL_RenderCopy(m, texK, NULL, &rectNoSignal);
    
    SDL_FreeSurface(surfK);
    SDL_DestroyTexture(texK);
}
			}
}
//=========отображение даты===========
void drawDate(SDL_Renderer* m, int screenH, int viewX) {
    // Полупрозрачный чёрный фон
    SDL_SetRenderDrawColor(m, 0, 0, 0, 180);
    SDL_Rect bg = {viewX + 10, screenH - 35, 210, 25};
    SDL_RenderFillRect(m, &bg);

    // Текст "2000-01-01"
    SDL_Color colorDate = {255, 255, 255, 220};
    SDL_Surface* surfDate = TTF_RenderText_Blended(fontRec, "2000-01-01", colorDate);
    if (!surfDate) return;

    SDL_Texture* texDate = SDL_CreateTextureFromSurface(m, surfDate);
    if (!texDate) {
        SDL_FreeSurface(surfDate);
        return;
    }

    SDL_Rect rectDate = {viewX + 18, screenH - 30, surfDate->w, surfDate->h};
    SDL_RenderCopy(m, texDate, NULL, &rectDate);

    SDL_FreeSurface(surfDate);
    SDL_DestroyTexture(texDate);
}

//=========отображение REC===========
void drawREC(SDL_Renderer* m, int viewX) {
    // Мигающий красный кружок
    if (recDotVisible) {
        SDL_SetRenderDrawColor(m, 255, 0, 0, 255);
        SDL_Rect dot = {viewX + 15, 15, 12, 12};
        SDL_RenderFillRect(m, &dot);
    }

    // Текст "REC"
    SDL_Color colorRec = {255, 0, 0, 255};
    SDL_Surface* surfRec = TTF_RenderText_Blended(fontRec, "REC", colorRec);
    if (!surfRec) return;

    SDL_Texture* texRec = SDL_CreateTextureFromSurface(m, surfRec);
    if (!texRec) {
        SDL_FreeSurface(surfRec);
        return;
    }

    SDL_Rect rectRec = {viewX + 32, 10, surfRec->w, surfRec->h};
    SDL_RenderCopy(m, texRec, NULL, &rectRec);

    SDL_FreeSurface(surfRec);
    SDL_DestroyTexture(texRec);
}
//=========обновление мира=========
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
/*
===================================
|| рейкастинг и отрисовка 3д графики  ||
===================================
*/
void raycast(SDL_Renderer *m, float playerX, float playerY, float angle, int screenW, int screenH, int viewX, int viewWidht)
{
	int rays = viewWidht;

	for (float x = 0.0; x < rays; x++)
	{
		float rayAngle = angle - fov / 2 + fov * x / rays;

		float rayX = playerX;
		float rayY = playerY;
		float distance = 0;
		float prevX, prevY;
		
float shakeOffsetX = vhsShakeX * 0.1;
float shakeOffsetY = vhsShakeY * 0.1;

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
// При отрисовке стены:
wall.x = viewX + x + shakeOffsetX;
wall.y = screenH / 2 - wallHeight / 2 + shakeOffsetY;
		wall.w = 1;
		wall.h = wallHeight;
		int shade = 255 - distance * 45;
		SDL_SetRenderDrawColor(m, shade, shade * 0.8, 50, 255);
		SDL_RenderFillRect(m, &wall);
	}
	// Чёрные полосы по бокам (если экран шире 4:3)
	if (viewX > 0)
	{
		SDL_SetRenderDrawColor(m, 0, 0, 0, 255);
		SDL_Rect leftBar = {0, 0, viewX, screenH};
		SDL_Rect rightBar = {viewX + viewWidht, 0, screenW - viewX - viewWidht, screenH};
		SDL_RenderFillRect(m, &leftBar);
		SDL_RenderFillRect(m, &rightBar);
	}
}
//==функция по получению касаний===
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
			rotatedLeft = true;

		if (x >= down.x && x <= down.x + down.w &&
			y >= down.y && y <= down.y + down.h)
			rotatedRight = true;

		if (x >= left.x && x <= left.x + left.w &&
			y >= left.y && y <= left.y + left.h)
			moveForwad = true;

		if (x >= right.x && x <= right.x + right.w &&
			y >= right.y && y <= right.y + right.h)
			moveBack = true;
	}
	if (sobitie.type == SDL_KEYDOWN)
	{
		switch (sobitie.key.keysym.sym)
		{
		case SDLK_w:
		case SDLK_UP:
			moveForwad = true;
			break;
		case SDLK_s:
		case SDLK_DOWN:
			moveBack = true;
			break;
		case SDLK_a:
		case SDLK_LEFT:
			rotatedLeft = true;
			break;
		case SDLK_d:
		case SDLK_RIGHT:
			rotatedRight = true;
			break;
		}
	}
	if (sobitie.type == SDL_KEYUP)
	{
		switch (sobitie.key.keysym.sym)
		{
		case SDLK_w:
		case SDLK_UP:
			moveForwad = false;
			break;
		case SDLK_s:
		case SDLK_DOWN:
			moveBack = false;
			break;
		case SDLK_a:
		case SDLK_LEFT:
			rotatedLeft = false;
			break;
		case SDLK_d:
		case SDLK_RIGHT:
			rotatedRight = false;
			break;
		}
	}
	float speed = 0.025;
	if (rotatedRight)
	{
		playerAngle += rotSpeed;
	}
	if (rotatedLeft)
	{
		playerAngle -= rotSpeed;
	}

	if (moveForwad)
	{
		float newX = playerX + cos(playerAngle) * speed;
		float newY = playerY + sin(playerAngle) * speed;

		if (canMove(newX, newY))
		{
			playerX = newX;
			playerY = newY;
		}
	}

	if (moveBack)
	{
		float newX = playerX - cos(playerAngle) * speed;
		float newY = playerY - sin(playerAngle) * speed;

		if (canMove(newX, newY))
		{
			playerX = newX;
			playerY = newY;
		}
	}
	//========звук шагов===========
	if (moveForwad || moveBack)
	{
		stepTimer++;
		if (stepTimer == 30)
		{
			if (stepSound)
			{
				Mix_PlayChannel(-1, stepSound, 0);
				stepTimer = 0;
			}
		}
		else if (stepTimer == 0)
		{
			stepTimer = 0;
		}
	}
}
//========главная функция==========
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
	SDL_Init(SDL_INIT_VIDEO | SDL_INIT_AUDIO);
	TTF_Init();
	fontRec = TTF_OpenFont("assets/PressStart2P-Regular.ttf", 20);
	// Инициализация звука
	Mix_OpenAudio(44100, MIX_DEFAULT_FORMAT, 2, 2048);

	// Загрузка звуков
	lampSound = Mix_LoadWAV("assets/ambient.wav");
	stepSound = Mix_LoadWAV("assets/steps.wav");

	Mix_VolumeChunk(lampSound, 40);
	Mix_VolumeChunk(stepSound, 80);
	// Запустить гул ламп (бесконечно)
	if (lampSound)
		Mix_PlayChannel(-1, lampSound, -1);
	SDL_Window *okno;
	okno = SDL_CreateWindow(
		"ICE",
		SDL_WINDOWPOS_CENTERED,
		SDL_WINDOWPOS_CENTERED,
		620,
		480,
		SDL_WINDOW_SHOWN | SDL_WINDOW_RESIZABLE);
	SDL_Rect up;
	SDL_Rect down;
	SDL_Rect left;
	SDL_Rect right;
	SDL_Renderer *m;
	m = SDL_CreateRenderer(okno, -1, 1);
	SDL_GetRendererOutputSize(m, &screenW, &screenH);
	int viewWidht = screenH * 4 / 3;
	int viewX = (screenW - viewWidht) / 2;
	updateButtons(up, down, left, right, screenW, screenH);
	//==========главный цикл===========
	while (run)
	{
		SDL_SetRenderDrawColor(m, 0, 0, 0, 255);
		SDL_RenderClear(m);
		updateWorld(playerX, playerY);
		SDL_SetRenderDrawColor(m, 30, 30, 30, 255);
		SDL_Rect floor = {viewX, screenH / 2, viewWidht, screenH / 2};
		SDL_RenderFillRect(m, &floor);

		SDL_SetRenderDrawColor(m, 100, 100, 100, 255);
		SDL_Rect ceiling = {viewX, 0, viewWidht, screenH / 2};
		SDL_RenderFillRect(m, &ceiling);
		raycast(m, playerX, playerY, playerAngle, screenW, screenH, viewX, viewWidht);
		// === Дрожание камеры (VHS-тряска) ===
if (rand() % 100 < 10) {
    vhsShakeX = (rand() % 6 - 3) * 0.5;
    vhsShakeY = (rand() % 6 - 3) * 0.5;
} else {
    vhsShakeX *= 0.9;
    vhsShakeY *= 0.9;
}
		horror(m, screenH, screenW, viewX, viewWidht);
		SDL_SetRenderDrawColor(m, 255, 0, 0, 255);
		SDL_RenderFillRect(m, &up);
		SDL_SetRenderDrawColor(m, 0, 255, 0, 255);
		SDL_RenderFillRect(m, &down);
		SDL_SetRenderDrawColor(m, 0, 0, 255, 255);
		SDL_RenderFillRect(m, &right);
		SDL_SetRenderDrawColor(m, 0, 255, 255, 255);
		SDL_RenderFillRect(m, &left);
		//=========обработка событий======
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
				goto end;
			}
		}
		if (sobitie.type == SDL_MOUSEBUTTONUP)
		{
			rotatedLeft = false;
			moveBack = false;
			moveForwad = false;
			rotatedRight = false;
		}
		touchMove(sobitie, playerX, playerY, up, down, left, right);
		recBlinkTime++;
if (recBlinkTime >= 60) {
    recBlinkTime = 0;
    recDotVisible = !recDotVisible;
}
drawDate(m, screenH, viewX);
drawREC(m, viewX);
		SDL_Delay(16);
		SDL_RenderPresent(m);
	}
	end:
		Mix_FreeChunk(lampSound);
		Mix_FreeChunk(stepSound);
		Mix_CloseAudio();
		SDL_DestroyRenderer(m);
		SDL_DestroyWindow(okno);
		SDL_Quit();
		TTF_Quit();
}