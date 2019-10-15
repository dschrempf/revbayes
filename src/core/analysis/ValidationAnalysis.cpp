#include <stddef.h>
#include <cmath>
#include <algorithm>
#include <iostream>
#include <map>
#include <string>
#include <utility>
#include <vector>

#include "DagNode.h"
#include "DistributionBinomial.h"
#include "MaxIterationStoppingRule.h"
#include "MonteCarloAnalysis.h"
#include "ProgressBar.h"
#include "RandomNumberFactory.h"
#include "RandomNumberGenerator.h"
#include "RlUserInterface.h"
#include "StochasticVariableMonitor.h"
#include "Trace.h"
#include "TraceReader.h"
#include "ValidationAnalysis.h"
#include "Cloneable.h"
#include "Model.h"
#include "Parallelizable.h"
#include "RbFileManager.h"
#include "RbVector.h"
#include "RbVectorImpl.h"
#include "StoppingRule.h"
#include "StringUtilities.h"


using namespace RevBayesCore;

ValidationAnalysis::ValidationAnalysis( const MonteCarloAnalysis &m, size_t n ) : Cloneable( ), Parallelizable( ),
    num_runs( n )
{
    
    std::string directory = "output";
    // some general constant variables
    RbFileManager fm = RbFileManager( directory );
    const std::string path_separator = fm.getPathSeparator();
    
    // remove all monitors if there are any
    MonteCarloAnalysis *sampler = m.clone();
    sampler->removeMonitors();
    
    StochasticVariableMonitor mntr = StochasticVariableMonitor(10, "output/posterior_samples.var", "\t");
    sampler->addMonitor( mntr );
    
    size_t run_block_start = size_t(floor( (double(pid)   / num_processes ) * num_runs) );
    size_t run_block_end   = std::max( int(run_block_start), int(floor( (double(pid+1) / num_processes ) * num_runs) ) - 1);
    int number_processes_per_run = ceil( double(num_processes) / num_runs );
    
    // we need to change the random number generator when using MPI so that they are not synchronized anymore
    for ( size_t i=0; i<pid; ++i )
    {
        GLOBAL_RNG->setSeed( int(floor( GLOBAL_RNG->uniform01()*1E5 )) );
    }
    
#ifdef RB_MPI
//    size_t active_proc = floor( pid / double(processors_per_likelihood) ) * processors_per_likelihood;
    size_t active_proc = 0;
    MPI_Comm analysis_comm;
    MPI_Comm_split(MPI_COMM_WORLD, active_proc, pid, &analysis_comm);
#endif
    
    runs = std::vector<MonteCarloAnalysis*>(num_runs,NULL);
    simulation_values = std::vector<Model*>(num_runs,NULL);
    for ( size_t i = 0; i < num_runs; ++i)
    {
        
        if ( i >= run_block_start && i <= run_block_end)
        {
            
            // create a new directory name for this simulation
            std::stringstream s;
            s << directory << path_separator << "Validation_Sim_" << i;
            std::string sim_directory_name = s.str();
            
            // create an independent copy of the analysis
            MonteCarloAnalysis *current_analysis = sampler->clone();
        
            // get the model of the analysis
            Model* current_model = current_analysis->getModel().clone();
        
            // get the DAG nodes of the model
            std::vector<DagNode *> current_ordered_nodes = current_model->getOrderedStochasticNodes();
        
            for (size_t j = 0; j < current_ordered_nodes.size(); ++j)
            {
                DagNode *the_node = current_ordered_nodes[j];
            
                if ( the_node->isStochastic() == true )
                {
                    the_node->redraw();
                    
                    // we need to store the new simulated data
                    the_node->writeToFile(sim_directory_name);
                    
                }
            
            }
        
            // now set the model of the current analysis
#ifdef RB_MPI
            current_analysis->setModel( current_model, false, analysis_comm );
#else
            current_analysis->setModel( current_model, false );
#endif
            
            std::stringstream ss;
            ss << "Validation_Sim_" << i;
        
            // set the monitor index
            current_analysis->addFileMonitorExtension(ss.str(), true);
        
            // add the current analysis to our vector of analyses
            runs[i] = current_analysis;
            Model *model_clone = current_model->clone();
            simulation_values[i] = model_clone;
            
            runs[i]->setActivePID( pid, number_processes_per_run );
            
        }
        
    }
    
    delete sampler;
    
}


ValidationAnalysis::ValidationAnalysis(const ValidationAnalysis &a) : Cloneable( a ), Parallelizable( a ),
    num_runs( a.num_runs )
{
    
    runs = std::vector<MonteCarloAnalysis*>(num_runs,NULL);
    simulation_values = std::vector<Model*>(num_runs,NULL);
    // create replicate Monte Carlo samplers
    for (size_t i=0; i < num_runs; ++i)
    {
        // only copy the runs which this process needs to execute
        if ( a.runs[i] != NULL )
        {
            runs[i]                 = a.runs[i]->clone();
            simulation_values[i]    = runs[i]->getModel().clone() ;
        }
        
    }
    
}


ValidationAnalysis::~ValidationAnalysis(void)
{
    // free the runs
    for (size_t i = 0; i < num_runs; ++i)
    {
        MonteCarloAnalysis *sampler = runs[i];
        delete sampler;
        
        Model *m = simulation_values[i];
        delete m;
    }
    
}


/**
 * Overloaded assignment operator.
 * We need to keep track of the MonteCarloSamplers
 */
ValidationAnalysis& ValidationAnalysis::operator=(const ValidationAnalysis &a)
{
    Parallelizable::operator=( a );
    
    if ( this != &a )
    {
        
        // free the runs
        for (size_t i = 0; i < num_runs; ++i)
        {
            MonteCarloAnalysis *sampler = runs[i];
            delete sampler;
            
            Model *m = simulation_values[i];
            delete m;
        }
        runs.clear();
        simulation_values.clear();
        
        num_runs                    = a.num_runs;
//        credible_interval_size      = a.credible_interval_size;
        
        
        runs = std::vector<MonteCarloAnalysis*>(num_runs,NULL);
        simulation_values = std::vector<Model*>(num_runs,NULL);
        
        // create replicate Monte Carlo samplers
        for (size_t i=0; i < num_runs; ++i)
        {
            // only copy the runs which this process needs to execute
            if ( a.runs[i] != NULL )
            {
                runs[i]                 = a.runs[i]->clone();
                simulation_values[i]    = runs[i]->getModel().clone() ;
            }
            
        }
        
    }
    
    return *this;
}


/** Run burnin and autotune */
void ValidationAnalysis::burnin(size_t generations, size_t tuningInterval)
{
    
    // start the progress bar
    ProgressBar progress = ProgressBar(num_runs, 0);
    
    if ( process_active == true )
    {
        // Let user know what we are doing
        std::stringstream ss;
        ss << "\n";
        ss << "Running burn-in phase of " << num_runs <<  " Monte Carlo samplers each for " << generations << " iterations.\n";
        RBOUT( ss.str() );
        
        // Print progress bar (68 characters wide)
        progress.start();
    }
    
    // compute which block of the data this process needs to compute
    size_t run_block_start = size_t(floor( (double(pid)   / num_processes ) * num_runs) );
    size_t run_block_end   = std::max( int(run_block_start), int(floor( (double(pid+1) / num_processes ) * num_runs) ) - 1);
    
    // Run the chain
    for (size_t i = run_block_start; i <= run_block_end; ++i)
    {
        if ( runs[i] == NULL ) std::cerr << "Running bad burnin (pid=" << pid <<", run="<< i << ") of runs.size()=" << runs.size() << "." << std::endl;
        // run the i-th analyses
#ifdef RB_MPI
        runs[i]->burnin(generations, MPI_COMM_WORLD, tuningInterval, false, false);
#else
        runs[i]->burnin(generations, tuningInterval, false, false);
#endif
        if ( process_active == true )
        {
            progress.update( i );
            
        }
        
        
    }
    
    if ( process_active == true )
    {
        progress.finish();
    }
    
}



ValidationAnalysis* ValidationAnalysis::clone( void ) const
{
    
    return new ValidationAnalysis( *this );
}


void ValidationAnalysis::runAll(size_t gen)
{
    
    // print some information to the screen but only if we are the active process
    if ( process_active )
    {
        std::cout << std::endl;
        std::cout << "Running validation analysis ..." << std::endl;
    }
    
    // compute which block of the runs this process needs to compute
    size_t run_block_start = size_t(floor( (double(pid)   / num_processes ) * num_runs) );
    size_t run_block_end   = std::max( int(run_block_start), int(floor( (double(pid+1) / num_processes ) * num_runs) ) - 1);
    
    // Run the chain
    for (size_t i = run_block_start; i <= run_block_end; ++i)
    {
        
        // run the i-th stone
        runSim(i, gen);
        
    }    
    
}



void ValidationAnalysis::runSim(size_t idx, size_t gen)
{
    // print some info
    if ( process_active )
    {
        size_t digits = size_t( ceil( log10( num_runs ) ) );
        std::cout << "Sim ";
        for (size_t d = size_t( ceil( log10( idx+1.1 ) ) ); d < digits; d++ )
        {
            std::cout << " ";
        }
        std::cout << (idx+1) << " / " << num_runs;
        std::cout << "\t\t";
        
        std::cout << std::endl;
    }
    
    // get the current sample
    MonteCarloAnalysis *analysis = runs[idx];
    
    // run the analysis
    RbVector<StoppingRule> rules;
    
    size_t currentGen = analysis->getCurrentGeneration();
    rules.push_back( MaxIterationStoppingRule(gen + currentGen) );
    
    
#ifdef RB_MPI
    analysis->run(gen, rules, MPI_COMM_WORLD, 100, "", 0, false);
#else
    analysis->run(gen, rules, 100, "", 0, false);
#endif

}



void ValidationAnalysis::summarizeAll( double credible_interval_size )
{
    
    // print some information to the screen but only if we are the active process
    if ( process_active )
    {
        std::cout << std::endl;
        std::cout << "Summarizing analysis ..." << std::endl;
    }
    
    // reset the counter
    coverage_count = std::map<std::string, int>();
    
    // compute which block of the runs this process needs to compute
    size_t run_block_start = size_t(floor( (double(pid)   / num_processes ) * num_runs) );
    size_t run_block_end   = std::max( int(run_block_start), int(floor( (double(pid+1) / num_processes ) * num_runs) ) - 1);
    //    size_t stone_block_size  = stone_block_end - stone_block_start;
    
    // Summarize the chain
    for (size_t i = run_block_start; i <= run_block_end; ++i)
    {
        
        // summarize the i-th simulation
        summarizeSim(credible_interval_size, i);
        
    }
    
#ifdef RB_MPI
    
    for (std::map<std::string, int>::iterator it = coverage_count.begin(); it != coverage_count.end(); ++it)
    {
        
        if ( pid == 0 )
        {
            // receive
            for (int i=1;i<num_processes;++i)
            {
                MPI_Status status;
                int counts = 0;
                MPI_Recv(&counts, 1, MPI_INT, int(i), 0, MPI_COMM_WORLD, &status);
                it->second += counts;
            }
        }
        else
        {
            // send
            MPI_Send(&it->second, 1, MPI_INT, (int)active_PID, 0, MPI_COMM_WORLD);
        }
        
    }
    MPI_Barrier(MPI_COMM_WORLD);

#endif
    
    if ( process_active )
    {
        
        std::cout << std::endl;
        std::cout << "The validation analysis ran " << num_runs << " simulations to validate the implementation." << std::endl;
        std::cout << "This analysis used a " << credible_interval_size << " credible interval." << std::endl;
        std::cout << "Coverage frequencies should be between " << (RbStatistics::Binomial::quantile(0.025, num_runs, credible_interval_size)/num_runs) << " and " << (RbStatistics::Binomial::quantile(0.975, num_runs, credible_interval_size)/num_runs) << " in 95% of the simulations." << std::endl;
        std::cout << std::endl;
        std::cout << "Coverage frequencies of parameters in validation analysis:" << std::endl;
        std::cout << "==========================================================" << std::endl;
        for (std::map<std::string, int>::iterator it = coverage_count.begin(); it != coverage_count.end(); ++it)
        {
            std::string n = it->first;
            StringUtilities::formatFixedWidth(n, 20, true);
            std::cout << n << "\t\t" << double(it->second) / num_runs << std::endl;
        }
        std::cout << std::endl;
    }
    
}



void ValidationAnalysis::summarizeSim(double credible_interval_size, size_t idx)
{
    
    std::stringstream ss;
    ss << "output/Validation_Sim_" << idx << "/" << "posterior_samples.var";
    std::string fn = ss.str();
        
    TraceReader reader;
    std::vector<ModelTrace> traces = reader.readStochasticVariableTrace( fn, "\t");
    
    size_t n_samples = traces[0].size();
    size_t n_traces = traces.size();
    
    std::vector<DagNode*> nodes = simulation_values[idx]->getDagNodes();
    
    std::map<std::string,AbstractTrace*> trace_map;
    // now for the numerical parameters
    for ( size_t j=0; j<n_traces; ++j )
    {
        std::string parameter_name = traces[j].getParameterName();
        
        // iterate over all DAG nodes (variables)
        for ( std::vector<DagNode*>::iterator it = nodes.begin(); it!=nodes.end(); ++it )
        {
            DagNode *the_node = *it;
            
            if ( the_node->getName() == parameter_name )
            {
                // create a trace
                AbstractTrace *t = the_node->createTraceObject();
                trace_map[parameter_name] = t;
            }
            
        }
        
    }
    
    // add each sample
    for (size_t i=0; i<n_samples; ++i)
    {
        // to each of the traces
        for ( size_t j=0; j<n_traces; ++j )
        {
            
            const std::string &parameter_name = traces[j].getParameterName();
            if ( trace_map.find( parameter_name ) != trace_map.end() )
            {
                std::string parameter_name = traces[j].getParameterName();
                trace_map[parameter_name]->addValueFromString( traces[j].objectAt( i ) );
            }
            
        }
        
    }

    
    // iterate over all DAG nodes (variables)
    for ( std::vector<DagNode*>::iterator it = nodes.begin(); it!=nodes.end(); ++it )
    {
        DagNode *the_node = *it;
        
        if ( the_node->isStochastic() == true )
        {
            const std::string &parameter_name = the_node->getName();
                        
            if ( trace_map.find( parameter_name ) != trace_map.end() )
            {
                // create a trace
                bool cov = trace_map[parameter_name]->isCoveredInInterval(the_node->getValueAsString(), credible_interval_size, false);
                
                if ( coverage_count.find(parameter_name) == coverage_count.end() )
                {
                    coverage_count.insert( std::pair<std::string,int>(parameter_name,0) );
                }
                if ( cov == true )
                {
                    coverage_count[ parameter_name ]++;
                }
                
            }
        
        }
        
    }
    
}

