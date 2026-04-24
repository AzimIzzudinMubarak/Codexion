#include "codexion.h"

static int is_valid_int(char *str)
{
	int i;

	i = 0;
	if (!str || !str[0])
		return (0);
	while (str[i])
	{
		if (str[i] < '0' || str[i] > '9')
			return (0);
		i++;
	}
	return (1);
}

int parse_args(t_sim *sim, int argc, char **argv)
{
	if (argc != 9)
		return (0);
	if (!is_valid_int(argv[1]) || !is_valid_int(argv[2])
		|| !is_valid_int(argv[3]) || !is_valid_int(argv[4])
		|| !is_valid_int(argv[5]) || !is_valid_int(argv[6])
		|| !is_valid_int(argv[7]))
		return (0);
	if (strcmp(argv[8], "fifo") != 0 && strcmp(argv[8], "edf") != 0)
		return (0);
	sim->nb_coders = atoi(argv[1]);
	sim->time_to_burnout = atoi(argv[2]);
	sim->time_to_compile = atoi(argv[3]);
	sim->time_to_debug = atoi(argv[4]);
	sim->time_to_refactor = atoi(argv[5]);
	sim->nb_compiles_required = atoi(argv[6]);
	sim->dongle_cooldown = atoi(argv[7]);
	sim->scheduler = (strcmp(argv[8], "edf") == 0) ? 1 : 0;
	if (sim->nb_coders < 1)
		return (0);
	return (1);
}