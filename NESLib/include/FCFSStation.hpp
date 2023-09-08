#pragma once
#include "Collections/LinkedList.hpp"
#include "DataProvider.hpp"
#include "Event.hpp"
#include "ISimulator.hpp"
#include "LogEngine.hpp"
#include "Station.hpp"
#include "ToString.hpp"
#include <optional>
#include <queue>
#include <vector>

class FCFSStation : public Station
{
  protected:
    EventList _eventQueue{};
    std::optional<Event> _eventUnderProcess;
    IScheduler *_scheduler;

  public:
    void ProcessArrival(Event &evt) override;
    void ProcessDeparture(Event &evt) override;
    void ProcessEnd(Event &evt) override;
    void ProcessProbe(Event &evt) override;
    void Reset() override;
    FCFSStation(IScheduler *scheduler, std::string name, int stationIndex);
};
