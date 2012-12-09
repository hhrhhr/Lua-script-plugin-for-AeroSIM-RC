#include "Plugin.h"
#include "lua.hpp"

#define WIN32_LEAN_AND_MEAN // Exclude rarely-used stuff from Windows headers
#include <windows.h>
#include <stdio.h>
#include <math.h>

// Custom Menu Item masks
#define MASK_MENU_ITEM__COMMAND_RESET               (1 <<  0)

bool g_bFirstRun = true;            // Used to initialise menu checkboxes showing OSD with HUD
char g_strDebugInfo[10000] = "";    // static to remain in memory when function exits
char g_strPluginFolder[MAX_PATH];
char g_strOutputFolder[MAX_PATH];

//
lua_State *L;
float fligthTime = 0.0;
float dt = 0.0;
float gyro[3] = { 0.0, 0.0, 0.0 };
float acc[3] = { 0.0, 0.0, 0.0 };
double gpsd[2] = { 0.0, 0.0 };
float gpsf[2] = { 0.0, 0.0 };
float rx[10] = { 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0 };

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
    strcat(strFullScriptName, "\\lua\\main.lua");
    luaL_loadfile(L, strFullScriptName);

    // send package.path
    char luaPackagePath[MAX_PATH];
    strcpy(luaPackagePath, g_strPluginFolder);
    strcat(luaPackagePath, "\\lua\\?.lua");

    lua_getglobal(L, "package");
    lua_pushstring(L, luaPackagePath);
    lua_setfield(L, -2, "path");
    lua_pop(L, 1);

    lua_pcall(L, 0, 0, 0);
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
            "TX = (%+1.4f %+1.4f %+1.4f %+1.4f %+1.4f %+1.4f %+1.4f %+1.4f %+1.4f)\n"
            "\nfrom Lua\n" // from Lua
            "Flight time = %f\n"
            "RX = (%+1.4f %+1.4f %+1.4f %+1.4f %+1.4f %+1.4f %+1.4f %+1.4f %+1.4f %+1.4f)\n"
            ,
            g_strPluginFolder,
            g_strOutputFolder,

            // Simulation Data
            ptDataFromAeroSimRC->Simulation_fIntegrationTimeStep,

            ptDataFromAeroSimRC->Model_fAngVel_Body_X,
            ptDataFromAeroSimRC->Model_fAngVel_Body_Y,
            ptDataFromAeroSimRC->Model_fAngVel_Body_Z,

            acc[0], acc[1], acc[2],

            gpsd[0], gpsd[1], gpsf[0], gpsf[1],

            ptDataFromAeroSimRC->Channel_afValue_TX[CH_AILERON],
            ptDataFromAeroSimRC->Channel_afValue_TX[CH_ELEVATOR],
            ptDataFromAeroSimRC->Channel_afValue_TX[CH_THROTTLE],
            ptDataFromAeroSimRC->Channel_afValue_TX[CH_RUDDER],
            ptDataFromAeroSimRC->Channel_afValue_TX[CH_5],
            ptDataFromAeroSimRC->Channel_afValue_TX[CH_6],
            ptDataFromAeroSimRC->Channel_afValue_TX[CH_7],
            ptDataFromAeroSimRC->Channel_afValue_TX[CH_PLUGIN_1],
            ptDataFromAeroSimRC->Channel_afValue_TX[CH_PLUGIN_2],


            // from Lua
            fligthTime,
            rx[0], rx[1], rx[2], rx[3], rx[4], rx[5], rx[6], rx[7], rx[8], rx[9]
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
        //  prepare vars ---------------------------------------------------------------------------
        dt = ptDataFromAeroSimRC->Simulation_fIntegrationTimeStep;

        gyro[0] = ptDataFromAeroSimRC->Model_fAngVel_Body_X;
        gyro[1] = ptDataFromAeroSimRC->Model_fAngVel_Body_Y;
        gyro[2] = ptDataFromAeroSimRC->Model_fAngVel_Body_Z;

        // rotate gravity (0.0, 0.0, -9.81) to model
        acc[0] = -9.81 * ptDataFromAeroSimRC->Model_fAxisRight_Z;
        acc[1] = -9.81 * ptDataFromAeroSimRC->Model_fAxisFront_Z;
        acc[2] = -9.81 * ptDataFromAeroSimRC->Model_fAxisUp_Z;
        // add gravity to acc
        acc[0] += ptDataFromAeroSimRC->Model_fAccel_Body_X;
        acc[1] += ptDataFromAeroSimRC->Model_fAccel_Body_Y;
        acc[2] += ptDataFromAeroSimRC->Model_fAccel_Body_Z;

        static float gps_dt = 0.0;
        if (gps_dt < 0.2) {
            gps_dt += ptDataFromAeroSimRC->Simulation_fIntegrationTimeStep;
        } else {
            gps_dt = 0.0;
            double lat = ptDataFromAeroSimRC->Model_fLatitude;
            double lon = ptDataFromAeroSimRC->Model_fLongitude;
            if ( (fabs(lat - gpsd[0]) > 0.0000001) or (fabs(lon - gpsd[1]) > 0.0000001) ) {
                double lat1 = gpsd[0]   * M_PI / 180.0;
                double lat2 = lat       * M_PI / 180.0;
                double lon1 = gpsd[1]   * M_PI / 180.0;
                double lon2 = lon       * M_PI / 180.0;
                double dLon = lon2 - lon1;
                double y = sin(dLon) * cos(lat2);
                double x = cos(lat1) * sin(lat2) - sin(lat1) * cos(lat2) * cos(dLon);
                float head = atan2(y, x) * 180.0 / M_PI;

                gpsd[0] = lat;
                gpsd[1] = lon;
                gpsf[0] = ptDataFromAeroSimRC->Model_fPosZ;
                gpsf[1] = head;
            }
        };

        // set global tables in Lua ----------------------------------------------------------------
        lua_getglobal(L, "RAW");
        {
            /* dt*/
            lua_pushstring(L, "dt");
            lua_pushnumber(L, dt);
            lua_rawset(L, -3);

            /* model gyro */
            lua_pushstring(L, "gyro");
            lua_gettable(L, -2);
            {
                lua_pushstring(L, "x");
                lua_pushnumber(L, gyro[0]);
                lua_rawset(L, -3);

                lua_pushstring(L, "y");
                lua_pushnumber(L, gyro[1]);
                lua_rawset(L, -3);

                lua_pushstring(L, "z");
                lua_pushnumber(L, gyro[2]);
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

            /* lat/lon/alt/head */
            lua_pushstring(L, "gps");
            lua_gettable(L, -2);
            {
                lua_pushstring(L, "lat");
                lua_pushnumber(L, gpsd[0]);
                lua_rawset(L, -3);

                lua_pushstring(L, "lon");
                lua_pushnumber(L, gpsd[1]);
                lua_rawset(L, -3);

                lua_pushstring(L, "alt");
                lua_pushnumber(L, gpsf[0]);
                lua_rawset(L, -3);

                lua_pushstring(L, "head");
                lua_pushnumber(L, gpsf[1]);
                lua_rawset(L, -3);
            }
            lua_pop(L, 1);
        }
        lua_pop(L, 1);

        int i = 1;
        lua_getglobal(L, "TX");
        {
            lua_pushinteger(L, i++);
            lua_pushnumber(L, ptDataFromAeroSimRC->Channel_afValue_TX[CH_AILERON]);
            lua_rawset(L, -3);

            lua_pushinteger(L, i++);
            lua_pushnumber(L, ptDataFromAeroSimRC->Channel_afValue_TX[CH_ELEVATOR]);
            lua_rawset(L, -3);

            lua_pushinteger(L, i++);
            lua_pushnumber(L, ptDataFromAeroSimRC->Channel_afValue_TX[CH_THROTTLE]);
            lua_rawset(L, -3);

            lua_pushinteger(L, i++);
            lua_pushnumber(L, ptDataFromAeroSimRC->Channel_afValue_TX[CH_RUDDER]);
            lua_rawset(L, -3);

            lua_pushinteger(L, i++);
            lua_pushnumber(L, ptDataFromAeroSimRC->Channel_afValue_TX[CH_5]);
            lua_rawset(L, -3);

            lua_pushinteger(L, i++);
            lua_pushnumber(L, ptDataFromAeroSimRC->Channel_afValue_TX[CH_6]);
            lua_rawset(L, -3);

            lua_pushinteger(L, i++);
            lua_pushnumber(L, ptDataFromAeroSimRC->Channel_afValue_TX[CH_7]);
            lua_rawset(L, -3);

            lua_pushinteger(L, i++);
            lua_pushnumber(L, ptDataFromAeroSimRC->Channel_afValue_TX[CH_PLUGIN_1]);
            lua_rawset(L, -3);

            lua_pushinteger(L, i++);
            lua_pushnumber(L, ptDataFromAeroSimRC->Channel_afValue_TX[CH_PLUGIN_2]);
            lua_rawset(L, -3);
        }
        lua_pop(L, 1);

        /* 0 params, 2 returned value */
        // call 'main' function --------------------------------------------------------------------
        int retn = 2;
        lua_getglobal(L, "main");
        lua_pcall(L, 0, retn, 0);

        // get values from Lua ---------------------------------------------------------------------
        i = -retn;
        fligthTime = lua_tonumber(L, i++);

        lua_pushnil(L);
        for (int j = 0; j < 10; j++) {
            lua_next(L, -2);
            //lua_tointeger(L, -2); // key not needed
            rx[j] = lua_tonumber(L, -1);
            lua_pop(L, 1);
        }
        lua_pop(L, 1);

        // var = lua_tonumber(L, i++);
        // ...
        lua_pop(L, -retn);

        ptDataToAeroSimRC->Channel_abOverride_RX[CH_AILERON]  = true;
        ptDataToAeroSimRC->Channel_afNewValue_RX[CH_AILERON]  = rx[0];
        ptDataToAeroSimRC->Channel_abOverride_RX[CH_ELEVATOR] = true;
        ptDataToAeroSimRC->Channel_afNewValue_RX[CH_ELEVATOR] = rx[1];
        ptDataToAeroSimRC->Channel_abOverride_RX[CH_RUDDER]   = true;
        ptDataToAeroSimRC->Channel_afNewValue_RX[CH_RUDDER]   = rx[3];
    }

    //-----------------------------------
}

