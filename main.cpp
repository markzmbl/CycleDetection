#include <iostream>
#include <tclap/CmdLine.h>
#include "DetectCycle.h"
#include "Timer.h"
#include "MemoryMonitor.h"
#include "CycleRootFinder.h"
#include "SignificanceTester.h"

using namespace std;

int main(int argc, char **argv) {
    // Wrap everything in a try block.  Do this every time,
    // because exceptions will be thrown for problems.
    try {

        TCLAP::CmdLine cmd("Cycle Detection in Temporal Graphs", ' ', "0.1");

        TCLAP::ValueArg<std::string> inputGraphArg("i", "input", "path of the temporal graph edge list", true, "",
                                                   "string");
        TCLAP::ValueArg<std::string> root_file_arg("f", "root_file_arg", "path of the root_file", false, "NA",
                                                   "string");
        TCLAP::ValueArg<int> projected_element_count("e", "projected_element_count", "projected element count", false, 1000,
                                                   "int");
        TCLAP::ValueArg<int> windowArg("w", "window", "time window in hours", false, 1, "int");
        TCLAP::ValueArg<std::string> resultArg("o", "result", "path to store the result", true, "", "string");

        TCLAP::ValueArg<int> cleanUp("p", "cleanUpLimit", "clean up size", false, 10000, "int");
        TCLAP::ValueArg<int> algo("a", "rootAlgo", "algorithm to find root 0 old 1 new", false, 1, "int");
        TCLAP::ValueArg<bool> reverse("r", "reverseDirection", "reverse Direction of edge", false, false, "bool");
        TCLAP::ValueArg<bool> isCompressed("z", "isCompressed", "the root node is compressed", false, false, "bool");
        TCLAP::ValueArg<bool> is_candidates_provided("c", "is_candidates_provided", "candidate list is provided", false,
                                                     true, "bool");
        TCLAP::ValueArg<int> cycle("l", "cycleLength", "cycle lenght", false, 80, "i nt");
        TCLAP::ValueArg<bool> use_bundle("b", "use_bundle", "candidate list is provided", false,
                                         false, "bool");
        cmd.add(inputGraphArg);
        cmd.add(windowArg);
        cmd.add(resultArg);
        cmd.add(is_candidates_provided);
        cmd.add(cleanUp);
        cmd.add(algo);
        cmd.add(reverse);
        cmd.add(isCompressed);
        cmd.add(use_bundle);
        cmd.add(projected_element_count);

        cmd.add(cycle);
        cmd.add(root_file_arg);

        // Parse the argv array.
        cmd.parse(argc, argv);

        cout << "root file " << root_file_arg.getValue() << endl;
        // Get the value parsed by each arg.
        std::string inputGraph = inputGraphArg.getValue();

        std::string resultFile = resultArg.getValue();
        int window = windowArg.getValue();
        bool candidates_provided = is_candidates_provided.getValue();
        bool reverseEdge = reverse.getValue();
        int cleanUpLimit = cleanUp.getValue();
        int cyclelenght = cycle.getValue();

        int rootAlgo = algo.getValue();
        // 0 : Run naive
        // 1 : Run Root node finder then cycle detector
        // 2 : Run Root node finder only
        // 3 : Run cycle detector only
        // Do what you intend.
        Platform::Timer timer;
        timer.Start();
        std::cout << "Memory start, " << getMem() << std::endl;
        long pend = 0l;
        // findWithLength(inputGraph,resultFile,window,timeInMsec,cleanUpLimit,cyclelenght);
        if (rootAlgo == 0) {
            //  findRootNodes(inputGraph, resultFile, window, timeInMsec, cleanUpLimit);
            std::cout << "Running naive: input: " << inputGraph << " result: " << resultFile << std::endl;
            findAllCycleNaive(inputGraph, resultFile, window, reverseEdge);
        } else if (rootAlgo == 1) {
            std::string cycleFile = resultFile;
            cycleFile.replace(cycleFile.end() - 3, cycleFile.end(), "cycle");;
            findRootNodesNew(inputGraph, resultFile, window, cleanUpLimit, reverseEdge);

            pend = timer.LiveElapsedSeconds();
            std::cout << "Found all root nodes and time " << pend << std::endl;
            findAllCycle(inputGraph, resultFile, cycleFile, window, isCompressed.getValue(), reverseEdge,
                         candidates_provided, use_bundle.getValue());
            std::cout << "Found all cycles nodes and time " << timer.LiveElapsedSeconds() - pend << std::endl;
        } else if (rootAlgo == 2) {
            findRootNodes(inputGraph, resultFile, window, cleanUpLimit, reverseEdge);

            pend = timer.LiveElapsedSeconds();
            std::cout << "Found all root nodes and time " << pend << std::endl;

        } else if (rootAlgo == 3) {
            //find cycles with the root folder and candidates
            std::string cycleFile = resultFile;
            cycleFile.replace(cycleFile.end() - 3, cycleFile.end(), "cycle");
            cout << "writing cycles in : " << cycleFile << endl;
            findAllCycle(inputGraph, resultFile, cycleFile, window, isCompressed.getValue(), reverseEdge,
                         candidates_provided, use_bundle.getValue());
            std::cout << "Found all cycles nodes and time " << timer.LiveElapsedSeconds() << std::endl;
        } else if (rootAlgo == 4) {
            // find root node using new method
            findRootNodesNew(inputGraph, resultFile, window, cleanUpLimit, reverseEdge);

            pend = timer.LiveElapsedSeconds();
            std::cout << "Found all root nodes and time " << pend << std::endl;
        } else if (rootAlgo == 5) {
            //find root node using bloom filter
            resultFile = inputGraph;
            std::string ext;
            ext = "-root-" + to_string(window) + '.' + "bloom";
            resultFile.replace(resultFile.end() - 4, resultFile.end(), ext);
            std::cout << "Finding root nodes using bloom: input: " << inputGraph << " result: " << resultFile
                      << std::endl;

            findRootNodesApprox(inputGraph, resultFile, window, cleanUpLimit, reverseEdge);

            pend = timer.LiveElapsedSeconds();
            std::cout << "Found all root nodes and time " << pend << std::endl;
        } else if (rootAlgo == 6) {
            //find candidates from the output of bloom filter algo
            string root_file = inputGraph;
            std::string ext;
            ext = "-root-" + to_string(window) + '.' + "bloom";
            root_file.replace(root_file.end() - 4, root_file.end(), ext);
            std::cout << "Finding candidates from bloom output: input: " << inputGraph << " root_file: " << root_file
                      << " result: " << resultFile << std::endl;
            findCandidateFromApprox(inputGraph, root_file, resultFile, window, cleanUpLimit, reverseEdge);

            pend = timer.LiveElapsedSeconds();
            std::cout << "Found all root nodes and candidates " << pend << std::endl;
        } else if (rootAlgo == 7) {
            //find root node using bloom filter
            string root_file = inputGraph;
            std::string ext;
            ext = "-root-" + to_string(window) + '.' + "bloom";
            root_file.replace(root_file.end() - 4, root_file.end(), ext);
            std::cout << "Finding root nodes using bidirectional bloom: input: " << inputGraph << " result: "
                      << root_file << std::endl;

            set<approxCandidatesNew> root_candidates = findRootNodesApproxBothDirectionNew(inputGraph, root_file,
                                                                                           window,
                                                                                           cleanUpLimit, reverseEdge,projected_element_count.getValue());
            pend = timer.LiveElapsedSeconds();
            std::cout << "Time to find all root candidates: " << pend << std::endl;
            std::cout << "Memory: " << getMem() << std::endl;
            //filtering root based on root node list
            if (root_file_arg.getValue().compare("NA") != 0) {
                cout<<"Root candidates to filter from file "<< root_file_arg.getValue()<<endl;
                root_candidates = filterRoots(root_candidates, root_file_arg.getValue());
                cout << "fillterd root count" << root_candidates.size() << endl;
            }
            std::cout << "Finding cycles using  bloom: input: " << inputGraph << " result: " << resultFile << std::endl;
            std::string cycleFile = resultFile;
            cycleFile.replace(cycleFile.end() - 3, cycleFile.end(), "cycle");

            findAllCycleUsingBloom(inputGraph, &root_candidates, cycleFile, window, reverseEdge, use_bundle.getValue());
            std::cout << "Time to find cycle using bloom: " << timer.LiveElapsedSeconds() - pend << std::endl;

        } else if (rootAlgo == 8) {
            //find root node using set
            string root_file = inputGraph;
            std::string ext;
            ext = "-root-" + to_string(window) + '.' + "exact";
            root_file.replace(root_file.end() - 4, root_file.end(), ext);
            std::cout << "Finding root nodes using bidirectional set: input: " << inputGraph << " result: "
                      << root_file << std::endl;

            set<exactCandidates> root_candidates = findRootNodesExactBothDirection(inputGraph, root_file, window,
                                                                                   cleanUpLimit, reverseEdge);

            pend = timer.LiveElapsedSeconds();
            std::cout << "Time to find all root candidates: " << pend << std::endl;
            std::cout << "Finding cycles using  set: input: " << inputGraph << " result: " << resultFile << std::endl;

            string cycle_file = resultFile;
            cycle_file.replace(cycle_file.end() - 3, cycle_file.end(), "cycle");
            findAllCycleUsingSet(inputGraph, &root_candidates, cycle_file,
                                 window, reverseEdge, use_bundle.getValue());

            std::cout << "Time to find cycle using set: " << timer.LiveElapsedSeconds() - pend << std::endl;

        } else if (rootAlgo == 9) {

            findRootNodesNew(inputGraph, resultFile, window, cleanUpLimit, reverseEdge);

            pend = timer.LiveElapsedSeconds();
            std::cout << "Time to find seeds: " << pend << std::endl;
            string compress_seeds = combineSeeds(resultFile, window);
            std::cout << "Time to find compress: " << timer.LiveElapsedSeconds() - pend << std::endl;

        } else if (rootAlgo == 10) {
            //testCountPath();
            map<int, bloom_filter> test;
            for (int i = 0; i < cleanUpLimit; i++) {
                bloom_filter b;
                test[i] = b;
            }
            cout << "Memory before:" << getMem() << endl;

            test.clear();
            cout << "Memory after:" << getMem() << endl;

            map<int, nodeid> test2;

            for (int i = 0; i < (100 * cleanUpLimit); i++) {

                test2[i] = 1000;
            }
            cout << "Memory before:" << getMem() << endl;
            test2.clear();
            cout << "Memory after:" << getMem() << endl;
        } else if (rootAlgo == 11) {
            //test for significance
            prepareData(inputGraph, resultFile, reverseEdge);
            string significance_file = "sig_" + inputGraph;
            getSignificantCycle(window, significance_file);
            //   cout<<binomialCoeff(5,5)<<endl;

        } else if (rootAlgo == 12) {
            //test for memory without cleanup forward only using bloom
            prepareData(inputGraph, resultFile, reverseEdge);
            string significance_file = "sig_" + inputGraph;
            getSignificantCycle(window, significance_file);
            //   cout<<binomialCoeff(5,5)<<endl;

        } else if (rootAlgo == 13) {
            //find root node using bloom filter
            string root_file = inputGraph;
            std::string ext;
            ext = "-root-" + to_string(window) + '.' + "bloom";
            root_file.replace(root_file.end() - 4, root_file.end(), ext);
            std::cout << "Finding root nodes using bidirectional bloom: input: " << inputGraph << " result: "
                      << root_file << std::endl;

            set<approxCandidatesNew> root_candidates = findRootNodesApproxBothDirectionWithSerialization(inputGraph,
                                                                                                         root_file,
                                                                                                         window,
                                                                                                         cleanUpLimit,
                                                                                                         reverseEdge,
                                                                                                         "D:\\dataset\\Temp\\");

            pend = timer.LiveElapsedSeconds();
            std::cout << "Time to find all root candidates: " << pend << std::endl;
            std::cout << "Memory: " << getMem() << std::endl;
            std::cout << "Finding cycles using  bloom: input: " << inputGraph << " result: " << resultFile << std::endl;
            std::string cycleFile = resultFile;
            cycleFile.replace(cycleFile.end() - 3, cycleFile.end(), "cycle");
            // findAllCycleUsingBloom(inputGraph, &root_candidates, resultFile, window, reverseEdge,use_bundle.getValue());
            findAllCycleUsingBloom(inputGraph, &root_candidates, cycleFile, window, reverseEdge, use_bundle.getValue());
            std::cout << "Time to find cycle using bloom: " << timer.LiveElapsedSeconds() - pend << std::endl;

        } else {
            std::cout << "Un defined Algorithm param " << rootAlgo << std::endl;
        }

        std::cout << "Memory end, " << getMem() << std::endl;
        std::cout << "Total Time, " << timer.LiveElapsedSeconds() << std::endl;

    } catch (TCLAP::ArgException &e)  // catch any exceptions
    { std::cerr << "error: " << e.error() << " for arg " << e.argId() << std::endl; }
    catch (std::exception &e) {
        std::cerr << "error: " << e.what() << std::endl;
    }

}