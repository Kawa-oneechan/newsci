//#pragma once

//For switches on enums, issue a warning if we're missing an option.
#pragma warning(1:4062)

//not typesafe, but I don't give a fuck.
#define min(A,B) (A < B ? A : B)
#define max(A,B) (A > B ? A : B)

#include <stdlib.h>
#include <stdint.h>
#include <stdio.h>
#include <string.h>
#include <SDL.h>
#include <map>
#include <vector>
#include "support/sol.hpp"
#include "support/json/JSON.h"
#include "lua.h"
#include "types.h"
#include "serializer.h"
#include "ports.h"
#include "fonts.h"
#include "pack.h"
#include "pictures.h"
#include "views.h"
#include "audio.h"
#include "controls.h"

