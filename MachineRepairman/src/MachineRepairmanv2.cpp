#include "MachineRepairmanv2.hpp"
#include "Core.hpp"
#include "DelayStation.hpp"
#include "Event.hpp"
#include "LogEngine.hpp"
#include "MachineRepairman.hpp"
#include "RepairStation.hpp"
#include "Scheduler.hpp"
#include "Station.hpp"
#include "rngs.hpp"
#include "rvgs.h"
#include <fmt/core.h>
#include <sched.h>

Event gen_maintenance(double clock, double occurTime, double serviceTime)
{
    auto evt = Event("MAINTENANCE", ARRIVAL, clock, occurTime, serviceTime, clock, -1);
    evt.SubType = MAINTENANCE;
    return evt;
}

MachineRepairmanv2::MachineRepairmanv2() : Scheduler("Scheduler")
{
    auto delay_station = new DelayStation(this, "delay_station", 10, []() {
        static VariableStream delay(1, [](auto rng) { return Exponential(3000); });
        return delay();
    });

    delay_station->OnDeparture([this](auto s, Event &evt) {
        evt.Station = 1;
        evt.Type = ARRIVAL;
        Schedule(evt);
    });

    AddStation(delay_station);

    auto srepstation = new RepairStation(this, "short_repair", 1);
    srepstation->OnArrival([](auto s, Event &evt) {
        static VariableStream repair_time(3, [evt](auto rng) { return Exponential(40); });
        evt.ServiceTime = repair_time();
    });

    srepstation->OnDeparture([this](auto stat, Event &evt) {
        static CompositionStream router{2,
                                        {0.2, 0.8},
                                        [this, &evt](auto s) {
                                            evt.Type = ARRIVAL;
                                            evt.Station = (*this)["long_repair"].value()->stationIndex();
                                            Schedule(evt);
                                            return 1;
                                        },
                                        [this, &evt](auto s) {
                                            evt.Type = ARRIVAL;
                                            evt.Station = (*this)["delay_station"].value()->stationIndex();
                                            Schedule(evt);
                                            return 0;
                                        }};
        if (evt.SubType != MAINTENANCE)
            router();
        else
        {
            Schedule(Event("MAINTENANCE", MAINTENANCE, _clock, _clock + _nominalWorkshift, _nominalRests, _clock, -1));
        }
    });
    AddStation(srepstation);

    auto lrepstation = new RepairStation(this, "long_repair", 2);
    lrepstation->OnArrival([](auto s, Event &evt) {
        static VariableStream longRepair{4, [](auto rng) { return Exponential(960); }};
        evt.ServiceTime = longRepair();
    });
    lrepstation->OnDeparture([this](auto s, Event &evt) {
        if (evt.SubType == MAINTENANCE)
            return;
        evt.Station = 0;
        evt.Type = ARRIVAL;
        Schedule(evt);
    });
    AddStation(lrepstation);
}

void MachineRepairmanv2::Initialize()
{
    (*this)["delay_station"].value()->Initialize();
    Schedule(gen_maintenance(_clock, _nominalWorkshift, _nominalRests));
}

void MachineRepairmanv2::Execute()
{
    while (!_end)
    {
        auto inProcess = _eventList.Dequeue();
        Process(inProcess);
        Route(inProcess);
    }
}

void MachineRepairmanv2::ProcessEnd(Event &evt)
{
    _end = true;
}

void MachineRepairmanv2::ProcessArrival(Event &evt)
{
    if (evt.SubType == MAINTENANCE)
    {
        _stations[1]->Process(evt);
        _stations[2]->Process(evt);
    }
}

void MachineRepairmanv2::ProcessDeparture(Event &evt)
{
}

void MachineRepairmanv2::Stop()
{
    _end = true;
}

void MachineRepairmanv2::Update()
{
    for (auto s : _stations)
    {
        s->Update();
    }
}
