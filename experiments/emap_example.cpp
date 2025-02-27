#include <iostream>
#include <string>
#include <vector>

#include <lorina/aiger.hpp>
#include <lorina/genlib.hpp>

#include <mockturtle/algorithms/emap.hpp>
#include <mockturtle/io/aiger_reader.hpp>
#include <mockturtle/io/genlib_reader.hpp>
#include <mockturtle/io/write_verilog.hpp>
#include <mockturtle/networks/aig.hpp>
#include <mockturtle/networks/block.hpp>
#include <mockturtle/utils/name_utils.hpp>
#include <mockturtle/utils/tech_library.hpp>
#include <mockturtle/views/cell_view.hpp>
#include <mockturtle/views/depth_view.hpp>
#include <mockturtle/views/names_view.hpp>

#include <mockturtle/experiments.hpp>

int main()
{
  using namespace experiments;  
  using namespace mockturtle;
  // Declaration of the network  
  names_view<aig_network> aig;
  // Read the network
  auto const result = lorina::read_aiger( "../experiments/benchmarks/adder.aig", mockturtle::aiger_reader( aig ) );
  if ( result != lorina::return_code::success )
  {
    std::cout << "Read benchmark failed\n";
    return -1;
  }
  
  // Set the library to map
  std::string library = "sky130";
  std::vector<gate> gates;
  std::ifstream in( cell_libraries_path( library ) );
  // Read the library
  if ( lorina::read_genlib( in, genlib_reader( gates ) ) != lorina::return_code::success )
  {
    std::cout << "Read library failed\n";
    return 1;
  }
  // Library parameters
  tech_library_params tps;
  tps.ignore_symmetries = false; // set to true to drastically speed-up mapping with minor delay increase
  tps.verbose = true;
  tech_library<9> tech_lib( gates, tps );
  std::cout << "Library complete\n";

  // Mapper parameters
  emap_params ps;
  ps.matching_mode = emap_params::hybrid;
  ps.area_oriented_mapping = false;
  ps.map_multioutput = false;
  ps.relax_required = 0;
  emap_stats st;
  // Mapping
  cell_view<block_network> res = emap<9>(aig, tech_lib, ps, &st);
  std::cout << "Mapper complete\n";

  // Fixing the names
  names_view res_names{ res };
  restore_network_name( aig, res_names );
  restore_pio_names_by_order( aig, res_names );

  // Print stats and cell usage
  res.report_stats();
  res.report_cells_usage();
  return 0;
}
