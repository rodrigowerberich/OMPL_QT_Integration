#ifndef LAZYPRMSTARPLANNER_H
#define LAZYPRMSTARPLANNER_H

#include "../plannerinterface.h"
#include "ompl/geometric/planners/prm/LazyPRMstar.h"
#include "../mapplannerconfiguration.h"
#include "../rangeconfiguration.h"

class LazyPRMStarPlanner : public PlannerInterface{
public:
    LazyPRMStarPlanner(){
        m_configurations.addConfiguration(new RangeConfiguration{0.0});
    }
    std::string getName() const override{
        return "LazyPRMstar";
    }
    std::shared_ptr<ompl::base::Planner> getPlanner(std::shared_ptr<ompl::base::SpaceInformation> si) override{
        auto planner = std::make_shared<ompl::geometric::LazyPRMstar>(si);
        auto* range_configuration = static_cast<RangeConfiguration*>(m_configurations.getConfiguration(RangeConfiguration().getName()));
        planner->setRange(range_configuration->getRange());
        return planner;
    }
    PlannerConfigurationInterface* getConfigurations(){
        return &m_configurations;
    }
    virtual PlannerInterface* copy(){
        auto * copy_planner = new LazyPRMStarPlanner;
        copy_planner->m_configurations = m_configurations;
        return copy_planner;
    }

private:
    MapPlannerConfiguration m_configurations;
};


#endif // LAZYPRMSTARPLANNER_H
