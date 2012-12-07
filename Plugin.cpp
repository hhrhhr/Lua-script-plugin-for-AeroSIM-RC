#include "Plugin.h"
#include "lua.hpp"
#include <math.h>

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

//
lua_State *L;
float fligthTime = 0.0;
float acc[3] = { 0.0, 0.0, 0.0 };
double gps[2] = { 0.0, 0.0 };
float gps2[2] = { 0.0, 0.0 };

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
    lua_pushstring(L, g_strPluginFolder);   // send package.path
    lua_pcall(L, 1, 0, 0);
}

//-----------------------------------------------------------------------------
void Run_Command_Reset(const TDataFromAeroSimRC *ptDataFromAeroSimRC,
                             TDataToAeroSimRC *ptDataToAeroSimRC)
{
  sprintf(g_strDebugInfo + strlen(g_strDebugInfo), "\nRESET");
}

void InfoText(const TDataFromAeroSimRC *ptDataFromAeroSimRC,
                    TDataToAeroSimRC *ptDataToAeroSimRC)
{
  sprintf(g_strDebugInfo,
    "----------------------------------------------------------------------\n"
    "Plugin Folder = %s\n"
    "Output Folder = %s\n"
    "\nSimulation Data\n" // Simulation Data
    "fIntegrationTimeStep = %f\n"
    "gyro = %f, %f, %f\n"
    "acc = %f, %f, %f\n"
    "lat = %f, lon = %f, alt = %f, heading = %f\n"
    "\nfrom Lua\n" // from Lua
    "Flight time = %f\n"
    ,
    g_strPluginFolder,
    g_strOutputFolder,
    
    // Simulation Data
    ptDataFromAeroSimRC->Simulation_fIntegrationTimeStep,

    ptDataFromAeroSimRC->Model_fAngVel_Body_X,
    ptDataFromAeroSimRC->Model_fAngVel_Body_Y,
    ptDataFromAeroSimRC->Model_fAngVel_Body_Z,

    acc[0],
    acc[1],
    acc[2],

    gps[0],
    gps[1],
    gps2[0],
    gps2[1],
    
    // from Lua
    fligthTime
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
  } else {

/* prepare for call 'main' function */
    lua_getglobal(L, "main");

/* 1 dt*/
    lua_pushnumber(L, ptDataFromAeroSimRC->Simulation_fIntegrationTimeStep);

/* 2 - 4 model gyro */
    lua_pushnumber(L, ptDataFromAeroSimRC->Model_fAngVel_Body_X);
    lua_pushnumber(L, ptDataFromAeroSimRC->Model_fAngVel_Body_Y);
    lua_pushnumber(L, ptDataFromAeroSimRC->Model_fAngVel_Body_Z);

/* 5 - 7 model acc */
    // rotate gravity to model
    float ax = -9.81 * ptDataFromAeroSimRC->Model_fAxisRight_Z;
    float ay = -9.81 * ptDataFromAeroSimRC->Model_fAxisFront_Z;
    float az = -9.81 * ptDataFromAeroSimRC->Model_fAxisUp_Z;
    // add gravity to acc
    ax += ptDataFromAeroSimRC->Model_fAccel_Body_X;
    ay += ptDataFromAeroSimRC->Model_fAccel_Body_Y;
    az += ptDataFromAeroSimRC->Model_fAccel_Body_Z;

    lua_pushnumber(L, ax);
    lua_pushnumber(L, ay);
    lua_pushnumber(L, az);

    acc[0] = ax;
    acc[1] = ay;
    acc[2] = az;

/* 8 - 11 lat/lon/alt/head */
    static float gps_dt = 0.0;
    if (gps_dt < 0.2) {
        gps_dt += ptDataFromAeroSimRC->Simulation_fIntegrationTimeStep;

        lua_pushnumber(L, gps[0]);
        lua_pushnumber(L, gps[1]);
        lua_pushnumber(L, gps2[0]);
        lua_pushnumber(L, gps2[1]);
    } else {
        gps_dt = 0.0;
        double lat = ptDataFromAeroSimRC->Model_fLatitude;
        double lon = ptDataFromAeroSimRC->Model_fLongitude;
        float alt = ptDataFromAeroSimRC->Model_fPosZ;

        double lat1 = gps[0] * M_PI / 180.0;
        double lat2 = lat * M_PI / 180.0;
        double lon1 = gps[1] * M_PI / 180.0;
        double lon2 = lon * M_PI / 180.0;
        double dLon = lon2 - lon1;
        double y = sin(dLon) * cos(lat2);
        double x = cos(lat1) * sin(lat2) - sin(lat1) * cos(lat2) * cos(dLon);

        float head = atan2(y, x) * 180.0 / M_PI;

        lua_pushnumber(L, lat);
        lua_pushnumber(L, lon);
        lua_pushnumber(L, alt);
        lua_pushnumber(L, head);

        gps[0] = lat;
        gps[1] = lon;
        gps2[0] = alt;
        gps2[1] = head;
    };

/* 11 params, 1 returned value */
    int retn = 1;
    lua_pcall(L, 11, retn, 0);

/*  */
    int i = -retn;
    fligthTime = lua_tonumber(L, i++);
    // var = lua_tonumber(L, i++);
    // ...
    lua_pop(L, -retn);
  }

  //-----------------------------------
}

