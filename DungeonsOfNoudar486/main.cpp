#if defined(WINVER)
#include <windows.h>
#endif

#ifdef __DJGPP__
#else
const long UCLOCKS_PER_SEC = 1000;

long uclock();

#endif


//general stuff below
#include <cstdlib>
#include <cstdio>
#include <cmath>
#include <cstring>
#include <functional>
#include <unordered_map>
#include <memory>
#include <utility>
#include <string>
#include <map>
#include <algorithm>
#include <chrono>
#include <sg14/fixed_point>
#include <EASTL/vector.h>
#include <EASTL/array.h>

using eastl::vector;
using eastl::array;
using namespace std::chrono;
using sg14::fixed_point;

#include "NativeBitmap.h"
#include "Vec2i.h"
#include "IMapElement.h"
#include "CTeam.h"
#include "CItem.h"
#include "CActor.h"
#include "CGameDelegate.h"
#include "CMap.h"
#include "IRenderer.h"
#include "IFileLoaderDelegate.h"
#include "CGame.h"
#include "Vec2i.h"
#include "IMapElement.h"
#include "CTeam.h"
#include "CItem.h"
#include "CActor.h"
#include "CGameDelegate.h"
#include "CMap.h"
#include "IRenderer.h"
#include "commands/IGameCommand.h"
#include "RasterizerCommon.h"
#include "CRenderer.h"
#include "CPackedFileReader.h"
#include "LoadPNG.h"

#ifdef __EMSCRIPTEN__
#include <emscripten/html5.h>
#include <emscripten/emscripten.h>
#endif

enum class ESoundDriver {
    kNone, kPcSpeaker, kOpl2Lpt, kTandy, kCovox
};

enum class EScreenState {
    kIntro, kLoading, kChapter, kGame, kGameOver, kVictory, kExit
};

ESoundDriver soundDriver = ESoundDriver::kNone;
EScreenState screenState = EScreenState::kIntro;

std::shared_ptr<odb::NativeBitmap> stateBG;
std::string stateText = "";
std::string stateTransitionText = "";
uint8_t bgColour;
uint8_t textStartingLine;
clock_t diff = 0;
std::shared_ptr<odb::CPackedFileReader> fileLoader = nullptr;

void setScreenState( EScreenState newState, std::shared_ptr<Knights::CGame> game ) {

    switch( newState ) {
        case EScreenState::kIntro:
            stateBG = loadPNG("logo.png", fileLoader);
            stateText = fileLoader->loadFileFromPath("title.txt");
            stateTransitionText = "Press Enter to continue";
            bgColour = 0;
            textStartingLine = 9;
            break;
        case EScreenState::kLoading:
            stateBG = loadPNG("intro.png", fileLoader);
            stateText = "";
            stateTransitionText = "Loading...";
            bgColour = 0;
            textStartingLine = 9;
            break;
        case EScreenState::kChapter:
            char buffer[41];
            snprintf(buffer, 39, "chapter%d.txt", game->getLevelNumber());
            stateBG = loadPNG("intro.png", fileLoader);
            stateText = fileLoader->loadFileFromPath(buffer);;
            stateTransitionText = "Press Enter to continue";
            bgColour = 0;
            textStartingLine = 9;
            break;
        case EScreenState::kGame:
            break;
        case EScreenState::kVictory:
            stateBG = loadPNG("finis_gloriae_mundi.png", fileLoader);
            stateText = fileLoader->loadFileFromPath("chapter7.txt");
            stateTransitionText = "                    Press enter to exit";
            bgColour = odb::renderer->getPaletteEntry(0xFFFFFFFF);
            textStartingLine = 2;
            break;
        case EScreenState::kGameOver:
            stateBG = loadPNG("in_ictu_oculi.png", fileLoader);
            stateText = fileLoader->loadFileFromPath("gameover.txt");
            stateTransitionText = "                    Press enter to exit";
            bgColour = odb::renderer->getPaletteEntry(0xFFFFFFFF);
            textStartingLine = 2;
            break;
        case EScreenState::kExit:
#ifdef __EMSCRIPTEN__
            game->setIsPlaying(true);
#else
            game->setIsPlaying(false);
#endif
            break;
    }

    screenState = newState;
}

void onContinuePressed( std::shared_ptr<Knights::CGame> game) {
    switch( screenState ) {
        case EScreenState::kIntro:
            game->playLevel(0);
            break;
        case EScreenState::kLoading:
            setScreenState( EScreenState::kChapter, game);
            break;
        case EScreenState::kChapter:
            setScreenState( EScreenState::kGame, game);
            break;
        case EScreenState::kGame:
            break;
        case EScreenState::kVictory:
            setScreenState( EScreenState::kExit, game);
            break;
        case EScreenState::kGameOver:
            setScreenState( EScreenState::kExit, game);
            break;
        case EScreenState::kExit:
#ifdef __EMSCRIPTEN__
            game->setIsPlaying(true);
            setScreenState( EScreenState::kIntro, game);
#endif
            break;
    }
}

void initOPL2();

void playTune(const std::string &);

void setupOPL2();

void stopSounds();

void soundTick();

void muteSound();

void playMusic(const std::string &musicTrack);

namespace odb {
    std::shared_ptr<CRenderer> renderer = nullptr;
}


void initOPL2() {
    setupOPL2();
}

void *operator new[](size_t size, const char *pName, int flags, unsigned debugFlags,
                     const char *file, int line) {
    return new uint8_t[size];
}

void *operator new[](size_t size, size_t alignment, size_t alignmentOffset, const char *pName,
                     int flags, unsigned debugFlags, const char *file, int line) {
    return new uint8_t[size];;
}

void showText(std::shared_ptr<odb::NativeBitmap> bg, const std::string &mainText, const std::string &bottom,
              uint8_t bgSolidColour, uint8_t startingLine) {


    odb::renderer->fill(0, 0, 320, 200, bgSolidColour);
    odb::renderer->drawBitmap(0, 0, bg);

    uint8_t mainTextColour = odb::renderer->getPaletteEntry(0xFFFF0000);
    uint8_t bottomTextColour = odb::renderer->getPaletteEntry(0xFF888888);
    odb::renderer->drawTextAt(1, startingLine, mainText.c_str(), mainTextColour);
    odb::renderer->drawTextAt(1, 25, bottom.c_str(), bottomTextColour);
    odb::renderer->flip();
}

void renderTick(long ms) {
    switch (screenState ) {
        case EScreenState::kGame:
            odb::renderer->render(ms);
            break;
        default:
            showText(stateBG, stateText, stateTransitionText, bgColour, textStartingLine );
            break;
    }

    odb::renderer->handleSystemEvents();
}

std::shared_ptr<Knights::CGame> game;

void playerIsDead() {
    setScreenState(EScreenState::kGameOver, game);
}

void handleConsoleLines(Knights::CommandType command, int playerHealthDiff, int targetHealthDiff,
                        std::shared_ptr<odb::CRenderer> renderer, std::shared_ptr<Knights::CActor> actorAtTarget) {

    uint8_t playerDealtDamageColour = renderer->getPaletteEntry(0xFFFF0000);
    uint8_t playerTookDamageColour = renderer->getPaletteEntry(0xFF0000FF);
    uint8_t playerGainedFaithColour = renderer->getPaletteEntry(0xFF00FF00);
    uint8_t commandColour = renderer->getPaletteEntry(0xFFFFFFFF);
    uint8_t effectColour = renderer->getPaletteEntry(0xFF888888);

    char *soundToPlay = nullptr;

    if (command != '.') {
        char buffer[41];
        snprintf(&buffer[0], 39, "%s", game->getLastCommand().c_str());
        renderer->appendToLog(buffer, commandColour);

        auto say = game->getMap()->getAvatar()->getCurrentSay();
        if (!say.empty()) {
            snprintf(&buffer[0], 39, "%s", say.c_str());
            renderer->appendToLog(buffer, effectColour);
            game->getMap()->getAvatar()->setCurrentSay("");
        }
    }


    if (command == Knights::kUseCurrentItemInInventoryCommand) {
        playTune("i114t1o8f");
    }

    if (command == Knights::kCycleRightInventoryCommand) {
        playTune("t200i98a");
    }

    if (command == Knights::kPickItemCommand) {
        playTune("t200i9o1fa");
    }

    if (targetHealthDiff < 0) {
        char buffer[41];
        snprintf(&buffer[0], 39, "Player dealt %d on %s", targetHealthDiff, actorAtTarget->getName().c_str());
        renderer->appendToLog(buffer, playerDealtDamageColour);
        if (actorAtTarget == nullptr || !actorAtTarget->isAlive()) {
            renderer->addDeathAt(actorAtTarget->getPosition());
            playTune("t200i101o8ao4ao2ao1a");
        } else {
            renderer->addSplatAt(actorAtTarget->getPosition());
            playTune("t200i101o3afo1a");
        }
    }

    if (playerHealthDiff < 0) {
        char buffer[41];
        snprintf(&buffer[0], 39, "Player took %d of damage", std::abs(playerHealthDiff));
        renderer->appendToLog(buffer, playerTookDamageColour);
        renderer->startDamageHighlight();
        playTune("t200i53o3fo1f");
    } else if (playerHealthDiff > 0) {
        char buffer[41];
        snprintf(&buffer[0], 39, "Player gained %d of faith", std::abs(playerHealthDiff));
        renderer->appendToLog(buffer, playerGainedFaithColour);
        renderer->startHealHighlight();
        playTune("t200i52o4defg");
    }
}

void loopTick() {

    int healthAtTargetBefore = 0;
    int healthAtTargetAfter = 0;
    clock_t t0 = 0;
    clock_t t1 = 0;

    t0 = uclock();

    auto playerHealthBefore = game->getMap()->getAvatar()->getHP();
    auto cursorPosition = game->getMap()->getTargetProjection(game->getMap()->getAvatar());
    auto actorAtTarget = game->getMap()->getActorAt(cursorPosition);

    if (actorAtTarget != nullptr) {
        healthAtTargetBefore = actorAtTarget->getHP();
    } else {
        healthAtTargetBefore = 0;
    }

    game->tick();
#ifdef __EMSCRIPTEN__
    diff = 50;
#endif
    renderTick(diff);
    Knights::CommandType command = odb::renderer->peekInput();
    game->tick();
    soundTick();

    if (actorAtTarget != nullptr) {
        healthAtTargetAfter = actorAtTarget->getHP();
    } else {
        healthAtTargetAfter = 0;
    }

    auto targetHealthDiff = healthAtTargetAfter - healthAtTargetBefore;
    auto playerHealthAfter = game->getMap()->getAvatar()->getHP();
    auto playerHealthDiff = playerHealthAfter - playerHealthBefore;

    handleConsoleLines(command, playerHealthDiff, targetHealthDiff, odb::renderer, actorAtTarget);

    t1 = uclock();
    diff = (1000 * (t1 - t0)) / UCLOCKS_PER_SEC;
    if (diff == 0) {
        diff = 1;
    }

    if (!game->getMap()->getAvatar()->isAlive()) {
        playerIsDead();
    }

    if ( command == 13 ) {

        odb::renderer->mBufferedCommand = '.';
        onContinuePressed(game);
    }
}

#ifdef __APPLE__
extern "C" int SDL_main(int argc, char **argv) {
#else

#if defined(WINVER)

namespace odb {
    LRESULT CALLBACK WindProcedure(HWND hWnd, UINT Msg, WPARAM wParam, LPARAM lParam);
}

int WINAPI WinMain(HINSTANCE hInstance, HINSTANCE hPrevInstance,
                   LPSTR lpCmdLine, int nCmdShow)
{

        HINSTANCE hInst;

        WNDCLASSEX WndCls;
        static char szAppName[] = "DungeonsOfNoudar95";
        MSG Msg;

        hInst = hInstance;
        WndCls.cbSize = sizeof(WndCls);
        WndCls.style = CS_OWNDC | CS_VREDRAW | CS_HREDRAW;
        WndCls.lpfnWndProc = odb::WindProcedure;
        WndCls.cbClsExtra = 0;
        WndCls.cbWndExtra = 0;
        WndCls.hInstance = hInst;
        WndCls.hIcon = LoadIcon(NULL, IDI_APPLICATION);
        WndCls.hCursor = LoadCursor(NULL, IDC_ARROW);
        WndCls.hbrBackground = (HBRUSH) GetStockObject(WHITE_BRUSH);
        WndCls.lpszMenuName = NULL;
        WndCls.lpszClassName = szAppName;
        WndCls.hIconSm = LoadIcon(hInstance, IDI_APPLICATION);
        RegisterClassEx(&WndCls);

        CreateWindowEx(WS_EX_OVERLAPPEDWINDOW,
                       szAppName,
                       "Dungeons of Noudar 95",
                       WS_OVERLAPPEDWINDOW | WS_VISIBLE,
                       CW_USEDEFAULT,
                       CW_USEDEFAULT,
                       320,
                       240,
                       NULL,
                       NULL,
                       hInstance,
                       NULL);

#else

int main(int argc, char **argv) {
    puts("Dungeons of Noudar 3D startup. Gonna load some stuff...");

    if (argc >= 2) {
        if (!std::strcmp(argv[1], "opl2lpt")) {
            soundDriver = ESoundDriver::kOpl2Lpt;
            initOPL2();
            playTune("t200i101o1a");
        }
    }

#endif

#endif

    const auto LEVEL_LIMIT = 7;

    auto delegate = std::make_shared<Knights::CGameDelegate>();
    fileLoader = std::make_shared<odb::CPackedFileReader>("data.pfs");
    odb::renderer = std::make_shared<odb::CRenderer>(fileLoader);
    game = std::make_shared<Knights::CGame>(fileLoader, odb::renderer, delegate);


    setScreenState(EScreenState::kIntro, game);

    auto onLevelWillLoad = [&]() {

        if (game != nullptr) {
            if (game->getLevelNumber() >= LEVEL_LIMIT) {

#ifndef __EMSCRIPTEN__
                game->setIsPlaying(false);
#endif

                setScreenState(EScreenState::kVictory, game);
                return;
            }
        }

        setScreenState(EScreenState::kLoading, game);
        renderTick(50);
    };

    delegate->setOnLevelWillLoadCallback(onLevelWillLoad);

    auto onLevelLoaded = [&]() {

        if (game->getLevelNumber() < LEVEL_LIMIT) {
            auto tileProperties = odb::loadTileProperties(game->getLevelNumber(), fileLoader);
            odb::renderer->loadTextures(odb::loadTexturesForLevel(game->getLevelNumber(), fileLoader),
                                        tileProperties);
        }

        setScreenState(EScreenState::kChapter, game);
    };
    delegate->setOnLevelLoadedCallback(onLevelLoaded);

#ifdef __EMSCRIPTEN__
    emscripten_set_main_loop(loopTick, 20, 1);
#else
    while (game->isPlaying()) {
        loopTick();
    }
#endif

    if (soundDriver != ESoundDriver::kNone) {
        stopSounds();
    }

    return 0;
}
