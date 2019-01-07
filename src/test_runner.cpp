#define CATCH_CONFIG_RUNNER
#include "logging.hpp"
#include <catch2/catch.hpp>

int main(int argc, char* argv[]) {
	psycris::init_logging(false);

	return Catch::Session().run(argc, argv);
}