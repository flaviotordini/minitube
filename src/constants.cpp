#include "constants.h"

#define STR(x) #x
#define STRINGIFY(x) STR(x)

const char *Constants::VERSION = STRINGIFY(APP_VERSION);
const char *Constants::APP_NAME = "Minitube";
const char *Constants::ORG_NAME = "Flavio Tordini";
const char *Constants::ORG_DOMAIN = "flavio.tordini.org";
const char *Constants::WEBSITE = "http://flavio.tordini.org/minitube";
const char *Constants::EMAIL = "flavio.tordini@gmail.com";
