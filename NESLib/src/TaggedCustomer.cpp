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
    auto l = [this, station](auto s, Event &e) {
        if (target_client == "")
            target_client = e.Name;
        if (e.Name == target_client)
            time = e.OccurTime;
    };
    if (arrival)
        station->OnArrival(l);
    else
        station->OnDeparture(l);
}

void TaggedCustomer::ConnectLeave(BaseStation *station, bool arrival)
{
    auto l = [this, station](auto s, Event &e) {
        if (e.Name == target_client)
        {
            double interval = e.OccurTime - time;
            _acc.Accumulate(interval);
        }
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
    _mean(_acc.sum(), _acc.Count(), false);
    _acc.Reset();
}

void TaggedCustomer::CompleteSimulation()
{
    _mean.Reset();
}
