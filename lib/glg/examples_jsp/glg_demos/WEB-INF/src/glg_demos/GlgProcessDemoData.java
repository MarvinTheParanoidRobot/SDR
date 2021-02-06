package glg_demos;

import java.util.*;

// Simulated data for the demo. In an application, the data may come from 
// any datasource, such as a PLC or process database.
  //
public class GlgProcessDemoData
{
   // Constants
   static final int UPDATE_INTERVAL = 1000;   // milliseconds

   static final double
     PROCESS_SPEED = 0.15,
     HEATER_LEVEL_SPEED = 0.15,
     WATER_LEVEL_SPEED = 0.06,
     VALVE_CHANGE_SPEED = 0.15,
     STEAM_VALVE_CHANGE_SPEED = 0.15;

   // Variables
   int
     heater_high = 0,
     heater_low = 0,
     water_high = 0,
     water_low = 0,
     steam_high = 0,
     steam_low = 0,
     cooling_high = 0,
     cooling_low = 0;
   public double
     SolventValve = 0.85,
     SteamValve = 1.,
     CoolingValve = 0.8,
     WaterValve = 0.4,
     SolventFlow = 0.,
     SteamFlow = 0.,
     CoolingFlow = 0.,
     WaterFlow = 0.,
     SteamTemperature = 0.,
     HeaterTemperature = 0.,
     BeforePreHeaterTemperature = 0.,
     PreHeaterTemperature = 0.,
     AfterPreHeaterTemperature = 0.,
     CoolingTemperature = 0.,
     HeaterPressure = 0.,
     HeaterLevel = 0.5,
     WaterLevel = 0.1;
   boolean
     WaterAlarm = false,
     HeaterAlarm = false;

   long last_time = 0;

   GlgProcessDemoData()
   {
   }

   //////////////////////////////////////////////////////////////////////////
   // Updates simulated data. An application would query its data from
   // a PLC or process database.
   //////////////////////////////////////////////////////////////////////////
   void UpdateProcessData()
   {
      long curr_time = new Date().getTime();

      // Update simulation data only as often as defined by UPDATE_INTERVAL:
      // once a second.
      if( curr_time < last_time + UPDATE_INTERVAL )
         return;

      IterateProcess();

      last_time = curr_time;
   }

   //////////////////////////////////////////////////////////////////////////
   // Recalculates new values for the process simulation model.
   //////////////////////////////////////////////////////////////////////////
   void IterateProcess()
   {
      SteamTemperature += ( SteamValve - 0.6 ) * 2 * PROCESS_SPEED;
      SteamTemperature = PutInRange( SteamTemperature, 0., 1. );
      
      HeaterTemperature +=
        ( SteamTemperature - HeaterTemperature * HeaterLevel ) * PROCESS_SPEED;
      HeaterTemperature = PutInRange( HeaterTemperature, 0., 1. );
      
      BeforePreHeaterTemperature +=
        ( 1.5 * HeaterTemperature - BeforePreHeaterTemperature ) *
          PROCESS_SPEED;
      BeforePreHeaterTemperature =
        PutInRange( BeforePreHeaterTemperature, 0., 1. );
      
      PreHeaterTemperature +=
        ( BeforePreHeaterTemperature - PreHeaterTemperature ) * PROCESS_SPEED;
      PreHeaterTemperature = PutInRange( PreHeaterTemperature, 0., 1. );

      AfterPreHeaterTemperature +=
        ( 0.9 * HeaterTemperature - AfterPreHeaterTemperature ) *
          PROCESS_SPEED ;
      AfterPreHeaterTemperature =
        PutInRange( AfterPreHeaterTemperature, 0., 1. );

      CoolingTemperature +=
        ( AfterPreHeaterTemperature - CoolingTemperature - CoolingValve )
          * PROCESS_SPEED;
      CoolingTemperature = PutInRange( CoolingTemperature, 0., 1. );
      
      HeaterLevel += ( SolventValve - 0.75 ) * HEATER_LEVEL_SPEED;
      HeaterLevel = PutInRange( HeaterLevel, 0., 1. );
      
      // Inversed
      WaterLevel += ( 0.5 - WaterValve ) * WATER_LEVEL_SPEED;
      WaterLevel = PutInRange( WaterLevel, 0., 1. );
      
      if( HeaterLevel > 0.9 || heater_high != 0 )
      {
         heater_high = LagVar( heater_high, 10 );
         SolventValve -= VALVE_CHANGE_SPEED;
      }
      else if( HeaterLevel < 0.45 || heater_low != 0 )
      {
         heater_low = LagVar( heater_low, 10 );
         SolventValve += VALVE_CHANGE_SPEED;
      }
      SolventValve = PutInRange( SolventValve, 0., 1. );
      
      // Inversed
      if( WaterLevel > 0.2 || water_high != 0 )
      {
         water_high = LagVar( water_high, 10 );
         WaterValve += VALVE_CHANGE_SPEED;
      }
      else if( WaterLevel < 0.05 || water_low != 0 )
      {
         water_low = LagVar( water_low, 10 );
         WaterValve -= VALVE_CHANGE_SPEED;
      }
      WaterValve = PutInRange( WaterValve, 0., 1. );
      
      if( SteamTemperature > 0.9 || steam_high != 0 )
      {
         LagVar( steam_high, 20 );
         SteamValve -= STEAM_VALVE_CHANGE_SPEED;
      }
      else if( SteamTemperature < 0.2 || steam_low != 0 )
      {
         LagVar( steam_low, 20 );
         SteamValve += STEAM_VALVE_CHANGE_SPEED;
      }
      SteamValve = PutInRange( SteamValve, 0., 1. );
      
      if( CoolingTemperature > 0.7 || cooling_high != 0 )
      {
         LagVar( cooling_high, 10 );
         CoolingValve += VALVE_CHANGE_SPEED;
      }
      else if( CoolingTemperature < 0.3 || cooling_low != 0 )
      {
         LagVar( cooling_low, 10 );
         CoolingValve -= VALVE_CHANGE_SPEED;
      }
      CoolingValve = PutInRange( CoolingValve, 0., 1. );
      
      HeaterPressure = HeaterLevel * ( HeaterTemperature + 1. ) / 2.;
      HeaterPressure = PutInRange( HeaterPressure, 0., 1. );

      HeaterAlarm = ( HeaterLevel < 0.45 || HeaterLevel > 0.9 );
      WaterAlarm = ( WaterLevel > 0.2 || WaterLevel < 0.05 );
   }

   //////////////////////////////////////////////////////////////////////////
   // Helps to implement lag behavior
   //////////////////////////////////////////////////////////////////////////
   int LagVar( int variable, int lag )
   {
      if( variable != 0 )
        return --variable;
      else
        return lag;
   }

   //////////////////////////////////////////////////////////////////////////
   double PutInRange( double variable, double low, double high )
   {
      if( variable < low )
        return low;
      else if( variable > high )
        return high;
      else
        return variable;
   }
}
