#include "Core.hpp"
#include "Event.hpp"
#include "Measure.hpp"
#include "Shell/SimulationShell.hpp"
#include "rngs.hpp"
#include <Strategies/TaggedCustomer.hpp>
#include <algorithm>
#include <fmt/core.h>
#include <functional>

void TaggedCustomer::ConnectEntrance(BaseStation *station, bool arrival)
{
    auto l = [this, station](auto s, Event &e) { _times[e.Name] = {station->clock(), 0}; };
    if (arrival)
        station->OnArrival(l);
    else
        station->OnDeparture(l);
}

void TaggedCustomer::ConnectLeave(BaseStation *station, bool arrival)
{
    auto l = [this, station](auto s, Event &e) {
        if (_times.count(e.Name) == 0)
            panic(fmt::format("Event with name {} not registered at entrance", e.Name));
        _times[e.Name].second = station->clock();
        auto pair = _times.at(e.Name);
        _regCycle(pair.second - pair.first);
    };
    if (arrival)
        station->OnArrival(l);
    else
        station->OnDeparture(l);
}

void TaggedCustomer::AddShellCommands(SimulationShell *shell)
{
    shell->AddCommand("ltgtstats", [this](SimulationShell *s, auto ctx) { s->Log()->Result("{}", _mean); });
}

void TaggedCustomer::CompleteRegCycle(double actualclock)
{
    _mean(_regCycle.mean(), actualclock);
    _regCycle.Reset();
}

void TaggedCustomer::CompleteSimulation()
{
    _mean.Reset();
}
