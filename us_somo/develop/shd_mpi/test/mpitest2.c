#include <mpi.h>

#include <string>

int main(int argc, char** argv) {
  // Initialize the MPI environment

  MPI_Group org_group;
  MPI_Group xhost_group;
  MPI_Group mic_group;
  MPI_Comm xhost_comm;
  MPI_Comm mic_comm;

  MPI_Init(&argc, &argv);

  // Get the number of processes
  int world_size;
  MPI_Comm_size(MPI_COMM_WORLD, &world_size);

  // Get the rank of the process
  int world_rank;
  MPI_Comm_rank(MPI_COMM_WORLD, &world_rank);

  MPI_Comm_group(MPI_COMM_WORLD, &org_group);

  // Get the name of the processor
  char processor_name[MPI_MAX_PROCESSOR_NAME];
  int name_len;
  MPI_Get_processor_name(processor_name, &name_len);

  int is_mic = std::string::npos != std::string(processor_name).find("-mic");

  // going to have to broadcast to 0 the mic status so it can assign comms /
  // ranks

  // MPI_Group_incl( org_group, world_size, world_rank, is_mic ? &mic_group :
  // &xhost_group );

  // MPI_Comm_create( MPI_COMM_WORLD, mic_group, &mic_comm );
  // MPI_Comm_create( MPI_COMM_WORLD, xhost_group, &xhost_comm );

  // Print off a hello world message
  printf(
      "Hello world from processor %s, rank %d"
      " out of %d processors and %s\n",
      processor_name, world_rank, world_size,
      is_mic ? "and I am a mic" : "and I am NOT a mic");

  // Finalize the MPI environment.
  MPI_Finalize();
}
