#include "Plugin.h"
#include "lua.hpp"

#define WIN32_LEAN_AND_MEAN // Exclude rarely-used stuff from Windows headers
#include <windows.h>
#include <stdio.h>
#include <math.h>

// Custom Menu Item masks
#define MASK_MENU_ITEM__COMMAND_RELOAD_LUA_SCRIPTS  (1 << 0)
#define MASK_MENU_ITEM__COMMAND_WINDOW_SIZE_AND_POS (1 << 1)

bool g_bFirstRun = true;            // Used to initialise menu checkboxes showing OSD with HUD
char g_strDebugInfo[10000] = "";    // static to remain in memory when function exits
char g_luaDebugInfo[1024] = "";
char g_lua2DebugInfo[1024] = "";
char g_strPluginFolder[MAX_PATH];
char g_strOutputFolder[MAX_PATH];

//
lua_State *L;
float acc[3];

//-----------------------------------------------------------------------------
BOOL APIENTRY DllMain( HANDLE hModule,
                       DWORD  ul_reason_for_call,
                       LPVOID lpReserved)
{
    switch (ul_reason_for_call) {
    case DLL_PROCESS_ATTACH:
        break;
    case DLL_THREAD_ATTACH:
        break;
    case DLL_THREAD_DETACH:
        break;
    case DLL_PROCESS_DETACH:
        // Free here any resources used by the plugin
        lua_close(L);   /* Cya, Lua */
        break;
    }
    return TRUE;
}

AeroSIMRC_DLL_EXPORT void AeroSIMRC_Plugin_ReportStructSizes(
        unsigned long *pnSizeOf_TDataFromAeroSimRC,
        unsigned long *pnSizeOf_TDataToAeroSimRC,
        unsigned long *pnSizeOf_TPluginInit)
{
    *pnSizeOf_TDataFromAeroSimRC = sizeof(TDataFromAeroSimRC);
    *pnSizeOf_TDataToAeroSimRC   = sizeof(TDataToAeroSimRC  );
    *pnSizeOf_TPluginInit        = sizeof(TPluginInit       );
}

// Lua errors handler --------------------------------------------------------
int errorHandler(lua_State* L)
{
    //stack: err
    const char* err = lua_tostring(L, 1);
    sprintf(g_luaDebugInfo,
            "Error code: %s\n", err);

    lua_getglobal(L, "debug"); // stack: err debug
    lua_getfield(L, -1, "traceback"); // stack: err debug debug.traceback

    // debug.traceback() return 1 value
    if(lua_pcall(L, 0, 1, 0)) {
        const char* err = lua_tostring(L, -1);
        sprintf(g_luaDebugInfo + strlen(g_luaDebugInfo),
                "Error in debug.traceback() call: %s\n", err);
    } else {
        const char* stackTrace = lua_tostring(L, -1);
        sprintf(g_luaDebugInfo + strlen(g_luaDebugInfo),
                "C++ stack traceback: %s\n", stackTrace);
    }

    return 1;
}

void Lua_Init()
{
    if (!g_bFirstRun)
        lua_close(L);

    L = luaL_newstate();
    luaL_openlibs(L);

    lua_pushcfunction(L, errorHandler);

    char strFullScriptName[MAX_PATH];
    strcpy(strFullScriptName, g_strPluginFolder);
    strcat(strFullScriptName, "\\lua\\main.lua");
    luaL_loadfile(L, strFullScriptName);

    // set package.path
    char luaPackagePath[MAX_PATH];
    strcpy(luaPackagePath, g_strPluginFolder);
    strcat(luaPackagePath, "\\lua\\?.lua");

    lua_getglobal(L, "package");
    lua_pushstring(L, luaPackagePath);
    lua_setfield(L, -2, "path");
    lua_pop(L, 1);

    // first call of script
    lua_pcall(L, 0, 0, -2);
    lua_pop(L, 1);
}

//-----------------------------------------------------------------------------
AeroSIMRC_DLL_EXPORT void AeroSIMRC_Plugin_Init(const TPluginInit *ptPluginInit)
{
    // Save paths for later use
    strcpy(g_strPluginFolder, ptPluginInit->strPluginFolder);
    strcpy(g_strOutputFolder, ptPluginInit->strOutputFolder);

    Lua_Init();
}

//-----------------------------------------------------------------------------
void Run_Command_WindowSizeAndPos(const TDataFromAeroSimRC *ptDataFromAeroSimRC,
                                  TDataToAeroSimRC *ptDataToAeroSimRC)
{
    short x = 0, y = 0, w = -1, h = -1;

    lua_getglobal(L, "next_resolution");
    lua_pcall(L, 0, 4, 0);
    x = lua_tointeger(L, 1);
    y = lua_tointeger(L, 2);
    w = lua_tointeger(L, 3);
    h = lua_tointeger(L, 4);
    lua_pop(L, 4);

    if (w == -1)
        w = ptDataFromAeroSimRC->Win_nScreenSizeDX;
    if (h == -1)
        h = ptDataFromAeroSimRC->Win_nScreenSizeDY;

    ptDataToAeroSimRC->Win_nNewPosX   = x;
    ptDataToAeroSimRC->Win_nNewPosY   = y;
    ptDataToAeroSimRC->Win_nNewSizeDX = w;
    ptDataToAeroSimRC->Win_nNewSizeDY = h;
}


void InfoText(const TDataFromAeroSimRC *ptDataFromAeroSimRC,
              TDataToAeroSimRC *ptDataToAeroSimRC)
{
    sprintf(g_strDebugInfo,
            "LUA_ERROR: %s\n"

            "\n--- simulation data ---\n"
            "fIntegrationTimeStep = %f\n"
            "gyro = % 2.6f, % 2.6f, % 2.6f\n"
            "acc  = % 2.6f, % 2.6f, % 2.6f\n"
            "lat = %f, lon = %f, alt = %.2f\n"

            "\n--- from Lua ---\n"
            "%s\n"
            ,
            g_luaDebugInfo,

            // Simulation Data
            ptDataFromAeroSimRC->Simulation_fIntegrationTimeStep,

            ptDataFromAeroSimRC->Model_fAngVel_Body_X,
            ptDataFromAeroSimRC->Model_fAngVel_Body_Y,
            ptDataFromAeroSimRC->Model_fAngVel_Body_Z,

            acc[0], acc[1], acc[2],

            ptDataFromAeroSimRC->Model_fLatitude,
            ptDataFromAeroSimRC->Model_fLongitude,
            ptDataFromAeroSimRC->Model_fPosZ,

            // from Lua
            g_lua2DebugInfo
            );
}

//-----------------------------------------------------------------------------
// This function is called at each program cycle from AeroSIM RC
//-----------------------------------------------------------------------------
AeroSIMRC_DLL_EXPORT void AeroSIMRC_Plugin_Run(const TDataFromAeroSimRC *ptDataFromAeroSimRC,
                                               TDataToAeroSimRC *ptDataToAeroSimRC)
{
    // debug info is shown on the screen
    ptDataToAeroSimRC->Debug_pucOnScreenInfoText = g_strDebugInfo;
    InfoText(ptDataFromAeroSimRC, ptDataToAeroSimRC);

    // By default do not change the Menu Items of type CheckBox
    ptDataToAeroSimRC->Menu_nFlags_MenuItem_New_CheckBox_Status = ptDataFromAeroSimRC->Menu_nFlags_MenuItem_Status;

    // Extract Menu Commands from Flags
    bool bCommand_ReloadLuaScripts = (ptDataFromAeroSimRC->Menu_nFlags_MenuItem_Status & MASK_MENU_ITEM__COMMAND_RELOAD_LUA_SCRIPTS) != 0;
    bool bCommand_WindowSizeAndPos = (ptDataFromAeroSimRC->Menu_nFlags_MenuItem_Status & MASK_MENU_ITEM__COMMAND_WINDOW_SIZE_AND_POS) != 0;

    if(g_bFirstRun) {
        //
        g_bFirstRun = false;
    }

    // Clear Override flags
    ptDataToAeroSimRC->Model_nOverrideFlags = 0;

    // Run commands
    if (bCommand_ReloadLuaScripts) {
        Lua_Init();
    } else if (bCommand_WindowSizeAndPos) {
        Run_Command_WindowSizeAndPos(ptDataFromAeroSimRC, ptDataToAeroSimRC);
    } else {
        // rotate gravity (0.0, 0.0, -9.81) to model
        acc[0] = -9.81 * ptDataFromAeroSimRC->Model_fAxisRight_Z;
        acc[1] = -9.81 * ptDataFromAeroSimRC->Model_fAxisFront_Z;
        acc[2] = -9.81 * ptDataFromAeroSimRC->Model_fAxisUp_Z;
        // add gravity to acc
        acc[0] += ptDataFromAeroSimRC->Model_fAccel_Body_X;
        acc[1] += ptDataFromAeroSimRC->Model_fAccel_Body_Y;
        acc[2] += ptDataFromAeroSimRC->Model_fAccel_Body_Z;

        // set global tables in Lua ----------------------------------------------------------------
        lua_getglobal(L, "RAW");
        {
            /* menu */
            lua_pushstring(L, "menu");
            lua_pushinteger(L, ptDataFromAeroSimRC->Menu_nFlags_MenuItem_Status);
            lua_rawset(L, -3);

            /* dt*/
            lua_pushstring(L, "dt");
            lua_pushnumber(L, ptDataFromAeroSimRC->Simulation_fIntegrationTimeStep);
            lua_rawset(L, -3);

            /* model gyro */
            lua_pushstring(L, "gyr");
            lua_gettable(L, -2);
            {
                lua_pushstring(L, "x");
                lua_pushnumber(L, ptDataFromAeroSimRC->Model_fAngVel_Body_X);
                lua_rawset(L, -3);

                lua_pushstring(L, "y");
                lua_pushnumber(L, ptDataFromAeroSimRC->Model_fAngVel_Body_Y);
                lua_rawset(L, -3);

                lua_pushstring(L, "z");
                lua_pushnumber(L, ptDataFromAeroSimRC->Model_fAngVel_Body_Z);
                lua_rawset(L, -3);
            }
            lua_pop(L, 1);

            /* model acc */
            lua_pushstring(L, "acc");
            lua_gettable(L, -2);
            {
                lua_pushstring(L, "x");
                lua_pushnumber(L, acc[0]);
                lua_rawset(L, -3);

                lua_pushstring(L, "y");
                lua_pushnumber(L, acc[1]);
                lua_rawset(L, -3);

                lua_pushstring(L, "z");
                lua_pushnumber(L, acc[2]);
                lua_rawset(L, -3);
            }
            lua_pop(L, 1);

            /* lat/lon/alt */
            lua_pushstring(L, "gps");
            lua_gettable(L, -2);
            {
                lua_pushstring(L, "lat");
                lua_pushnumber(L, ptDataFromAeroSimRC->Model_fLatitude);
                lua_rawset(L, -3);

                lua_pushstring(L, "lon");
                lua_pushnumber(L, ptDataFromAeroSimRC->Model_fLongitude);
                lua_rawset(L, -3);

                lua_pushstring(L, "alt");
                lua_pushnumber(L, ptDataFromAeroSimRC->Model_fPosZ);
                lua_rawset(L, -3);
            }
            lua_pop(L, 1);
        }
        lua_pop(L, 1);

        lua_getglobal(L, "TX");
        for (int i = 0; i < AEROSIMRC_MAX_CHANNELS; i++) {
            lua_pushinteger(L, i+1);
            lua_pushnumber(L, ptDataFromAeroSimRC->Channel_afValue_TX[i]);
            lua_rawset(L, -3);
        }
        lua_pop(L, 1);

//float Model_fAxisRight_X; float Model_fAxisRight_Y; float Model_fAxisRight_Z;
//float Model_fAxisFront_X; float Model_fAxisFront_Y; float Model_fAxisFront_Z;
//float Model_fAxisUp_X;    float Model_fAxisUp_Y;    float Model_fAxisUp_Z;

        lua_getglobal(L, "M");
        const float *p = &ptDataFromAeroSimRC->Model_fAxisRight_X;
        for (int i = 0; i < 9; i++) {
            lua_pushinteger(L, i+1);
            lua_pushnumber(L, p[i]);
            lua_rawset(L, -3);
        }
        lua_pop(L, 1);

        // call 'main' function --------------------------------------------------------------------
        lua_pushcfunction(L, errorHandler);
        lua_getglobal(L, "main");
        int result = lua_pcall(L, 0, 0, -2);
        lua_pop(L, 1);

        // get values from Lua ---------------------------------------------------------------------
        if (!result) {
            strcpy(g_luaDebugInfo, "no errors");

            lua_getglobal(L, "DBGSTR");
            strcpy(g_lua2DebugInfo, lua_tostring(L, -1));
            lua_pop(L, 1);

            lua_getglobal(L, "RX");
            for (int i = 0; i < AEROSIMRC_MAX_CHANNELS; i++) {
                lua_rawgeti(L, -1, i+1);
                ptDataToAeroSimRC->Channel_abOverride_RX[i] = true;
                ptDataToAeroSimRC->Channel_afNewValue_RX[i] = lua_tonumber(L, -1);
                lua_pop(L, 1);
            }
            lua_pop(L, 1);
        }
    }
    //-----------------------------------
}

