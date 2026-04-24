#include "codexion.h"

int main(int argc, char **argv)
{
	t_sim sim;

	if(!init_sim(&sim, argc, argv))
	{
		write(2, "Error: invalid arguments\n", 25);
		return (1);
	}
	cleanup_sim(&sim);
	return (0);
}