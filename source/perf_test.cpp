#include "statistics.hpp"

#include <glm/gtc/type_precision.hpp>

#include <cmdline.h>

#include <mpi.h>

#include <chrono>
#include <thread>
#include <iostream>
#include <vector>

void master();
void worker();
glm::uvec2 cell_resolution(glm::uvec2 const& resolution, uint32_t num_workers);

glm::uvec2 resolution{0, 0};
glm::uvec2 res_worker{0, 0};

int main(int argc, char* argv[]) {
  int provided;
  // auto thread_type = MPI_THREAD_FUNNELED;
  auto thread_type = MPI_THREAD_SERIALIZED;
  MPI_Init_thread(&argc, &argv, thread_type, &provided);
  bool threads_ok = provided >= thread_type;
  assert(threads_ok);

  MPI::COMM_WORLD.Set_errhandler(MPI::ERRORS_THROW_EXCEPTIONS);
  if (MPI::COMM_WORLD.Get_errhandler() != MPI::ERRORS_THROW_EXCEPTIONS) {
    throw std::runtime_error{"handler wrong"};
  }
  uint32_t num_processes = MPI::COMM_WORLD.Get_size();
  if (!(num_processes == 2 || (num_processes > 2 && num_processes % 2 == 1))) {
    std::cerr << "number of processes must be 2 or a larger poer of two +1" << std::endl;
    throw std::runtime_error{"invalid process number"};
  }
  // logic start
  cmdline::parser cmd_parse{};
  cmd_parse.add<uint>("width", 'w', "horizontal resolution, default 1920", false, 1920, cmdline::range(640, 7680));
  cmd_parse.add<uint>("height", 'h', "vertical resolution, default 1080", false, 1080, cmdline::range(480, 7680));
  cmd_parse.parse(argc, argv);

  resolution.x = cmd_parse.get<uint>("width");
  resolution.y = cmd_parse.get<uint>("height");

  res_worker = cell_resolution(resolution, MPI::COMM_WORLD.Get_size() - 1);
  if (MPI::COMM_WORLD.Get_rank() == 0) {
    master();
  }
  else {
    worker();
  }

  MPI::Finalize();
}
// logic
Statistics statistics{};
std::vector<glm::u8vec2> buffer;
size_t num_iter = 100;
uint32_t num_workers = 0;
glm::fmat4 mat_view{};
glm::fmat4 mat_frustum{};
std::vector<glm::fmat4> mat_frustra{};

glm::uvec2 cell_resolution(glm::uvec2 const& resolution, uint32_t num_workers) {
  // unsigned num_workers = MPI::COMM_WORLD.Get_size() - 1;
  glm::fvec3 right{1.0, 0.0, 0.0};
  glm::fvec3 up{0.0, 1.0, 0.0};

  auto frustum_cells = glm::uvec2{1, 1};
  if (num_workers == 1) {
    return resolution;
  }
  else {
    float l2 = float(log2(num_workers));

    glm::fvec2 frustum_dims{right.x, up.y};
    glm::fvec2 cell_dims{right.x, up.y};
    while(l2 > 0) {
      // split larger dimension
      if (cell_dims.x > cell_dims.y) {
        frustum_cells.x *= 2;
      }
      else {
        frustum_cells.y *= 2;
      }
      cell_dims = frustum_dims / glm::fvec2{frustum_cells};
      l2 -= 1.0f;
    }
    if (MPI::COMM_WORLD.Get_rank() == 0) {
      std::cout << "Full resolution: " << resolution.x << " x " << resolution.y << " - size " << resolution.x * resolution.y * 4 << "Bytes" << std::endl;
      std::cout << "Cell division: " << frustum_cells.x << " x " << frustum_cells.y << std::endl;
      auto cell_res = resolution / frustum_cells;
      std::cout << "Cell resolution: " << cell_res.x << " x " << cell_res.y << " - size " << cell_res.x * cell_res.y * 4 << "Bytes" << std::endl;
      std::cout << "Running loop " << num_iter << " times " << std::endl;
    }
  }
  return resolution / frustum_cells;
}

void master() {
  buffer = std::vector<glm::u8vec2>{resolution.x * resolution.y * 4};
  statistics.addTimer("receive");
  statistics.addTimer("send");
 
  num_workers = MPI::COMM_WORLD.Get_size() - 1;
  mat_frustra =  std::vector<glm::fmat4>{num_workers, glm::fmat4{}};

  for (size_t i = 0; i < num_iter; ++i) {
    statistics.start("send");
   
    // sending
    MPI::COMM_WORLD.Bcast((void*)&mat_view, 16, MPI::FLOAT, 0);
    //different projection for each, copy frustra[0] to process 1 
    MPI::COMM_WORLD.Scatter(mat_frustra.data() - 1, 16, MPI::FLOAT, MPI::IN_PLACE, 16, MPI::FLOAT, 0);
    statistics.stop("send");

    // receive
    statistics.start("receive");
    int size_chunk = int(res_worker.x * res_worker.y * 4);
    // copy into current subregion
    // size_t offset = res.buffer_views.at("transfer").offset();
    // copy chunk from process [1] to beginning
    // offset -= size_chunk; 
    MPI::COMM_WORLD.Gather(MPI::IN_PLACE, size_chunk, MPI::BYTE, buffer.data(), size_chunk, MPI::BYTE, 0);
    statistics.stop("receive");
  }

  std::cout << "Master send time: " << statistics.get("send") << " milliseconds " << std::endl;
  std::cout << "Master receive time: " << statistics.get("receive") << " milliseconds " << std::endl;
}

void worker() {
  buffer = std::vector<glm::u8vec2>{res_worker.x * res_worker.y * 4};
  statistics.addTimer("send");
  statistics.addTimer("receive");

  for (size_t i = 0; i < num_iter; ++i) {
    statistics.start("receive");
    // receive
    MPI::COMM_WORLD.Bcast(&mat_view, 16, MPI::FLOAT, 0);
    //different projection for each 
    MPI::COMM_WORLD.Scatter(nullptr, 16, MPI::FLOAT, &mat_frustum, 16, MPI::FLOAT, 0);
    statistics.stop("receive");

    // simulate drawing etc.
    std::this_thread::sleep_for(std::chrono::milliseconds(7));

    // sending
    statistics.start("send");
    // write data to presenter
    int size = int(buffer.size());
    MPI::COMM_WORLD.Gather(buffer.data(), size, MPI::BYTE, nullptr, size, MPI::BYTE, 0);
    statistics.stop("send");
  }
 
  std::cout << " Worker " << MPI::COMM_WORLD.Get_rank() << " receive time: " << statistics.get("receive") << " milliseconds " << std::endl;
  std::cout << " Worker " << MPI::COMM_WORLD.Get_rank() << " send time: " << statistics.get("send") << " milliseconds " << std::endl;
}