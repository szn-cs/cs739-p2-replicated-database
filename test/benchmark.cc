#include "benchmark/benchmark.h"

#include <chrono>
#include <string>

#include "../src/declaration.h"

void SomeFunction(size_t size) {
  cout << termcolor::grey << utility::getClockTime() << termcolor::reset << endl;

  string address = "127.0.1.1:8000";

  rpc::call::ConsensusRPCWrapperCall c{grpc::CreateChannel(address, grpc::InsecureChannelCredentials())};

  std::pair<Status, std::string> res = c.get_leader();
  if (!res.first.ok())
    throw std::runtime_error("RPC FAILURE");

  std::string s1(size, '-');
  std::string s2(size, '-');
  benchmark::DoNotOptimize(s1.compare(s2));
}

static void BM_SomeFunction(benchmark::State& state) {
  // Perform setup here
  for (auto _ : state) {
    // This code gets timed
    SomeFunction(state.range(0));
  }
}
// Register the function as a benchmark
BENCHMARK(BM_SomeFunction)->Arg(1);  // ->Arg(200000)->Arg(400000);

// BENCHMARK(BM_SomeFunction)->RangeMultiplier(2)->Range(1 << 10, 1 << 20);

/** 
 * initialize google/benchmark main with custom code
 * 
 * Alternatively run the benchmark:
 * BENCHMARK_MAIN(); // google/benchmark main macro
*/
int main(int argc, char** argv) {
  {  // required initialization to prevent segmentation errors - similar to app main() function
    cout << termcolor::grey << utility::getClockTime() << termcolor::reset << endl;

    struct stat info;
    std::shared_ptr<utility::parse::Config> config = std::make_shared<utility::parse::Config>();

    boost::program_options::variables_map variables;
    utility::parse::parse_options<utility::parse::Mode::NODE>(argc, argv, config, variables);  // parse options from different sources

    // Initialize Cluster data & Node instances
    app::initializeStaticInstance(config->cluster, config);

    if (config->flag.debug)
      cout << termcolor::grey << "Using config file at: " << config->config << termcolor::reset << endl;
  }

  ::benchmark::Initialize(&argc, argv);
  ::benchmark::RunSpecifiedBenchmarks();
}
