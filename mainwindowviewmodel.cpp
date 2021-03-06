#include "mainwindowviewmodel.h"
#include "mapstatevaliditychecker.h"
#include <ompl/geometric/SimpleSetup.h>
#include <ompl/tools/benchmark/Benchmark.h>
#include <algorithm>

using std::unordered_map;
using std::vector;
using std::string;

void MainWindowViewModel::addEnviroment(EnvInterface* interface){
    if(interface){
        m_environments.insert(std::make_pair(interface->getName(),interface));
    }
}
void MainWindowViewModel::addPlanner(PlannerInterface* interface){
    if(interface){
        m_planners.insert(std::make_pair(interface->getName(),interface));
    }
}

vector<string> MainWindowViewModel::getEnvironmentNames(){
    vector<string> names;
    for(auto & kv: m_environments){
        names.push_back(kv.first);
    }
    return names;
}

vector<string> MainWindowViewModel::getPlannerNames(){
    vector<string> names;
    for(auto & kv: m_planners){
        names.push_back(kv.first);
    }
    return names;
}

PlannerInterface* MainWindowViewModel::getPlanner(string name){
    if(m_planners.count(name)){
        return m_planners[name];
    }
    return nullptr;
}

EnvInterface* MainWindowViewModel::getEnvironment(string name){
    if(m_environments.count(name)){
        return m_environments[name];
    }
    return nullptr;
}

void MainWindowViewModel::environmentChanged(std::string name){
    if(auto * env = getEnvironment(name)){
        m_custom_drawer.drawMap2d(env->getMap());
        m_custom_plot->xAxis->setRange(-100, 100);
        m_custom_plot->yAxis->setRange(-100, 100);
        m_menu_variables->startPoint().set(env->getStartPoint());
        m_menu_variables->goalPoint().set(env->getGoalPoint());
        drawStartGoalPoints(env->getStartPoint(),env->getGoalPoint());
        m_custom_plot->replot();
    }
}

void MainWindowViewModel::environmentRedraw(std::string name){
    if(auto * env = getEnvironment(name)){
        m_custom_drawer.drawMap2d(env->getMap());
        m_custom_plot->xAxis->setRange(-100, 100);
        m_custom_plot->yAxis->setRange(-100, 100);
        drawStartGoalPoints(m_menu_variables->startPoint().get(),m_menu_variables->goalPoint().get());
        m_custom_plot->replot();
    }
}

void MainWindowViewModel::drawStartGoalPoints(Point start, Point goal){
    m_custom_drawer.drawPoint(start,Qt::green);
    m_custom_drawer.drawPoint(goal,Qt::black);
}

void MainWindowViewModel::plan(PlannerInterface * planner_interface, EnvInterface * environment_interface, Point start_point, Point goal_point){
    // construct the state space we are planning in
    auto space(std::make_shared<ob::RealVectorStateSpace>(2));

    // set the bounds for the R^3 part of SE(3)
    ob::RealVectorBounds bounds(2);
    bounds.setLow(-100);
    bounds.setHigh(100);

    space->setBounds(bounds);

    // construct an instance of  space information from this state space
    auto si(std::make_shared<ob::SpaceInformation>(space));

    auto map = environment_interface->getMap();

    auto map_state_checker(std::make_shared<MapStateValidityChecker>(si,map));

    // set state validity checking for this space
//     si->setStateValidityChecker(isStateValid);
    si->setStateValidityChecker(map_state_checker);

    // create a random start state
    ob::ScopedState<ob::RealVectorStateSpace> start(space);
    start->values[0] = start_point.x;
    start->values[1] = start_point.y;


    // create a random goal state
    ob::ScopedState<ob::RealVectorStateSpace> goal(space);
    goal->values[0] = goal_point.x;
    goal->values[1] = goal_point.y;

    // create a problem instance
    auto pdef(std::make_shared<ob::ProblemDefinition>(si));

     // set the start and goal states
     pdef->setStartAndGoalStates(start, goal);

     // create a planner for the defined space
     auto planner(planner_interface->getPlanner(si));

    //planner->setRange(1);

     // set the problem we are trying to solve for the planner
     planner->setProblemDefinition(pdef);

     // perform setup steps for the planner
     planner->setup();


     // print the settings for this space
     si->printSettings(std::cout);

     // print the problem settings
     pdef->print(std::cout);

     // attempt to solve the problem within one second of planning time
     ob::PlannerStatus solved = planner->ob::Planner::solve(1.0);

     ob::PlannerData planner_data{si};
     planner->getPlannerData(planner_data);

     if (solved)
     {
         std::cout << "Numero de vertices: " << planner_data.numVertices() << std::endl;
         std::cout << "Numero de cantos: " << planner_data.numEdges() << std::endl;

         // get the goal representation from the problem definition (not the same as the goal state)
         // and inquire about the found path
         auto path = pdef->getSolutionPath()->as<og::PathGeometric>();

         std::cout << "Found solution:" << std::endl;
         m_custom_drawer.drawSearchGraph(planner_data);
         m_custom_drawer.drawGeometricPath(path);
     }
     else
         std::cout << "No solution found" << std::endl;
}

void MainWindowViewModel::benchmark(std::vector<PlannerInterface*> planners_interface, EnvInterface* environment_interface, Point start_point, Point goal_point, double max_time, double max_mem, int run_count){
    std::locale::global(std::locale::classic());
    // construct the state space we are planning in
    auto space(std::make_shared<ob::RealVectorStateSpace>(2));

    // set the bounds for the R^3 part of SE(3)
    ob::RealVectorBounds bounds(2);
    bounds.setLow(-100);
    bounds.setHigh(100);

    space->setBounds(bounds);

    // construct an instance of  space information from this state space
    auto si(std::make_shared<ob::SpaceInformation>(space));

    auto map = environment_interface->getMap();

    auto map_state_checker(std::make_shared<MapStateValidityChecker>(si,map));

    // set state validity checking for this space
//     si->setStateValidityChecker(isStateValid);
    si->setStateValidityChecker(map_state_checker);

    // create a random start state
    ob::ScopedState<ob::RealVectorStateSpace> start(space);
    start->values[0] = start_point.x;
    start->values[1] = start_point.y;


    // create a random goal state
    ob::ScopedState<ob::RealVectorStateSpace> goal(space);
    goal->values[0] = goal_point.x;
    goal->values[1] = goal_point.y;

    ompl::geometric::SimpleSetup ss(si);

    ss.setStartState(start);
    ss.setGoalState(goal);

    // First we create a benchmark class:
    ompl::tools::Benchmark b(ss, "my experiment");

    vector<string> names;
    for(auto * planner_interface:planners_interface){
        auto planner = planner_interface->getPlanner(si);
        names.push_back(planner->getName());
        int count = std::count(names.begin(),names.end(),planner->getName());
        planner->setName(planner->getName()+std::to_string(count));
        b.addPlanner(planner);
    }

    // Now we can benchmark: 5 second time limit for each plan computation,
    // 100 MB maximum memory usage per plan computation, 50 runs for each planner
    // and true means that a text-mode progress bar should be displayed while
    // computation is running.
    ompl::tools::Benchmark::Request req;
    req.maxTime = max_time;
    req.maxMem = max_mem;
    req.runCount = run_count;
    req.displayProgress = true;
    b.benchmark(req);
    b.saveResultsToFile("ResultadosBenchmark.log");
}
