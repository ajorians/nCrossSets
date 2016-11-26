#include <os.h>
#include <libndls.h>
#include "Game.h"
//#include "Piece.h"
//#include "Indicators.h"

void CreateGame(struct Game** ppGame, const char* pstrLevelData, int nLevelNum, struct Config* pConfig, struct SDL_Surface* pScreen)
{
   *ppGame = malloc(sizeof(struct Game));
   struct Game* pGame = *ppGame;
   CrossLibCreate(&(pGame->m_Cross), pstrLevelData);
   pGame->m_nLevelNum = nLevelNum;
   pGame->m_pConfig = pConfig;
   //pGame->m_bWon = IsSquareGameOver(pGame->m_Square);

   pGame->m_pScreen = pScreen;
   CreateBackground(&(pGame->m_pBackground), pGame->m_pScreen, pGame->m_pConfig);
   pGame->m_pMetrics = NULL;
   CreateMetrics(&pGame->m_pMetrics, pGame->m_Cross);

   int nWidth = GetCrossWidth(pGame->m_Cross);
   int nHeight = GetCrossHeight(pGame->m_Cross);
   int nNumPtrs = nWidth * nHeight;
   pGame->m_apPieces = malloc(nNumPtrs*sizeof(struct Piece));
   for(int x=0; x<nWidth; x++) {
      for(int y=0; y<nHeight; y++) {
         struct Piece* pPiece = &pGame->m_apPieces[x+y*nWidth];
         CreatePiece(pPiece, x, y, pGame->m_Cross, pGame->m_pMetrics);
      }
   }

   pGame->m_pSelector = NULL;
   CreateSelector(&pGame->m_pSelector, pGame->m_pScreen, pGame->m_pConfig, pGame->m_pMetrics);

   /*pGame->m_pIndicators = NULL;
   CreateIndicators(&pGame->m_pIndicators, pGame->m_Square, pGame->m_pMetrics);

   pGame->m_pBackground = NULL;

   pGame->m_pStarDrawer = NULL;
   CreateStarDrawer(&pGame->m_pStarDrawer);*/

   pGame->m_bShouldQuit = 0;
}

void FreeGame(struct Game** ppGame)
{
   struct Game* pGame = *ppGame;
   //FreeMetrics(&pGame->m_pMetrics);
   //pGame->m_pMetrics = NULL;

   int nWidth = GetCrossWidth(pGame->m_Cross);
   int nHeight = GetCrossHeight(pGame->m_Cross);
   for(int x=0; x<nWidth; x++) {
      for(int y=0; y<nHeight; y++) {
         struct Piece* pPiece = &pGame->m_apPieces[x+y*nWidth];
         FreePiece(pPiece);
      }
   }
   free(pGame->m_apPieces);

   FreeSelector(&pGame->m_pSelector);
   //FreeIndicators(&pGame->m_pIndicators);
   FreeBackground(&pGame->m_pBackground);
   FreeMetrics(&pGame->m_pMetrics);
   //FreeStarDrawer(&pGame->m_pStarDrawer);

   pGame->m_pConfig = NULL;//Does not own
   pGame->m_pScreen = NULL;//Does not own

   free(pGame);
   *ppGame = NULL;
}

void DrawBoard(struct Game* pGame)
{
   DrawBackground(pGame->m_pBackground);

   int nWidth = GetCrossWidth(pGame->m_Cross);
   int nHeight = GetCrossHeight(pGame->m_Cross);

   //Draw outlines
#if 0
   gui_gc_setColorRGB(gc, 0, 0, 0);
   for(int x=0; x<=nWidth; x++) {
      gui_gc_drawLine(gc, nLeftBoard + x*nPieceDim, nTopBoard, nLeftBoard + x*nPieceDim, nTopBoard + nHeight*nPieceDim);
   }

   for(int y=0; y<=nHeight; y++) {
      gui_gc_drawLine(gc, nLeftBoard, nTopBoard + y*nPieceDim, nLeftBoard + nWidth*nPieceDim, nTopBoard + y*nPieceDim);
   }
#endif

   //Draw indicators
   //IndicatorsDraw(pGame->m_pIndicators, &pGame->m_gc);

   //Draw pieces
   for(int x=0; x<nWidth; x++) {
      for(int y=0; y<nHeight; y++) {
         struct Piece* pPiece = &pGame->m_apPieces[x+y*nWidth];
         PieceDraw(pPiece, pGame->m_pScreen);
      }
   }

   //Draw selector
   DrawSelector(pGame->m_pSelector);
   SDL_UpdateRect(pGame->m_pScreen, 0, 0, SCREEN_WIDTH, SCREEN_HEIGHT);
}

void UpdateGameWon(struct Game* pGame)
{
   //if( pGame->m_bWon && pGame->m_nLevelNum > 0 && pGame->m_nLevelNum <= 36 ) {
   //   int nStars = GetStarCount(pGame);
   //   SetBeatLevel(pGame->m_pConfig, pGame->m_nLevelNum-1/*To 0-base*/, nStars);
   //}
}

int GamePollEvents(struct Game* pGame)
{
   SDL_Event event;
   while( SDL_PollEvent( &event ) ) {
      switch( event.type ) {
         case SDL_KEYDOWN:
            switch( event.key.keysym.sym) {
              case SDLK_ESCAPE:
                  return 0;
                  break;

               case SDLK_UP:
                  Move(pGame->m_pSelector, Up);
                  break;

	       case SDLK_DOWN:
                  Move(pGame->m_pSelector, Down);
                  break;

               case SDLK_LEFT:
                  Move(pGame->m_pSelector, Left);
                  break;

               case SDLK_RIGHT:
                  Move(pGame->m_pSelector, Right);
                  break;

               case SDLK_RETURN:
               case SDLK_LCTRL:
               case SDLK_RCTRL:
                  ToggleCrossCellValue(pGame->m_Cross, GetCurrentX(pGame->m_pSelector), GetCurrentY(pGame->m_pSelector));
                  break;
  
               default:
                  break;
            }
          default:
             break;
      }
   }
   return 1;
}

int GameLoop(struct Game* pGame)
{
   if( GamePollEvents(pGame) == 0 )
      return 0;

   DrawBoard(pGame);

   SDL_Delay(30);

   /*int nSelectionX = pGame->m_pSelector->m_nSelectionX;
   int nSelectionY = pGame->m_pSelector->m_nSelectionY;
   int nDestroyed = 0, nMarked = 0;
   if( IsKeyPressed(KEY_NSPIRE_ESC) ) {
      pGame->m_bShouldQuit = 0;//Could change to completly close program
      return 0;
   }
   else if( ( IsKeyPressed(KEY_NSPIRE_LEFT) || IsKeyPressed(KEY_NSPIRE_4) ) && pGame->m_bWon != SQUARELIB_GAMEOVER ) {
      wait_no_key_pressed();
      SelectorMove(pGame->m_pSelector, Sel_Left);
   }
   else if( ( IsKeyPressed(KEY_NSPIRE_RIGHT) || IsKeyPressed(KEY_NSPIRE_6) ) && pGame->m_bWon != SQUARELIB_GAMEOVER ) {
      wait_no_key_pressed();
      SelectorMove(pGame->m_pSelector, Sel_Right);
   }
   else if( ( IsKeyPressed(KEY_NSPIRE_UP) || IsKeyPressed(KEY_NSPIRE_8) ) && pGame->m_bWon != SQUARELIB_GAMEOVER ) {
      wait_no_key_pressed();
      SelectorMove(pGame->m_pSelector, Sel_Up);
   }
   else if( ( IsKeyPressed(KEY_NSPIRE_DOWN) || IsKeyPressed(KEY_NSPIRE_2) ) && pGame->m_bWon != SQUARELIB_GAMEOVER ) {
      wait_no_key_pressed();
      SelectorMove(pGame->m_pSelector, Sel_Down);
   }
   else if( IsKeyPressed(KEY_NSPIRE_CTRL) && pGame->m_bWon != SQUARELIB_GAMEOVER ) {
      wait_no_key_pressed();

      IsSquareMarked(pGame->m_Square, nSelectionX, nSelectionY, &nMarked);
      if( nMarked == SQUARELIB_NOT_MARKED ) {

         IsSquareDestroyed(pGame->m_Square, nSelectionX, nSelectionY, &nDestroyed);
         if( nDestroyed == SQUARELIB_NOT_DESTROYED ) {
            DestroySquare(pGame->m_Square, nSelectionX, nSelectionY);

            pGame->m_bWon = IsSquareGameOver(pGame->m_Square);
            if( pGame->m_bWon )
               UpdateGameWon( pGame );
         }

         //Check if destroyed because if not you made a mistake
         IsSquareDestroyed(pGame->m_Square, nSelectionX, nSelectionY, &nDestroyed);
         if( nDestroyed == SQUARELIB_NOT_DESTROYED ) {
            int nWidth = GetSquareWidth(pGame->m_Square);
            struct Piece* pPiece = &pGame->m_apPieces[nSelectionX+nSelectionY*nWidth];
            PieceMistaken(pPiece);
         }

      }
   }
   else if( ( IsKeyPressed(KEY_NSPIRE_SHIFT) || IsKeyPressed(KEY_NSPIRE_ENTER) ) && pGame->m_bWon != SQUARELIB_GAMEOVER ) {
      wait_no_key_pressed();
      IsSquareDestroyed(pGame->m_Square, nSelectionX, nSelectionY, &nDestroyed);
      if( nDestroyed == SQUARELIB_NOT_DESTROYED ) {
         ToggleSquareMark(pGame->m_Square, nSelectionX, nSelectionY);
         pGame->m_bWon = IsSquareGameOver(pGame->m_Square);
         if( pGame->m_bWon ) {
            UpdateGameWon( pGame );
         }
      }
   }*/

   return 1;
}

int GameShouldQuit(struct Game* pGame)
{
   return pGame->m_bShouldQuit;
}

