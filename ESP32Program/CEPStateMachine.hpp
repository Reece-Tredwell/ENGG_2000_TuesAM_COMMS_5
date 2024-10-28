namespace CEP {
  enum CEPState : uint32_t {
    INIT, // Initialising
    STOPPED,
    MOVING,
    EMERGENCY_STOP, // Anything that requires the CEP to emergency stop
    APPROACHING_STATION, // Slowing down to locate station
    OVERSHOT_STATION, // Overshot the station, reverse and find it again
    STATION_ARRIVAL, // Arrival at station
    BOARDING, // Boarding procedure
    STATION_DEPARTURE, // Departing station
    NETWORK_DISCONNECTED // Failure state
  };

  class CEPStateMachine {
    private:
      CEPState state;
      // Stores the state before an emergency stop, used to return to normal operations
      CEPState preEmergencyState;
      int32_t emergencyStopTime = 0;
    public:
      CEPStateMachine() {
        state = CEPState::INIT;
      }
      CEPState getState() {
        return state;
      }
      String getStateName() {
        switch (state) {
          case CEPState::INIT: return "INIT";
          case CEPState::STOPPED: return "STOPPED";
          case CEPState::MOVING: return "MOVING";
          case CEPState::EMERGENCY_STOP: return "EMERGENCY_STOP";
          case CEPState::APPROACHING_STATION: return "APPROACHING_STATION";
          case CEPState::STATION_ARRIVAL: return "STATION_ARRIVAL";
          case CEPState::BOARDING: return "BOARDING";
          case CEPState::STATION_DEPARTURE: return "STATION_DEPARTURE";
        }
        return "Unknown State";
      }
      void initialisationFinished() {
        if (state == CEPState::INIT)
          state = CEPState::STOPPED;
      }
      void beginMoving() {
        if (state == CEPState::STOPPED || state == CEPState::STATION_DEPARTURE)
          state = CEPState::MOVING;
      }
      void stopped() {
        if (state == CEPState::MOVING)
          state = CEPState::STOPPED;
      }
      // Stores the previous state
      void emergencyStop(int32_t time) {
        if (state != CEPState::EMERGENCY_STOP) {
          preEmergencyState = state;
          emergencyStopTime = time;
          state = CEPState::EMERGENCY_STOP;
        }
      }
      // Once cleared, restores the previous state
      void clearEmergency() {
        if (state == CEPState::EMERGENCY_STOP)
          state = preEmergencyState;
      }
      int32_t timeSinceEmergencyStop(int32_t currentTime) {
        return currentTime - emergencyStopTime;
      }
      void approachStation() {
        if (state == CEPState::MOVING) {
          state = CEPState::APPROACHING_STATION;
        }
      }
      void overshotStation() {
        if (state == CEPState::APPROACHING_STATION) {
          state = CEPState::OVERSHOT_STATION;
        }
      }
      void arriveAtStation() {
        if (state == CEPState::APPROACHING_STATION || state == CEPState::OVERSHOT_STATION)
          state = CEPState::STATION_ARRIVAL;
      }
      void beginBoarding() {
        if (state == CEPState::STATION_ARRIVAL)
          state = CEPState::BOARDING;
      }
      void departStation() {
        if (state == CEPState::BOARDING)
          state = CEPState::STATION_DEPARTURE;
      }
      void proceedAfterDeparture() {
        if (state == CEPState::STATION_DEPARTURE)
          state = CEPState::STOPPED;
      }
      void networkDisconnected() {
        state = CEPState::NETWORK_DISCONNECTED;
      }
  };
}