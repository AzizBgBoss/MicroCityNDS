#include <nds.h>
#include <stdio.h>
#include <fat.h>

#include "Draw.h"
#include "Interface.h"
#include "Game.h"
#include "Simulation.h"

uint16_t ScreenBuffer[SCREEN_WIDTH * SCREEN_HEIGHT];

uint8_t GetInput()
{
  scanKeys();
  int keys = keysHeld();

  uint8_t result = 0;

  if (keys & KEY_A || keys & KEY_X)
  {
    result |= INPUT_B; // Flipped A and B for the comfort of the nintendo gamer
  }
  if (keys & KEY_B || keys & KEY_Y)
  {
    result |= INPUT_A;
  }
  if (keys & KEY_UP)
  {
    result |= INPUT_UP;
  }
  if (keys & KEY_DOWN)
  {
    result |= INPUT_DOWN;
  }
  if (keys & KEY_LEFT)
  {
    result |= INPUT_LEFT;
  }
  if (keys & KEY_RIGHT)
  {
    result |= INPUT_RIGHT;
  }

  return result;
}

void PutPixel(int x, int y, uint8_t colour)
{
  ScreenBuffer[x + y * SCREEN_WIDTH] = colour ? RGB15(31, 31, 31) : RGB15(0, 0, 0);
}

#define BLACK 0
#define WHITE 1

void DrawBitmap(const uint8_t *bitmap, uint8_t x, uint8_t y, uint8_t w, uint8_t h)
{
    for (uint8_t row = 0; row < h; row++)
    {
        if (y + row >= SCREEN_HEIGHT) break;  // clip bottom
        for (uint8_t col = 0; col < w; col++)
        {
            if (x + col >= SCREEN_WIDTH) break;  // clip right
            
            // Calculate which byte and which bit in the bitmap
            uint16_t byteIndex = (row / 8) * w + col;
            uint8_t bitMask = 1 << (row % 8);
            
            // Read bit and decide color
            uint16_t color = (bitmap[byteIndex] & bitMask) ? RGB15(31,31,31) : RGB15(0,0,0);
            
            // Write to buffer
            ScreenBuffer[(y + row) * SCREEN_WIDTH + (x + col)] = color;
        }
    }
}

void SaveCity()
{
  FILE *f = fopen("microcitynds.map", "wb");
  fprintf(f, "CTY1");
  fwrite(&State, sizeof(GameState), 1, f);
  fclose(f);
}

bool LoadCity()
{
  FILE *f = fopen("microcitynds.map", "rb");
  if (!f)
    return false;

  char header[4];
  fread(header, 1, 4, f);
  if (header[0] != 'C' || header[1] != 'T' || header[2] != 'Y' || header[3] != '1')
    return false;

  fread(&State, sizeof(GameState), 1, f);
  fclose(f);
  return true;
}

uint8_t *GetPowerGrid()
{
  static uint8_t PowerGrid[DISPLAY_WIDTH * DISPLAY_HEIGHT / 8];
  return PowerGrid;
}

void loop()
{
  swiWaitForVBlank();
  TickGame();
  memcpy(VRAM_A, ScreenBuffer, sizeof(ScreenBuffer));
}

int main()
{
  videoSetMode(MODE_FB0);
  vramSetBankA(VRAM_A_LCD);

  fatInitDefault();

  InitGame();

  while (1)
    loop();

  return 0;
}
