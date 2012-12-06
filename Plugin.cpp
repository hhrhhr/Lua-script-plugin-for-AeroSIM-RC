#include "Plugin.h"
#include "lua.hpp"

#define WIN32_LEAN_AND_MEAN // Exclude rarely-used stuff from Windows headers
#include <windows.h>
#include <stdio.h>

// Custom Menu Item masks
#define MASK_MENU_ITEM__COMMAND_RESET               (1 <<  0)

//-----------------------------------
bool g_bFirstRun = true;            // Used to initialise menu checkboxes showing OSD with HUD
char g_strDebugInfo[10000] = "";    // static to remain in memory when function exits
char g_strPluginFolder[MAX_PATH];
char g_strOutputFolder[MAX_PATH];

lua_State *L;
float fligthTime = 0.0;
float gee[3] = {0.0, 0.0, 0.0};

//-----------------------------------------------------------------------------
BOOL APIENTRY DllMain( HANDLE hModule,
                       DWORD  ul_reason_for_call,
                       LPVOID lpReserved)
{
  switch (ul_reason_for_call)
  {
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

AeroSIMRC_DLL_EXPORT void AeroSIMRC_Plugin_ReportStructSizes(unsigned long *pnSizeOf_TDataFromAeroSimRC,
                                                             unsigned long *pnSizeOf_TDataToAeroSimRC,
                                                             unsigned long *pnSizeOf_TPluginInit)
{
  *pnSizeOf_TDataFromAeroSimRC = sizeof(TDataFromAeroSimRC );
  *pnSizeOf_TDataToAeroSimRC   = sizeof(TDataToAeroSimRC   );
  *pnSizeOf_TPluginInit        = sizeof(TPluginInit        );
}


//-----------------------------------------------------------------------------
AeroSIMRC_DLL_EXPORT void AeroSIMRC_Plugin_Init(TPluginInit *ptPluginInit)
{
    // Save paths for later use
    strcpy(g_strPluginFolder, ptPluginInit->strPluginFolder);
    strcpy(g_strOutputFolder, ptPluginInit->strOutputFolder);

    L = luaL_newstate();
    luaL_openlibs(L); /* Load Lua libraries */

    char strFullScriptName[MAX_PATH];
    strcpy(strFullScriptName, g_strPluginFolder);
    strcat(strFullScriptName, "\\lua\\script.lua");

    luaL_loadfile(L, strFullScriptName);
    lua_pushstring(L, g_strPluginFolder);
    lua_pcall(L, 1, 0, 0);
}

//-----------------------------------------------------------------------------
void Run_Command_Reset(const TDataFromAeroSimRC  *ptDataFromAeroSimRC, TDataToAeroSimRC *ptDataToAeroSimRC)
{
  sprintf(g_strDebugInfo + strlen(g_strDebugInfo), "\nRESET");
}

void InfoText(const TDataFromAeroSimRC *ptDataFromAeroSimRC, TDataToAeroSimRC *ptDataToAeroSimRC)
{
  sprintf(g_strDebugInfo,
    "----------------------------------------------------------------------\n"
    "Plugin Folder = %s\n"
    "Output Folder = %s\n"
    "nStructSize = %d\n"
    "\n" // Simulation Data
    "fIntegrationTimeStep = %f\n"
    "\n" // from Lua
    "Flight time = %f\n"
    "gee = %f, %f, %f\n"
    ,
    g_strPluginFolder,
    g_strOutputFolder,
    ptDataFromAeroSimRC->nStructSize,
    
    // Simulation Data
    ptDataFromAeroSimRC->Simulation_fIntegrationTimeStep,

    // from Lua
    fligthTime,
    gee[0], gee[1], gee[2]
  );
}

//-----------------------------------------------------------------------------
// This function is called at each program cycle from AeroSIM RC
//-----------------------------------------------------------------------------
AeroSIMRC_DLL_EXPORT void AeroSIMRC_Plugin_Run(const TDataFromAeroSimRC  *ptDataFromAeroSimRC,
                                                     TDataToAeroSimRC    *ptDataToAeroSimRC)
{
  // debug info is shown on the screen
  ptDataToAeroSimRC->Debug_pucOnScreenInfoText = g_strDebugInfo;
  InfoText(ptDataFromAeroSimRC, ptDataToAeroSimRC);

  // By default do not change the Menu Items of type CheckBox
  ptDataToAeroSimRC->Menu_nFlags_MenuItem_New_CheckBox_Status = ptDataFromAeroSimRC->Menu_nFlags_MenuItem_Status;

  // Extract Menu Commands from Flags
  bool bCommand_Reset = (ptDataFromAeroSimRC->Menu_nFlags_MenuItem_Status & MASK_MENU_ITEM__COMMAND_RESET) != 0;

  if(g_bFirstRun) {
    //
    g_bFirstRun = false;
  }

  // Clear Override flags
  ptDataToAeroSimRC->Model_nOverrideFlags = 0;

  // Run commands
  if(bCommand_Reset) {
    Run_Command_Reset(ptDataFromAeroSimRC, ptDataToAeroSimRC);
  } else { // run checkboxes code
/* call 'main' function */
    lua_getglobal(L, "main");
/* 1 dt*/
    lua_pushnumber(L, ptDataFromAeroSimRC->Simulation_fIntegrationTimeStep);
/* 2 - 10 matrix*/
    lua_pushnumber(L, ptDataFromAeroSimRC->Model_fAxisRight_X);
    lua_pushnumber(L, ptDataFromAeroSimRC->Model_fAxisRight_Y);
    lua_pushnumber(L, ptDataFromAeroSimRC->Model_fAxisRight_Z);

    lua_pushnumber(L, ptDataFromAeroSimRC->Model_fAxisFront_X);
    lua_pushnumber(L, ptDataFromAeroSimRC->Model_fAxisFront_Y);
    lua_pushnumber(L, ptDataFromAeroSimRC->Model_fAxisFront_Z);

    lua_pushnumber(L, ptDataFromAeroSimRC->Model_fAxisUp_X);
    lua_pushnumber(L, ptDataFromAeroSimRC->Model_fAxisUp_Y);
    lua_pushnumber(L, ptDataFromAeroSimRC->Model_fAxisUp_Z);
/* 11 - 13 model acc */
    lua_pushnumber(L, ptDataFromAeroSimRC->Model_fAccel_Body_X);
    lua_pushnumber(L, ptDataFromAeroSimRC->Model_fAccel_Body_Y);
    lua_pushnumber(L, ptDataFromAeroSimRC->Model_fAccel_Body_Z);
/* 14 - 16 model gyro */
    lua_pushnumber(L, ptDataFromAeroSimRC->Model_fAngVel_Body_X);
    lua_pushnumber(L, ptDataFromAeroSimRC->Model_fAngVel_Body_Y);
    lua_pushnumber(L, ptDataFromAeroSimRC->Model_fAngVel_Body_Z);

/* 16 param, 4 returned value */
    lua_pcall(L, 16, 4, 0);

    //
    fligthTime = lua_tonumber(L, -4);
    gee[0] = lua_tonumber(L, -3);
    gee[1] = lua_tonumber(L, -2);
    gee[2] = lua_tonumber(L, -1);
    lua_pop(L, 4);
  }

  //-----------------------------------
}

