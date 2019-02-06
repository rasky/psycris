#define CATCH_CONFIG_RUNNER
#include <catch2/catch.hpp>

#include "../logging.hpp"

int main(int argc, char* argv[]) {
	psycris::init_logging(false);

	return Catch::Session().run(argc, argv);
}